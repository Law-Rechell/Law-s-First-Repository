#include"EasyTcpServer.hpp"
#include<thread>//c++�߳̿�
using namespace std;
//�����̺߳�����������������ʹ��
bool g_bRun = true;
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
	EasyTcpServer server1;
	server1.InitSocket();
	server1.Bind(nullptr,4567);
	server1.Listen(5);


	//�����߳�
	thread t1(cmdThread);//��һ���Ǻ��������ڶ����Ǻ����Ĳ���
	t1.detach();

	while (g_bRun)
	{
		server1.OnRun();
	}

	server1.Close();
	cout << "�ͻ������˳������������" << endl;
	getchar();//��ֹexe����һ������
	return 0;
}