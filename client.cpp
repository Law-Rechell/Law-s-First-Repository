#define WIN32_LEAN_AND_MEAN//该宏定义会避免引入一些早期的容易引起冲突的库

#include<iostream>
#include<thread>//c++线程库
#include"EasyTcpClient.hpp"

using namespace std;

bool g_bRun = true;
//创建线程函数，用来输入命令使用
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		cin >> cmdBuf;
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			cout << "exit命令，退出输入线程。" << endl;
			break;
		}
		else
		{
			cout << "无效命令！请重新输入！" << endl;
		}
	}
}

int main()
{
	int num = 0;
	const int cCount = 1000;//-1还要除去主机
	//EasyTcpClient client1[cCount];//栈内存爆了
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
	}//一个小小的优化
	
	//启动线程
	thread t1(cmdThread);//第一个是函数名，第二个是函数的参数
	t1.detach();//和主线程进行分离,否则有可能主线程退出了该线程还没被释放

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

	cout << "客户端已退出，任务结束。" << endl;
	//getchar();//防止exe程序一闪而过
	return 0;
}