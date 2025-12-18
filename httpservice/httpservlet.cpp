#include "httpservice/httpservlet.h"

namespace lon
{
namespace httpservice
{
HttpServlet::HttpServlet(const std::string &name) : m_name(name) {}

HttpServlet::~HttpServlet() {}

const std::string &HttpServlet::getName() const { return m_name; }

HttpServletFunction::HttpServletFunction(callback cb, const std::string &name)
    : HttpServlet(name), m_cb(cb)
{
}

HttpServletFunction::~HttpServletFunction() {}

int32_t HttpServletFunction::handle(const http::HttpRequest::Ptr &req,
                                    const http::HttpResponse::Ptr &res,
                                    const HttpSession::Ptr &session)
{
    return m_cb(req, res, session);
}

HttpServletDispatch::HttpServletDispatch(const std::string &name) : HttpServlet(name)
{
    setDefaultServlet(std::make_shared<HttpServlet404NotFound>());
}

HttpServletDispatch::~HttpServletDispatch() {}

int32_t HttpServletDispatch::handle(const http::HttpRequest::Ptr &req,
                                    const http::HttpResponse::Ptr &res,
                                    const HttpSession::Ptr &session)
{
    auto servlet = getServlet(req->getPath());
    if (servlet)
    {
        return servlet->handle(req, res, session);
    }
    return -1;
}

void HttpServletDispatch::addServlet(const std::string &uri, const HttpServlet::Ptr &servlet)
{
    MutexType::WrLock lock(m_mutex);
    m_servlets[uri] = servlet;
}

void HttpServletDispatch::addServlet(const std::string &uri, HttpServletFunction::callback cb)
{
    addServlet(uri, std::make_shared<HttpServletFunction>(cb));
}

void HttpServletDispatch::addGlobServlet(const std::string &uri, const HttpServlet::Ptr &servlet)
{
    MutexType::WrLock lock(m_mutex);
    for (auto it = m_glob_servlets.begin(); it != m_glob_servlets.end(); ++it)
    {
        if (it->first == uri)
        {
            m_glob_servlets.erase(it);
            break;
        }
    }
    m_glob_servlets.push_back(std::make_pair(uri, servlet));
}

void HttpServletDispatch::addGlobServlet(const std::string &uri, HttpServletFunction::callback cb)
{
    addGlobServlet(uri, std::make_shared<HttpServletFunction>(cb));
}

void HttpServletDispatch::delServlet(const std::string &uri)
{
    MutexType::WrLock lock(m_mutex);
    m_servlets.erase(uri);
}

void HttpServletDispatch::delGlobServlet(const std::string &uri)
{
    MutexType::WrLock lock(m_mutex);
    for (auto it = m_glob_servlets.begin(); it != m_glob_servlets.end(); ++it)
    {
        if (it->first == uri)
        {
            m_glob_servlets.erase(it);
            break;
        }
    }
}

HttpServlet::Ptr HttpServletDispatch::findServlet(const std::string &uri)
{
    MutexType::RdLock lock(m_mutex);
    auto it = m_servlets.find(uri);
    return it == m_servlets.end() ? nullptr : it->second;
}

HttpServlet::Ptr HttpServletDispatch::findGlobServlet(const std::string &uri)
{
    MutexType::RdLock lock(m_mutex);
    for (auto it = m_glob_servlets.begin(); it != m_glob_servlets.end(); ++it)
    {
        if (uri.find(it->first) == 0)
        {
            return it->second;
        }
    }
    return nullptr;
}

HttpServlet::Ptr HttpServletDispatch::getDefaultServlet()
{
    MutexType::RdLock lock(m_mutex);
    return m_default_servlet;
}

void HttpServletDispatch::setDefaultServlet(const HttpServlet::Ptr &servlet)
{
    MutexType::WrLock lock(m_mutex);
    m_default_servlet = servlet;
}

HttpServlet::Ptr HttpServletDispatch::getServlet(const std::string &uri)
{
    MutexType::RdLock lock(m_mutex);
    auto it = m_servlets.find(uri);
    if (it != m_servlets.end())
    {
        return it->second;
    }
    for (auto it = m_glob_servlets.begin(); it != m_glob_servlets.end(); ++it)
    {
        if (util::globMatch(it->first.c_str(), uri.c_str()))
        {
            return it->second;
        }
    }
    return m_default_servlet;
}

HttpServlet404NotFound::HttpServlet404NotFound()
    : HttpServletFunction(
          [](const http::HttpRequest::Ptr &req, const http::HttpResponse::Ptr &res,
             const HttpSession::Ptr &session) -> int32_t {
              const std::string body =
                  "<html><head><title>404 Not Found</title></head><body><center><h1>404 Not "
                  "Found</h1></center><hr><center>LoNHttpfw/" LONETFW_VERSION
                  "</center></body></html>";
              res->setStatus(http::HttpStatus::NOT_FOUND);
              res->setHeader("Content-Type", "text/html");

              res->setBody(body);
              //   session->sendResponse(res);
              return 0;
          },
          "404")
{
}

HttpServlet404NotFound::~HttpServlet404NotFound() {}

} // namespace httpservice
} // namespace lon