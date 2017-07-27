#ifndef SJC_HTTPCONTEXT_H
#define SJC_HTTPCONTEXT_H

#include "Copyable.h"
#include "HttpRequest.h"

#include <utility>

namespace sjc
{

class Buffer;
// class HttpRequest;

class HttpContext : public copyable {
public:
  enum HttpRequestParseState {
    kExpectRequestLine,
    kExpectHeaders,
    kGotAll,
  };

  HttpContext()
    : state_(kExpectRequestLine) { }

  bool parseRequest(Buffer* buf);

  bool gotAll() const { return state_ == kGotAll; }

  void reset() {
    state_ = kExpectRequestLine;
    HttpRequest dummy;
    std::swap(request_, dummy);
  }

  HttpRequest& request() { return request_; }

private:
  bool processRequestLine(const char* begin, const char* end);

  HttpRequestParseState state_;
  HttpRequest request_;
};

}

#endif // SJC_HTTPCONTEXT_H
