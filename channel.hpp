#pragma once

//#include <boost/asio/io_service.hpp>
//#include <boost/asio/ip/tcp.hpp>
#include "baseevent.hpp"
#include <string>
#include <memory>
#include <utility>

namespace bitchat {

class Communication;

class Channel : private boost::noncopyable
{
protected:
    Channel();

public:
    using ChannelPtr = std::shared_ptr<Channel>;
    using CommunicationPtr = std::shared_ptr<Communication>;
    class Event : public BaseEvent{};

    static constexpr auto kBufferSize{ 4 * 1024 }; //! 4KB
    static constexpr auto kEndLine{ '\n' };

    static const Event kOnOpen;
    static const Event kOnClose;

public:
    virtual ~Channel();

    virtual CommunicationPtr & getCommunication() = 0;

    virtual void open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;

    virtual std::string read( const std::size_t size ) = 0;
    virtual void write( const std::string & data ) = 0;
};

} // bitchat

