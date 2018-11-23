#include "blockchain.hpp"
#include "blockchain_block.hpp"
#include "communication.hpp"
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/log/trivial.hpp>
#include <boost/filesystem/operations.hpp>

using bitchat::Blockchain;

//namespace
//{
//constexpr auto kMaximumMessageSize{ 512 * 1024 * 1024 }; //! 512MB
//}
const Blockchain::Event Blockchain::kOnSave{};

Blockchain::Blockchain( const CommunicationPtr & communication,
                        const std::string & path ) :
    FileChannel{ communication },
    m_path{ path },
    m_headIndex{ 0 }
{
//    m_headBlock->difficult = 0;
}

void Blockchain::open()
{
    const auto flags{ boost::filesystem::exists( m_path ) ?
                      O_RDWR | O_NONBLOCK :
                      O_CREAT | O_RDWR | O_NONBLOCK };

    auto descriptor{ ::open( m_path.c_str(), flags, 0640 ) };

    try
    {
        bytes_readable command{};

        assign( descriptor );
        non_blocking( true );
        io_control( command );

        m_headIndex = command.get() / getBlockSize();

        if ( m_headIndex > 0 )
        {
            --m_headIndex;
        }
        else
        {
            Block block{};
            block.key[ 0 ] = '@';
            saveBlock( block );
        }
    }
    catch ( ... )
    {
        ::close( descriptor );
        ::remove( m_path.c_str() );
        throw;
    }
    getCommunication()->notify( kOnOpen, this );
    BOOST_LOG_TRIVIAL( debug ) << "The blockhain opened";
}

void Blockchain::close()
{
    getCommunication()->perform( kOnClose, this );
    FileChannel::close();
    m_headIndex = 0;
    //    m_headBlock.reset();
}

std::size_t Blockchain::getHeadIndex()
{
    return m_headIndex;
}

std::string Blockchain::getHeadHash()
{
    return Block::convertToString( getBlock( 0 ).calculateHash() );
}

std::string Blockchain::getHeadKey()
{
    return Block::convertToString( getBlock( 0 ).key );
}

std::string Blockchain::getHeadValue()
{
    return Block::convertToString( getBlock( 0 ).value );
}

std::int64_t Blockchain::getHeadTimestamp()
{
    return getBlock( 0 ).timestamp;
}

void Blockchain::save( const std::string & rawBlock )
{
    const auto block{ convertBlock( rawBlock ) };

    saveBlock( block );
    m_headIndex = block.index;
    getCommunication()->notify( kOnSave, this );
}

void Blockchain::store( const std::string & key,
                        const std::string & value )
{
    BOOST_ASSERT( ! key.empty() );
    BOOST_ASSERT( ! value.empty() );

    auto block = getBlock( 0 );
    block.previousHash = block.calculateHash();
    block.timestamp = Block::getCurrentTimestamp();
    std::copy( key.begin(), key.end(), block.key.begin() );
    std::copy( value.begin(), value.end(), block.value.begin() );
    ++block.index;

    saveBlock( block );
    m_headIndex = block.index;
    getCommunication()->notify( kOnSave, this );
}

std::string Blockchain::makeBlockRequest( const std::uint64_t index )
{
    std::string result{};
    const auto begin{ reinterpret_cast<const char *>( & index ) };
    const auto end{ begin + sizeof( index ) };

    result.reserve( 1 + sizeof( index ) );
    result += kRequestBlock;
    result.append( begin, end );

    return result;
}

std::string Blockchain::makeBlockResponse( const std::uint64_t index )
{
    const auto block{ getBlock( index ) };
    return kResponseBlock + convertBlock( block );
}

std::string Blockchain::makeNewBlock( const std::uint64_t index )
{
    const auto block{ getBlock( index ) };
    return kNewBlock + convertBlock( block );
}

std::uint64_t Blockchain::extractBlockIndex( const std::string & data )
{
    return * reinterpret_cast<const std::uint64_t*>( data.data() );
}

std::size_t Blockchain::getBlockSize()
{
    return Block::getSize();
}

Blockchain::Block Blockchain::getBlock( const std::uint64_t index )
{
    return loadBlock( index == 0 ? getHeadIndex() : index );
}

Blockchain::Block Blockchain::loadBlock( const std::uint64_t index )
{
    Block result{};

    if ( is_open() )
    {
        const auto position{ static_cast<std::int64_t>( index * getBlockSize() ) };

        ::lseek64( native_handle(), position, SEEK_SET );
        read( result.getRawPointer(), getBlockSize() );
    }

    return result;
}

void Blockchain::saveBlock( const Block & block )
{
    write( block.getRawPointer(), getBlockSize() );
}

std::string Blockchain::convertBlock( const Block & block )
{
    return std::string{ block.getRawPointer(), getBlockSize() };
}

Blockchain::Block Blockchain::convertBlock( const std::string & data )
{
    Block result{};

    std::copy( data.begin(), data.end(), result.getRawPointer() );

    return result;
}

//void Blockchain::proofOfWork( Block & block )
//{
//    if ( block.difficult != 0 )
//    {
//        for ( auto found{ false }; ! found; ++ block.nonce )
//        {
//            auto hash{ block.calculateHash() };
//            const auto predicate{ []( const auto byte )
//            {
//                    return byte != 0;
//            } };
//            BOOST_ASSERT( block.difficult < hash.size() );

    //        std::cout << Block::convertHashToString( hash )
    //                  << std::endl
    //                  << std::endl;
//            found = hash.end() == std::find_if( hash.begin(),
//                                                hash.begin() + block.difficult,
//                                                predicate );
//        }
//    }
//}


