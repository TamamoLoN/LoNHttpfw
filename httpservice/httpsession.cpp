#include "httpservice/httpsession.h"

namespace lon
{
namespace httpservice
{
static auto g_logger = LON_LOG_ROOT;

HttpSession::HttpSession(const net::Socket::Ptr &socket, bool proxy, size_t buffer_size)
    : net::SocketStream(socket, proxy), m_buffer_size(buffer_size)
{
}

HttpSession::~HttpSession() {}

http::HttpRequest::Ptr HttpSession::recvRequest()
{
    auto parser      = std::make_shared<http::HttpRequestParser>();
    auto buffer_size = m_buffer_size;
    std::shared_ptr<char> buffer(new char[buffer_size](), [](char *ptr) {
        delete[] ptr;
        ptr = nullptr;
    });
    auto data     = buffer.get();
    size_t offset = 0;
    do
    {
        ssize_t len = read(data + offset, buffer_size - offset);
        if (len < 0)
        {
            LON_ERROR(g_logger) << "recv failed, len=" << len;
            close();
            return nullptr;
        }
        else if (len == 0)
        {
            m_eof = true;
            close();
            return nullptr;
        }
        len += offset;
        auto ret = parser->execute(data, len);
        if (parser->error())
        {
            LON_ERROR(g_logger) << "http parse has error, error=" << parser->error()
                                << "data=" << std::string(data, len);
            close();
            return nullptr;
        }
        offset = len - ret;
        if (offset == buffer_size)
        {
            LON_ERROR(g_logger) << "buffer overflow";
            close();
            return nullptr;
        }
        if (parser->finished())
        {
            break;
        }
    } while (true);
    int64_t content_len = parser->getContentLength();
    if (content_len > 0)
    {
        std::string body;
        body.resize(content_len);
        ssize_t len = 0;
        if (content_len >= offset)
        {
            memcpy(&body[0], data, offset);
            len = offset;
        }
        else
        {
            memcpy(&body[0], data, content_len);
            len = content_len;
        }
        content_len -= offset;
        if (content_len > 0)
        {
            if (readF(&body[len], content_len) <= 0)
            {
                LON_ERROR(g_logger) << "read body failed";
                close();
                return nullptr;
            }
        }
        parser->getData()->setBody(body);
    }
    return std::static_pointer_cast<http::HttpRequest>(parser->getData());
}

ssize_t HttpSession::sendResponse(const http::HttpResponse::Ptr &response)
{
    std::stringstream ss;
    ss << *response;
    auto str = ss.str();
    return writeF(str.c_str(), str.size());
}
} // namespace httpservice
} // namespace lon