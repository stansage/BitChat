#include "dispatcher.hpp"
#include "communication.hpp"
#include "console.hpp"
#include "network.hpp"
#include "blockchain.hpp"
#include "socketchannel.hpp"
#include <boost/log/trivial.hpp>

using bitchat::Dispatcher;

namespace
{
constexpr auto kEmailPromt{ "Please, input your email address: " };
constexpr auto kMessagePromt{ "Type your message: " };
}

Dispatcher::Dispatcher( Console & console,
                        Network & network,
                        Blockchain & blockchain ) :
    m_console{ console },
    m_network{ network },
    m_blockchain{ blockchain }
{
}

bool Dispatcher::start( void * arg )
{
    auto communication{ static_cast<Communication*>( arg ) };
    BOOST_ASSERT( communication != nullptr );

    communication->subscribe( Channel::kOnOpen, * this, & Dispatcher::onChannelOpened );
    communication->subscribe( Channel::kOnClose, * this, & Dispatcher::onChannelClosed );
    communication->doLater( m_blockchain, & Blockchain::open );

    std::make_unique<Work>( communication->getIos() ).swap( m_work );

    BOOST_LOG_TRIVIAL( debug ) << "Started communication - " << communication;
    return true;
}

bool Dispatcher::stop( void * arg )
{
    auto communication{ static_cast<Communication*>( arg ) };
    BOOST_ASSERT( communication != nullptr );

    m_console.close();

    BOOST_LOG_TRIVIAL( debug ) << "Stopping communication - " << communication;
    return true;
}

bool Dispatcher::onChannelOpened( void * arg )
{
    auto channel{ static_cast<Channel*>( arg ) };
    BOOST_ASSERT( channel != nullptr );
    const auto communication{ channel->getCommunication().get() };

    if ( arg == & m_blockchain )
    {
        BOOST_LOG_TRIVIAL( debug ) << "The blockchain opened - " << channel;
        m_console.open();
        communication->subscribe( Blockchain::kOnSave, * this, & Dispatcher::onBlochainSaved );
    }
    else if ( arg == & m_console )
    {
        m_network.open();
        m_console.write( "Listening on " +
                         std::to_string( m_network.getListenningPort() ) );
        m_console.write( "\n" );
        onBlochainSaved( nullptr );
        communication->doLater( * this, & Dispatcher::promptEmail );

    }
    else if ( arg == m_network.getServerSocket().get() )
    {
        communication->doLater( * this, & Dispatcher::readServerRequest );
    }
    else
    {
        for ( const auto & client : m_network.getClientSockets() )
        {
            if ( arg == client.get() )
            {
                client->write( m_blockchain.makeBlockRequest( 0 ) );
                communication->getIos().
                        post( std::bind( & Dispatcher::readClientResponse, this, client.get() ) );
            }
        }
    }

    return false;
}

bool Dispatcher::onChannelClosed( void * arg )
{
    const auto channel{ static_cast<Channel*>( arg ) };
    BOOST_ASSERT( channel != nullptr );
    const auto communication{ channel->getCommunication().get() };

    if ( arg == & m_console )
    {
        BOOST_LOG_TRIVIAL( debug ) << "The console closed - " << channel;
        communication->unsubscribe( Blockchain::kOnSave, * this, & Dispatcher::onBlochainSaved );
        m_network.close();
        m_blockchain.close();
    }
    else if ( arg == & m_blockchain )
    {
        BOOST_LOG_TRIVIAL( debug ) << "The blockchain closed";
        communication->unsubscribe( Channel::kOnOpen, * this, & Dispatcher::onChannelOpened );
        communication->unsubscribe( Channel::kOnClose, * this, & Dispatcher::onChannelClosed );
        m_work.reset();
    }
    return false;
}

bool Dispatcher::onBlochainSaved( void * arg )
{
    std::stringstream stream{};

    if ( m_blockchain.getHeadIndex() > 0 )
    {
        for ( auto i{ std::strlen( kMessagePromt ) }; i > 0; --i )
        {
            stream << '\b';
        }
        stream << m_blockchain.getHeadTimestamp() << ' ';
        stream << m_blockchain.getHeadKey() << '>';
        stream << m_blockchain.getHeadValue() << std::endl;
        m_console.write( stream.str() );

        for ( const auto client : m_clients )
        {
            client->write( m_blockchain.makeNewBlock( 0 ) );
        }
    }

    if ( arg != nullptr )
    {
        promptMessage();
    }

    return false;
}


void Dispatcher::promptEmail()
{
    m_console.write( kEmailPromt );
    m_email = m_console.read( 0 );

    if ( ! m_email.empty() &&
         m_email.size() <= Blockchain::kKeySize )
    {
        m_console.getCommunication()->doLater( * this, & Dispatcher::promptMessage );
    }
    else
    {
        m_console.getCommunication()->doLater( * this, & Dispatcher::promptEmail );
    }
}

void Dispatcher::promptMessage()
{
    m_console.write( kMessagePromt );

    const auto message{ m_console.read( 0 ) };

    if ( ! message.empty() &&
         message.size() <= Blockchain::kValueSize )
    {
        m_console.getCommunication()->getIos().
                post( std::bind( & Blockchain::store, & m_blockchain, m_email, message ) );
    }
}

void Dispatcher::readServerRequest()
{
    const auto server{ m_network.getServerSocket() };

    switch ( server->read( 1 )[ 0 ] )
    {
    case Blockchain::kRequestBlock:
        writeServerResponce();
        break;

    case Blockchain::kNewBlock:
        m_blockchain.save( readRawBlock( server.get() ) );
        break;
    }
}

void Dispatcher::writeServerResponce()
{
    const auto indexSize{ m_blockchain.makeBlockRequest( 0 ).size() - 1 };
    const auto indexData{ m_network.getServerSocket()->read( indexSize ) };
    const auto index{ Blockchain::extractBlockIndex( indexData ) };

    m_network.getServerSocket()->write( m_blockchain.makeBlockResponse( index ) );
}

void Dispatcher::readClientResponse( Channel * client )
{
    switch ( m_network.getServerSocket()->read( 1 )[ 0 ] )
    {
    case Blockchain::kResponseBlock:
        //! TODO: complete blockchain integrity
        break;

    case Blockchain::kNewBlock:
        m_blockchain.save( readRawBlock( client ) );
        break;
    }
}

std::string Dispatcher::readRawBlock( Channel * channel  )
{
    const auto size{ m_blockchain.getBlockSize() };
    return channel->read( size );
}

