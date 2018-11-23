#pragma once

#include "filechannel.hpp"

namespace bitchat {

class Communication;

class Blockchain : protected FileChannel
{
    class Block;

public:
    class Event : public BaseEvent{};

    static constexpr auto kKeySize{ 20 };
    static constexpr auto kValueSize{ 140 };
    static constexpr auto kRequestBlock{ 'r' };
    static constexpr auto kResponseBlock{ 'b' };
    static constexpr auto kNewBlock{ 'n' };
    static const Event kOnSave;

    explicit Blockchain( const CommunicationPtr & communication,
                         const std::string & path );

    void open() override;
    void  close() override;

    std::size_t getHeadIndex();
    std::string getHeadHash();
    std::string getHeadKey();
    std::string getHeadValue();
    std::int64_t getHeadTimestamp();

//    std::string loadBlockDataByIndex( const std::uint64_t index );

    void save( const std::string & rawBlock );
    void store( const std::string & key,
                const std::string & value );

    std::string makeBlockRequest( const std::uint64_t index );
    std::string makeBlockResponse( const std::uint64_t index );
    std::string makeNewBlock( const std::uint64_t index );

    static std::uint64_t extractBlockIndex( const std::string & data );
    static std::size_t getBlockSize();

private:
    Block getBlock( const std::uint64_t index );

    Block loadBlock( const std::uint64_t index );
    void saveBlock( const Block & block );

    static std::string convertBlock( const Block & block );
    static Block convertBlock( const std::string & block );
//    void proofOfWork( Block & block );

private:
    const std::string m_path;
    std::size_t m_headIndex;
//    std::shared_ptr<Block> m_headBlock;
};


} // bitchat
