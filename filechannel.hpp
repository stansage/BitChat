#pragma once

#include "channel.hpp"
#include <boost/asio/io_service.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

namespace bitchat {

class FileChannel :
        virtual public Channel,
        public boost::asio::posix::stream_descriptor
{

protected:
    explicit FileChannel( const CommunicationPtr & communication );

public:
    CommunicationPtr & getCommunication() override;

    void close() override;
    bool isOpen() const override;

    std::string read( const std::size_t size ) override;
    void write( const std::string & data ) override;

    std::size_t read( char * buffer,
                      const std::size_t size );
    std::size_t write( const char * buffer,
                       const std::size_t size );

private:
    CommunicationPtr m_communication;
};

} // bitchat
