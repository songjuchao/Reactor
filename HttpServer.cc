#include "HttpServer.h"
// #include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include <iostream>

using namespace sjc;

void
defaultHttpCallback(const HttpRequest&, HttpResponse* resp) {
  resp->setStatusCode(HttpResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop* loop,
                       const InetAddress& listenAddr)
                       //TcpServer::Option option)
                         : server_(loop, listenAddr) {
  setHttpCallback(defaultHttpCallback);
  server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this,
    std::placeholders::_1));
  server_.setMessageCallback(std::bind(&HttpServer::onMessage, this,
    std::placeholders::_1, std::placeholders::_2));
}

void
HttpServer::start() {
  std::clog << "HttpServer[" << server_.name() << "] starts listening on\n";
  server_.start();
}

void
HttpServer::onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    std::cout << "New connection from [" << conn->peerAddr().toHostPort() << "]"
              << std::endl;
  }
}

void
HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf) {
  std::string str(buf->peek(), buf->readableBytes());
  std::clog << str << std::endl; // 获取的请求正确
  if (!context_.parseRequest(buf)) {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
  }

  if (context_.gotAll()) {
    onRequest(conn, context_.request());
    context_.reset();
  }
}

void
HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req) {
  const std::string& connection = req.getHeader("Connection");
  bool close = (connection == "close") ||
               (req.version() == HttpRequest::kHttp10
                && connection != "Keep-Alive");
  HttpResponse response(close);
  httpCallback_(req, &response);
  // response.appendToBuffer(&buf);
  // conn->send(&buf);
  std::string str;
  response.addToString(str);
  conn->send(str);
  if (response.closeConnection()) {
    conn->shutdown();
  }
}
