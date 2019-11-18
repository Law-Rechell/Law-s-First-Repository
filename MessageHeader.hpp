#ifndef _MessageHeader_hpp_//确保宏定义只会被编译一次
#define _MessageHeader_hpp_

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,//新的用户进入
	CMD_ERROR
};
struct DataHeader//消息头，（包头）
{
	DataHeader()
	{
		dataLength = sizeof(DataHeader);
		cmd = CMD_ERROR;
	}
	short dataLength;//数据长度
	short cmd;//命令
};

struct Login :public DataHeader//继承
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}//构造函数，这样就不用每次都要先写一个Dataheader了
	char userName[32];
	char passWord[32];
	char data[932];//凑个1000B
};

struct LoginResult :public DataHeader//继承
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;//登录结果，是否成功
	char data[992];
};

struct Logout :public DataHeader//继承
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult :public DataHeader//继承
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;//登出结果，是否成功
};

struct NewUserJoin :public DataHeader//继承
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;//socket ID
};

#endif