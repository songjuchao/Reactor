#include "HttpContext.h"
// #include "HttpRequest.h"
#include "Buffer.h"

#include <algorithm>

using namespace sjc;

// 请求行
// 方法 URI HTTP版本
bool
HttpContext::processRequestLine(const char* begin, const char* end) {
  bool succeed = false;
  const char* start = begin;
  const char* space = std::find(start, end, ' ');
  if (space != end && request_.setMethod(start, space)) { // 获取方法
    start = space + 1;
    space = std::find(start, end, ' '); // 获取URI
    if (space != end) {
      request_.setPath(start, space);
    }
    start = space + 1;
    succeed = (end - start == 8) && (std::equal(start, end - 1, "HTTP/1."));
    if (succeed) {
      if (*(end - 1) == '0')
        request_.setVersion(HttpRequest::kHttp10);
      else if (*(end - 1) == '1')
        request_.setVersion(HttpRequest::kHttp11);
      else
        succeed = false;
    }
  }
  return succeed;
}

bool
HttpContext::parseRequest(Buffer* buf) {
  bool requestRet = true; // 请求行是否完整
  bool hasMore = true;
  while (hasMore) {
    if (state_ == kExpectRequestLine) {
      const char* crlf = buf->findCRLF();
      if (crlf) {
        requestRet = processRequestLine(buf->peek(), crlf);
        buf->retrieveUntil(crlf + 2);
        state_ = kExpectHeaders;
      } else {
        hasMore = false;
      }
    } else if (state_ == kExpectHeaders) {
      const char* crlf = buf->findCRLF();
      if (crlf) {
        const char* colon = std::find(buf->peek(), crlf, ':');
        if (colon != crlf) {
          request_.addHeader(buf->peek(), colon, crlf);
        } else { // colon == crlf 结束
          hasMore = false;
          state_ = kGotAll;
        }
        buf->retrieveUntil(crlf + 2);
      } else { // 没找到换行回车 消息接受不完整
        hasMore = false;
      }
    }

  }
  return requestRet;
}
