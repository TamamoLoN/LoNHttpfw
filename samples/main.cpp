#include "lonhttpfw/lonhttpfw.h"

REGISTER_SERVERS

int main(int argc, char *argv[])
{
    lon::system::Application app;
    if (app.init(argc, argv))
    {
        return app.run();
    }
    return 0;
}
