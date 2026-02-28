#pragma once

#include "httpservice/httpserver.h"
#include "server/serverfactory.h"

namespace lon
{
namespace assembly
{
class ServerRegistry
{
  public:
    ServerRegistry();
};
#define REGISTER_SERVERS                                                                           \
    static auto g_server_registry = lon::util::Singleton<lon::assembly::ServerRegistry>::Instance();
} // namespace assembly
} // namespace lon