#include<iostream>
#include"Client.h"

using namespace std;

//客户端构造函数
Client::Client(){
	
	serverAddr.sin_family = PF_INET;
	//htons 将端口号由主机字节序转换为网络字节序
	serverAddr.sin_port = htons(SERVER_PORT);
	//inet_addr 将IP字符串转化为网络字节序
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	//初始化socket
	sock = 0;

	//初始化进程号
	pid = 0;

	//状态
	isClientwork = true;

	//epoll fd
	epfd = 0;
}

void Client::Connect(){
	cout<<"Connect Server:"<<SERVER_IP<<":"<<SERVER_PORT<<endl;

	//创建socket
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		//输出标准错误信息到stderr
		perror("sock error");
		exit(-1);
	}

	//连接服务器
	if(connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
		perror("connect error");
		exit(-1);
	}

	//创建管道，fd[0]用于父进程读，fd[1]用于子进程写
	if(pipe(pipe_fd) < 0){
		perror("pipe error");
		exit(-1);
	}

	//创建epoll
	epfd = epoll_create(EPOLL_SIZE);

	if(epfd < 0){
		perror("epfd error");
		exit(-1);
	}

	//将sock和管道读端描述符都添加到内核事件表
	addfd(epfd, sock, true);
	addfd(epfd, pipe_fd[0], true);
}


//断开连接，清理并关闭文件描述符
void Client::Close(){
	if(pid){
		//关闭父进程的管道和sock
		close(pipe_fd[0]);
		close(sock);
	}
	else{
		//关闭子进程的管道
		close(pipe_fd[1]);
	}
}

void Client::Start(){

	//epoll 事件队列
	static struct epoll_event events[2];

	//连接服务器
	Connect();

	pid = fork();

	if(pid < 0){
		perror("fork error");
		close(sock);
		exit(-1);
	}
	//子进程
	else if(pid == 0){//子进程负责写入管道
		//关闭读端
		close(pipe_fd[0]);

		//输入exit可以退出聊天室
		cout<<"Please input 'exit' to exit the chat room"<<endl;

		//如果客户端运行正常,不断读取输入发送给服务端
		while(isClientwork){
			//将字符串的前n个字节置0
			bzero(&message, BUF_SIZE);
			//从指定流中读取一行数据
			fgets(message, BUF_SIZE, stdin);

			//客户输入exit，退出
			//比较前n个字符，忽略大小写的差异
			if(strncasecmp(message, EXIT, strlen(EXIT)) == 0){
				isClientwork = 0;
			}
			//将信息写入管道
			else{
				if(write(pipe_fd[1], message, strlen(message) - 1) < 0){
					perror("fork error");
					exit(-1);
				}
			}
		}

	}
	else{
		//父进程负责读管道数据，关闭写端
		close(pipe_fd[1]);

		while(isClientwork){
			int epoll_events_count = epoll_wait(epfd, events, 2, -1);

			//处理就绪事件
			for(int i = 0; i < epoll_events_count; i++){
				bzero(&message, BUF_SIZE);

				//服务器端发来消息
				if(events[i].data.fd == sock){
					//接受服务端消息
					int ret = recv(sock, message, BUF_SIZE, 0);
					//ret = 0 服务端关闭
					if(ret == 0){
						cout<<"Server closed connection:"<<sock<<endl;
						close(sock);
						isClientwork = 0;
					}
					else{
						cout<<message<<endl;
					}
				}
				//子进程写入事件发生
				else{
					//从管道读取数据
					int ret = read(events[i].data.fd, message, BUF_SIZE);

					//ret =0
					if(ret == 0) isClientwork = 0;
					else send(sock, message, BUF_SIZE, 0);
				}
			}//for
		}//while
	}

	//父进程和子进程都退出，close
	Close();
}