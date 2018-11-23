#pragma once

#include "channel.hpp"
#include <boost/asio/ip/tcp.hpp>

namespace bitchat {

class SocketChannel :
        virtual public Channel,
        public boost::asio::ip::tcp::socket
{
    using Tcp = boost::asio::ip::tcp;

public:
    explicit SocketChannel( const CommunicationPtr & communication );

    void open() override;
    void close() override;
    bool isOpen() const override;

    CommunicationPtr & getCommunication() override;

    std::string read( std::size_t size ) override;
    void write( const std::string & data ) override;

    std::string getLocalAddress();
    std::string getRemoteAddress();

private:
    std::string makeEndpointAddress( const Tcp::endpoint & endpoint );

private:
    CommunicationPtr m_communication;
};

} // bitchat
