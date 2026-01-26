#include "lonhttpfw/lonhttpfw.h"

static auto g_logger = LON_LOG_ROOT;

int main(int argc, char const *argv[])
{
    std::string str = "http://www.baidu.com/test/中文/uri?id=100&name=中文#frg中文";
    if (argv[1])
    {
        str = argv[1];
    }
    lon::config::Config::parseFromYaml("./.config/log.yaml");
    auto uri = lon::net::Uri::create(str);
    LON_INFO(g_logger) << *uri;
    LON_INFO(g_logger) << uri->getScheme();
    LON_INFO(g_logger) << uri->getHost();
    LON_INFO(g_logger) << uri->getPort();
    LON_INFO(g_logger) << uri->getPath();
    LON_INFO(g_logger) << uri->getQuery();
    LON_INFO(g_logger) << uri->getFragment();
    auto addr = uri->create();
    LON_INFO(g_logger) << addr->toString();
    return 0;
}
