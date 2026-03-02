#include "websocket/wsservlet.h"

namespace lon
{
namespace ws
{
WSServlet::WSServlet(const std::string &name) : httpservice::HttpServlet(name) {}

WSServlet::~WSServlet() {}

int32_t WSServlet::handle(const http::HttpRequest::Ptr &req, const http::HttpResponse::Ptr &res,
                          const httpservice::HttpSession::Ptr &session)
{
    return 0;
}

WSServletFunction::WSServletFunction(callback cb, on_connect_cb connect_cb, on_close_cb close_cb,
                                     const std::string &name)
    : WSServlet(name), m_cb(std::move(cb)), m_connect_cb(std::move(connect_cb)),
      m_close_cb(std::move(close_cb))
{
}

WSServletFunction::~WSServletFunction() {}

int32_t WSServletFunction::onConnect(const http::HttpRequest::Ptr &req,
                                     const WSSession::Ptr &session)
{
    if (m_connect_cb)
    {
        return m_connect_cb(req, session);
    }
    return 0;
}

int32_t WSServletFunction::onClose(const http::HttpRequest::Ptr &req, const WSSession::Ptr &session)
{
    if (m_close_cb)
    {
        return m_close_cb(req, session);
    }
    return 0;
}

int32_t WSServletFunction::handle(const http::HttpRequest::Ptr &req, const WSFrameMessage::Ptr &msg,
                                  const WSSession::Ptr &session)
{
    if (m_cb)
    {
        return m_cb(req, msg, session);
    }
    return 0;
}

WSServletDispatch::WSServletDispatch(const std::string &name) : HttpServletDispatch(name) {}

WSServletDispatch::~WSServletDispatch() {}

void WSServletDispatch::addServlet(const std::string &uri, WSServletFunction::callback cb,
                                   WSServletFunction::on_connect_cb connect_cb,
                                   WSServletFunction::on_close_cb close_cb)
{
    HttpServletDispatch::addServlet(uri, std::make_shared<WSServletFunction>(std::move(cb),
                                                                             std::move(connect_cb),
                                                                             std::move(close_cb)));
}

void WSServletDispatch::addGlobServlet(const std::string &uri, WSServletFunction::callback cb,
                                       WSServletFunction::on_connect_cb connect_cb,
                                       WSServletFunction::on_close_cb close_cb)
{
    HttpServletDispatch::addGlobServlet(
        uri, std::make_shared<WSServletFunction>(std::move(cb), std::move(connect_cb),
                                                 std::move(close_cb)));
}

WSServlet::Ptr WSServletDispatch::getWSServlet(const std::string &uri)
{
    return std::dynamic_pointer_cast<WSServlet>(HttpServletDispatch::getServlet(uri));
}
} // namespace ws
} // namespace lon