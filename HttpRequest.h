#ifndef SJC_HTTPREQUEST_H
#define SJC_HTTPREQUEST_H

#include "Copyable.h"

#include <map>
#include <string>

#include <assert.h>
#include <stdio.h>
#include <ctype.h>

namespace sjc
{

class HttpRequest : copyable {
public:
  enum Method {
    kInvalid,
    kGet, // 申请获取资源
    kPost, // 客户端想服务器提交数据的方法 这种方法可能会影响服务器
    kHead, // 和GET方法类似 不过仅要求服务器返回头部信息
    kPut, // 上传某个资源
    kDelete, // 删除某个资源
    kTrace, // 要求目标服务器返回原始http请求的内容
    kOptions, //
    kConnection, //
    kPatch, // 对某个资源做部分修改
  };
  enum Version {
    kUnkonw,
    kHttp10,
    kHttp11,
  };

  HttpRequest()
    : method_(kInvalid),
      version_(kUnkonw) { }

  bool setMethod(const char* start, const char* end) {
    assert(method_ == kInvalid);
    std::string m(start, end);
    if (m == "GET") {
      method_ = kGet;
    } else if (m == "POST") {
      method_ = kPost;
    } else if (m == "HEAD") {
      method_ = kHead;
    } else if (m == "PUT") {
      method_ = kPut;
    } else if (m == "DELETE") {
      method_ = kDelete;
    } else if (m == "TRACE") {
      method_ = kTrace;
    } else if (m == "OPTIONS") {
      method_ = kOptions;
    } else if (m == "CONNECTION") {
      method_ = kConnection;
    } else if (m == "PATCH") {
      method_ = kPatch;
    } else {
      method_ = kInvalid;
    }

    return method_ != kInvalid;
  }

  Method method() const { return method_; }

  const char* methodString() const {
    const char* result = "UNKONWN";
    switch (method_) {
      case kGet:
      result = "GET";
      break;
      case kPost:
      result = "POST";
      break;
      case kHead:
      result = "HEAD";
      break;
      case kPut:
      result = "PUT";
      break;
      case kDelete:
      result = "DELETE";
      break;
      case kTrace:
      result = "TRACE";
      break;
      case kOptions:
      result = "OPTIONS";
      break;
      case kConnection:
      result = "CONNECTION";
      break;
      case kPatch:
      result = "PATHC";
      break;
    }
    return result;
  }

  void setVersion(Version version) { version_ = version; }
  Version version() const { return version_; }

  void setPath(const char* start, const char* end) {
    path_.assign(start, end);
  }

  const std::string& path() const { return path_; }

  void addHeader(const char* start, const char* colon, const char* end) {
    std::string field(start, colon);
    ++colon;
    while (colon < end && isspace(*colon)) {
      ++colon;
    }

    std::string value(colon, end);
    while (!value.empty() && isspace(value[value.size() - 1])) {
      value.resize(value.size() - 1); // 去除末尾空格
    }

    headers_[field] = value;
  }

  std::string getHeader(const std::string& field) const {
    std::string ret;
    auto iter = headers_.find(field);
    if (iter != headers_.end()) {
      ret = iter->second;
    }

    return ret;
  }

  const std::map<std::string, std::string>& headers() const { return headers_; }

private:
  Method method_;
  Version version_;
  std::string path_;
  std::map<std::string, std::string> headers_;
};

}

#endif // SJC_HTTPREQUEST_H
