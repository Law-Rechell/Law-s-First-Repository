#define WIN32_LEAN_AND_MEAN//�ú궨����������һЩ���ڵ����������ͻ�Ŀ�

#include<iostream>
#include<thread>//c++�߳̿�
#include"EasyTcpClient.hpp"

using namespace std;

bool g_bRun = true;
//�����̺߳�����������������ʹ��
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		cin >> cmdBuf;
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			cout << "exit����˳������̡߳�" << endl;
			break;
		}
		else
		{
			cout << "��Ч������������룡" << endl;
		}
	}
}

int main()
{
	int num = 0;
	const int cCount = 1000;//-1��Ҫ��ȥ����
	//EasyTcpClient client1[cCount];//ջ�ڴ汬��
	EasyTcpClient* client1[cCount];

	for (int n = 0; n < cCount; n++)
	{
		if (!g_bRun)
		{
			return 0;
		}
		client1[n] = new EasyTcpClient();
	}
	for (int n = 0; n < cCount; n++)
	{
		if (!g_bRun)
		{
			return 0;
		}
		client1[n]->Connect("127.0.0.1", 4567);
		cout << num++ << endl;
	}//һ��СС���Ż�
	
	//�����߳�
	thread t1(cmdThread);//��һ���Ǻ��������ڶ����Ǻ����Ĳ���
	t1.detach();//�����߳̽��з���,�����п������߳��˳��˸��̻߳�û���ͷ�

	Login login;
	strcpy(login.userName, "Lane");
	strcpy(login.passWord, "Lane'spsd");
	while (g_bRun)
	{
		for (int n = 0; n < cCount; n++)
		{
			client1[n]->SendData(&login);
			client1[n]->OnRun();
		}
	}
	
	for (int n = 0; n < cCount; n++)
	{
		client1[n]->Close();
	}

	cout << "�ͻ������˳������������" << endl;
	//getchar();//��ֹexe����һ������
	return 0;
}