#include "httpserver/httpservlet.h"

namespace lon
{
namespace httpserver
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
        if (uri.find(it->first) == 0)
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
              const std::string body = R"html(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>404 — Not Found</title>
  <style>
    :root{
      --bg:#0f1724;
      --card:#0b1220;
      --accent:#60a5fa;
      --muted:#94a3b8;
      --glass: rgba(255,255,255,0.04);
    }
    *{box-sizing:border-box}
    html,body{height:100%}
    body{
      margin:0; font-family:Inter,ui-sans-serif,system-ui,-apple-system,"Segoe UI",Roboto,"Helvetica Neue",Arial;
      background: radial-gradient(1200px 600px at 10% 20%, rgba(96,165,250,0.08), transparent),
                  radial-gradient(900px 500px at 90% 80%, rgba(99,102,241,0.06), transparent),
                  var(--bg);
      color:#e6eef8;
      -webkit-font-smoothing:antialiased;
      display:flex;align-items:center;justify-content:center;padding:32px;
    }
    .card{
      width:100%; max-width:980px; background:linear-gradient(180deg, rgba(255,255,255,0.02), rgba(255,255,255,0.01));
      border-radius:18px; padding:36px; box-shadow: 0 8px 30px rgba(2,6,23,0.6); display:grid; grid-template-columns: 1fr 420px; gap:28px; align-items:center;
      backdrop-filter: blur(6px);
    }
    .illus{display:flex;align-items:center;justify-content:center}
    .plate{
      width:360px; height:360px; border-radius:14px; background:linear-gradient(180deg, rgba(11,18,32,0.6), rgba(255,255,255,0.02));
      display:flex;align-items:center;justify-content:center; box-shadow: inset 0 -6px 30px rgba(0,0,0,0.5), 0 8px 30px rgba(2,6,23,0.6);
      position:relative; overflow:hidden;
    }
    svg{width:70%; height:70%;}
    .text{
      padding:6px 12px; border-radius:10px; background:var(--glass); color:var(--muted); font-size:13px; position:absolute; left:16px; bottom:16px;
    }

