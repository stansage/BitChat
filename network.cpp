#include "network.hpp"
#include "communication.hpp"
#include "socketchannel.hpp"
#include <utility>
#include <boost/log/trivial.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/exception/diagnostic_information.hpp>

using bitchat::Network;


Network::Network( const std::shared_ptr<Communication> & communication,
                  const std::string & ip,
                  const std::uint16_t port,
                  const int reconnectInterval ) :
    m_ip{ ip },
    m_port{ port },
    m_acceptor{ communication->getIos() },
    m_timer{ m_acceptor.get_io_service() },
    m_reconnectInterval{ boost::posix_time::seconds{ reconnectInterval } }
{
    std::make_shared<SocketChannel>( communication ).swap( m_server );
}

void Network::open()
{
    const auto channel{ std::make_shared<SocketChannel>( m_server->getCommunication() ) };

    if ( ! m_acceptor.is_open() )
    {
        Tcp::endpoint endpoint{ Tcp::v4(), 0 };

        m_acceptor.open( endpoint.protocol() );
        m_acceptor.set_option( Tcp::acceptor::reuse_address{ true } );
        m_acceptor.bind( endpoint );

        endpoint = m_acceptor.local_endpoint();

        BOOST_LOG_TRIVIAL( info ) << "Listenning on port " << endpoint.port();
        m_acceptor.listen();
    }

    m_acceptor.async_accept( * channel, [ this, channel ] ( auto error ) {
        if ( ! error )
        {
            BOOST_LOG_TRIVIAL( info ) << "Accepted connection from " << channel->remote_endpoint();
            m_clients.push_back( channel );
            channel->open();
        }
        else if ( ! wasAborted( error.value() ) )
        {
            BOOST_LOG_TRIVIAL( warning ) << "Failed to accept connection - " << error.message();
        }

        if ( ! wasAborted( error.value() ) )
        {
            BOOST_LOG_TRIVIAL( info ) << "Waiting for the next connection";
            open();
        }
    } );

    if ( ! m_ip.empty() && m_port > 0 )
    {
        try
        {
            connect( Tcp::endpoint{ Address::from_string( m_ip ), m_port } );
        }
        catch ( const boost::exception & )
        {
            throw std::invalid_argument( "The host IP address is invalid" );
        }

    }
}


void Network::close()
{
    try
    {
        m_timer.cancel();

        for ( auto & client : m_clients )
        {
            client->close();
        }

        if ( m_acceptor.is_open() )
        {
            m_acceptor.cancel();
            m_acceptor.close();
        }

        if ( m_server )
        {
            disconnect();
            m_server.reset();
        }

        m_clients.clear();
    }
    catch ( const boost::exception & exception )
    {
        BOOST_LOG_TRIVIAL( error ) << boost::diagnostic_information_what( exception );
        BOOST_LOG_TRIVIAL( debug ) << boost::diagnostic_information( exception );
    }
}

std::uint16_t Network::getListenningPort() const
{
    return m_acceptor.local_endpoint().port();
}

std::shared_ptr<bitchat::SocketChannel> Network::getServerSocket() const
{
    return m_server;
}

std::vector<std::shared_ptr<bitchat::SocketChannel> > Network::getClientSockets() const
{
    return m_clients;
}


void Network::connect( const Tcp::endpoint endpoint )
{
    BOOST_ASSERT( ! m_server->isOpen() );

    m_server->async_connect( endpoint, [ this, endpoint ]( auto error ) {
        if ( ! error )
        {
            BOOST_LOG_TRIVIAL( info ) << "Connected to " << m_server->remote_endpoint();
            m_server->open();
        }
        else if ( ! wasAborted( error.value() ) )
        {
            BOOST_LOG_TRIVIAL( warning ) << "Failed to connect: " << error.message();
            reconnect( endpoint );
        }
    } );
}

void Network::reconnect( const Tcp::endpoint endpoint )
{
    disconnect();

    m_timer.expires_from_now( m_reconnectInterval );
    m_timer.async_wait( [ this, endpoint ]( const auto error ) {
        if ( ! wasAborted( error.value() ) )
        {
            connect( endpoint );
        }
    } );
}

void Network::disconnect()
{
    BOOST_ASSERT( m_server != nullptr );

    if ( m_server->is_open() )
    {
        m_server->close();
    }
}

bool Network::wasAborted( int errorValue )
{
    return errorValue == boost::asio::error::operation_aborted;
}
