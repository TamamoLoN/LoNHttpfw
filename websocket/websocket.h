#pragma once

#include "http/httpglobalconfig.h"
#include "util/stream.h"

namespace lon
{
namespace ws
{
/*
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-------+-+-------------+-------------------------------+
|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
|I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
|N|V|V|V|       |S|             |   (if payload len==126/127)   |
| |1|2|3|       |K|             |                               |
+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
|     Extended payload length continued, if payload len == 127  |
+ - - - - - - - - - - - - - - - +-------------------------------+
|                               | Masking-key, if MASK set to 1 |
+-------------------------------+-------------------------------+
| Masking-key (continued)       |          Payload Data         |
+-------------------------------- - - - - - - - - - - - - - - -+
:                     Payload Data continued ...                :
+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
|                     Payload Data continued ...                |
+---------------------------------------------------------------+
*/
#ifdef _WIN32
#pragma pack(push, 1)
struct LON_API WSFrameHead
{
    enum OPCODE
    {
        CONTINUE   = 0,
        TEXT_FRAME = 1,
        BIN_FRAME  = 2,
        CLOSE      = 8,
        PING       = 0x9,
        PONG       = 0xA
    };

    uint8_t opcode : 4;
    uint8_t rsv3 : 1;
    uint8_t rsv2 : 1;
    uint8_t rsv1 : 1;
    uint8_t fin : 1;

    uint8_t payload : 7;
    uint8_t mask : 1;

    std::string toString() const;
};
#pragma pack(pop)
#else
#pragma pack(1)
struct LON_API WSFrameHead
{
    enum OPCODE
    {
        /// 数据分片帧
        CONTINUE = 0,
        /// 文本帧
        TEXT_FRAME = 1,
        /// 二进制帧
        BIN_FRAME = 2,
        /// 断开连接
        CLOSE = 8,
        /// PING
        PING = 0x9,
        /// PONG
        PONG = 0xA
    };
    uint32_t opcode : 4;
    bool rsv3 : 1;
    bool rsv2 : 1;
    bool rsv1 : 1;
    bool fin : 1;
    uint32_t payload : 7;
    bool mask : 1;

    std::string toString() const;
};
#pragma pack()
#endif
static_assert(sizeof(WSFrameHead) == 2, "WSFrameHead must be 2 bytes");

class LON_API WSFrameMessage
{
  public:
    using Ptr = std::shared_ptr<WSFrameMessage>;
    WSFrameMessage(int opcode = 0, const std::string &data = "");

    int getOpcode() const;
    void setOpcode(int opcode);
    const std::string &getData() const;
    std::string &getData();
    void setData(const std::string &data);

  private:
    int m_opcode;
    std::string m_data;
};

class LON_API WebSocket
{
  public:
    static WSFrameMessage::Ptr recvMessage(util::Stream *stream, bool client);
    static int32_t sendMessage(util::Stream *stream, const WSFrameMessage::Ptr &msg, bool client,
                               bool fin);
    static int32_t ping(util::Stream *stream);
    static int32_t pong(util::Stream *stream);
};
} // namespace ws
} // namespace lon