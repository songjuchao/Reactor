#ifndef SJC_HTTPRESPONSE_H
#define SJC_HTTPRESPONSE_H

#include "Copyable.h"

#include <map>
#include <string>

namespace sjc
{

class Buffer;

class HttpResponse : public copyable {
public:
  enum HttpStatusCode {
    kUnkonw,
    k200Ok = 200, // 请求成功
    k301MovedPermanetly = 301, // 资源被转移 请求将被重定向
    k400BadRequest = 400, // 客户端请求错误
    k403Forbidden = 403, // 访问被服务器禁止 通常是没有足够权限
    k404NotFound = 404, // 资源没找到
  };

  explicit HttpResponse(bool close)
    : statusCode_(kUnkonw),
      closeConnection_(close) { }

  void setStatusCode(HttpStatusCode statusCode) { statusCode_ = statusCode; }
  void setStatusMessage(const std::string& statusMessage) {
    statusMessage_ = statusMessage_;
  }
  void setCloseConnection(bool on) { closeConnection_ = on; }
  bool closeConnection() { return closeConnection_; }
  void setContextType(const std::string& contextType) {
    addHeader("Content-Type", contextType);
  }
  void addHeader(const std::string& key, const std::string& value) {
    headers_[key] = value;
  }
  void setBody(const std::string& body) { body_ = body; }
  void addToString(std::string& output);

private:
  HttpStatusCode statusCode_;
  std::string statusMessage_; // 状态行信息
  std::map<std::string, std::string> headers_;
  bool closeConnection_;
  std::string body_;
};

}

#endif // SJC_HTTPRESPONSE_H
