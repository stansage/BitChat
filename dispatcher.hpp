#pragma once

#include <boost/asio/io_service.hpp>

namespace bitchat {

class Channel;
class Console;
class Network;
class Blockchain;
class Communication;

class Dispatcher : boost::noncopyable
{
    using Work = boost::asio::io_service::work;

public:
    Dispatcher( Console & console,
                Network & network,
                Blockchain & blockchain );

    bool start( void * arg );
    bool stop( void * arg );

private:
    bool onChannelOpened( void * arg );
    bool onChannelClosed( void * arg );
    bool onBlochainSaved( void * arg );

    void promptEmail();
    void promptMessage();
    

    void readServerRequest();
    void writeServerResponce();

    void readClientResponse( Channel * client );

    std::string readRawBlock( Channel * channel );

private:
    std::string m_email;
    std::unique_ptr<Work> m_work;
    Console & m_console;
    Network & m_network;
    Blockchain & m_blockchain;
    std::vector<Channel*> m_clients;
};

} // bitchat
