#include "socketchannel.hpp"
#include "communication.hpp"
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio/streambuf.hpp>

using bitchat::SocketChannel;

SocketChannel::SocketChannel( const CommunicationPtr & communication ) :
    Tcp::socket{ communication->getIos() },
    m_communication{ communication }
{
}

void SocketChannel::open()
{
    BOOST_ASSERT( isOpen() );
    getCommunication()->notify( kOnOpen, this );
}

void SocketChannel::close()
{
    getCommunication()->perform( kOnClose, this );
    Tcp::socket::close();
}

bool SocketChannel::isOpen() const
{
    return is_open();
}


bitchat::Channel::CommunicationPtr & SocketChannel::getCommunication()
{
    return m_communication;
}

std::string SocketChannel::read( std::size_t size )
{
    std::string result{};
    boost::asio::streambuf stream{};
    auto readed{ 0ull };

    if ( size == 0 )
    {
        readed = boost::asio::read_until( * this, stream, kEndLine );
    }
    else
    {
        result.resize( size );
        readed = boost::asio::read( * this, stream );
    }

    std::istream input{ & stream };
    std::getline( input, result );

    return result;
}

void SocketChannel::write( const std::string & data )
{
    boost::asio::write( * this, boost::asio::buffer( data ) );
}

std::string SocketChannel::getLocalAddress()
{
    return makeEndpointAddress( local_endpoint() );
}

std::string SocketChannel::getRemoteAddress()
{
    return makeEndpointAddress( remote_endpoint() );
}

std::string SocketChannel::makeEndpointAddress( const Tcp::endpoint & endpoint )
{
    std::string result{ endpoint.address().to_string() };

    result += ':' + std::to_string( endpoint.port() );

    return result;
}
