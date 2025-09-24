#include "lonhttpfw/lonhttpfw.h"

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml("./.config/log.yaml");
    auto uri = lon::net::Uri::create("http://www.baidu.com/test/中文/uri?id=100&name=中文#frg中文");
    LON_INFO(LON_LOG_ROOT) << *uri;
    LON_INFO(LON_LOG_ROOT) << uri->getScheme();
    LON_INFO(LON_LOG_ROOT) << uri->getHost();
    LON_INFO(LON_LOG_ROOT) << uri->getPort();
    LON_INFO(LON_LOG_ROOT) << uri->getPath();
    LON_INFO(LON_LOG_ROOT) << uri->getQuery();
    LON_INFO(LON_LOG_ROOT) << uri->getFragment();
    auto addr = uri->create();
    LON_INFO(LON_LOG_ROOT) << addr->toString();
    return 0;
}
