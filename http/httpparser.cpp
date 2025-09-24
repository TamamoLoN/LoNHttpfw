#include "http/httpparser.h"

namespace lon
{
namespace http
{
HttpParser::HttpParser() : m_error(0) {}

HttpMessage::Ptr HttpParser::parse(char *data, size_t len)
{
    size_t ret = execute(data, len);
    if (LON_UNLIKELY(ret == static_cast<size_t>(-1)))
    {
        LON_ERROR(LON_LOG_ROOT) << "HttpParser::parse error: " << m_error;
        return nullptr;
    }
    if (LON_LIKELY(ret < len))
    {
        std::string body(data + ret, len - ret);
        m_data->setBody(std::move(body));
    }
    else
    {
        m_data->setBody("");
    }

    return m_data;
}

size_t HttpParser::getContentLength() const
{
    return m_data->getHeader<size_t>("Content-Length", 0);
}

void HttpParser::setError(int32_t error) { m_error = error; }

HttpMessage::Ptr HttpParser::getData() const { return m_data; }

HttpRequestParser::HttpRequestParser() : HttpParser()
{
    m_data    = std::make_shared<HttpRequest>();
    m_request = std::static_pointer_cast<HttpRequest>(m_data);
    http_parser_init(&m_parser);
    m_parser.request_method = onRequestMethod;
    m_parser.request_uri    = onRequestUri;
    m_parser.request_path   = onRequestPath;
    m_parser.fragment       = onRequestFragment;
    m_parser.query_string   = onRequestQueryString;
    m_parser.http_version   = onRequestHttpVersion;
    m_parser.header_done    = onRequesHeaderDone;
    m_parser.http_field     = onRequestHttpField;
    m_parser.data           = this;
}

HttpRequestParser::~HttpRequestParser() { http_parser_finish(&m_parser); }

size_t HttpRequestParser::execute(char *data, size_t len)
{
    // ret: 实际解析了多少，-1表示出错
    size_t ret = http_parser_execute(&m_parser, data, len, 0);
    memmove((void *)data, data + ret, len - ret);

    return ret;
}

int32_t HttpRequestParser::finished() { return http_parser_finish(&m_parser); }

int32_t HttpRequestParser::error() { return m_error || http_parser_has_error(&m_parser); }

const http_parser &HttpRequestParser::getParser() const { return m_parser; }

void HttpRequestParser::onRequestMethod(void *data, const char *at, size_t length)
{
    auto parser = static_cast<HttpRequestParser *>(data);
    auto method = HttpMethodConverter::fromString(at);
    if (method == HttpMethod::UNKNOWN)
    {
        LON_WARN(LON_LOG_ROOT) << "Unknown HTTP method: " << at;
        parser->setError((int32_t)HttpParserError::UNKNOWN_METHOD);
        return;
    }
    parser->m_request->setMethod(method);
}

void HttpRequestParser::onRequestUri(void *data, const char *at, size_t length) {}

void HttpRequestParser::onRequestPath(void *data, const char *at, size_t length)
{
    auto parser = static_cast<HttpRequestParser *>(data);
    parser->m_request->setPath(std::string(at, length));
}

void HttpRequestParser::onRequestFragment(void *data, const char *at, size_t length)
{
    auto parser = static_cast<HttpRequestParser *>(data);
    parser->m_request->setFragment(std::string(at, length));
}

void HttpRequestParser::onRequestQueryString(void *data, const char *at, size_t length)
{
    auto parser = static_cast<HttpRequestParser *>(data);
    parser->m_request->setQuery(std::string(at, length));
}

void HttpRequestParser::onRequestHttpVersion(void *data, const char *at, size_t length)
{
    auto parser = static_cast<HttpRequestParser *>(data);
    if (strncmp(at, "HTTP/1.0", length) == 0)
    {
        parser->m_request->setVersion(0x10);
    }
    else if (strncmp(at, "HTTP/1.1", length) == 0)
    {
        parser->m_request->setVersion(0x11);
    }
    else
    {
        LON_WARN(LON_LOG_ROOT) << "Unknown HTTP version: " << at;
        parser->setError((int32_t)HttpParserError::UNKNOWN_VERSION);
        return;
    }
}

void HttpRequestParser::onRequesHeaderDone(void *data, const char *at, size_t length)
{
    // auto parser = static_cast<HttpRequestParser *>(data);
}

void HttpRequestParser::onRequestHttpField(void *data, const char *field, size_t flen,
                                           const char *value, size_t vlen)
{
    auto parser = static_cast<HttpRequestParser *>(data);
    if (flen == 0)
    {
        LON_WARN(LON_LOG_ROOT) << "Invalid HTTP field: ";
        // parser->setError((int32_t)HttpParserError::INVALID_FIELD);
        return;
    }
    parser->m_request->setHeader(std::string(field, flen), std::string(value, vlen));
}

HttpResponseParser::HttpResponseParser() : HttpParser()
{
    m_data     = std::make_shared<HttpResponse>();
    m_response = std::static_pointer_cast<HttpResponse>(m_data);
    httpclient_parser_init(&m_parser);
    m_parser.reason_phrase = onResponseReasonPhrase;
    m_parser.status_code   = onResponseStatusCode;
    m_parser.chunk_size    = onResponseChunkSize;
    m_parser.http_version  = onResponseHttpVersion;
    m_parser.header_done   = onResponseHeaderDone;
    m_parser.last_chunk    = onResponseLastChunk;
    m_parser.http_field    = onResponseHttpField;
    m_parser.data          = this;
}

HttpResponseParser::~HttpResponseParser() { httpclient_parser_finish(&m_parser); }

size_t HttpResponseParser::execute(char *data, size_t len)
{
    size_t ret = httpclient_parser_execute(&m_parser, data, len, 0);
    memmove((void *)data, data + ret, len - ret);

    return ret;
}

int32_t HttpResponseParser::finished() { return httpclient_parser_finish(&m_parser); }

int32_t HttpResponseParser::error() { return m_error || httpclient_parser_has_error(&m_parser); }

const httpclient_parser &HttpResponseParser::getParser() const { return m_parser; }

void HttpResponseParser::onResponseReasonPhrase(void *data, const char *at, size_t length)
{
    auto parser = static_cast<HttpResponseParser *>(data);
    parser->m_response->setReason(std::string(at, length));
}

void HttpResponseParser::onResponseStatusCode(void *data, const char *at, size_t length)
{
    auto parser = static_cast<HttpResponseParser *>(data);
    // auto status = HttpStatusConverter::fromString(at);
    // std::cout << "1111status=" << (int)status << std::endl;
    // if (status == HttpStatus::UNKNOWN)
    // {
    //     LON_WARN(LON_LOG_ROOT) << "Unknown HTTP status: " << at;
    //     parser->setError((int32_t)HttpParserError::UNKNOWN_STATUS);
    //     return;
    // }
    HttpStatus status = (HttpStatus)(atoi(at));
    parser->m_response->setStatus(status);
}

void HttpResponseParser::onResponseChunkSize(void *data, const char *at, size_t length) {}

void HttpResponseParser::onResponseHttpVersion(void *data, const char *at, size_t length)
{
    auto parser = static_cast<HttpResponseParser *>(data);
    if (strncmp(at, "HTTP/1.0", length) == 0)
    {
        parser->m_response->setVersion(0x10);
    }
    else if (strncmp(at, "HTTP/1.1", length) == 0)
    {
        parser->m_response->setVersion(0x11);
    }
    else
    {
        LON_WARN(LON_LOG_ROOT) << "Unknown HTTP version: " << at;
        parser->setError((int32_t)HttpParserError::UNKNOWN_VERSION);
        return;
    }
}

void HttpResponseParser::onResponseHeaderDone(void *data, const char *at, size_t length) {}

void HttpResponseParser::onResponseLastChunk(void *data, const char *at, size_t length) {}

void HttpResponseParser::onResponseHttpField(void *data, const char *field, size_t flen,
                                             const char *value, size_t vlen)
{
    auto parser = static_cast<HttpResponseParser *>(data);
    if (flen == 0)
    {
        LON_WARN(LON_LOG_ROOT) << "Invalid HTTP field: ";
        // parser->setError((int32_t)HttpParserError::INVALID_FIELD);
        return;
    }
    parser->m_response->setHeader(std::string(field, flen), std::string(value, vlen));
}

} // namespace http
} // namespace lon