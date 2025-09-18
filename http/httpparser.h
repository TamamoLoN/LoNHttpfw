#pragma once

#include "http/httpglobalconfig.h"
#include "http/httprequest.h"
#include "http/httpresponse.h"
#include "http11_parser.h"
#include "httpclient_parser.h"

namespace lon
{
namespace http
{
class HttpParser
{
  public:
    HttpParser()                                                          = default;
    virtual ~HttpParser()                                                 = default;
    virtual size_t parse(const char *data, size_t len, size_t offset = 0) = 0;
    virtual int32_t finished() const                                      = 0;
    int32_t error() const;
    void setError(int32_t error);

  protected:
    int32_t m_error;
};

class HttpRequestParser : public HttpParser
{
  public:
    using Ptr = std::shared_ptr<HttpRequestParser>;
    HttpRequestParser();
    ~HttpRequestParser();
    size_t parse(const char *data, size_t len, size_t offset = 0) override;
    int32_t finished() const override;

  private:
    static void onRequestMethod(void *data, const char *at, size_t length);
    static void onRequestUri(void *data, const char *at, size_t length);
    static void onRequestPath(void *data, const char *at, size_t length);
    static void onRequestFragment(void *data, const char *at, size_t length);
    static void onRequestQueryString(void *data, const char *at, size_t length);
    static void onRequestHttpVersion(void *data, const char *at, size_t length);
    static void onRequesHeaderDone(void *data, const char *at, size_t length);
    static void onRequestHttpField(void *data, const char *field, size_t flen, const char *value,
                                   size_t vlen);

  private:
    http_parser m_parser;
    HttpRequest::Ptr m_request;
};

class HttpResponseParser : public HttpParser
{
  public:
    using Ptr = std::shared_ptr<HttpResponseParser>;
    HttpResponseParser();
    ~HttpResponseParser();
    size_t parse(const char *data, size_t len, size_t offset = 0) override;
    int32_t finished() const override;

  private:
    static void onResponseReasonPhrase(void *data, const char *at, size_t length);
    static void onResponseStatusCode(void *data, const char *at, size_t length);
    static void onResponseChunkSize(void *data, const char *at, size_t length);
    static void onResponseHttpVersion(void *data, const char *at, size_t length);
    static void onResponseHeaderDone(void *data, const char *at, size_t length);
    static void onRequestHttpVersion(void *data, const char *at, size_t length);
    static void onResponseLastChunk(void *data, const char *at, size_t length);
    static void onResponseHttpField(void *data, const char *field, size_t flen, const char *value,
                                    size_t vlen);

  private:
    httpclient_parser m_parser;
    HttpResponse::Ptr m_response;
};

} // namespace http
} // namespace lon