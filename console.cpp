#include "console.hpp"
#include "communication.hpp"
#include <unistd.h>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <istream>

using bitchat::Console;

bitchat::StdIn::StdIn( const CommunicationPtr & communication ) :
    FileChannel{ communication }
{
}

bitchat::StdIn::~StdIn()
{
}

void bitchat::StdIn::open()
{
    FileChannel::assign( ::dup( STDIN_FILENO ) );
    non_blocking( false );
}

bitchat::StdOut::StdOut( const CommunicationPtr & communication ) :
    FileChannel{ communication }
{

}

bitchat::StdOut::~StdOut()
{
}

void bitchat::StdOut::open()
{
    assign( ::dup( STDOUT_FILENO ) );
    non_blocking( false );
}

Console::Console( const CommunicationPtr & communication ) :
    StdIn{ communication },
    StdOut{ communication }
{
}

Console::~Console()
{
}

bitchat::Channel::CommunicationPtr & Console::getCommunication()
{
    return StdIn::getCommunication();
}

void Console::open()
{
    StdIn::open();
    StdOut::open();
    getCommunication()->notify( kOnOpen, this );
}

void Console::close()
{
    getCommunication()->perform( kOnClose, this );
    StdIn::close();
    StdOut::close();
}

bool Console::isOpen() const
{
    return StdIn::isOpen() &&
           StdOut::isOpen();
}

std::string Console::read( const std::size_t size )
{
    return StdIn::read( size );
}

void Console::write( const std::string & data )
{
    StdOut::write( data );
}

