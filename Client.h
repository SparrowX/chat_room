#ifndef CHATROOM_CLIENT_H
#define CHATROOM_CLIENT_H

#include<string>
#include"common.h"

using namespace std;

//连接服务器发送和接受消息
class Client{

public:
	Client();

	void Connect();

	void Close();

	void Start();

private:

	//连接服务器端创建的socket
	int sock;

	int pid;

	//epoll_create创建后的返回值
	int epfd;

	//创建管道，fd[0]用于父进程读，fd[1]用于子进程写
	int pipe_fd[2];

	//表示客户端是否正常工作
	bool isClientwork;

	//聊天信息缓冲
	char message[BUF_SIZE];

	//服务器IP+port
	struct sockaddr_in serverAddr;
};

#endif