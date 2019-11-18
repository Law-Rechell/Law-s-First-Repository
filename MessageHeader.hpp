#ifndef _MessageHeader_hpp_//ȷ���궨��ֻ�ᱻ����һ��
#define _MessageHeader_hpp_

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,//�µ��û�����
	CMD_ERROR
};
struct DataHeader//��Ϣͷ������ͷ��
{
	DataHeader()
	{
		dataLength = sizeof(DataHeader);
		cmd = CMD_ERROR;
	}
	short dataLength;//���ݳ���
	short cmd;//����
};

struct Login :public DataHeader//�̳�
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}//���캯���������Ͳ���ÿ�ζ�Ҫ��дһ��Dataheader��
	char userName[32];
	char passWord[32];
	char data[932];//�ո�1000B
};

struct LoginResult :public DataHeader//�̳�
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;//��¼������Ƿ�ɹ�
	char data[992];
};

struct Logout :public DataHeader//�̳�
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult :public DataHeader//�̳�
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;//�ǳ�������Ƿ�ɹ�
};

struct NewUserJoin :public DataHeader//�̳�
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