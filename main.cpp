#include "application.hpp"
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <string>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace
{
constexpr auto kReconnectInterval{ 1 };
constexpr auto kOptionHelp{ "help" };
constexpr auto kOptionServer{ "server" };
constexpr auto kUsage{ "Usage: %1% [--%2%|--%3% ip:port] \n"
                        "Description" };
}

int main( int argc,
          char * argv[] )
{
    auto result{ 0 };
    try
    {
        const auto app{ fs::path{ argv[ 0 ] }.stem().string() };
        po::variables_map values{};
        po::options_description options{ boost::str( boost::format{ kUsage } %
                                                     app %
                                                     kOptionHelp %
                                                     kOptionServer ) };

        options.add_options()
                ( kOptionHelp, "print program help" )
                ( kOptionServer, po::value<std::string>(), "connect to remote server" );

        po::store( po::parse_command_line( argc, argv, options), values );
        po::notify( values );

        if ( values.count( kOptionHelp ) > 0 )
        {
            std::cout << options << std::endl;
        }
        else
        {
            auto host{ app };
            auto port{ 0 };

            host.clear();

            if ( values.count( kOptionServer ) > 0 )
            {
                const auto server{ values[ kOptionServer ].as<std::string>() };
                const auto idx{ server.find( ':' ) };

                if ( idx != std::string::npos )
                {
                    host = server.substr( 0, idx );
                    port = std::stoi( server.substr( idx + 1 ) );
                }
                if ( port < 1024 || port > 65535 )
                {
                    throw  std::invalid_argument( "Invalid port - " + server );
                }
                if ( host.empty() )
                {
                    throw  std::invalid_argument( "Invalid host - " + server );
                }
            }

            bitchat::Application::run( app, host, port, kReconnectInterval );
        }
    }
    catch ( const std::runtime_error & exception )
    {
        result = 2;
        std::cerr << "Error while running program! " << exception.what() << std::endl;
    }
    catch ( const std::exception & exception )
    {
        result = 1;
        std::cerr << "Error while parsing command line! " << exception.what() << std::endl;
    }

    return result;
}
