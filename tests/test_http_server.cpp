#include "lonhttpfw/lonhttpfw.h"

void test_http_server()
{
    lon::net::Address::Ptr addr;
    lon::net::Address::parse(addr, "0.0.0.0:8080", AF_INET);

    LON_INFO(LON_LOG_ROOT) << "addr=" << addr->toString();

    std::vector<lon::net::Address::Ptr> addrs;
    addrs.push_back(addr);
    auto server = std::make_shared<lon::httpserver::HttpServer>(
        lon::scheduler::IOScheduler::getThis(), lon::scheduler::IOScheduler::getThis(),
        lon::config::GlobalConfig::Instance().config_tcp_server_client_timeout->getData(),
        "http_server", false);
    std::vector<lon::net::Address::Ptr> bind_failed_addrs;
    while (!server->bind(addrs, bind_failed_addrs))
    {
        bind_failed_addrs.clear();
        sleep(2);
    }
    server->start();
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml(".config/log.yaml");
    auto ios = std::make_shared<lon::scheduler::IOScheduler>(
        2, true, "io_scheduler", lon::config::GlobalConfig::Instance().config_fiber->getData());
    ios->schedule(test_http_server);

    return 0;
}
