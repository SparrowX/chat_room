#include<iostream>

#include"Server.h"

using namespace std;

Server::Server(){
	//初始化服务器地址和端口
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	//初始化socket
	listener = 0;

	//epoll fd
	epfd = 0;
}

void Server::Init(){
	cout<<"Init Server..."<<endl;

	//创建监听socket
	listener = socket(PF_INET, SOCK_STREAM, 0);
	if(listener < 0) {
		perror("listener");
		exit(-1);
	}

	//绑定地址
	if(bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("bind error");
		exit(-1);
	}

	//监听
	int ret = listen(listener, 5);
	if(ret < 0){
		perror("listen error");
		exit(-1);
	}

	cout<<"Start to listen:"<<SERVER_IP<<endl;

	//在内核中创建事件表
	epfd = epoll_create(EPOLL_SIZE);

	if(epfd < 0){
		perror("epfd error");
		exit(-1);
	}

	//往事件表里添加监听事件
	addfd(epfd, listener, true);
}

//关闭服务，清理并关闭文件描述符
void Server::Close(){
	close(listener);

	close(epfd);
}


// 发送广播消息给所有客户端
int Server::SendBroadcastMessage(int clientfd){
	//buf[BUF_SIZE] 接收新消息
	//message[BUF_SIZE] 保存格式化的消息
	
	char buf[BUF_SIZE], message[BUF_SIZE];
	bzero(buf, BUF_SIZE);
	bzero(message, BUF_SIZE);

	//接收新消息
	cout<<"read from client(ClientID = "<<clientfd<<")"<<endl;
	int len = recv(clientfd, buf, BUF_SIZE, 0);

	//如果客户端关闭了连接
	if(len == 0){
		close(clientfd);

		//删除客户端
		clients_list.remove(clientfd);
		cout<<"ClientID = "<< clientfd
			<<"closed.\n Now there are"
			<<clients_list.size()
			<<"client in the chat room"
			<<endl;
	}
	//发送广播消息给所有客户端
	else{
		if(clients_list.size() == 1){
			send(clientfd, CAUTION, strlen(CAUTION),0);
			return len;
		}
		//格式化要发送的消息
		sprintf(message, SERVER_MESSAGE, clientfd, buf);

		list<int>::iterator it;
		for(it = clients_list.begin(); it != clients_list.end(); it++){
			if(*it != clientfd){
				if(send(*it, message, BUF_SIZE, 0) < 0){
					return -1;
				}
			}
		}
	}
	
	return len;
}


void Server::Start(){

	//epoll 事件队列
	static struct epoll_event events[EPOLL_SIZE];

	Init();

	while(1){
		int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);

		if(epoll_events_count < 0){
			perror("epoll failure");
			break;
		}

		cout<<"epoll_events_count = \n"<<epoll_events_count<<endl;

		//处理这epoll_events_count个就绪事件
		for(int i = 0; i < epoll_events_count; i++){
			
			int sockfd = events[i].data.fd;
			//新用户连接
			if(sockfd == listener){
				struct sockaddr_in client_address;
				socklen_t client_addrLength = sizeof(struct sockaddr_in);
				int clientfd = accept(listener, (struct sockaddr*)&client_address, &client_addrLength);

				cout<<"Client connection from:"
				//将十进制网络字节序转换成点分十进制IP格式的字符串
					<<inet_ntoa(client_address.sin_addr)<<":"
					//将网络字节顺序转换成主机字节顺序
					<<ntohs(client_address.sin_port)<<",clientfd = "
					<<clientfd<<endl;

				addfd(epfd, clientfd, true);

				//使用list保存用户连接
				clients_list.push_back(clientfd);
				cout<<"Add new clientfd = "<<clientfd<<"to epoll"<<endl;
				cout<<"Now there are "<<clients_list.size()<<"clients in the chat room"<<endl;

				//发送欢迎信息
				cout<<"welcome message"<<endl;
				char message[BUF_SIZE];
				bzero(message, BUF_SIZE);
				sprintf(message, SERVER_WELCOME, clientfd);
				int ret = send(clientfd, message, BUF_SIZE, 0);
				if(ret < 0){
					perror("send error");
					Close();
					exit(-1);
				}

			}
			//处理用户发来的消息，并广播
			else{
				int ret = SendBroadcastMessage(sockfd);
				if(ret < 0){
					perror("error");
					Close();
					exit(-1);
				}
			}
		}
	}

	Close();
}