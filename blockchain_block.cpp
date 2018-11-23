#include "blockchain_block.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

using bitchat::Blockchain;


Blockchain::Block::Sha256 Blockchain::Block::calculateHash() const
{
    Sha256 result{};
    const auto begin{ getRawPointer() };
    const auto end{ begin + getSize() };

    picosha2::hash256( begin, end, result.begin(), result.end() );

    return result;
}

char * Blockchain::Block::getRawPointer()
{
    return reinterpret_cast<char *>( this );
}

const char *Blockchain::Block::getRawPointer() const
{
    return reinterpret_cast<const char *>( this );
}

std::size_t Blockchain::Block::getSize()
{
    return sizeof( Block );
}

std::string Blockchain::Block::convertToString( const Sha256 & hash )
{
    return picosha2::bytes_to_hex_string( hash.begin(), hash.end() );
}

std::string Blockchain::Block::convertToString( const Value & value )
{
    return std::string{ value.data() };
}

std::string Blockchain::Block::convertToString( const Blockchain::Block::Key & key )
{
    return std::string{ key.data() };
}

std::int64_t Blockchain::Block::getCurrentTimestamp()
{
    const auto epoch{ boost::posix_time::from_time_t( 0 ) };
    const auto now{ boost::posix_time::microsec_clock::universal_time() };
    const auto timestamp{ now - epoch };
    return timestamp.total_milliseconds();
}
