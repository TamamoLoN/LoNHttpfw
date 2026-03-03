#include "lonhttpfw/lonhttpfw.h"

static auto g_logger = LON_LOG_ROOT;
uint16_t g_port      = 8080;

void test_ws_client()
{
    auto ret = lon::ws::WSConnection::create(
        "http://127.0.0.1:" + lon::util::lexical_cast<std::string>(g_port) + "/ws", 3000);
    auto res = ret.first;
    if (res->result != 0)
    {
        LON_ERROR(g_logger) << res->error;
        return;
    }
    auto connection = ret.second;
    while (true)
    {
        static int count = 0;
        if (++count > 10)
        {
            return;
        }
        for (int i = 0; i < 1; ++i)
        {
            // connection->sendMessage(lon::util::HashUtil::random_string(60),
            //                         lon::ws::WSFrameHead::TEXT_FRAME, false);
        }
        connection->sendMessage(lon::util::HashUtil::random_string(2000),
                                lon::ws::WSFrameHead::TEXT_FRAME, true);
        auto msg = connection->recvMessage();
        if (!msg)
        {
            break;
        }
        std::cout << "opcode=" << msg->getOpcode() << " data=" << msg->getData() << std::endl;

        sleep(1);
    }
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
    ios->schedule(test_ws_client);
    return 0;
}
