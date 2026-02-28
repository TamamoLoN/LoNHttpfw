#include "lonhttpfw/lonhttpfw.h"

static auto g_logger = LON_LOG_ROOT;

void test_http_client()
{
    lon::net::Address::Ptr addr;
    lon::net::Address::parse(addr, "httpbin.org:80", AF_INET);

    LON_INFO(g_logger) << "addr=" << addr->toString();

    auto socket = lon::net::Socket::create(addr);
    auto ret    = socket->connect(addr);
    if (!ret)
    {
        LON_ERROR(g_logger) << "connect failed: " << socket->toString();
        return;
    }

    auto connection = std::make_shared<lon::httpservice::HttpConnection>(
        socket, true,
        lon::http::HttpGlobalConfig::Instance().config_http->getData().response.buffer_size);

    auto request = std::make_shared<lon::http::HttpRequest>();
    request->setPath("/stream/3");
    request->setHeader("Host", "httpbin.org");
    LON_INFO(g_logger) << "request=" << request->toString();
    connection->sendRequest(request);
    auto response = connection->recvResponse();
    if (!response)
    {
        LON_ERROR(g_logger) << "recvResponse failed";
        return;
    }
    LON_INFO(g_logger) << "response=" << response->toString();
    LON_INFO(g_logger) << "================================";
    auto res = lon::httpservice::HttpConnection::request(lon::http::HttpMethod::GET,
                                                         "http://127.0.0.1:8080", 1000);
    if (res->result != 0)
    {
        LON_ERROR(g_logger) << "request failed: " << res->error;
        return;
    }
    LON_INFO(g_logger) << "response=" << res->response->toString();
}

void test_https_client()
{
    auto res = lon::httpservice::HttpConnection::get("https://www.baidu.com", 3000);
    if (res->result != (int)lon::httpservice::HttpResult::Error::OK)
    {
        LON_ERROR(g_logger) << "do get failed: " << res->error;
        return;
    }
    LON_INFO(g_logger) << res->response->toString();
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml(".config/log.yaml");
    auto ios = std::make_shared<lon::scheduler::IOScheduler>(
        2, true, "io_scheduler", lon::config::GlobalConfig::Instance().config_fiber->getData());
    ios->schedule(test_https_client);
}
