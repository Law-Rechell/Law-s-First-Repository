#ifndef _EasyTcpClient_hpp_//确保宏定义只会被编译一次
#define _EasyTcpClient_hpp_

#ifdef _WIN32
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
#include<thread>//c++线程库
#include"MessageHeader.hpp"
using namespace std;

class EasyTcpClient
{
	SOCKET _sock;
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	}

	virtual ~EasyTcpClient()//习惯：析构函数用虚函数
	{
		Close();
	}

	//初始化socket函数
	void InitSocket()
	{
		//启动Win Sock 2.x环境
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);//版本号WinSock2,所以填2.2
		WSADATA dat;//lpWSAData，基本上lp开头的把lp去掉就好
		WSAStartup(ver, &dat);//Windows Socket启动函数
#endif

		if (INVALID_SOCKET != _sock)
		{
			cout <<"socket "<< _sock <<":关闭旧的链接" << endl;
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
	}

	//连接服务器函数
	int Connect(char* ip,unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		//2.连接服务器connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(_sin));//另一种写法
		if (SOCKET_ERROR == ret)
		{
			cout << "ERROR : 连接服务器失败" << endl;
		}
		else
		{
			cout << "连接服务器成功！" << endl;
		}

		return ret;
	}

	//关闭socket函数
	void Close()
	{
		//关闭Win Sock 2.x环境
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			//4.关闭socket closesocket
			closesocket(_sock);
			//=====================================================================
			WSACleanup();//有startup就有cleanup
#else
			close(_sock);
#endif
		}
		_sock = INVALID_SOCKET;
	}
	
	int numCount = 0;
	//查询网络消息
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 0,0 };
			int ret = select(_sock + 1, &fdReads, 0, 0, &t);
			//cout << "select ret = " << ret << "   count = " << numCount++ << endl;
			if (ret < 0)
			{
				cout << "socket " << _sock << ":select任务结束1。" << endl;
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock))
				{
					cout << "socket " << _sock << ":select任务结束2。" << endl;//做一个区分
					Close();
					return false;
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

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240//缓冲区最小单元大小
#endif // !RECV_BUFF_SIZE

	//接收缓冲区
	char _szRecv[RECV_BUFF_SIZE] = {};//双缓冲，socket本身有一个接受和发送的缓冲区
	//第二缓冲区，消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	//消息缓冲区的尾部位置
	int _lastPos = 0;

	//接受数据,处理粘包拆包
	int RecvData(SOCKET cSock)
	{
		//4.1.接受客户端的请求数据
		int nlen = recv(cSock, _szRecv, RECV_BUFF_SIZE, 0);
		//cout << "数据长度：" << nlen << endl;
		if (nlen <= 0)//recv接受到数据时，长度会大于0
		{
			cout << "ERROR : 与服务器断开链接" << endl;
			return -1;//客户端退出，结束循环
		}
		//将收取到的数据拷贝到消息缓冲区
		memcpy(_szMsgBuf+_lastPos, _szRecv,nlen);
		//消息缓冲区的尾部位置后移
		_lastPos += nlen;
		//判断消息缓冲区的数据长度是否大于消息头DataHeader长度
		while (_lastPos >= sizeof(DataHeader))//处理粘包少包问题
		{
			//这时就可以知道当前消息体的长度
			DataHeader* header = (DataHeader*)_szMsgBuf;
			//判断消息缓冲区的数据长度是否大于消息长度
			if (_lastPos >= header->dataLength)
			{
				//消息缓冲区剩余未处理数据的长度
				int nSize = _lastPos - header->dataLength;
				//处理网络消息
				OnNetMsg(header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(_szMsgBuf, _szMsgBuf+ header->dataLength, nSize);
				//消息缓冲区的尾部位置前移
				_lastPos = nSize;
			}
			else
			{
				//剩余消息不够一条完整消息
				break;
			}
		}
		/*
		
		recv(cSock, _szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(header);
		*/
		return 0;//没有这一句vs也不会报错，会自动处理
	}

	//响应网络消息
	virtual void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			//加减偏移量,上面已经把消息头接收掉了
			LoginResult* loginRet = (LoginResult*)header;
			cout << "登录结果：" << loginRet->result << "数据长度：" << loginRet->dataLength << endl;
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult* logoutRet = (LogoutResult*)header;
			cout << "登出结果：" << logoutRet->result << "数据长度：" << logoutRet->dataLength << endl;
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			cout << "数据长度：" << userJoin->dataLength << endl;
		}
		break;
		case CMD_ERROR:
		{
			cout << "CMD_ERROR,数据长度：" << header->dataLength << endl;
		}
		break;
		default:
		{
			cout << "未知消息数据长度：" << header->dataLength << endl;
		}
		break;
		}
	}

	//发送数据
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

private:

};

#endif