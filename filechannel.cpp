#include "filechannel.hpp"
#include "communication.hpp"
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/log/trivial.hpp>

using bitchat::FileChannel;

FileChannel::FileChannel( const CommunicationPtr & communication ) :
    stream_descriptor{ communication->getIos() },
    m_communication{ communication }
{
}

bitchat::Channel::CommunicationPtr & FileChannel::getCommunication()
{
    return m_communication;
}


void FileChannel::close()
{
    try
    {
        if ( is_open() )
        {
            stream_descriptor::close();
            ::close( native_handle() );
        }
    }
    catch ( std::exception & exception )
    {
        BOOST_LOG_TRIVIAL( error ) << "Error while closing file channel - " << exception.what();
    }
}

bool FileChannel::isOpen() const
{
    return stream_descriptor::is_open();
}

std::string FileChannel::read( const std::size_t size )
{
    std::string result{};
    boost::asio::streambuf buffer{};
    auto readed{ 0ull };

    if ( size == 0 )
    {
        readed = boost::asio::read_until( * this, buffer, kEndLine );
    }
    else
    {
        result.resize( size );
        readed = boost::asio::read( * this, buffer );
    }

    if ( readed > 0 )
    {
        std::istream input{ & buffer };
        std::getline( input, result );
    }

    return result;
}

void FileChannel::write( const std::string & data )
{
    boost::asio::write( * this, boost::asio::buffer( data ) );
}

std::size_t FileChannel::read( char * buffer,
                        const std::size_t size )
{
    return boost::asio::read( * this,
                              boost::asio::buffer( buffer, size ),
                              boost::asio::transfer_all() );
}

std::size_t FileChannel::write( const char * buffer,
                                const std::size_t size )
{
    return boost::asio::write( * this, boost::asio::buffer( buffer, size ) );
}


