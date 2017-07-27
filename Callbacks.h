#ifndef SJC_CALLBACKS_H
#define SJC_CALLBACKS_H

#include <memory> // shared_ptr
#include <functional> // funciton bind

namespace sjc
{

class Buffer;
class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void()> TimerCallback;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&,
                           Buffer* buffer)> MessageCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;

}

#endif // SJC_CALLBACKS_H
