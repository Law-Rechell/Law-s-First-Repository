#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#define WIN32_LEAN_AND_MEAN//该宏定义会避免引入一些早期的容易引起冲突的库

#ifdef _WIN32
	#define FD_SETSIZE      1024
	#include<windows.h>
	#include<WinSock2.h>
#else
	#include<unistd.h>	//uni std
	#include<arpa/inet.h>
	#include<string.h>

	#define SOCKET int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR			(-1)
#endif

#include<iostream>
#include<vector>//动态数组头文件
#include"MessageHeader.hpp"
using namespace std;

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240//缓冲区最小单元大小
#endif // !RECV_BUFF_SIZE

class  ClientSocket
{
public:
	ClientSocket(SOCKET sockfd=INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}

	char* msgBuf()
	{
		return _szMsgBuf;
	}

	int getLastPos()
	{
		return _lastPos;
	}

	void setLastPos(int pos)
	{
		_lastPos = pos;
	}

private:
	SOCKET _sockfd;//fd_set
	//第二缓冲区，消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	//消息缓冲区的尾部位置
	int _lastPos;
};

class EasyTcpServer
{
private:
	SOCKET _sock;
	vector<ClientSocket*> _clients;//全局动态数组,用指针避免爆栈,栈空间比较小(几兆)，堆内存比较大

public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}

	virtual~EasyTcpServer()
	{
		Close();
	}

	//初始化socket
	SOCKET InitSocket()
	{
		//启动Win Sock 2.x环境
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);//版本号WinSock2,所以填2.2
		WSADATA dat;//lpWSAData，基本上lp开头的把lp去掉就好
		WSAStartup(ver, &dat);//Windows Socket启动函数
#endif

		if (INVALID_SOCKET != _sock)
		{
			cout << "socket " << _sock << ":关闭旧的链接" << endl;
			Close();
		}
		//1.建立一个socket
		_sock = socket(AF_INET, SOCK_STREAM, 0);//这里不用填tcp
		if (INVALID_SOCKET == _sock)
		{
			cout << "ERROR : 建立套接字失败" << endl;
		}
		else {
			cout << "建立套接字成功！" << endl;
		}

		return _sock;
	}

	//绑定IP地址和端口号
	int Bind(char* ip,unsigned short port)
	{
		//绑定接受客户端连接的端口bind
		sockaddr_in _sin = {};//转到查看定义，进行初始化
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsigned short的缩写，因为网络端口跟主机端口不是同一事物
#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		//socket对象(_sock),sockaddr地址对象(sockaddr_in代替)，长度(sizeof(_sin))
		if (SOCKET_ERROR == ret)//绑定有可能失败
		{
			cout << "ERROR : 绑定端口:" << port << "失败" << endl;
		}
		else {
			cout << "绑定端口:"<<port<<" 成功！" << endl;
		}

		return ret;
	}

	//监听端口号
	int Listen(int n)
	{
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)//监听有可能失败
		{
			cout << "ERROR : 监听网络端口失败" << endl;
		}
		else {
			cout << "监听网络端口成功！" << endl;
		}
		return ret;
	}

	//接受客户端链接
	SOCKET Accept()
	{
		//4.等待接受客户端连接accept
		sockaddr_in clientAddr = {};//客户端地址
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;//客户端socket对象
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif // _WIN32

		if (INVALID_SOCKET == cSock)
		{
			cout << "ERROR : 接受到无效客户端" << endl;
		}
		else
		{
			//新加入的socket用户群发一条消息
			//NewUserJoin userJoin;
			//SendDataToAll(&userJoin);

			_clients.push_back(new ClientSocket(cSock));
			cout << "新客户端接入，IP地址是：" << inet_ntoa(clientAddr.sin_addr) << endl;//转为IP地址打印
		}

		return cSock;
	}

	//关闭socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];//new了对象出来，关闭时记得删掉
			}
			//关闭socket
			closesocket(_sock);
			//=====================================================================
			WSACleanup();//有startup就有cleanup
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
				delete _clients[n];//new了对象出来，关闭时记得删掉
			}
			close(_sock);
