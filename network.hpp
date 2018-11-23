#pragma once

//#include <boost/asio/io_service.hpp>
#include "baseevent.hpp"
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace bitchat {

class SocketChannel;
class Communication;

class Network : boost::noncopyable
{
    using Tcp = boost::asio::ip::tcp;
    using Address = boost::asio::ip::address;

public:
    explicit Network( const std::shared_ptr<Communication> & communication,
                      const std::string & ip,
                      const std::uint16_t port,
                      const int reconnectInterval );

    void open();
    void close();

    std::uint16_t getListenningPort() const;

    std::shared_ptr<SocketChannel> getServerSocket() const;
    std::vector<std::shared_ptr<SocketChannel>> getClientSockets() const;

protected:
    void connect( const Tcp::endpoint endpoint );
    void reconnect( const Tcp::endpoint endpoint );
    void disconnect();

    static bool wasAborted( int errorValue );

private:
    const std::string m_ip;
    const std::uint16_t m_port;
    Tcp::acceptor m_acceptor;
    boost::asio::deadline_timer m_timer;
    boost::posix_time::time_duration m_reconnectInterval;
    std::shared_ptr<SocketChannel> m_server;
    std::vector<std::shared_ptr<SocketChannel>> m_clients;
};

} // bitchat
