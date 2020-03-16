#ifndef CHATROOM_SERVER_H
#define CHATROOM_SERVER_H

#include<string>
#include"common.h"

using namespace std;

class Server{
public:
	Server();

	void Init();

	void Close();

	void Start();

private:
	//广播消息给所有客户端
	int SendBroadcastMessage(int clientfd);

	//服务器端的serverAddr
	struct sockaddr_in serverAddr;

	//创建监听的socket
	int listener;

	//epoll_create创建后的返回信息
	int epfd;

	//客户端列表
	list<int> clients_list;
};

#endif