#endif
			_clients.clear();
		}
	}

	int numCount = 0;
	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;

			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);//清空计数

			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);
			SOCKET maxSock = _sock;
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock < _clients[n]->sockfd())
				{
					maxSock = _clients[n]->sockfd();
				}
			}

			//nfds是一个整数值,指fd_set集合中所有socket的范围而不是数量
			//即是所有socket最大值+1，Windows中无所谓
			//select分为阻塞和非阻塞模式，最后一个参数填NULL表示阻塞
			timeval t = { 1,0 };//F12查看定义，了解怎么初始化
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			//cout <<"select ret = "<<ret<<"   count = "<<numCount++<< endl;
			if (ret < 0)
			{
				cout << "select任务失败" << endl;
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))//如果_sock已经插入到fdRead中
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}

			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
				{
					if (-1 == RecvData(_clients[n]))//客户端失效了，要想办法移除
					{
						auto iter = _clients.begin() + n;
						if (iter != _clients.end())//找到了
						{
							delete _clients[n];
							_clients.erase(iter);//移除失效客户端
						}
					}
				}
			}
			return true;
		}
		return false;
	}

	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//使用一个缓冲来接受数据
	char _szRecv[RECV_BUFF_SIZE] = {};

	//接受数据，处理粘包，拆分包
	int RecvData(ClientSocket* pClient)
	{
		//4.1.接受客户端的请求数据
		int nlen = recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);//(int)
		//cout << "数据长度：" << nlen << endl;
		
		if (nlen <= 0)//recv接受到数据时，长度会大于0
		{
			cout << "ERROR : 接受客户端消息失败" << endl;
			return -1;//客户端退出，结束循环
		}
		//将收取到的数据拷贝到消息缓冲区
		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nlen);
		//消息缓冲区的尾部位置后移
		pClient->setLastPos(pClient->getLastPos()+nlen);
		//判断消息缓冲区的数据长度是否大于消息头DataHeader长度
		while (pClient->getLastPos() >= sizeof(DataHeader))//处理粘包少包问题
		{
			//这时就可以知道当前消息体的长度
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//判断消息缓冲区的数据长度是否大于消息长度
			if (pClient->getLastPos() >= header->dataLength)
			{
				//消息缓冲区剩余未处理数据的长度
				int nSize = pClient->getLastPos() - header->dataLength;
				//处理网络消息
				OnNetMsg(pClient->sockfd(),header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//消息缓冲区的尾部位置前移
				pClient->setLastPos(nSize);
			}
			else
			{
				//剩余消息不够一条完整消息
				break;
			}
		}
		return 0;
	}

	//响应网络消息
	virtual void OnNetMsg(SOCKET cSock,DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			//加减偏移量,上面已经把消息头接收掉了
			
			Login* login = (Login*)header;
			cout << "收到命令：Login & 数据长度：" << login->dataLength << "用户名："
				<< login->userName << " 密码：" << login->passWord << endl;
			//忽略判断用户名是否正确的过程
			LoginResult loginRet;
			SendData(cSock, &loginRet);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			cout << "收到命令：Logout & 数据长度：" << logout->dataLength << "用户名："
				<< logout->userName << endl;
			//忽略判断用户名是否正确的过程
			LogoutResult logoutRet;
			SendData(cSock, &logoutRet);
		}
		break;
		default:
		{
			cout << "未知消息数据长度：" << header->dataLength << endl;
			DataHeader header;
			SendData(cSock, &header);
		}
		break;
		}
	}

	//发送指定socket数据
	int SendData(SOCKET cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	//群发socket数据
	void SendDataToAll( DataHeader* header)
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			SendData(_clients[n]->sockfd(), header);
		}
	}

};

#endif // _EasyTcpServer_hpp_
