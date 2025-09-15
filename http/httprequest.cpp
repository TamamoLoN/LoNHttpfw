#include "http/httprequest.h"

namespace lon
{
namespace http
{
HttpRequest::HttpRequest(uint8_t http_version, bool close)
    : m_version(http_version), m_close(close), m_method(HttpMethod::UNKNOWN)
{
}

HttpRequest::~HttpRequest() {}

HttpMethod HttpRequest::getMethod() const { return m_method; }

HttpStatus HttpRequest::getStatus() const { return m_status; }

uint8_t HttpRequest::getVersion() const { return m_version; }

bool HttpRequest::isClose() const { return m_close; }

const std::string &HttpRequest::getPath() const { return m_path; }

const std::string &HttpRequest::getQuery() const { return m_query; }

const std::string &HttpRequest::getFragment() const { return m_fragment; }

const std::string &HttpRequest::getBody() const { return m_body; }

const HttpRequest::MapType &HttpRequest::getHeaders() const { return m_headers; }

const HttpRequest::MapType &HttpRequest::getParams() const { return m_params; }

const HttpRequest::MapType &HttpRequest::getCookies() const { return m_cookies; }

void HttpRequest::setMethod(HttpMethod method) { m_method = method; }

void HttpRequest::setStatus(HttpStatus status) { m_status = status; }

void HttpRequest::setVersion(uint8_t version) { m_version = version; }

void HttpRequest::setClose(bool close) { m_close = close; }

void HttpRequest::setPath(const std::string &path) { m_path = path; }

void HttpRequest::setQuery(const std::string &query) { m_query = query; }

void HttpRequest::setFragment(const std::string &fragment) { m_fragment = fragment; }

void HttpRequest::setBody(const std::string &body) { m_body = body; }

void HttpRequest::setHeaders(const MapType &headers) { m_headers = headers; }

void HttpRequest::setParams(const MapType &params) { m_params = params; }

void HttpRequest::setCookies(const MapType &cookies) { m_cookies = cookies; }

std::string HttpRequest::getHeader(const std::string &key, const std::string &default_value)
{
    auto it = m_headers.find(key);
    return it != m_headers.end() ? it->second : default_value;
}

std::string HttpRequest::getParam(const std::string &key, const std::string &default_value)
{
    auto it = m_params.find(key);
    return it != m_params.end() ? it->second : default_value;
}

std::string HttpRequest::getCookie(const std::string &key, const std::string &default_value)
{
    auto it = m_cookies.find(key);
    return it != m_cookies.end() ? it->second : default_value;
}

void HttpRequest::setHeader(const std::string &key, const std::string &value)
{
    m_headers[key] = value;
}

void HttpRequest::setParam(const std::string &key, const std::string &value)
{
    m_params[key] = value;
}

void HttpRequest::setCookie(const std::string &key, const std::string &value)
{
    m_cookies[key] = value;
}

bool HttpRequest::hasHeader(const std::string &key, std::string &value) const
{
    auto it = m_headers.find(key);
    if (it != m_headers.end())
    {
        value = it->second;
        return true;
    }
    value.clear();
    return false;
}

bool HttpRequest::hasParam(const std::string &key, std::string &value) const
{
    auto it = m_params.find(key);
    if (it != m_params.end())
    {
        value = it->second;
        return true;
    }
    value.clear();
    return false;
}

bool HttpRequest::hasCookie(const std::string &key, std::string &value) const
{
    auto it = m_cookies.find(key);
    if (it != m_cookies.end())
    {
        value = it->second;
        return true;
    }
    value.clear();
    return false;
}

void HttpRequest::delHeader(const std::string &key) { m_headers.erase(key); }

void HttpRequest::delParam(const std::string &key) { m_params.erase(key); }

void HttpRequest::delCookie(const std::string &key) { m_cookies.erase(key); }

std::ostream &HttpRequest::toString(std::ostream &os) const
{
    os << HttpMethodConverter::toString(m_method) << " " << m_path << (m_query.empty() ? "" : "?")
       << m_query << (m_query.empty() ? "" : "#") << m_fragment << " HTTP/"
       << (uint32_t)(m_version >> 4) << "." << (uint32_t)(m_version & 0xF) << "\r\n";
    os << "Connection: " << (m_close ? "Close" : "Keep-Alive") << "\r\n";
    for (const auto &header : m_headers)
    {
        if (util::toLower(header.first) == "connection")
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
    os << "\r\n";

    return os;
}

std::string HttpRequest::toString() const
{
    std::stringstream ss;
    toString(ss);
    return ss.str();
}

} // namespace http
} // namespace lon