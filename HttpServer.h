#ifndef SJC_HTTPSERVER_H
#define SJC_HTTPSERVER_H

#include "Noncopyable.h"
#include "TcpServer.h"
#include "HttpContext.h"

namespace sjc
{

class HttpRequest;
class HttpResponse;
// class HttpContext;

class HttpServer : noncopyable {
public:
  typedef std::function<void(const HttpRequest&, HttpResponse*)> HttpCallback;

  HttpServer(EventLoop* loop,
             const InetAddress& listenAddr);
             //TcpServer::Option option = TcpServer::kNoReusePort);

  void start();

  void setHttpCallback(const HttpCallback& httpCallback) {
    httpCallback_ = httpCallback;
  }
  void setThreadNum(int numThread) { server_.setThreadNum(numThread); }

private:
  void onConnection(const TcpConnectionPtr& conn);
  void onMessage(const TcpConnectionPtr& conn, Buffer* buf);
  void onRequest(const TcpConnectionPtr&, const HttpRequest&);

  TcpServer server_;
  HttpCallback httpCallback_;
  HttpContext context_;
};

}

#endif // SJC_HTTPSERVER_H