    h1{margin:0;font-size:88px;line-height:0.9;letter-spacing:-2px;color:var(--accent);}
    h2{margin:6px 0 0 0;font-size:20px;color:#d6e7ff}
    p{color:var(--muted); margin-top:12px}
    .actions{margin-top:20px; display:flex; gap:12px; align-items:center}
    .btn{
      display:inline-flex; align-items:center; gap:10px; padding:10px 16px; border-radius:12px; border:1px solid rgba(255,255,255,0.06);
      background: linear-gradient(180deg, rgba(255,255,255,0.02), rgba(255,255,255,0.01)); color:inherit; text-decoration:none; font-weight:600; cursor:pointer;
      transition: transform .12s ease, box-shadow .12s ease;
    }
    .btn:active{transform:translateY(1px)}
    .btn-primary{box-shadow: 0 6px 20px rgba(96,165,250,0.12); border-color: rgba(96,165,250,0.18)}
    .search{display:flex; gap:8px; align-items:center; background:rgba(255,255,255,0.02); padding:8px; border-radius:12px; border:1px solid rgba(255,255,255,0.03)}
    .search input{background:transparent; border:0; outline:none; color:inherit; font-size:14px}
    .meta{font-size:13px; color:var(--muted); margin-top:14px}

    @media (max-width:880px){
      .card{grid-template-columns: 1fr; padding:22px}
      h1{font-size:64px}
      .plate{width:260px;height:260px}
    }

    /* small playful animation */
    .robot-arm{transform-origin:50% 50%; animation: wave 3s ease-in-out infinite}
    @keyframes wave{ 0%,100%{transform:rotate(-6deg)} 50%{transform:rotate(6deg)} }
  </style>
</head>
<body>
  <main class="card" role="main" aria-labelledby="title">
    <section>
      <h1 id="title">404</h1>
      <h2>抱歉，页面找不到了（Not Found）</h2>
      <p>你访问的页面可能已被删除、重命名，或暂时不可用。试试下面的操作：</p>

      <div class="actions">
        <a class="btn btn-primary" href="/">🏠 返回首页</a>
        <a class="btn" href="/sitemap">🧭 网站地图</a>
        <div class="search" role="search">
          <input type="search" placeholder="在本站搜索..." aria-label="search" id="q">
          <button class="btn" id="btnSearch">🔎</button>
        </div>
      </div>

      <div class="meta">如果问题仍然存在，请联系管理员或稍后重试。</div>
    </section>

    <aside class="illus" aria-hidden="true">
      <div class="plate">
        <!-- playful 404 illustration (SVG) -->
        <svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg" role="img" aria-label="broken-robot">
          <defs>
            <linearGradient id="g1" x1="0" x2="1">
              <stop offset="0" stop-color="#60a5fa" stop-opacity="0.9"/>
              <stop offset="1" stop-color="#7c3aed" stop-opacity="0.9"/>
            </linearGradient>
          </defs>
          <g transform="translate(100,100)">
            <circle r="70" fill="url(#g1)" opacity="0.12" />
            <!-- robot body -->
            <rect x="-38" y="-18" width="76" height="56" rx="8" fill="#071126" stroke="#13304a" stroke-width="1"/>
            <!-- eye -->
            <g transform="translate(-10,-2)">
              <circle r="8" fill="#051425" stroke="#9fbbe9" stroke-width="2"/>
              <circle r="3.5" fill="#9fbbe9" transform="translate(0,0)"/>
            </g>
            <g transform="translate(22,-2)">
              <circle r="8" fill="#051425" stroke="#9fbbe9" stroke-width="2"/>
              <circle r="3.5" fill="#9fbbe9"/>
            </g>
            <!-- mouth (broken) -->
            <rect x="-20" y="16" width="40" height="6" rx="3" fill="#0b1220" stroke="#ffd1d1" stroke-width="0.8"/>

            <!-- left arm -->
            <g transform="translate(-46,6)">
              <rect x="-6" y="-4" width="12" height="36" rx="4" fill="#071126" stroke="#13304a"/>
              <g class="robot-arm" transform="translate(-1,22) rotate(-6)">
                <rect x="-2" y="0" width="10" height="18" rx="3" fill="#0b1220"/>
              </g>
            </g>

            <!-- right arm (detached) -->
            <g transform="translate(46,-8)">
              <rect x="-6" y="-4" width="12" height="18" rx="4" fill="#071126" stroke="#13304a"/>
              <circle cx="10" cy="18" r="8" fill="#0b1220" stroke="#ffd1d1" stroke-width="0.9"/>
            </g>

            <!-- tiny glitch squares -->
            <rect x="-70" y="-70" width="8" height="8" fill="#7c3aed" opacity="0.6"/>
            <rect x="58" y="-60" width="6" height="6" fill="#60a5fa" opacity="0.7"/>
            <rect x="-10" y="72" width="10" height="10" fill="#60a5fa" opacity="0.12"/>

            <text x="-36" y="92" font-family="Inter, sans-serif" font-size="8" fill="#a7c2e8">Error • 404</text>
          </g>
        </svg>

        <div class="text">Try / or search — or report this to support</div>
      </div>
    </aside>
  </main>

  <script>
    // simple client-side search demo
    document.getElementById('btnSearch').addEventListener('click', function(){
      var q = document.getElementById('q').value.trim();
      if(!q) return alert('请输入搜索关键字');
      // redirect to site search (adjust path as needed)
      window.location.href = '/search?q=' + encodeURIComponent(q);
    });
    // allow Enter
    document.getElementById('q').addEventListener('keydown', function(e){ if(e.key === 'Enter') document.getElementById('btnSearch').click(); });
  </script>
</body>
</html>

)html";
              //   const std::string body =
              //       "<html><head><title>404 Not Found</title></head><body><center><h1>404 Not "
              //       "Found</h1></center><hr><center>LoNHttpfw/" LONETFW_VERSION
              //       "</center></body></html>";
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

} // namespace httpserver
} // namespace lon