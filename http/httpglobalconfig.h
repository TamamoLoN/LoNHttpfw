#pragma once
#include "config/config.h"

namespace lon
{
namespace http
{
struct HttpRequestResponseConfig
{
    explicit HttpRequestResponseConfig(size_t buffer_size = 1024 * 4ull,
                                       size_t body_size   = 1024 * 1024ull);
    bool operator==(const HttpRequestResponseConfig &other) const;
    bool operator<(const HttpRequestResponseConfig &other) const;
    size_t buffer_size;
    size_t body_size;
};

struct HttpConfig
{
    explicit HttpConfig(HttpRequestResponseConfig request  = HttpRequestResponseConfig(),
                        HttpRequestResponseConfig response = HttpRequestResponseConfig());
    bool operator==(const HttpConfig &other) const;
    bool operator<(const HttpConfig &other) const;
    HttpRequestResponseConfig request;
    HttpRequestResponseConfig response;
};

struct HttpGlobalConfig
{
    explicit HttpGlobalConfig();
    static HttpGlobalConfig &Instance();
    config::ConfigData<HttpConfig>::Ptr config_http;
};

//全局变量，使其在main函数之前初始化
// static auto http_global_conifg = HttpGlobalConfig::Instance();

} // namespace http

template <> class util::LexicalCast<http::HttpRequestResponseConfig, std::string>
{
  public:
    http::HttpRequestResponseConfig operator()(const std::string &source) const
    {
        YAML::Node node = YAML::Load(source);
        http::HttpRequestResponseConfig res;
        std::stringstream ss;
        if (node["buffer_size"].IsDefined())
            res.buffer_size = node["buffer_size"].as<size_t>();
        if (node["body_size"].IsDefined())
            res.body_size = node["body_size"].as<size_t>();
        return res;
    }
};

template <> class util::LexicalCast<std::string, http::HttpRequestResponseConfig>
{
  public:
    std::string operator()(const http::HttpRequestResponseConfig &source) const
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
        {
            std::stringstream ss;
            ss << node["request"];
            res.request = LexicalCast<http::HttpRequestResponseConfig, std::string>()(ss.str());
        }
        if (node["response"].IsDefined())
        {
            std::stringstream ss;
            ss << node["response"];
            res.response = LexicalCast<http::HttpRequestResponseConfig, std::string>()(ss.str());
        }
        return res;
    }
};

template <> class util::LexicalCast<std::string, http::HttpConfig>
{
  public:
    std::string operator()(const http::HttpConfig &source) const
    {
        YAML::Node node;
        node["request"] =
            LexicalCast<std::string, http::HttpRequestResponseConfig>()(source.request);
        node["response"] =
            LexicalCast<std::string, http::HttpRequestResponseConfig>()(source.response);
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
} // namespace lon