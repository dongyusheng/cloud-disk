#ifndef LOGIN_H
#define LOGIN_H
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

#include <QDialog>
#include <QNetworkAccessManager>
#include <mainwindow.h>
#include "common/common.h"

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();
    
    // 设置登陆用户信息的json包
    QByteArray setLoginJson(QString user, QString pwd);

    // 设置注册用户信息的json包
    QByteArray setRegisterJson(QString userName, QString nickName, QString firstPwd, QString phone, QString email);

    // 得到服务器回复的登陆状态， 状态码返回值为 "000", 或 "001"，还有登陆section
    QStringList getLoginStatus(QByteArray json);

protected:
    void paintEvent(QPaintEvent *);

private slots:
    void on_register_btn_clicked();

    void on_login_btn_clicked();

    void on_set_ok_btn_clicked();

private:
    // 读取配置信息，设置默认登录状态，默认设置信息
    void readCfg();

private:
    Ui::Login *ui;

    // 处理网络请求类对象
    QNetworkAccessManager* m_manager;
    // 主窗口指针
    MainWindow* m_mainWin;
    Common m_cm;
};

#endif // LOGIN_H
