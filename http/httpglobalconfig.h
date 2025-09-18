#pragma once
#include "config/config.h"

namespace lon
{
namespace http
{
struct HttpRequestConfig
{
    explicit HttpRequestConfig(size_t buffer_size = 1024 * 4ull, size_t body_size = 1024 * 1024ull)
        : buffer_size(buffer_size), body_size(body_size)
    {
    }
    bool operator==(const HttpRequestConfig &other) const
    {
        return buffer_size == other.buffer_size && body_size == other.body_size;
    }
    bool operator<(const HttpRequestConfig &other) const
    {
        return buffer_size < other.buffer_size && body_size < other.body_size;
    }
    size_t buffer_size;
    size_t body_size;
};

struct HttpConfig
{
    explicit HttpConfig(HttpRequestConfig request = HttpRequestConfig()) : request(request) {}
    bool operator==(const HttpConfig &other) const { return request == other.request; }
    bool operator<(const HttpConfig &other) const { return request < other.request; }
    HttpRequestConfig request;
};

struct HttpGlobalConfig
{
    explicit HttpGlobalConfig()
    {
        config_http = config::Config::setData("http", HttpConfig(), "http config");
        config_http->addConfigDataChangeCB(
            [](const HttpConfig &old_data, const HttpConfig &new_data) {
                LON_INFO(LON_LOG_ROOT) << "on config http data changed";
            });
    }
    static HttpGlobalConfig &Instance()
    {
        static HttpGlobalConfig instance;
        return instance;
    }
    config::ConfigData<HttpConfig>::Ptr config_http;
};

//全局变量，使其在main函数之前初始化
static auto http_global_conifg = HttpGlobalConfig::Instance();

} // namespace http

template <> class util::LexicalCast<http::HttpRequestConfig, std::string>
{
  public:
    http::HttpRequestConfig operator()(const std::string &source) const
    {
        YAML::Node node = YAML::Load(source);
        http::HttpRequestConfig res;
        std::stringstream ss;
        if (node["buffer_size"].IsDefined())
            res.buffer_size = node["buffer_size"].as<size_t>();
        if (node["body_size"].IsDefined())
            res.body_size = node["body_size"].as<size_t>();
        return res;
    }
};

template <> class util::LexicalCast<std::string, http::HttpRequestConfig>
{
  public:
    std::string operator()(const http::HttpRequestConfig &source) const
    {
        YAML::Node node;
        node["buffer_size"] = source.buffer_size;
        node["body_size"]   = source.body_size;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <> class util::LexicalCast<http::HttpConfig, std::string>
{
  public:
    http::HttpConfig operator()(const std::string &source) const
    {
        YAML::Node node = YAML::Load(source);
        http::HttpConfig res;
        std::stringstream ss;
        if (node["request"].IsDefined())
            ss << node["request"];
        res.request = LexicalCast<http::HttpRequestConfig, std::string>()(ss.str());
        return res;
    }
};

template <> class util::LexicalCast<std::string, http::HttpConfig>
{
  public:
    std::string operator()(const http::HttpConfig &source) const
    {
        YAML::Node node;
        node["request"] = LexicalCast<std::string, http::HttpRequestConfig>()(source.request);
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
} // namespace lon