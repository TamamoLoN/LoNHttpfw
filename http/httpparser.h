#pragma once

#include "http/httpglobalconfig.h"
#include "http/httprequest.h"
#include "http/httpresponse.h"
#include "mongrel2-http11/http11_parser.h"
#include "mongrel2-http11/httpclient_parser.h"

namespace lon
{
namespace http
{
enum class HttpParserError
{
    OK             = 0,
    UNKNOWN_METHOD = 0x1000,
    UNKNOWN_VERSION,
    UNKNOWN_STATUS,
    INVALID_FIELD,
};

class HttpParser
{
  public:
    HttpParser();
    virtual ~HttpParser()                                               = default;
    virtual ssize_t execute(char *data, size_t len, bool chunk = false) = 0;
    virtual int32_t finished()                                          = 0;
    virtual int32_t error()                                             = 0;
    HttpMessage::Ptr parse(char *data, size_t len);
    size_t getContentLength() const;
    void setError(int32_t error);
    HttpMessage::Ptr getData() const;

  protected:
    HttpMessage::Ptr m_data;
    int32_t m_error;
};

class HttpRequestParser : public HttpParser
{
  public:
    using Ptr = std::shared_ptr<HttpRequestParser>;
    HttpRequestParser();
    ~HttpRequestParser();
    /**
     * @brief 解析HTTP请求
     * @param data 待解析的数据
     * @param len 待解析数据的长度
     * @return size_t 实际解析了多少，-1表示出错, 1表示成功,
     * >0表示已处理的字节数
     */
    ssize_t execute(char *data, size_t len, bool chunk = false) override;
    int32_t finished() override;
    int32_t error() override;
    const http_parser &getParser() const;

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
    HttpRequest::Ptr m_request;
    http_parser m_parser;
};

class HttpResponseParser : public HttpParser
{
  public:
    using Ptr = std::shared_ptr<HttpResponseParser>;
    HttpResponseParser();
    ~HttpResponseParser();
    /**
     * @brief 解析HTTP响应
     * @param data 待解析的数据
     * @param len 待解析数据的长度
     * @return size_t 实际解析了多少，-1表示出错, 1表示成功,
     * >0表示已处理的字节数
     */
    ssize_t execute(char *data, size_t len, bool chunk = false) override;
    int32_t finished() override;
    int32_t error() override;
    const httpclient_parser &getParser() const;

  private:
    static void onResponseReasonPhrase(void *data, const char *at, size_t length);
    static void onResponseStatusCode(void *data, const char *at, size_t length);
    static void onResponseChunkSize(void *data, const char *at, size_t length);
    static void onResponseHttpVersion(void *data, const char *at, size_t length);
    static void onResponseHeaderDone(void *data, const char *at, size_t length);
    static void onResponseLastChunk(void *data, const char *at, size_t length);
    static void onResponseHttpField(void *data, const char *field, size_t flen, const char *value,
                                    size_t vlen);

  private:
    HttpResponse::Ptr m_response;
    httpclient_parser m_parser;
};

} // namespace http
} // namespace lon