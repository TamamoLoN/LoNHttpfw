#include "httpservice/httpservlet.h"

namespace lon
{
namespace httpservice
{
static auto g_logger = LON_LOG_ROOT;

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

HttpServlet404NotFound::HttpServlet404NotFound() : HttpServlet("404") {}

HttpServlet404NotFound::~HttpServlet404NotFound() {}

int32_t HttpServlet404NotFound::handle(const http::HttpRequest::Ptr &req,
                                       const http::HttpResponse::Ptr &res,
                                       const HttpSession::Ptr &session)
{
    const std::string body =
        "<html><head><title>404 Not Found</title></head><body><center><h1>404 Not "
        "Found</h1></center><hr><center>LoNHttpfw/" LONETFW_VERSION "</center></body></html>";
    res->setStatus(http::HttpStatus::NOT_FOUND);
    res->setHeader("Content-Type", "text/html");

    res->setBody(body);
    //   session->sendResponse(res);
    return 0;
}

HttpServletDownload::RangeParser::RangeParser(const std::string &header_val) : m_val(header_val) {}

HttpServletDownload::RangeParser::~RangeParser() {}

HttpServletDownload::RangeParser::RangeResultVec HttpServletDownload::RangeParser::parse()
{
    RangeResultVec result{};
    if (m_val.find("bytes=") == std::string::npos)
    {
        return result;
    }
    auto ranges     = m_val.substr(m_val.find("=") + 1);
    auto range_list = util::split(ranges, ",");
    for (auto &range : range_list)
    {
        try
        {
            range = util::trim(range);
            if (range.size() <= 1)
            {
                continue;
            }
            if (range.find("-") != std::string::npos)
            {
                if (range[0] == '-')
                {
                    result.push_back(
                        RangeResult{RangeType::RANGE_FROM_END, {std::stoul(range.substr(1))}});
                }
                else if (range[range.size() - 1] == '-')
                {
                    result.push_back(RangeResult{RangeType::RANGE_FROM_START,
                                                 {
                                                     std::stoul(range.substr(0, range.size() - 1)),
                                                 }});
                }
                else
                {
                    auto range_pair = util::split(range, "-");
                    if (range_pair.size() == 2)
                    {
                        result.push_back(
                            RangeResult{RangeType::RANGE_NORMAL,
                                        {std::stoul(range_pair[0]), std::stoul(range_pair[1])}});
                    }
                }
            }
        }
        catch (...)
        {
            continue;
        }
    }
    return result;
}

HttpServletDownload::HttpServletDownload(bool enable_range, const std::string &boundary)
    : HttpServlet("download"), m_enable_range(enable_range), m_boundary(boundary)
{
}

HttpServletDownload::~HttpServletDownload() {}

int32_t HttpServletDownload::handle(const http::HttpRequest::Ptr &req,
                                    const http::HttpResponse::Ptr &res,
                                    const HttpSession::Ptr &session)
{
    auto path         = getFilePath(req->getPath());
    auto range_header = req->getHeader(util::toLower("Range"));
    std::stringstream body;
    bool is_multipart = false;
    if (!util::FSUtil::isFileExist(path))
    {
        LON_ERROR(g_logger) << "HttpServletDownload::handle: file not exist, path: " + path;
        return -1;
    }
    if (range_header.empty())
    {
        range_header = req->getHeader(util::toLower("Content-Range"));
    }
    if (!range_header.empty())
    {
        if (m_enable_range)
        {
            auto ranges = RangeParser(range_header).parse();
            if (ranges.empty())
            {
            range_invalid:
                res->setStatus(http::HttpStatus::RANGE_NOT_SATISFIABLE);
                return 0;
            }

            std::ifstream in(path, std::ios::binary);
            if (!in.is_open())
            {
                LON_ERROR(g_logger)
                    << "HttpServletDownload::handle: open file failed, path: " + path;
                return -1;
            }

            auto file_size = util::FSUtil::getFileSize(path);
            std::stringstream body;
            bool is_multipart = ranges.size() > 1;

            if (is_multipart)
            {
                res->setHeader("Content-Type", "multipart/byteranges; boundary=" + m_boundary);
            }
            else
            {
                res->setHeader("Content-Type", "application/octet-stream");
            }

            for (size_t i = 0; i < ranges.size(); ++i)
            {
                const auto &range = ranges[i];

                size_t start = 0, end = 0;
                switch (range.type)
                {
                case RangeParser::RangeType::RANGE_NORMAL:
                    start = range.ranges[0];
                    end   = range.ranges[1];
                    if (start > end || end >= file_size)
                        goto range_invalid;
                    break;

                case RangeParser::RangeType::RANGE_FROM_START:
                    start = range.ranges[0];
                    end   = file_size - 1;
                    if (start >= file_size)
                        goto range_invalid;
                    break;

                case RangeParser::RangeType::RANGE_FROM_END:
                    if (range.ranges[0] >= file_size)
                        goto range_invalid;
                    start = file_size - range.ranges[0];
                    end   = file_size - 1;
                    break;

                default:
                    goto range_invalid;
                }

                size_t length = end - start + 1;
                std::shared_ptr<char> buf(new char[4096], [](char *p) { delete[] p; });

                if (is_multipart)
                {
                    body << "--" << m_boundary << "\r\n";
                    body << "Content-Type: application/octet-stream\r\n";
                    body << "Content-Range: bytes " << start << "-" << end << "/" << file_size
                         << "\r\n\r\n";
                }
                else
                {
                    res->setHeader("Content-Range", "bytes " + std::to_string(start) + "-" +
                                                        std::to_string(end) + "/" +
                                                        std::to_string(file_size));
                }

                in.clear();
                in.seekg(start);
                size_t remaining = length;
                while (remaining > 0 && in.good())
                {
                    size_t to_read = std::min<size_t>(remaining, 4096);
                    in.read(buf.get(), to_read);
                    std::streamsize read_size = in.gcount();
                    body.write(buf.get(), read_size);
                    remaining -= read_size;
                }

                if (is_multipart)
                    body << "\r\n";
            }

            if (is_multipart)
            {
                body << "--" << m_boundary << "--\r\n";
            }

            res->setStatus(http::HttpStatus::PARTIAL_CONTENT);
            res->setBody(body.str());
        }
        else
        {
            goto no_range;
        }
    }
    else
    {
    no_range:
        res->setStatus(http::HttpStatus::OK);
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open())
        {
            LON_ERROR(g_logger) << "HttpServletDownload::handle: open file failed, path: " + path;
            return -1;
        }
        std::ostringstream content;
        content << in.rdbuf();
        res->setStatus(http::HttpStatus::OK);
        res->setHeader("Content-Type", "application/octet-stream");
        res->setBody(content.str());
    }
    return 0;
}

std::string HttpServletDownload::getFilePath(const std::string &path) const
{
    if (path.find("/download/") == std::string::npos)
    {
        return "";
    }
    return path.substr(strlen("/download/"));
}

} // namespace httpservice
} // namespace lon