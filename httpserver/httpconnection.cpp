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
    std::shared_ptr<char> buffer(new char[buffer_size + 1](), [](char *ptr) {
        delete[] ptr;
        ptr = nullptr;
    });
    auto data      = buffer.get();
    ssize_t offset = 0;

    do
    {
        ssize_t len = read(data + offset, buffer_size - offset);
        if (len <= 0)
        {
            close();
            return nullptr;
        }
        len += offset;
        data[len] = '\0';

        size_t nparse = parser->execute(data, len, false);
        if (parser->error())
        {
            close();
            return nullptr;
        }
        offset = len - nparse;
        if (offset == (ssize_t)buffer_size)
        {
            close();
            return nullptr;
        }
        if (parser->finished())
        {
            break;
        }
    } while (true);

    auto &http_parser = parser->getParser();
    std::string body;

    if (http_parser.chunked)
    {
        ssize_t len = offset;
        do
        {
            bool begin = true;
            do
            {
                if (!begin || len == 0)
                {
                    auto rt = read(data + len, buffer_size - len);
                    if (rt <= 0)
                    {
                        close();
                        return nullptr;
                    }
                    len += rt;
                }
                data[len]     = '\0';
                size_t nparse = parser->execute(data, len, true);
                if (parser->error())
                {
                    close();
                    return nullptr;
                }
                len -= nparse;
                if (len == (ssize_t)buffer_size)
                {
                    close();
                    return nullptr;
                }
                begin = false;
            } while (!parser->finished());

            if (http_parser.content_len + 2 <= len)
            {
                body.append(data, http_parser.content_len);
                memmove(data, data + http_parser.content_len + 2,
                        len - http_parser.content_len - 2);
                len -= http_parser.content_len + 2;
            }
            else
            {
                body.append(data, len);
                int left = http_parser.content_len - len + 2;
                while (left > 0)
                {
                    int rt = read(data, left > (int)buffer_size ? (int)buffer_size : left);
                    if (rt <= 0)
                    {
                        close();
                        return nullptr;
                    }
                    body.append(data, rt);
                    left -= rt;
                }
                body.resize(body.size() - 2); // 去掉最后的 CRLF
                len = 0;
            }
        } while (!http_parser.chunks_done);
    }
    else
    {
        int64_t length = parser->getContentLength();
        if (length > 0)
        {
            body.resize(length);
            ssize_t len = 0;
            if (length >= offset)
            {
                memcpy(&body[0], data, offset);
                len = offset;
            }
            else
            {
                memcpy(&body[0], data, length);
                len = length;
            }
            length -= offset;
            if (length > 0)
            {
                if (readF(&body[len], length) <= 0)
                {
                    close();
                    return nullptr;
                }
            }
        }
    }

    // 设置 body
    parser->getData()->setBody(body);
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