#include "websocket/websocket.h"

namespace lon
{
namespace ws
{
static auto g_logger = LON_LOG_ROOT;

std::string WSFrameHead::toString() const
{
    std::stringstream ss;
    ss << "[WSFrameHead fin=" << fin << " rsv1=" << rsv1 << " rsv2=" << rsv2 << " rsv3=" << rsv3
       << " opcode=" << opcode << " mask=" << mask << " payload=" << payload << "]";
    return ss.str();
}

WSFrameMessage::WSFrameMessage(int opcode, const std::string &data) : m_opcode(opcode), m_data(data)
{
}

int WSFrameMessage::getOpcode() const { return m_opcode; }

void WSFrameMessage::setOpcode(int opcode) { m_opcode = opcode; }

const std::string &WSFrameMessage::getData() const { return m_data; }

std::string &WSFrameMessage::getData() { return m_data; }

void WSFrameMessage::setData(const std::string &data) { m_data = data; }

WSFrameMessage::Ptr WebSocket::recvMessage(util::Stream *stream, bool client)
{
    int opcode = 0;
    std::string data{};
    int cur_len = 0;
    do
    {
        WSFrameHead ws_head;
        if (stream->readF(&ws_head, sizeof(ws_head)) <= 0)
        {
            break;
        }
        LON_DEBUG(g_logger) << "WSFrameHead: " << ws_head.toString();

        if (ws_head.opcode == WSFrameHead::PING)
        {
            LON_DEBUG(g_logger) << "PING";
            if (pong(stream) <= 0)
            {
                break;
            }
        }
        else if (ws_head.opcode == WSFrameHead::PONG)
        {
        }
        // else if (ws_head.opcode == WSFrameHead::CLOSE)
        // {
        //     sendMessage(stream, std::make_shared<WSFrameMessage>(WSFrameHead::CLOSE), client,
        //     true); break;
        // }
        else if (ws_head.opcode == WSFrameHead::CONTINUE ||
                 ws_head.opcode == WSFrameHead::TEXT_FRAME ||
                 ws_head.opcode == WSFrameHead::BIN_FRAME)
        {
            if (!client && !ws_head.mask)
            {
                LON_DEBUG(g_logger) << "WSFrameHead mask != 1";
                break;
            }
            uint64_t length = 0;
            if (ws_head.payload == 126)
            {
                uint16_t read_length = 0;
                if (stream->readF(&read_length, sizeof(read_length)) <= 0)
                {
                    break;
                }
                length = util::byteswapToLittleEndian(read_length);
            }
            else if (ws_head.payload == 127)
            {
                uint64_t read_length = 0;
                if (stream->readF(&read_length, sizeof(read_length)) <= 0)
                {
                    break;
                }
                length = util::byteswapToLittleEndian(read_length);
            }
            else
            {
                length = ws_head.payload;
            }

            if ((cur_len + length) >=
                http::HttpGlobalConfig::Instance().config_websocket_message_max_size->getData())
            {
                LON_WARN(g_logger) << "WSFrameMessage length > "
                                   << http::HttpGlobalConfig::Instance()
                                          .config_websocket_message_max_size->getData()
                                   << " (" << (cur_len + length) << ")";
                break;
            }

            char mask[4] = {0};
            if (ws_head.mask)
            {
                if (stream->readF(mask, sizeof(mask)) <= 0)
                {
                    break;
                }
            }
            data.resize(cur_len + length);
            if (stream->readF(&data[cur_len], length) <= 0)
            {
                break;
            }
            if (ws_head.mask)
            {
                for (int i = 0; i < (int)length; ++i)
                {
                    data[cur_len + i] ^= mask[i % 4];
                }
            }
            cur_len += length;

            if (!opcode && ws_head.opcode != WSFrameHead::CONTINUE)
            {
                opcode = ws_head.opcode;
            }

            if (ws_head.fin)
            {
                LON_DEBUG(g_logger) << data;
                return std::make_shared<WSFrameMessage>(opcode, std::move(data));
            }
        }
        else
        {
            LON_ERROR(g_logger) << "invalid opcode=" << ws_head.opcode;
        }
    } while (true);
    stream->close();
    return nullptr;
}

int32_t WebSocket::sendMessage(util::Stream *stream, const WSFrameMessage::Ptr &msg, bool client,
                               bool fin)
{
    do
    {
        WSFrameHead ws_head;
        memset(&ws_head, 0, sizeof(ws_head));
        ws_head.fin    = fin;
        ws_head.opcode = msg->getOpcode();
        ws_head.mask   = client;
        auto size      = msg->getData().size();
        if (size < 126)
        {
            ws_head.payload = size;
        }
        else if (size < 65536)
        {
            ws_head.payload = 126;
        }
        else
        {
            ws_head.payload = 127;
        }

        if (stream->writeF(&ws_head, sizeof(ws_head)) <= 0)
        {
            break;
        }
        if (ws_head.payload == 126)
        {
            uint16_t len = size;
            len          = util::byteswapToLittleEndian(len);
            if (stream->writeF(&len, sizeof(len)) <= 0)
            {
                break;
            }
        }
        else if (ws_head.payload == 127)
        {
            uint64_t len = util::byteswapToLittleEndian(size);
            if (stream->writeF(&len, sizeof(len)) <= 0)
            {
                break;
            }
        }
        if (client)
        {
            char mask[4];
            uint32_t rand_value = rand();
            memcpy(mask, &rand_value, sizeof(mask));
            std::string &data = msg->getData();
            for (size_t i = 0; i < data.size(); ++i)
            {
                data[i] ^= mask[i % 4];
            }

            if (stream->writeF(mask, sizeof(mask)) <= 0)
            {
                break;
            }
        }
        if (stream->writeF(msg->getData().c_str(), size) <= 0)
        {
            break;
        }
        return size + sizeof(ws_head);
    } while (0);
    stream->close();
    return -1;
}

int32_t WebSocket::ping(util::Stream *stream)
{
    WSFrameHead ws_head;
    memset(&ws_head, 0, sizeof(ws_head));
    ws_head.fin    = 1;
    ws_head.opcode = WSFrameHead::PING;
    auto ret       = stream->writeF(&ws_head, sizeof(ws_head));
    if (ret <= 0)
    {
        stream->close();
    }
    return ret;
}

int32_t WebSocket::pong(util::Stream *stream)
{
    WSFrameHead ws_head;
    memset(&ws_head, 0, sizeof(ws_head));
    ws_head.fin    = 1;
    ws_head.opcode = WSFrameHead::PONG;
    auto ret       = stream->writeF(&ws_head, sizeof(ws_head));
    if (ret <= 0)
    {
        stream->close();
    }
    return ret;
}
} // namespace ws
} // namespace lon