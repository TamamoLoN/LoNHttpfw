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
    auto dispatch = server->getDispatch();
    dispatch->addServlet(
        "/", std::make_shared<lon::httpserver::HttpServletFunction>(
                 [](const lon::http::HttpRequest::Ptr &req, const lon::http::HttpResponse::Ptr &res,
                    const lon::httpserver::HttpSession::Ptr &session) -> int32_t {
                     const std::string body =
                         "<!DOCTYPE html>\n"
                         "<html>\n"
                         "<head>\n"
                         "  <meta charset=\"UTF-8\">\n"
                         "  <title>Welcome to LoNHttpfw!</title>\n"
                         "  <style>\n"
                         "    body { width: 35em; margin: 0 auto; font-family: Tahoma, Verdana, "
                         "Arial, sans-serif; }\n"
                         "    h1 { text-align:center; margin-top: 40px; }\n"
                         "    p { line-height: 1.5; }\n"
                         "    footer { text-align:center; margin-top:50px; color:#888; "
                         "font-size:14px; }\n"
                         "  </style>\n"
                         "</head>\n"
                         "<body>\n"
                         "  <h1>Welcome to LoNHttpfw!</h1>\n"
                         "  <p>If you see this page, the LoNHttpfw web server is successfully "
                         "installed and working. Further configuration is required.</p>\n"
                         "  <p>For online documentation and support please refer to <a "
                         "href=\"http://tamamolon.site/\">tamamolon.site</a>.<br/>\n"
                         "  Commercial support is available at <a "
                         "href=\"http://tamamolon.site/\">tamamolon.site</a>.</p>\n"
                         "  <footer><hr/>LoNHttpfw/" LONETFW_VERSION "</footer>\n"
                         "</body>\n"
                         "</html>";
                     res->setStatus(lon::http::HttpStatus::OK);
                     res->setHeader("Content-Type", "text/html");
                     res->setBody(body);
                     return 0;
                 },
                 "menu"));

    dispatch->addServlet(
        "/test/test",
        std::make_shared<lon::httpserver::HttpServletFunction>(
            [](const lon::http::HttpRequest::Ptr &req, const lon::http::HttpResponse::Ptr &res,
               const lon::httpserver::HttpSession::Ptr &session) -> int32_t {
                const std::string body = req->toString();
                res->setStatus(lon::http::HttpStatus::OK);
                res->setHeader("Content-Type", "text/html");
                res->setBody(body);
                return 0;
            },
            "test"));
    dispatch->addGlobServlet(
        "/test/*",
        std::make_shared<lon::httpserver::HttpServletFunction>(
            [](const lon::http::HttpRequest::Ptr &req, const lon::http::HttpResponse::Ptr &res,
               const lon::httpserver::HttpSession::Ptr &session) -> int32_t {
                const std::string body = "test glob\r\n" + req->toString();
                res->setStatus(lon::http::HttpStatus::OK);
                res->setHeader("Content-Type", "text/html");
                res->setBody(body);
                return 0;
            },
            "test glob"));
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
