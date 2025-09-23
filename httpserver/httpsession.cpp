#include "httpserver/httpsession.h"

namespace lon
{
namespace httpserver
{
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
        if (len <= 0)
        {
            return nullptr;
        }
        len += offset;
        auto ret = parser->execute(data, len);
        if (parser->error())
        {
            return nullptr;
        }
        offset = len - ret;
        if (offset == buffer_size)
        {
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
        body.reserve(content_len);
        if (content_len >= offset)
        {
            body.append(data, offset);
        }
        else
        {
            body.append(data, content_len);
        }
        content_len -= offset;
        if (content_len > 0)
        {
            if (readF(&body[body.size()], content_len) <= 0)
            {
                return nullptr;
            }
        }
        parser->getData()->setBody(body);
    }
    return std::static_pointer_cast<http::HttpRequest>(parser->getData());
}

size_t HttpSession::sendResponse(const http::HttpResponse::Ptr &response)
{
    std::stringstream ss;
    ss << *response;
    auto str = ss.str();
    return writeF(str.c_str(), str.size());
}
} // namespace httpserver
} // namespace lon