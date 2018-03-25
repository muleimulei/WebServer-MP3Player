# WebServer-MP3Player
一个基于cs的多线程音乐播放器及其客户端实现(实现在Ubuntu操作系统上，集成开发环境为QT，可直接导入)


### 运行服务器端
`./server <port>`

### 运行客户端
`./client <hostname> <port>`

> hostname必须是点分十进制形式

## 编译服务器端
`g++ main.cpp clientthread.cpp server.cpp -lpthread -o sever`

## 编译客户端
`g++ main.cpp client.cpp -lpthread -o client`
