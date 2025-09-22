#include "http/httpmessage.h"

namespace lon
{
namespace http
{
HttpMessage::HttpMessage(uint8_t version, bool close) : m_version(version), m_close(close) {}

HttpMessage::~HttpMessage() {}

uint8_t HttpMessage::getVersion() const { return m_version; }

bool HttpMessage::isClose() const { return m_close; }

const std::string &HttpMessage::getBody() const { return m_body; }

const HttpMessage::MapType &HttpMessage::getHeaders() const { return m_headers; }

const HttpMessage::MapType &HttpMessage::getCookies() const { return m_cookies; }

void HttpMessage::setVersion(uint8_t version) { m_version = version; }

void HttpMessage::setClose(bool close) { m_close = close; }

void HttpMessage::setBody(const std::string &body) { m_body = body; }

void HttpMessage::setHeaders(const MapType &headers) { m_headers = headers; }

void HttpMessage::setCookies(const MapType &cookies) { m_cookies = cookies; }

std::string HttpMessage::getHeader(const std::string &key, const std::string &default_value)
{
    auto it = m_headers.find(key);
    return it != m_headers.end() ? it->second : default_value;
}

std::string HttpMessage::getCookie(const std::string &key, const std::string &default_value)
{
    auto it = m_cookies.find(key);
    return it != m_cookies.end() ? it->second : default_value;
}

void HttpMessage::setHeader(const std::string &key, const std::string &value)
{
    m_headers[key] = value;
}

void HttpMessage::setCookie(const std::string &key, const std::string &value)
{
    m_cookies[key] = value;
}

bool HttpMessage::hasHeader(const std::string &key, std::string &value) const
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

bool HttpMessage::hasCookie(const std::string &key, std::string &value) const
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

void HttpMessage::delHeader(const std::string &key) { m_headers.erase(key); }

void HttpMessage::delCookie(const std::string &key) { m_cookies.erase(key); }

std::ostream &operator<<(std::ostream &os, const HttpMessage &msg) { return msg.toString(os); }
} // namespace http
} // namespace lon
