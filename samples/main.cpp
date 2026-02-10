#include "lonhttpfw/lonhttpfw.h"

REGISTER_HTTPSERVER

int main(int argc, char *argv[])
{
    auto &app = lon::system::Application::Instance();
    if (app.init(argc, argv))
    {
        return app.run();
    }
    return 0;
}
