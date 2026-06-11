#pragma once

#include "http/httpmessage.h"

namespace lon
{
namespace http
{
class LON_API HttpResponse : public HttpMessage
{
  public:
    using Ptr = std::shared_ptr<HttpResponse>;
    HttpResponse(uint8_t version = 0x11, bool close = true);
    ~HttpResponse();

    HttpStatus getStatus() const;
    const std::string &getReason() const;

    void setStatus(HttpStatus status);
    void setReason(const std::string &reason);

    /**
     * @brief 将HttpResponse对象转换为字符串
     * @param os 输出流
     * @return std::ostream& 返回输出流引用，方便链式调用
     * @note 输出格式:
     * HTTP/1.1 200 OK
     * ...
     */
    std::ostream &toString(std::ostream &os) const override;
    std::string toString() const override;

  private:
    HttpStatus m_status;
    std::string m_reason;
};
} // namespace http
} // namespace lon