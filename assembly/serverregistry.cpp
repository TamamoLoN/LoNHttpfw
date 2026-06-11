#include "assembly/serverregistry.h"

namespace lon
{
namespace assembly
{
ServerRegistry::ServerRegistry()
{
    auto &factory = server::ServerFactory::Instance();
    factory.registerServer("http", [](scheduler::IOScheduler *scheduler,
                                      scheduler::IOScheduler *accept_scheduler,
                                      const config::ConfigServer &config_server) {
        return std::make_shared<httpservice::HttpServer>(
            scheduler, accept_scheduler, G_CONFIG.config_tcp_server_client_timeout->getData(),
            config_server.name, config_server.keepalive);
    });
    factory.registerServer("ws", [](scheduler::IOScheduler *scheduler,
                                    scheduler::IOScheduler *accept_scheduler,
                                    const config::ConfigServer &config_server) {
        return std::make_shared<ws::WSServer>(
            scheduler, accept_scheduler, G_CONFIG.config_tcp_server_client_timeout->getData(),
            config_server.name);
    });
}

} // namespace assembly
} // namespace lon