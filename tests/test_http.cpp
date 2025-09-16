#include "http/http.h"
#include "http/httprequest.h"
#include "http/httpresponse.h"

void test_request()
{
    std::map<std::string, std::string, lon::util::InsensitiveStringCompare> tests;
    tests["aaa"] = "a";
    tests["A"]   = "A";
    tests["bbb"] = "b";
    tests["B"]   = "B";
    for (auto &test : tests)
        LON_DEBUG(LON_LOG_ROOT) << test.first << " => " << test.second;

    lon::http::HttpRequest request;
    LON_DEBUG(LON_LOG_ROOT) << lon::http::HttpMethodConverter::toString(lon::http::HttpMethod::GET);
    LON_DEBUG(LON_LOG_ROOT) << (int)lon::http::HttpMethodConverter::fromString("GET");

    // request.setMethod(lon::http::HttpMethod::GET);
    // request.setPath("/");
    request.setHeader("Host", "117.72.171.212");
    request.setHeader("Connection", "keep-alive");
    request.setBody("hello lon");
    LON_DEBUG(LON_LOG_ROOT) << request.toString();
}

void test_response()
{
    lon::http::HttpResponse response;
    response.setHeader("Tamamo", "LoN");
    response.setBody("hello lon");
    response.setClose(false);
    response.setStatus(lon::http::HttpStatus::FORBIDDEN);
    LON_DEBUG(LON_LOG_ROOT) << response.toString();
}

int main(int argc, char const *argv[])
{
    test_request();
    test_response();
    return 0;
}
