#include "application.hpp"
#include "communication.hpp"
#include "console.hpp"
#include "network.hpp"
#include "blockchain.hpp"
#include "dispatcher.hpp"
#include <boost/asio/signal_set.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <thread>
#include <csignal>

using bitchat::Application;
namespace logging = boost::log::trivial;

void Application::run( const std::string name,
                       const std::string host,
                       const int port,
                       const int reconnectTimeout )
{
    BOOST_ASSERT( static_cast<uint16_t>( port ) == port );

#ifdef NDEBUG
    boost::log::core::get()->set_filter( logging::severity >= logging::info );
#elseif DEBUG_TRACE
    boost::log::core::get()->set_filter( logging::severity >= logging::trace );
#else
    boost::log::core::get()->set_filter( logging::severity >= logging::debug );
#endif
    const auto log{ boost::log::add_file_log( name + ".log" ) };

    try
    {
        auto communication{ std::make_shared<Communication>() };
        auto blockchain{ std::make_unique<Blockchain>( communication, name + ".blockchain" ) };
        auto network{ std::make_unique<Network>( communication, host, port, reconnectTimeout ) };
        auto console{ std::make_unique<Console>( communication ) };
        auto dispatcher{ std::make_unique<Dispatcher>( * console,
                                                       * network,
                                                       * blockchain ) };
        communication->subscribe( Communication::kOnStart, * dispatcher, & Dispatcher::start );
        communication->subscribe( Communication::kOnStop, * dispatcher, & Dispatcher::stop );
        communication->doLater( * communication, & Communication::open );

        runPool( communication );

        communication->unsubscribe( Communication::kOnStart, * dispatcher, & Dispatcher::start );
        communication->unsubscribe( Communication::kOnStop, * dispatcher, & Dispatcher::stop );

        log->flush();
    }
    catch ( const std::logic_error & )
    {
        log->flush();
        throw;
    }
    catch ( const std::exception & exception )
    {
        log->flush();
        throw std::runtime_error( exception.what() );
    }
    catch ( ... )
    {
        log->flush();
        throw std::runtime_error( "Unknown exception" );
    }
}

void Application::runPool( CommunicationPtr communication )
{
    ThreadPool pool{ std::thread::hardware_concurrency() };
    boost::asio::signal_set sigs{ communication->getIos(), SIGINT, SIGABRT, SIGTERM };

    sigs.async_wait( [ communication ]( const auto & error, const auto sig ) {
        if ( ! error )
        {
            BOOST_LOG_TRIVIAL( info ) << "Interrupted by signal " << sig << ", aborting...";
            communication->close();
        }
    } );

    for ( auto & thread : pool )
    {
        const auto functor{ std::bind( & Application::runLoop, communication ) };
        std::make_unique<std::thread>( functor ).swap( thread );
    }

    for ( auto & thread : pool )
    {
        thread->join();
    }

    pool.clear();
}


void Application::runLoop( CommunicationPtr communication )
{
    do
    {
        try
        {
            communication->getIos().run();
        }
        catch ( const boost::exception & exception )
        {
            BOOST_LOG_TRIVIAL( debug ) << boost::diagnostic_information( exception );
            BOOST_LOG_TRIVIAL( warning ) << boost::diagnostic_information_what( exception );
        }
        catch ( const std::exception & exception )
        {
            BOOST_LOG_TRIVIAL( error ) << exception.what();
        }
        communication->close();
    }
    while ( ! communication->getIos().stopped() );
}
