#include "HttpResponse.h"
#include "Buffer.h"

using namespace sjc;

void
HttpResponse::addToString(std::string& output) {
  char buf[32];
  snprintf(buf, sizeof buf, "HTTP/1.1 %d", statusCode_); // 响应报文的状态行
  output.append(buf); // 添加响应报文的状态行 版本 状态码
  output.append(statusMessage_); // 状态行的信息 etc. Not Found
  output.append("\r\n");

  if (closeConnection_) {
    output.append("Connection: close\r\n");
  } else {
    snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
    // 根据body_长度填充大小
    output.append(buf);
    output.append("Connection: Keep-Alive\r\n");
  }

  for (auto iter = headers_.begin(); iter != headers_.end(); ++iter) {
    output.append(iter->first);
    output.append(": ");
    output.append(iter->second);
    output.append("\r\n");
  }
  // 添加http首部字段

  output.append("\r\n");
  output.append(body_);
}
