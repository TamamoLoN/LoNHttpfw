#include "lonhttpfw/lonhttpfw.h"
#include <signal.h>

uint16_t port = 8080;

void test_http_server()
{
    lon::net::Address::Ptr addr;
    lon::net::Address::parse(addr, "0.0.0.0:" + std::to_string(port), AF_INET);

    LON_INFO(LON_LOG_ROOT) << "addr=" << addr->toString();

    std::vector<lon::net::Address::Ptr> addrs;
    addrs.push_back(addr);
    auto server = std::make_shared<lon::httpservice::HttpServer>(
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
        "/", std::make_shared<lon::httpservice::HttpServletFunction>(
                 [](const lon::http::HttpRequest::Ptr &req, const lon::http::HttpResponse::Ptr &res,
                    const lon::httpservice::HttpSession::Ptr &session) -> int32_t {
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
        std::make_shared<lon::httpservice::HttpServletFunction>(
            [](const lon::http::HttpRequest::Ptr &req, const lon::http::HttpResponse::Ptr &res,
               const lon::httpservice::HttpSession::Ptr &session) -> int32_t {
                const std::string body = req->toString();
                res->setStatus(lon::http::HttpStatus::OK);
                res->setBody(body);
                return 0;
            },
            "test"));
    dispatch->addGlobServlet(
        "/test/*",
        std::make_shared<lon::httpservice::HttpServletFunction>(
            [](const lon::http::HttpRequest::Ptr &req, const lon::http::HttpResponse::Ptr &res,
               const lon::httpservice::HttpSession::Ptr &session) -> int32_t {
                const std::string body = "test glob\r\n" + req->toString();
                res->setStatus(lon::http::HttpStatus::OK);
                res->setBody(body);
                return 0;
            },
            "test glob"));

    dispatch->addServlet(
        "/get/fds",
        std::make_shared<lon::httpservice::HttpServletFunction>(
            [](const lon::http::HttpRequest::Ptr &req, const lon::http::HttpResponse::Ptr &res,
               const lon::httpservice::HttpSession::Ptr &session) -> int32_t {
                const std::string body = req->toString();
                res->setStatus(lon::http::HttpStatus::OK);
                res->setBody("fds: " + lon::util::lexical_cast<std::string>(FDMGR.size()));
                return 0;
            },
            "get fds"));

    dispatch->addServlet(
        "/get/fibers",
        std::make_shared<lon::httpservice::HttpServletFunction>(
            [](const lon::http::HttpRequest::Ptr &req, const lon::http::HttpResponse::Ptr &res,
               const lon::httpservice::HttpSession::Ptr &session) -> int32_t {
                const std::string body = req->toString();
                res->setStatus(lon::http::HttpStatus::OK);
                res->setBody("fibers: " +
                             lon::util::lexical_cast<std::string>(lon::fiber::Fiber::getFibers()));
                return 0;
            },
            "get fiber count"));

    dispatch->addServlet(
        "/get/tasks",
        std::make_shared<lon::httpservice::HttpServletFunction>(
            [](const lon::http::HttpRequest::Ptr &req, const lon::http::HttpResponse::Ptr &res,
               const lon::httpservice::HttpSession::Ptr &session) -> int32_t {
                const std::string body = req->toString();
                res->setStatus(lon::http::HttpStatus::OK);
                res->setBody("tasks: " +
                             lon::util::lexical_cast<std::string>(
                                 lon::scheduler::IOScheduler::getThis()->getTaskCount()));
                return 0;
            },
            "get task count"));

    dispatch->addServlet(
        "/get/config",
        std::make_shared<lon::httpservice::HttpServletFunction>(
            [](const lon::http::HttpRequest::Ptr &req, const lon::http::HttpResponse::Ptr &res,
               const lon::httpservice::HttpSession::Ptr &session) -> int32_t {
                const std::string body = req->toString();
                res->setStatus(lon::http::HttpStatus::OK);
                res->setBody("config:\n" + lon::config::Config::toString());
                return 0;
            },
            "get task count"));

    dispatch->addGlobServlet("/download/*",
                             std::make_shared<lon::httpservice::HttpServletDownload>());
    server->start();
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        std::cout << "usage: " << argv[0] << " <port>" << std::endl;
        return -1;
    }
    signal(SIGPIPE, SIG_IGN);
    port = std::atoi(argv[1]);
    lon::config::Config::parseFromYaml(".config/log.yaml");
    auto ios = std::make_shared<lon::scheduler::IOScheduler>(
        1, true, "io_scheduler", lon::config::GlobalConfig::Instance().config_fiber->getData());
    ios->schedule(test_http_server);

    return 0;
}
