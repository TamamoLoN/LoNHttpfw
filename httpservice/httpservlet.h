#pragma once

#include "http/httprequest.h"
#include "http/httpresponse.h"
#include "httpservice/httpsession.h"
#include "thread/mutex.h"

namespace lon
{
namespace httpservice
{
class HttpServlet
{
  public:
    using Ptr = std::shared_ptr<HttpServlet>;
    HttpServlet(const std::string &name);
    virtual ~HttpServlet();

    virtual int32_t handle(const http::HttpRequest::Ptr &req, const http::HttpResponse::Ptr &res,
                           const HttpSession::Ptr &session) = 0;
    const std::string &getName() const;

  protected:
    std::string m_name;
};

class HttpServletFunction : public HttpServlet
{
  public:
    using Ptr = std::shared_ptr<HttpServletFunction>;
    using callback =
        std::function<int32_t(const http::HttpRequest::Ptr &req, const http::HttpResponse::Ptr &res,
                              const HttpSession::Ptr &session)>;
    HttpServletFunction(callback cb, const std::string &name = "function");
    virtual ~HttpServletFunction();

    virtual int32_t handle(const http::HttpRequest::Ptr &req, const http::HttpResponse::Ptr &res,
                           const HttpSession::Ptr &session) override;

  private:
    callback m_cb;
};

class HttpServlet404NotFound : public HttpServletFunction
{
  public:
    using Ptr = std::shared_ptr<HttpServlet404NotFound>;
    HttpServlet404NotFound();
    virtual ~HttpServlet404NotFound();
};

class HttpServletDispatch : public HttpServlet
{
  public:
    using Ptr       = std::shared_ptr<HttpServletDispatch>;
    using MutexType = thread::RWMutex;
    HttpServletDispatch(const std::string &name = "dispatch");
    virtual ~HttpServletDispatch();
    virtual int32_t handle(const http::HttpRequest::Ptr &req, const http::HttpResponse::Ptr &res,
                           const HttpSession::Ptr &session) override;

    void addServlet(const std::string &uri, const HttpServlet::Ptr &servlet);
    void addServlet(const std::string &uri, HttpServletFunction::callback cb);
    void addGlobServlet(const std::string &uri, const HttpServlet::Ptr &servlet);
    void addGlobServlet(const std::string &uri, HttpServletFunction::callback cb);

    void delServlet(const std::string &uri);
    void delGlobServlet(const std::string &uri);

    HttpServlet::Ptr findServlet(const std::string &uri);
    HttpServlet::Ptr findGlobServlet(const std::string &uri);

    HttpServlet::Ptr getDefaultServlet();
    void setDefaultServlet(const HttpServlet::Ptr &servlet);

    /**
     * @brief 核心API, 获取经过融合逻辑后最终的servlet
     * @param uri
     * @return HttpServlet::Ptr 返回最终的servlet，如果没有匹配到则为default
     */
    HttpServlet::Ptr getServlet(const std::string &uri);

  private:
    // uri(/xxx/xxx) -> servlet 精准匹配
    std::unordered_map<std::string, HttpServlet::Ptr> m_servlets;
    // uri(/xxx/*) -> servlet 模糊匹配
    std::vector<std::pair<std::string, HttpServlet::Ptr>> m_glob_servlets;
    // 默认servlet，所有路径都没匹配到时使用
    HttpServlet::Ptr m_default_servlet;

    MutexType m_mutex;
};

} // namespace httpservice
} // namespace lon