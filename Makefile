OBJ = Buffer.o Channel.o EventLoop.o Poller.o SocketsOps.o \
			InetAddress.o Socket.o Acceptor.o \
			TcpConnection.o TcpServer.o \
			EventLoopThread.o EventLoopThreadPool.o
CC = g++
CXXFLAGS = -g -std=c++11 -pthread

test1 : $(OBJ) test1.cc
	$(CC) $(CXXFLAGS) -o test1 test1.cc $(OBJ)

test2 : $(OBJ) test2.cc
	$(CC) $(CXXFLAGS) -o test2 test2.cc $(OBJ)

test3 : $(OBJ) test3.cc
	$(CC) $(CXXFLAGS) -o test3 test3.cc $(OBJ)

test4 : $(OBJ) test4.cc
	$(CC) $(CXXFLAGS) -o test4 test4.cc $(OBJ)

test5 : $(OBJ) test5.cc
	$(CC) $(CXXFLAGS) -o test5 test5.cc $(OBJ)

test6 : $(OBJ) test6.cc
	$(CC) $(CXXFLAGS) -o test6 test6.cc $(OBJ)

Buffer.o : Buffer.h
Channel.o : Channel.h EventLoop.h
EventLoop.o : EventLoop.h
Poller.o : Poller.h
SocketsOps.o : SocketsOps.h
InetAddress.o : InetAddress.h SocketsOps.h
Socket.o : Socket.h InetAddress.h SocketsOps.h
Accetor.o : Accetor.h EventLoop.h Channel.h InetAddress.h Socket.h SocketsOps.h
TcpConnection.o : TcpConnection.h EventLoop.h Channel.h InetAddress.h Socket.h \
									SocketsOps.h Callbacks.h Noncopyable.h
TcpServer.o : TcpServer.h TcpConnection.h EventLoop.h Acceptor.h Callbacks.h \
							Noncopyable.h
EventLoopThread.o : EventLoopThread.h EventLoop.h
EventLoopThreadPool.o : EventLoopThreadPool.h EventLoopThread.h EventLoop.h

clean:
	rm $(OBJ)

.PHONY :
	clean
