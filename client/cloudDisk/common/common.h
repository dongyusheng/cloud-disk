#ifndef COMMON_H
#define COMMON_H

#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif


#include <QDebug>
#include <QWidget>
#include <QString>
#include <QListWidgetItem>
#include <QNetworkAccessManager>

#define cout qDebug() << "[ " << __FILE__ << ":"  << __LINE__ << " ] "

#define CONFFILE        "conf/cfg.json"     // 配置文件
#define RECORDDIR       "conf/record/"      // 用户文件上传下载记录
#define FILETYPEDIR     "conf/fileType"     // 存放文件类型图片目录

// 正则表达式
#define USER_REG        "^[a-zA-Z\\d_@#-\*]\{3,16\}$"
#define PASSWD_REG      "^[a-zA-Z\\d_@#-\*]\{6,18\}$"
#define PHONE_REG       "1\\d\{10\}"
#define EMAIL_REG       "^[a-zA-Z\\d\._-]\+@[a-zA-Z\\d_\.-]\+(\.[a-zA-Z0-9_-]\+)+$"
#define IP_REG          "((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)"
#define PORT_REG        "^[1-9]$|(^[1-9][0-9]$)|(^[1-9][0-9][0-9]$)|(^[1-9][0-9][0-9][0-9]$)|(^[1-6][0-5][0-5][0-3][0-5]$)"

// 文件信息
struct FileInfo
{
    QString md5;            // 文件md5码
    QString fileName;       // 文件名字
    QString user;           // 用户
    QString createTime;     // 上传时间
    QString url;            // url
    QString type;           // 文件类型
    qint64 size;            // 文件大小
    int shareStatus;        // 是否共享, 1共享， 0不共享
    int pv;                 // 下载量
    QListWidgetItem *item;  // list widget 的item
};
// 传输状态
enum TransferStatus{Download, Uplaod, Recod};


class Common : public QObject
{
    Q_OBJECT

public:
    Common(QObject* parent = 0);
    ~Common();

    // 窗口在屏幕中央显示
    void moveToCenter(QWidget *tmp);

    // 从配置文件中得到相对应的参数
    QString getCfgValue(QString title, QString key, QString path = CONFFILE);

    // 通过读取文件, 得到文件类型, 存放在typeList
    void getFileTypeList();

    // 得到文件后缀，参数为文件类型，函数内部判断是否有此类型，如果有，使用此类型，没有，使用other.png
    QString getFileType(QString type);

    // 登录信息，写入配置文件
    void writeLoginInfo(QString user, QString pwd, bool isRemeber, QString path = CONFFILE);

    // 服务器信息，写入配置文件
    void writeWebInfo(QString ip, QString port, QString path=CONFFILE);

    // 获取某个文件的md5码
    QString getFileMd5(QString filePath);

    // 将某个字符串加密成md5码
    QString getStrMd5(QString str = "");

    // 产生分隔线
    QString getBoundary();

    // 得到服务器回复的状态码， 返回值为 "000", 或 "001"
    QString getCode(QByteArray json);

    // 传输数据记录到本地文件，user：操作用户，name：操作的文件, code: 操作码， path: 文件保存的路径
    void writeRecord(QString user, QString name, QString code, QString path = RECORDDIR);

    // 得到http通信类对象
    static QNetworkAccessManager* getNetManager();
public:
    static QStringList  m_typeList;

private:
    // 文件类型路径
    static QString      m_typePath;
    // 主要保存文件类型的后缀
    // http类
    static QNetworkAccessManager *m_netManager;
};

#endif // COMMON_H

