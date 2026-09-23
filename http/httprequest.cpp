#include "http/httprequest.h"

namespace lon
{
namespace http
{
HttpRequest::HttpRequest(uint8_t http_version, bool close)
    : HttpMessage(http_version, close), m_method(HttpMethod::GET), m_path("/"),
      m_parser_param_flag(0)
{
}

HttpRequest::~HttpRequest() {}

HttpMethod HttpRequest::getMethod() const { return m_method; }

const std::string &HttpRequest::getPath() const { return m_path; }

const std::string &HttpRequest::getQuery() const { return m_query; }

const std::string &HttpRequest::getFragment() const { return m_fragment; }

const HttpRequest::MapType &HttpRequest::getParams() const { return m_params; }

void HttpRequest::setMethod(HttpMethod method) { m_method = method; }

void HttpRequest::setPath(const std::string &path) { m_path = path; }

void HttpRequest::setQuery(const std::string &query) { m_query = query; }

void HttpRequest::setFragment(const std::string &fragment) { m_fragment = fragment; }

void HttpRequest::setParams(const MapType &params) { m_params = params; }

std::string HttpRequest::getParam(const std::string &key, const std::string &default_value)
{
    initQueryParam();
    initBodyParam();
    auto it = m_params.find(key);
    return it != m_params.end() ? it->second : default_value;
}

void HttpRequest::setParam(const std::string &key, const std::string &value)
{
    m_params[key] = value;
}

bool HttpRequest::hasParam(const std::string &key, std::string &value)
{
    initQueryParam();
    initBodyParam();
    auto it = m_params.find(key);
    if (it != m_params.end())
    {
        value = it->second;
        return true;
    }
    value.clear();
    return false;
}

void HttpRequest::delParam(const std::string &key) { m_params.erase(key); }

std::string HttpRequest::getCookie(const std::string &key, const std::string &default_value)
{
    initCookies();
    auto it = m_cookies.find(key);
    return it != m_cookies.end() ? it->second : default_value;
}

bool HttpRequest::hasCookie(const std::string &key, std::string &value)
{
    initCookies();
    auto it = m_cookies.find(key);
    if (it != m_cookies.end())
    {
        value = it->second;
        return true;
    }
    value.clear();
    return false;
}

std::ostream &HttpRequest::toString(std::ostream &os) const
{
    os << HttpMethodConverter::toString(m_method) << " " << (m_path.empty() ? "/" : m_path)
       << (m_query.empty() ? "" : "?") << m_query << (m_fragment.empty() ? "" : "#") << m_fragment
       << " HTTP/" << (uint32_t)(m_version >> 4) << "." << (uint32_t)(m_version & 0xF) << "\r\n";
    if (!m_is_websocket)
    {
        os << "Connection: " << (m_close ? "Close" : "Keep-Alive") << "\r\n";
    }
    for (const auto &header : m_headers)
    {
        if (!m_is_websocket && util::toLower(header.first) == "connection")
        {
            continue;
        }
        os << header.first << ": " << header.second << "\r\n";
    }
    if (!m_body.empty())
    {
        os << "Content-Length: " << m_body.size() << "\r\n\r\n";
        os << m_body;
    }
    else
    {
        os << "\r\n";
    }

    return os;
}

std::string HttpRequest::toString() const
{
    std::stringstream ss;
    toString(ss);
    return ss.str();
}

void HttpRequest::initParam()
{
    initQueryParam();
    initBodyParam();
    initCookies();
}

void HttpRequest::initQueryParam()
{
    if (m_parser_param_flag & 0x1)
    {
        return;
    }

#define PARSE_PARAM(str, m, flag)                                                                  \
    size_t pos = 0;                                                                                \
    do                                                                                             \
    {                                                                                              \
        size_t last = pos;                                                                         \
        pos         = str.find('=', pos);                                                          \
        if (pos == std::string::npos)                                                              \
        {                                                                                          \
            break;                                                                                 \
        }                                                                                          \
        size_t key = pos;                                                                          \
        pos        = str.find(flag, pos);                                                          \
        m.insert(                                                                                  \
            std::make_pair(str.substr(last, key - last),                                           \
                           util::urlDecode(str.substr(key + 1, pos - key - 1))));     \
        if (pos == std::string::npos)                                                              \
        {                                                                                          \
            break;                                                                                 \
        }                                                                                          \
        ++pos;                                                                                     \
    } while (true);

    PARSE_PARAM(m_query, m_params, '&');
    m_parser_param_flag |= 0x1;
}

void HttpRequest::initBodyParam()
{
    if (m_parser_param_flag & 0x2)
    {
        return;
    }
    std::string content_type = getHeader("content-type");
    if (util::toLower(content_type) == "application/x-www-form-urlencoded")
    {
        m_parser_param_flag |= 0x2;
        return;
    }
    PARSE_PARAM(m_body, m_params, '&');
    m_parser_param_flag |= 0x2;
}

void HttpRequest::initCookies()
{
    if (m_parser_param_flag & 0x4)
    {
        return;
    }
    std::string cookie = getHeader("cookie");
    if (cookie.empty())
    {
        m_parser_param_flag |= 0x4;
        return;
    }
    PARSE_PARAM(cookie, m_cookies, ';');
    m_parser_param_flag |= 0x4;
}

} // namespace http
} // namespace lon