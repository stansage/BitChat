#pragma once

#include "baseevent.hpp"
#include <boost/asio/io_service.hpp>
#include <memory>

namespace bitchat {

class Communication : boost::noncopyable
{
    struct Context;

public:
    class Event : public BaseEvent{};

    static const Event kOnStart;
    static const Event kOnStop;

public:
    Communication();
    ~Communication();

    boost::asio::io_service & getIos();

    void open();
    void close();
    void shutdown();

    std::size_t subscribersCount( const BaseEvent & event );

    void subscribe( const BaseEvent & event,
                    const BaseEvent::Target target );

    void unsubscribe( const BaseEvent & event,
                      const BaseEvent::Target target );

    template<typename T, typename F>
    void subscribe( const BaseEvent & event,
                    T & handler,
                    F method )
    {
        subscribe( event, std::bind( method,
                                     std::ref( handler ),
                                     std::placeholders::_1 ) );
    }

    template<typename T, typename F>
    void unsubscribe( const BaseEvent & event,
                      T & handler,
                      F method )
    {
        forgetTarget( event.getTag(),
                      std::bind( method,
                                 std::ref( handler ),
                                 std::placeholders::_1 ), false );
    }

    template<typename T, typename F>
    void doLater( T & handler,
                  F method  )
    {
        getIos().post( std::bind( method, std::ref( handler ) ) );
    }

    void perform( const BaseEvent & event,
                         void * arg = nullptr );
    void notify( const BaseEvent & event,
                        void * arg = nullptr );

private:
    bool procceedEvent( const BaseEvent & event,
                        void * arg,
                        const bool async );
    bool procceedTarget( BaseEvent::Target target,
                         void * arg );
    void forgetTarget( const int tag,
                       BaseEvent::Target target,
                       const bool byAddress );


//    void communicate( const std::shared_ptr<Network> network,
//                      const std::shared_ptr<Blockchain> blockchain );

private:
    std::unique_ptr<Context> m_context;
//    static Communication * m_instance;

//    Impl * m_impl;
//    std::weak_ptr<Network> m_network;
//    std::weak_ptr<Blockchain> m_blockchain;
};

} // bitchat
