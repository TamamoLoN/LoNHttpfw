#pragma once

#include "http/httpmethod.h"
#include "http/httpstatus.h"
#include "util/util.h"

namespace lon
{
namespace http
{
class HttpRequest
{
  public:
    using Ptr     = std::shared_ptr<HttpRequest>;
    using MapType = std::map<std::string, std::string, util::InsensitiveStringCompare>;
    // 0x10: HTTP/1.0 0x11: HTTP/1.1
    HttpRequest(uint8_t http_version = 0x11, bool close = true);
    ~HttpRequest();

    HttpMethod getMethod() const;
    HttpStatus getStatus() const;
    uint8_t getVersion() const;
    bool isClose() const;
    const std::string &getPath() const;
    const std::string &getQuery() const;
    const std::string &getFragment() const;
    const std::string &getBody() const;
    const MapType &getHeaders() const;
    const MapType &getParams() const;
    const MapType &getCookies() const;

    void setMethod(HttpMethod method);
    void setStatus(HttpStatus status);
    void setVersion(uint8_t version);
    void setClose(bool close);
    void setPath(const std::string &path);
    void setQuery(const std::string &query);
    void setFragment(const std::string &fragment);
    void setBody(const std::string &body);
    void setHeaders(const MapType &headers);
    void setParams(const MapType &params);
    void setCookies(const MapType &cookies);

    std::string getHeader(const std::string &key, const std::string &default_value = "");
    std::string getParam(const std::string &key, const std::string &default_value = "");
    std::string getCookie(const std::string &key, const std::string &default_value = "");

    void setHeader(const std::string &key, const std::string &value);
    void setParam(const std::string &key, const std::string &value);
    void setCookie(const std::string &key, const std::string &value);

    bool hasHeader(const std::string &key, std::string &value) const;
    bool hasParam(const std::string &key, std::string &value) const;
    bool hasCookie(const std::string &key, std::string &value) const;

    void delHeader(const std::string &key);
    void delParam(const std::string &key);
    void delCookie(const std::string &key);

    /**
     * @brief 将HttpRequest对象转换为字符串
     * @param os 输出流
     * @return std::ostream& 返回输出流引用，方便链式调用
     * @note 输出格式:
     * GET /uri HTTP/1.1
     * Host: www.example.com
     * ...
     */
    std::ostream &toString(std::ostream &os) const;
    std::string toString() const;

  public:
    template <typename T>
    bool getCheckHeader(const MapType &map, const std::string &key, T &out,
                        const T &default_value = T())
    {
        return getCheckHelper(map, key, out, default_value);
    }

    template <typename T>
    bool getHeader(const MapType &map, const std::string &key, const T &default_value = T())
    {
        return getHelper(map, key, default_value);
    }

    template <typename T>
    bool getCheckParam(const MapType &map, const std::string &key, T &out,
                       const T &default_value = T())
    {
        return getCheckHelper(map, key, out, default_value);
    }

    template <typename T>
    bool getParam(const MapType &map, const std::string &key, const T &default_value = T())
    {
        return getHelper(map, key, default_value);
    }

    template <typename T>
    bool getCheckCookie(const MapType &map, const std::string &key, T &out,
                        const T &default_value = T())
    {
        return getCheckHelper(map, key, out, default_value);
    }

    template <typename T>
    bool getCookie(const MapType &map, const std::string &key, const T &default_value = T())
    {
        return getHelper(map, key, default_value);
    }

  private:
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

  private:
    HttpMethod m_method;
    HttpStatus m_status;
    uint8_t m_version;
    bool m_close;

    // uri: http://www.example.com:80/path?id=10&v=20#fr
    // http: 协议
    // www.example.com: host
    // 80: 端口
    // /path: 路径
    // id=10&v=20: 参数
    std::string m_path;
    std::string m_query;
    std::string m_fragment;
    std::string m_body;
    MapType m_headers;
    MapType m_params;
    MapType m_cookies;
};
} // namespace http
} // namespace lon