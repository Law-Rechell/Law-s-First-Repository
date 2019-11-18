#ifndef _EasyTcpClient_hpp_//ȷ���궨��ֻ�ᱻ����һ��
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
#include<thread>//c++�߳̿�
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

	virtual ~EasyTcpClient()//ϰ�ߣ������������麯��
	{
		Close();
	}

	//��ʼ��socket����
	void InitSocket()
	{
		//����Win Sock 2.x����
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);//�汾��WinSock2,������2.2
		WSADATA dat;//lpWSAData��������lp��ͷ�İ�lpȥ���ͺ�
		WSAStartup(ver, &dat);//Windows Socket��������
#endif

		if (INVALID_SOCKET != _sock)
		{
			cout <<"socket "<< _sock <<":�رվɵ�����" << endl;
			Close();
		}
	//1.����һ��socket
		_sock = socket(AF_INET, SOCK_STREAM, 0);//���ﲻ����tcp
		if (INVALID_SOCKET == _sock)
		{
			cout << "ERROR : �����׽���ʧ��" << endl;
		}
		else {
			cout << "�����׽��ֳɹ���" << endl;
		}
	}

	//���ӷ���������
	int Connect(char* ip,unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		//2.���ӷ�����connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(_sin));//��һ��д��
		if (SOCKET_ERROR == ret)
		{
			cout << "ERROR : ���ӷ�����ʧ��" << endl;
		}
		else
		{
			cout << "���ӷ������ɹ���" << endl;
		}

		return ret;
	}

	//�ر�socket����
	void Close()
	{
		//�ر�Win Sock 2.x����
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			//4.�ر�socket closesocket
			closesocket(_sock);
			//=====================================================================
			WSACleanup();//��startup����cleanup
#else
			close(_sock);
#endif
		}
		_sock = INVALID_SOCKET;
	}
	
	int numCount = 0;
	//��ѯ������Ϣ
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
				cout << "socket " << _sock << ":select�������1��" << endl;
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock))
				{
					cout << "socket " << _sock << ":select�������2��" << endl;//��һ������
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240//��������С��Ԫ��С
#endif // !RECV_BUFF_SIZE

	//���ջ�����
	char _szRecv[RECV_BUFF_SIZE] = {};//˫���壬socket������һ�����ܺͷ��͵Ļ�����
	//�ڶ�����������Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	//��Ϣ��������β��λ��
	int _lastPos = 0;

	//��������,����ճ�����
	int RecvData(SOCKET cSock)
	{
		//4.1.���ܿͻ��˵���������
		int nlen = recv(cSock, _szRecv, RECV_BUFF_SIZE, 0);
		//cout << "���ݳ��ȣ�" << nlen << endl;
		if (nlen <= 0)//recv���ܵ�����ʱ�����Ȼ����0
		{
			cout << "ERROR : ��������Ͽ�����" << endl;
			return -1;//�ͻ����˳�������ѭ��
		}
		//����ȡ�������ݿ�������Ϣ������
		memcpy(_szMsgBuf+_lastPos, _szRecv,nlen);
		//��Ϣ��������β��λ�ú���
		_lastPos += nlen;
		//�ж���Ϣ�����������ݳ����Ƿ������ϢͷDataHeader����
		while (_lastPos >= sizeof(DataHeader))//����ճ���ٰ�����
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ��ĳ���
			DataHeader* header = (DataHeader*)_szMsgBuf;
			//�ж���Ϣ�����������ݳ����Ƿ������Ϣ����
			if (_lastPos >= header->dataLength)
			{
				//��Ϣ������ʣ��δ�������ݵĳ���
				int nSize = _lastPos - header->dataLength;
				//����������Ϣ
				OnNetMsg(header);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(_szMsgBuf, _szMsgBuf+ header->dataLength, nSize);
				//��Ϣ��������β��λ��ǰ��
				_lastPos = nSize;
			}
			else
			{
				//ʣ����Ϣ����һ��������Ϣ
				break;
			}
		}
		/*
		
		recv(cSock, _szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(header);
		*/
		return 0;//û����һ��vsҲ���ᱨ�����Զ�����
	}

	//��Ӧ������Ϣ
	virtual void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			//�Ӽ�ƫ����,�����Ѿ�����Ϣͷ���յ���
			LoginResult* loginRet = (LoginResult*)header;
			cout << "��¼�����" << loginRet->result << "���ݳ��ȣ�" << loginRet->dataLength << endl;
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult* logoutRet = (LogoutResult*)header;
			cout << "�ǳ������" << logoutRet->result << "���ݳ��ȣ�" << logoutRet->dataLength << endl;
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			cout << "���ݳ��ȣ�" << userJoin->dataLength << endl;
		}
		break;
		case CMD_ERROR:
		{
			cout << "CMD_ERROR,���ݳ��ȣ�" << header->dataLength << endl;
		}
		break;
		default:
		{
			cout << "δ֪��Ϣ���ݳ��ȣ�" << header->dataLength << endl;
		}
		break;
		}
	}

	//��������
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