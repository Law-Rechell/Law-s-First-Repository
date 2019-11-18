#include"EasyTcpServer.hpp"
#include<thread>//c++线程库
using namespace std;
//创建线程函数，用来输入命令使用
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
	EasyTcpServer server1;
	server1.InitSocket();
	server1.Bind(nullptr,4567);
	server1.Listen(5);


	//启动线程
	thread t1(cmdThread);//第一个是函数名，第二个是函数的参数
	t1.detach();

	while (g_bRun)
	{
		server1.OnRun();
	}

	server1.Close();
	cout << "客户端已退出，任务结束。" << endl;
	getchar();//防止exe程序一闪而过
	return 0;
}