#pragma once

#include "blockchain.hpp"
#include "picosha2.hpp"

namespace bitchat {

#pragma pack(push, 1)

struct Blockchain::Block
{
    using Sha256 = std::array<char, picosha2::k_digest_size>;
    using Value = std::array<char, Blockchain::kValueSize>;
    using Key = std::array<char, Blockchain::kKeySize>;

//    std::size_t getIndex() const;
//    std::uint64_t getTimestamp() const;
//    const Data & getData() const;
//    const Sha256 & getCurrentHash() const;
//    const Sha256 & getPreviousHash() const;

//    const char * beginIterator() const;
//    const char * endIterator() const;

//    std::string toString();
//    void fromString( const std::string & str );

    Sha256 calculateHash() const;

    char * getRawPointer();
    const char * getRawPointer() const;

    static std::size_t getSize();
    static std::string convertToString( const Sha256 & hash );
    static std::string convertToString( const Value & value );
    static std::string convertToString( const Key & key );
    static std::int64_t getCurrentTimestamp();


//private:
//    Block();

//private:
    std::uint64_t index;
    std::int64_t timestamp;
    Key key;
    Value value;
    Sha256 previousHash;
//    std::int64_t nonce;
//    std::uint8_t difficult;
};

#pragma pack(pop)

} // bitchat

