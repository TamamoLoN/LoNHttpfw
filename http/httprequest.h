#pragma once

#include "http/httpmessage.h"

namespace lon
{
namespace http
{
class HttpRequest : public HttpMessage
{
  public:
    using Ptr = std::shared_ptr<HttpRequest>;
    // 0x10: HTTP/1.0 0x11: HTTP/1.1
    HttpRequest(uint8_t http_version = 0x11, bool close = true);
    ~HttpRequest();

    HttpMethod getMethod() const;
    const std::string &getPath() const;
    const std::string &getQuery() const;
    const std::string &getFragment() const;
    const MapType &getParams() const;

    void setMethod(HttpMethod method);
    void setPath(const std::string &path);
    void setQuery(const std::string &query);
    void setFragment(const std::string &fragment);
    void setParams(const MapType &params);

    std::string getParam(const std::string &key, const std::string &default_value = "");

    void setParam(const std::string &key, const std::string &value);

    bool hasParam(const std::string &key, std::string &value) const;

    void delParam(const std::string &key);

    /**
     * @brief 将HttpRequest对象转换为字符串
     * @param os 输出流
     * @return std::ostream& 返回输出流引用，方便链式调用
     * @note 输出格式:
     * GET /uri HTTP/1.1
     * Host: www.example.com
     * ...
     */
    std::ostream &toString(std::ostream &os) const override;
    std::string toString() const override;

  public:
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

  private:
    HttpMethod m_method;

    // uri: http://www.example.com:80/path?id=10&v=20#fr
    // http: 协议
    // www.example.com: host
    // 80: 端口
    // /path: 路径
    // id=10&v=20: 参数
    std::string m_path;
    std::string m_query;
    std::string m_fragment;
    MapType m_params;
};
} // namespace http
} // namespace lon