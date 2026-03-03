#include "http/httpresponse.h"

namespace lon
{
namespace http
{
HttpResponse::HttpResponse(uint8_t version, bool close)
    : HttpMessage(version, close), m_status(HttpStatus::OK),
      m_reason(HttpStatusConverter::toString(m_status))
{
}

HttpResponse::~HttpResponse() {}

HttpStatus HttpResponse::getStatus() const { return m_status; }

const std::string &HttpResponse::getReason() const { return m_reason; }

void HttpResponse::setStatus(HttpStatus status)
{
    m_status = status;
    m_reason = HttpStatusConverter::toString(m_status);
}

void HttpResponse::setReason(const std::string &reason)
{
    m_reason    = reason;
    auto status = HttpStatusConverter::fromString(reason);
    m_status    = status == HttpStatus::UNKNOWN ? m_status : status;
}

std::ostream &HttpResponse::toString(std::ostream &os) const
{
    os << "HTTP/" << (uint32_t)(m_version >> 4) << "." << (uint32_t)(m_version & 0x0F) << " "
       << (uint32_t)m_status << " "
       << (m_reason.empty() ? HttpStatusConverter::toString(m_status) : m_reason) << "\r\n";

    if (!m_is_websocket)
    {
        os << "Connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
    }
    for (const auto &header : m_headers)
    {
        if (!m_is_websocket && util::toLower(header.first) == "connection")
        {
            continue;
        }
        os << header.first << ": " << header.second << "\r\n";
    }
    if (!m_body.empty())
    {
        os << "Content-Length: " << m_body.size() << "\r\n\r\n";
        os << m_body;
    }
    else
    {
        os << "\r\n";
    }

    return os;
}

std::string HttpResponse::toString() const
{
    std::stringstream ss;
    toString(ss);
    return ss.str();
}

} // namespace http
} // namespace lon