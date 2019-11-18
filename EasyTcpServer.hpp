#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#define WIN32_LEAN_AND_MEAN//�ú궨����������һЩ���ڵ����������ͻ�Ŀ�

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
#include<vector>//��̬����ͷ�ļ�
#include"MessageHeader.hpp"
using namespace std;

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240//��������С��Ԫ��С
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
	//�ڶ�����������Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	//��Ϣ��������β��λ��
	int _lastPos;
};

class EasyTcpServer
{
private:
	SOCKET _sock;
	vector<ClientSocket*> _clients;//ȫ�ֶ�̬����,��ָ����ⱬջ,ջ�ռ�Ƚ�С(����)�����ڴ�Ƚϴ�

public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}

	virtual~EasyTcpServer()
	{
		Close();
	}

	//��ʼ��socket
	SOCKET InitSocket()
	{
		//����Win Sock 2.x����
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);//�汾��WinSock2,������2.2
		WSADATA dat;//lpWSAData��������lp��ͷ�İ�lpȥ���ͺ�
		WSAStartup(ver, &dat);//Windows Socket��������
#endif

		if (INVALID_SOCKET != _sock)
		{
			cout << "socket " << _sock << ":�رվɵ�����" << endl;
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

		return _sock;
	}

	//��IP��ַ�Ͷ˿ں�
	int Bind(char* ip,unsigned short port)
	{
		//�󶨽��ܿͻ������ӵĶ˿�bind
		sockaddr_in _sin = {};//ת���鿴���壬���г�ʼ��
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsigned short����д����Ϊ����˿ڸ������˿ڲ���ͬһ����
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
		//socket����(_sock),sockaddr��ַ����(sockaddr_in����)������(sizeof(_sin))
		if (SOCKET_ERROR == ret)//���п���ʧ��
		{
			cout << "ERROR : �󶨶˿�:" << port << "ʧ��" << endl;
		}
		else {
			cout << "�󶨶˿�:"<<port<<" �ɹ���" << endl;
		}

		return ret;
	}

	//�����˿ں�
	int Listen(int n)
	{
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)//�����п���ʧ��
		{
			cout << "ERROR : ��������˿�ʧ��" << endl;
		}
		else {
			cout << "��������˿ڳɹ���" << endl;
		}
		return ret;
	}

	//���ܿͻ�������
	SOCKET Accept()
	{
		//4.�ȴ����ܿͻ�������accept
		sockaddr_in clientAddr = {};//�ͻ��˵�ַ
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;//�ͻ���socket����
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif // _WIN32

		if (INVALID_SOCKET == cSock)
		{
			cout << "ERROR : ���ܵ���Ч�ͻ���" << endl;
		}
		else
		{
			//�¼����socket�û�Ⱥ��һ����Ϣ
			//NewUserJoin userJoin;
			//SendDataToAll(&userJoin);

			_clients.push_back(new ClientSocket(cSock));
			cout << "�¿ͻ��˽��룬IP��ַ�ǣ�" << inet_ntoa(clientAddr.sin_addr) << endl;//תΪIP��ַ��ӡ
		}

		return cSock;
	}

	//�ر�socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];//new�˶���������ر�ʱ�ǵ�ɾ��
			}
			//�ر�socket
			closesocket(_sock);
			//=====================================================================
			WSACleanup();//��startup����cleanup
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
				delete _clients[n];//new�˶���������ر�ʱ�ǵ�ɾ��
			}
			close(_sock);
#endif
			_clients.clear();
		}
	}

	int numCount = 0;
	//����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;

			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);//��ռ���

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

			//nfds��һ������ֵ,ָfd_set����������socket�ķ�Χ����������
			//��������socket���ֵ+1��Windows������ν
			//select��Ϊ�����ͷ�����ģʽ�����һ��������NULL��ʾ����
			timeval t = { 1,0 };//F12�鿴���壬�˽���ô��ʼ��
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			//cout <<"select ret = "<<ret<<"   count = "<<numCount++<< endl;
			if (ret < 0)
			{
				cout << "select����ʧ��" << endl;
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))//���_sock�Ѿ����뵽fdRead��
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}

			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
				{
					if (-1 == RecvData(_clients[n]))//�ͻ���ʧЧ�ˣ�Ҫ��취�Ƴ�
					{
						auto iter = _clients.begin() + n;
						if (iter != _clients.end())//�ҵ���
						{
							delete _clients[n];
							_clients.erase(iter);//�Ƴ�ʧЧ�ͻ���
						}
					}
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

	//ʹ��һ����������������
	char _szRecv[RECV_BUFF_SIZE] = {};

	//�������ݣ�����ճ������ְ�
	int RecvData(ClientSocket* pClient)
	{
		//4.1.���ܿͻ��˵���������
		int nlen = recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);//(int)
		//cout << "���ݳ��ȣ�" << nlen << endl;
		
		if (nlen <= 0)//recv���ܵ�����ʱ�����Ȼ����0
		{
			cout << "ERROR : ���ܿͻ�����Ϣʧ��" << endl;
			return -1;//�ͻ����˳�������ѭ��
		}
		//����ȡ�������ݿ�������Ϣ������
		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nlen);
		//��Ϣ��������β��λ�ú���
		pClient->setLastPos(pClient->getLastPos()+nlen);
		//�ж���Ϣ�����������ݳ����Ƿ������ϢͷDataHeader����
		while (pClient->getLastPos() >= sizeof(DataHeader))//����ճ���ٰ�����
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ��ĳ���
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//�ж���Ϣ�����������ݳ����Ƿ������Ϣ����
			if (pClient->getLastPos() >= header->dataLength)
			{
				//��Ϣ������ʣ��δ�������ݵĳ���
				int nSize = pClient->getLastPos() - header->dataLength;
				//����������Ϣ
				OnNetMsg(pClient->sockfd(),header);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//��Ϣ��������β��λ��ǰ��
				pClient->setLastPos(nSize);
			}
			else
			{
				//ʣ����Ϣ����һ��������Ϣ
				break;
			}
		}
		return 0;
	}

	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET cSock,DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			//�Ӽ�ƫ����,�����Ѿ�����Ϣͷ���յ���
			
			Login* login = (Login*)header;
			cout << "�յ����Login & ���ݳ��ȣ�" << login->dataLength << "�û�����"
				<< login->userName << " ���룺" << login->passWord << endl;
			//�����ж��û����Ƿ���ȷ�Ĺ���
			LoginResult loginRet;
			SendData(cSock, &loginRet);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			cout << "�յ����Logout & ���ݳ��ȣ�" << logout->dataLength << "�û�����"
				<< logout->userName << endl;
			//�����ж��û����Ƿ���ȷ�Ĺ���
			LogoutResult logoutRet;
			SendData(cSock, &logoutRet);
		}
		break;
		default:
		{
			cout << "δ֪��Ϣ���ݳ��ȣ�" << header->dataLength << endl;
			DataHeader header;
			SendData(cSock, &header);
		}
		break;
		}
	}

	//����ָ��socket����
	int SendData(SOCKET cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	//Ⱥ��socket����
	void SendDataToAll( DataHeader* header)
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			SendData(_clients[n]->sockfd(), header);
		}
	}

};

#endif // _EasyTcpServer_hpp_
