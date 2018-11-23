#pragma once

#include <string>
#include <vector>
#include <memory>
#include <thread>

namespace bitchat {

class Communication;

class Application
{
    using ThreadPtr = std::unique_ptr<std::thread>;
    using ThreadPool = std::vector<ThreadPtr>;
    using CommunicationPtr = std::shared_ptr<Communication>;

public:
    Application() = delete;

    static void run( const std::string name,
                     const std::string host,
                     const int port,
                     const int reconnectTimeout );

private:
    static void runPool( CommunicationPtr communication );
    static void runLoop( CommunicationPtr communication );
};

} // bitchat
