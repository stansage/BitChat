#include "communication.hpp"
#include <boost/asio/io_service.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/log/trivial.hpp>
#include <unordered_map>
#include <deque>
#include <mutex>

using bitchat::Communication;

struct Communication::Context
{
    std::mutex mutex;
    boost::asio::io_service ios;
    std::unordered_map<int, std::deque<Event::Target>> subscriptions;
};

const Communication::Event Communication::kOnStart{};
const Communication::Event Communication::kOnStop{};

namespace
{
    template<typename T, typename... U>
    size_t getFunctionAddress( std::function<T(U...)> f ) {
        typedef T(F)(U...);
        F ** fp = f.template target<F*>();
        return reinterpret_cast<size_t>( * fp );
    }
}

Communication::Communication()
{
    std::make_unique<Context>().swap( m_context );
    BOOST_LOG_TRIVIAL( debug ) << "Opened communication - " << this;
}

Communication::~Communication()
{
    m_context.reset();
    BOOST_LOG_TRIVIAL( debug ) << "Closed communication - " << this;
}

boost::asio::io_service & Communication::getIos()
{
    BOOST_ASSERT( m_context != nullptr );
    return m_context->ios;
}


void Communication::open()
{
    notify( kOnStart, this );
}

void Communication::close()
{
    perform( kOnStop, this );
    doLater( * this, & Communication::shutdown );
}


std::size_t Communication::subscribersCount( const bitchat::BaseEvent & event )
{
    BOOST_ASSERT( m_context != nullptr );

    std::lock_guard<std::mutex> lock{ m_context->mutex };
    auto result{ 0ull };
    const auto it{ m_context->subscriptions.find( event.getTag() ) };
    boost::ignore_unused( lock );

    if ( it != m_context->subscriptions.end() )
    {
        result = it->second.size();
    }

    return result;
}

void Communication::shutdown()
{
    BOOST_ASSERT( m_context != nullptr );
    std::lock_guard<std::mutex> lock{ m_context->mutex };
    boost::ignore_unused( lock );

    try {
        m_context->ios.stop();
    } catch ( ... ) {
        BOOST_LOG_TRIVIAL( error ) << "Error while closing communication";
    }
}


void Communication::subscribe( const BaseEvent & event,
                               const Event::Target target )
{
    BOOST_ASSERT( m_context != nullptr );
    std::lock_guard<std::mutex> lock{ m_context->mutex };
    boost::ignore_unused( lock );

    m_context->subscriptions[ event.getTag() ].
            emplace_back( target );
}

void Communication::unsubscribe( const BaseEvent & event ,
                                 const Event::Target target )
{
    forgetTarget( event.getTag(), target, true );
}

void Communication::perform( const BaseEvent & event,
                             void * arg )
{
    procceedEvent( event, arg, false );
}

void Communication::notify( const BaseEvent & event,
                            void * arg )
{
    procceedEvent( event, arg, true );
}


bool Communication::procceedEvent( const BaseEvent & event,
                                   void * arg,
                                   const bool async )
{
    std::deque<Event::Target> targets{};
    do
    {
        BOOST_ASSERT( m_context != nullptr );
        std::lock_guard<std::mutex> lock{ m_context->mutex };
        boost::ignore_unused( lock );
        const auto it{ m_context->subscriptions.find( event.getTag() ) };

        if ( it != m_context->subscriptions.end() )
        {
            for ( auto target : it->second )
            {
                targets.emplace_back( target );
            }
        }
    } while( false );


    for ( auto & target : targets )
    {
        const auto forget{ std::bind( & Communication::forgetTarget, this, event.getTag(), target, false ) };
        const auto procceed{  std::bind( & Communication::procceedTarget, this, target, arg ) };
        const auto functor{ [ forget, procceed ]() {
            if ( procceed() )
            {
                forget();
            }
        } };

        if ( async )
        {
            m_context->ios.post( functor );
        }
        else
        {
            m_context->ios.dispatch( functor );
        }
    }

    return ! targets.empty();
}

bool Communication::procceedTarget( Event::Target target,
                                    void * arg )
{
    return target( arg );
}

void Communication::forgetTarget( const int tag,
                                  Event::Target target,
                                  const bool byAddress )
{
    BOOST_ASSERT( m_context != nullptr );
    std::lock_guard<std::mutex> lock{ m_context->mutex };
    boost::ignore_unused( lock );

    auto & targets{ m_context->subscriptions[ tag ] };

    if ( byAddress )
    {
        const auto hash{ getFunctionAddress( target ) };
        auto it = std::remove_if( targets.begin(), targets.end(), [ hash ]( const auto t ) {
            return hash == getFunctionAddress( t );
        } );

        targets.erase( it , targets.end() );
    }
    else
    {
        auto it = std::remove_if( targets.begin(), targets.end(), [ target ]( const auto t ) {
            return target.target_type() == t.target_type();
        } );

        targets.erase( it , targets.end() );
    }


    if ( targets.empty() )
    {
        m_context->subscriptions.erase( tag );
    }
}

