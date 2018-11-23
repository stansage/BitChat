#pragma once

#include "filechannel.hpp"

namespace bitchat {

class StdIn : protected FileChannel
{
public:
    explicit StdIn( const CommunicationPtr & communication );
    ~StdIn() override;

    void open() override;

private:
    using FileChannel::write;
};

class StdOut : protected FileChannel
{
public:
    explicit StdOut( const CommunicationPtr & communication );
    ~StdOut() override;

    void open() override;

private:
    using FileChannel::read;
};

class Console :
        protected StdIn,
        protected StdOut
{
public:
    explicit Console( const CommunicationPtr & communication );
    ~Console() override;

    CommunicationPtr & getCommunication() override;

    void open() override;
    void close() override;
    bool isOpen() const override;

    std::string read( const std::size_t size ) override;
    void write( const std::string & data ) override;
};

} // bitchat
