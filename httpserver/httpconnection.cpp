#include "httpserver/httpconnection.h"

namespace lon
{
namespace httpserver
{
HttpConnection::HttpConnection(const net::Socket::Ptr &socket, bool proxy, size_t buffer_size)
    : net::SocketStream(socket, proxy), m_buffer_size(buffer_size)
{
}

HttpConnection::~HttpConnection() {}

http::HttpResponse::Ptr HttpConnection::recvResponse()
{
    auto parser      = std::make_shared<http::HttpResponseParser>();
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
    auto http_parser = parser->getParser();
    if (http_parser.chunked)
    {
        do
        {

        } while (http_parser.chunks_done);
    }
    else
    {
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
                    return nullptr;
                }
            }
            parser->getData()->setBody(body);
        }
    }

    return std::static_pointer_cast<http::HttpResponse>(parser->getData());
}

ssize_t HttpConnection::sendRequest(const http::HttpRequest::Ptr &request)
{
    std::stringstream ss;
    ss << *request;
    auto str = ss.str();
    return writeF(str.c_str(), str.size());
}
} // namespace httpserver
} // namespace lon