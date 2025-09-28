#include "lonhttpfw/lonhttpfw.h"

static int cnt = 0;
static lon::scheduler::Timer::Ptr timer;

void test_http_pool()
{
    auto pool =
        std::make_shared<lon::httpservice::HttpConnectionPool>("127.0.0.1", "", 8080, 10, 30000, 3);

    auto ios = lon::scheduler::IOScheduler::getThis();
    timer    = ios->addTimer(
        1000,
        [pool]() {
            auto res = pool->get("/", 300);
            // LON_INFO(LON_LOG_ROOT) << "res=" << res->response->toString();
            LON_INFO(LON_LOG_ROOT) << "pool size=" << pool->size();
            if (++cnt == 5)
            {
                timer->cancel();
            }
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
