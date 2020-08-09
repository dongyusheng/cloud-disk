#include "logininfoinstance.h"
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

//static类的析构函数在main()退出后调用
//静态数据成员，类中声明，类外定义
LoginInfoInstance::Garbo LoginInfoInstance::tmp;

//静态变量动态分配空间
//静态数据成员，类中声明，类外必须定义
LoginInfoInstance* LoginInfoInstance::instance = new LoginInfoInstance;

LoginInfoInstance::LoginInfoInstance()
{

}

LoginInfoInstance::~LoginInfoInstance()
{

}

//把复制构造函数和=操作符也设为私有,防止被复制
LoginInfoInstance::LoginInfoInstance(const LoginInfoInstance& )
{
}

LoginInfoInstance& LoginInfoInstance::operator=(const LoginInfoInstance&)
{
    return *this;
}

//获取唯一的实例
LoginInfoInstance *LoginInfoInstance::getInstance()
{
    return instance;
}

//释放堆区空间
void LoginInfoInstance::destroy()
{
    if(NULL != LoginInfoInstance::instance)
    {
        delete LoginInfoInstance::instance;
        LoginInfoInstance::instance = NULL;
        cout << "instance is detele";
    }
}

//设置登陆信息
void LoginInfoInstance::setLoginInfo( QString tmpUser, QString tmpIp, QString tmpPort, QString token)
{
    user = tmpUser;
    ip = tmpIp;
    port = tmpPort;
    this->token = token;
}

//获取登陆用户
QString LoginInfoInstance::getUser() const
{
    return user;
}

//获取服务器ip
QString LoginInfoInstance::getIp() const
{
    return ip;
}

//获取服务器端口
QString LoginInfoInstance::getPort() const
{
    return port;
}

//获取登陆token
QString LoginInfoInstance::getToken() const
{
    return token;
}

