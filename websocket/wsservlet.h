#pragma once

#include "httpservice/httpservlet.h"
#include "websocket/wssession.h"

namespace lon
{
namespace ws
{
class WSServlet : public httpservice::HttpServlet
{
  public:
    using Ptr = std::shared_ptr<WSServlet>;
    WSServlet(const std::string &name);
    virtual ~WSServlet();

    virtual int32_t handle(const http::HttpRequest::Ptr &req, const http::HttpResponse::Ptr &res,
                           const httpservice::HttpSession::Ptr &session) override;

    virtual int32_t onConnect(const http::HttpRequest::Ptr &req, const WSSession::Ptr &session) = 0;
    virtual int32_t onClose(const http::HttpRequest::Ptr &req, const WSSession::Ptr &session)   = 0;
    virtual int32_t handle(const http::HttpRequest::Ptr &req, const WSFrameMessage::Ptr &msg,
                           const WSSession::Ptr &session)                                       = 0;
};

class WSServletFunction : public WSServlet
{
  public:
    using Ptr = std::shared_ptr<WSServletFunction>;
    using callback =
        std::function<int32_t(const http::HttpRequest::Ptr &req, const WSFrameMessage::Ptr &msg,
                              const WSSession::Ptr &session)>;
    using on_connect_cb =
        std::function<int32_t(const http::HttpRequest::Ptr &req, const WSSession::Ptr &session)>;
    using on_close_cb = on_connect_cb;
    WSServletFunction(callback cb, on_connect_cb connect_cb = nullptr,
                      on_close_cb close_cb = nullptr, const std::string &name = "wsfunction");
    virtual ~WSServletFunction();

    virtual int32_t onConnect(const http::HttpRequest::Ptr &req,
                              const WSSession::Ptr &session) override;
    virtual int32_t onClose(const http::HttpRequest::Ptr &req,
                            const WSSession::Ptr &session) override;
    virtual int32_t handle(const http::HttpRequest::Ptr &req, const WSFrameMessage::Ptr &msg,
                           const WSSession::Ptr &session) override;

  protected:
    callback m_cb;
    on_connect_cb m_connect_cb;
    on_close_cb m_close_cb;
};

class WSServletDispatch : public httpservice::HttpServletDispatch
{
  public:
    using Ptr = std::shared_ptr<WSServletDispatch>;
    WSServletDispatch(const std::string &name = "wsdispatch");
    virtual ~WSServletDispatch();

    void addServlet(const std::string &uri, WSServletFunction::callback cb,
                    WSServletFunction::on_connect_cb connect_cb = nullptr,
                    WSServletFunction::on_close_cb close_cb     = nullptr);
    void addGlobServlet(const std::string &uri, WSServletFunction::callback cb,
                        WSServletFunction::on_connect_cb connect_cb = nullptr,
                        WSServletFunction::on_close_cb close_cb     = nullptr);
    WSServlet::Ptr getWSServlet(const std::string &uri);
};
} // namespace ws
} // namespace lon