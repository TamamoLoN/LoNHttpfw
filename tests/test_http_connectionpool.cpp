#include "lonhttpfw/lonhttpfw.h"

void test_http_pool()
{
    auto pool = std::make_shared<lon::httpservice::HttpConnectionPool>("127.0.0.1:8080", "", 8080,
                                                                       10, 10000, 20);

    auto ios = lon::scheduler::IOScheduler::getThis();
    ios->addTimer(
        1000,
        [pool]() {
            pool->get("/", 1000);
            LON_INFO(LON_LOG_ROOT) << "pool size=" << pool->size();
        },
        true);
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml(".config/log.yaml");
    auto ios = std::make_shared<lon::scheduler::IOScheduler>(
        2, true, "io_scheduler", lon::config::GlobalConfig::Instance().config_fiber->getData());
    ios->schedule(test_http_pool);
    return 0;
}
