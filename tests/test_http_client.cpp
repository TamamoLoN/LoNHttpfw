#include "lonhttpfw/lonhttpfw.h"

void test_http_client()
{
    lon::net::Address::Ptr addr;
    lon::net::Address::parse(addr, "httpbin.org:80", AF_INET);

    LON_INFO(LON_LOG_ROOT) << "addr=" << addr->toString();

    auto socket = lon::net::Socket::create(addr);
    auto ret    = socket->connect(addr);
    if (!ret)
    {
        LON_ERROR(LON_LOG_ROOT) << "connect failed: " << socket->toString();
        return;
    }

    auto connection = std::make_shared<lon::httpserver::HttpConnection>(
        socket, true,
        lon::http::HttpGlobalConfig::Instance().config_http->getData().response.buffer_size);

    auto request = std::make_shared<lon::http::HttpRequest>();
    request->setPath("/stream/3");
    request->setHeader("Host", "httpbin.org");
    LON_INFO(LON_LOG_ROOT) << "request=" << request->toString();
    connection->sendRequest(request);
    auto response = connection->recvResponse();
    if (!response)
    {
        LON_ERROR(LON_LOG_ROOT) << "recvResponse failed";
        return;
    }
    LON_INFO(LON_LOG_ROOT) << "response=" << response->toString();
    LON_INFO(LON_LOG_ROOT) << "================================";
    auto res =
        lon::httpserver::HttpClient::request(lon::http::HttpMethod::GET, "http://127.0.0.1", 1000);
    if (res->result != 0)
    {
        LON_ERROR(LON_LOG_ROOT) << "request failed: " << res->error;
        return;
    }
    LON_INFO(LON_LOG_ROOT) << "response=" << res->response->toString();
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml(".config/log.yaml");
    auto ios = std::make_shared<lon::scheduler::IOScheduler>(
        2, true, "io_scheduler", lon::config::GlobalConfig::Instance().config_fiber->getData());
    ios->schedule(test_http_client);
}
