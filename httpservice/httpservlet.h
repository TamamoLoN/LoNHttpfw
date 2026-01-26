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

class HttpServlet404NotFound : public HttpServlet
{
  public:
    using Ptr = std::shared_ptr<HttpServlet404NotFound>;
    HttpServlet404NotFound();
    virtual ~HttpServlet404NotFound();
    int32_t handle(const http::HttpRequest::Ptr &req, const http::HttpResponse::Ptr &res,
                   const HttpSession::Ptr &session) override;
};

class HttpServletDownload : public HttpServlet
{
  public:
    /**
     * 处理Content-Range: bytes=start-end, bytes=-count, bytes=start- 或
     * bytes=start-end,200-299的请求;
     * 以及Range: bytes=start-end, bytes=-count, bytes=start- 或
     * bytes=start-end,200-299的请求
     */
    class RangeParser
    {
      public:
        enum class RangeType
        {
            RANGE_INVALID = 0, // 格式错误或超出范围
            RANGE_NORMAL,      // bytes=start-end
            RANGE_FROM_START,  // bytes=start-
            RANGE_FROM_END,    // bytes=-count
        };
        struct RangeResult
        {
            RangeType type;
            // bytes=start-end ranges[0] = start, ranges[1] = end
            // bytes=-count ranges[0] = -1, ranges[1] = count
            // bytes=start- ranges[0] = start, ranges[1] = -1
            std::vector<size_t> ranges;
        };

      public:
        using RangeResultVec = std::vector<RangeResult>;
        RangeParser(const std::string &header_val);
        ~RangeParser();
        RangeResultVec parse();

      private:
        std::string m_val;
    };
    using Ptr = std::shared_ptr<HttpServletDownload>;
    HttpServletDownload(bool enable_range = true, const std::string &boundary = "boundary");
    virtual ~HttpServletDownload();
    int32_t handle(const http::HttpRequest::Ptr &req, const http::HttpResponse::Ptr &res,
                   const HttpSession::Ptr &session) override;

  private:
    std::string getFilePath(const std::string &path) const;

  private:
    std::string m_boundary;
    bool m_enable_range;
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