#include "lonhttpfw/lonhttpfw.h"

static auto g_logger = LON_LOG_ROOT;
uint16_t g_port      = 8080;

void test_ws_server()
{
    lon::net::Address::Ptr addr;
    lon::net::Address::parse(addr, "0.0.0.0:" + std::to_string(g_port), AF_INET);

    LON_INFO(LON_LOG_ROOT) << "addr=" << addr->toString();
    auto server = std::make_shared<lon::ws::WSServer>(
        lon::scheduler::IOScheduler::getThis(), lon::scheduler::IOScheduler::getThis(),
        lon::config::GlobalConfig::Instance().config_tcp_server_client_timeout->getData(),
        "ws_server");
    auto websocket_handle = [](const lon::http::HttpRequest::Ptr &req,
                               const lon::ws::WSFrameMessage::Ptr &msg,
                               const lon::ws::WSSession::Ptr &session) {
        session->sendMessage(msg);
        return 0;
    };

    server->getDispatch()->addServlet("/ws", websocket_handle);
    while (!server->bind(addr))
    {
        LON_ERROR(g_logger) << "bind " << addr->toString() << " fail";
        sleep(2);
    }
    server->start();
}

int main(int argc, char *argv[])
{
    auto parser = lon::util::ArgumentParser();
    parser.addArgument(std::vector<std::string>{"-p", "--port"})
        ->help("ipv4 port")
        ->defaultValue("8080");
    try
    {
        parser.parse(argc, argv);
    }
    catch (...)
    {
        return 0;
    }
    g_port = parser.get<uint16_t>("-p");
    lon::config::Config::parseFromYaml(".config/log.yaml");
    auto ios = std::make_shared<lon::scheduler::IOScheduler>(
        1, true, "io_scheduler", lon::config::GlobalConfig::Instance().config_fiber->getData());
    ios->schedule(test_ws_server);
    return 0;
}
