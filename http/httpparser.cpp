#include "http/httpparser.h"

namespace lon
{
namespace http
{
int32_t HttpParser::error() const { return m_error; }

void HttpParser::setError(int32_t error) { m_error = error; }

HttpRequestParser::HttpRequestParser()
{
    m_request = std::make_shared<HttpRequest>();
    http_parser_init(&m_parser);
    m_parser.request_method = onRequestMethod;
    m_parser.request_uri    = onRequestUri;
    m_parser.request_path   = onRequestPath;
    m_parser.fragment       = onRequestFragment;
    m_parser.query_string   = onRequestQueryString;
    m_parser.http_version   = onRequestHttpVersion;
    m_parser.header_done    = onRequesHeaderDone;
    m_parser.http_field     = onRequestHttpField;
}

HttpRequestParser::~HttpRequestParser() { http_parser_finish(&m_parser); }

size_t HttpRequestParser::parse(const char *data, size_t len, size_t offset) { return 0; }

int32_t HttpRequestParser::finished() const { return 0; }

void HttpRequestParser::onRequestMethod(void *data, const char *at, size_t length) {}

void HttpRequestParser::onRequestUri(void *data, const char *at, size_t length) {}

void HttpRequestParser::onRequestPath(void *data, const char *at, size_t length) {}

void HttpRequestParser::onRequestFragment(void *data, const char *at, size_t lengt) {}

void HttpRequestParser::onRequestQueryString(void *data, const char *at, size_t length) {}

void HttpRequestParser::onRequestHttpVersion(void *data, const char *at, size_t length) {}

void HttpRequestParser::onRequesHeaderDone(void *data, const char *at, size_t length) {}

void HttpRequestParser::onRequestHttpField(void *data, const char *field, size_t flen,
                                           const char *value, size_t vlen)
{
}

HttpResponseParser::HttpResponseParser()
{
    m_response = std::make_shared<HttpResponse>();
    httpclient_parser_init(&m_parser);
    m_parser.reason_phrase = onResponseReasonPhrase;
    m_parser.status_code   = onResponseStatusCode;
    m_parser.chunk_size    = onResponseChunkSize;
    m_parser.http_version  = onResponseHttpVersion;
    m_parser.header_done   = onResponseHeaderDone;
    m_parser.last_chunk    = onResponseLastChunk;
    m_parser.http_field    = onResponseHttpField;
}

HttpResponseParser::~HttpResponseParser() { httpclient_parser_finish(&m_parser); }

size_t HttpResponseParser::parse(const char *data, size_t len, size_t offset) { return 0; }

int32_t HttpResponseParser::finished() const { return 0; }

void HttpResponseParser::onResponseReasonPhrase(void *data, const char *at, size_t length) {}

void HttpResponseParser::onResponseStatusCode(void *data, const char *at, size_t length) {}

void HttpResponseParser::onResponseChunkSize(void *data, const char *at, size_t length) {}

void HttpResponseParser::onResponseHttpVersion(void *data, const char *at, size_t length) {}

void HttpResponseParser::onResponseHeaderDone(void *data, const char *at, size_t length) {}

void HttpResponseParser::onRequestHttpVersion(void *data, const char *at, size_t length) {}

void HttpResponseParser::onResponseLastChunk(void *data, const char *at, size_t length) {}

void HttpResponseParser::onResponseHttpField(void *data, const char *field, size_t flen,
                                             const char *value, size_t vlen)
{
}

} // namespace http
} // namespace lon