#pragma once

#include "http/httpmethod.h"
#include "http/httpstatus.h"
#include "util/util.h"

namespace lon
{
namespace http
{
class HttpMessage
{
  public:
    using Ptr     = std::shared_ptr<HttpMessage>;
    using MapType = std::map<std::string, std::string, util::InsensitiveStringCompare>;
    HttpMessage(uint8_t version = 0x11, bool close = true);
    ~HttpMessage();

    uint8_t getVersion() const;
    bool isClose() const;
    const std::string &getBody() const;
    const MapType &getHeaders() const;
    const MapType &getCookies() const;
    const bool isWebsocket() const;

    void setVersion(uint8_t version);
    void setClose(bool close);
    void setBody(const std::string &body);
    void setHeaders(const MapType &headers);
    void setCookies(const MapType &cookies);
    void setWebsocket(bool is_websocket);

    std::string getHeader(const std::string &key, const std::string &default_value = "");
    std::string getCookie(const std::string &key, const std::string &default_value = "");

    void setHeader(const std::string &key, const std::string &value);
    void setCookie(const std::string &key, const std::string &value);

    bool hasHeader(const std::string &key, std::string &value) const;
    bool hasCookie(const std::string &key, std::string &value) const;

    void delHeader(const std::string &key);
    void delCookie(const std::string &key);

    virtual std::ostream &toString(std::ostream &os) const = 0;
    virtual std::string toString() const                   = 0;
    friend std::ostream &operator<<(std::ostream &os, const HttpMessage &msg);

  public:
    template <typename T>
    bool getCheckHeader(const std::string &key, T &out, const T &default_value = T())
    {
        return getCheckHelper(m_headers, key, out, default_value);
    }

    template <typename T> T getHeader(const std::string &key, const T &default_value = T())
    {
        return getHelper(m_headers, key, default_value);
    }

    template <typename T>
    bool getCheckCookie(const std::string &key, T &out, const T &default_value = T())
    {
        return getCheckHelper(m_cookies, key, out, default_value);
    }

    template <typename T> T getCookie(const std::string &key, const T &default_value = T())
    {
        return getHelper(m_cookies, key, default_value);
    }

  protected:
    template <typename T>
    bool getCheckHelper(const MapType &map, const std::string &key, T &out,
                        const T &default_value = T())
    {
        auto it = map.find(key);
        if (it != map.end())
        {
            try
            {
                out = util::lexical_cast<T>(it->second);
                return true;
            }
            catch (...)
            {
                out = default_value;
                return false;
            }
        }
        out = default_value;
        return false;
    }

    template <typename T>
    T getHelper(const MapType &map, const std::string &key, const T &default_value = T())
    {
        auto it = map.find(key);
        if (it != map.end())
        {
            try
            {
                return util::lexical_cast<T>(it->second);
            }
            catch (...)
            {
                return default_value;
            }
        }
        return default_value;
    }

  protected:
    uint8_t m_version;
    bool m_close;
    bool m_is_websocket;
    std::string m_body;
    MapType m_headers;
    MapType m_cookies;
};

} // namespace http
} // namespace lon