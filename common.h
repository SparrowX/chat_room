#ifndef CHATROOM_COMMON_H
#define CHATROOM_COMMON_H

#include<iostream>
#include<list>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
using namespace std;

#define SERVER_IP "127.0.0.1"

#define SERVER_PORT 8888

#define EPOLL_SIZE 5000

#define BUF_SIZE 0xFFFF

#define SERVER_WELCOME "Welcome you join to the chat room! Your chat ID is: Client #%d"

#define SERVER_MESSAGE "ClientID %d say >> %s"

#define EXIT "EXIT"

#define CAUTION "There is only one in the chat room"

//注册新的fd到epollfd中
//enable_et表示是否启用ET模式，true为启用，否则使用LT模式
static void addfd(int epollfd, int fd, bool enable_et){
	struct epoll_event ev;
	ev.data.fd =fd;
	ev.events = EPOLLIN;
	if(enable_et) ev.events = EPOLLIN | EPOLLET;
	
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
	//设置socket为nonblocking模式
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
	cout<<"fd added to epoll!\n\n";
}

#endif 