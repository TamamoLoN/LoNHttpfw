#include "http/http.h"
#include "http/httpparser.h"
#include "http/httprequest.h"
#include "http/httpresponse.h"

static auto g_logger = LON_LOG_ROOT;

std::string test_request()
{
    std::map<std::string, std::string, lon::util::InsensitiveStringCompare> tests;
    tests["aaa"] = "a";
    tests["A"]   = "A";
    tests["bbb"] = "b";
    tests["B"]   = "B";
    for (auto &test : tests)
        LON_DEBUG(g_logger) << test.first << " => " << test.second;

    lon::http::HttpRequest request;
    LON_DEBUG(g_logger) << lon::http::HttpMethodConverter::toString(lon::http::HttpMethod::GET);
    LON_DEBUG(g_logger) << (int)lon::http::HttpMethodConverter::fromString("GET");

    // request.setMethod(lon::http::HttpMethod::GET);
    // request.setPath("/");
    request.setHeader("Host", "117.72.171.212");
    request.setHeader("Connection", "keep-alive");
    request.setBody("hello lon");
    LON_DEBUG(g_logger) << request.toString();
    return request.toString();
}

std::string test_response()
{
    lon::http::HttpResponse response;
    response.setHeader("Tamamo", "LoN");
    response.setBody("hello lon");
    response.setClose(false);
    response.setStatus(lon::http::HttpStatus::FORBIDDEN);
    LON_DEBUG(g_logger) << response.toString();

    return response.toString();
}

void test_request_parser()
{
    lon::http::HttpRequestParser parser;
    auto text    = test_request();
    auto request = parser.parse(&text[0], text.size());
    LON_INFO(g_logger) << "parser status=" << parser.finished() << ", error=" << parser.error()
                       << ", total_size=" << text.size()
                       << ", content_length=" << parser.getContentLength();
    LON_INFO(g_logger) << request->toString();
}

void test_response_parser()
{
    lon::http::HttpResponseParser parser;
    auto text = test_response();
    text      = "HTTP/1.1 200 OK\r\n"
           "Content-Length: 14\r\n"
           "access-control-allow-origin: *\r\n"
           "content-type: text/plain\r\n"
           "date: Fri, 19 Sep 2025 02:43:23 GMT\r\n"
           "via: 1.1 google\r\n"
           "\r\n"
           "117.72.171.212\r\n";
    auto response = parser.parse(&text[0], text.size());
    LON_INFO(g_logger) << "parser status=" << parser.finished() << ", error=" << parser.error()
                       << ", total_size=" << text.size()
                       << ", content_length=" << parser.getContentLength();
    LON_INFO(g_logger) << response->toString();
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml("./.config/log.yaml");
    test_request();
    test_response();
    test_request_parser();
    test_response_parser();
    return 0;
}
