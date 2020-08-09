#include <QFile>
#include <QMap>
#include <QDir>
#include <QTime>
#include <QFileInfo>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QApplication>
#include <QJsonDocument>
#include <QFileInfoList>
#include <QDesktopWidget>
#include <QCryptographicHash>
#include "des.h"
#include "common.h"
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

// 初始化变量
QString Common::m_typePath = FILETYPEDIR;
QStringList Common::m_typeList = QStringList();
QNetworkAccessManager* Common::m_netManager = new QNetworkAccessManager;

Common::Common(QObject *parent)
{
    Q_UNUSED(parent)
}

// 窗口在屏幕中央显示
void Common::moveToCenter(QWidget *tmp)
{
     // 显示窗口
     tmp->show();
     // 屏幕中间显示
     // 使用qApp->desktop();也可以
     QDesktopWidget* desktop = QApplication::desktop();
     // 移动窗口
     tmp->move((desktop->width() - tmp->width())/2, (desktop->height() - tmp->height())/2);
}

/* -------------------------------------------*/
/**
 * @brief           从配置文件中得到相对应的参数
 *
 * @param title     配置文件title名称[title]
 * @param key       key
 * @param path      配置文件路径
 *
 * @returns
 *                  success: 得到的value
 *                  fail:    空串
 */
/* -------------------------------------------*/
QString Common::getCfgValue(QString title, QString key, QString path)
{
    QFile file(path);

    // 只读方式打开
    if( false == file.open(QIODevice::ReadOnly) )
    {
        // 打开失败
        cout << "file open err";
        return "";
    }

    QByteArray json = file.readAll(); // 读取所有内容
    file.close(); // 关闭文件

    QJsonParseError error;

    // 将来源数据json转化为JsonDocument
    // 由QByteArray对象构造一个QJsonDocument对象，用于我们的读写操作
    QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    if (error.error == QJsonParseError::NoError) // 没有出错
    {
        if (doc.isNull() || doc.isEmpty())
        {
            cout << "doc.isNull() || doc.isEmpty()";
            return "";
        }

        if( doc.isObject()) // 如果对象不为空
        {
            // QJsonObject json对象，描述json数据中{}括起来部分
            QJsonObject obj = doc.object();// 取得最外层这个大对象

            QJsonObject tmp = obj.value( title ).toObject();
            QStringList list = tmp.keys(); // 取出key列表
            for(int i = 0; i < list.size(); ++i)
            {
                if( list.at(i) == key )
                {
                    return tmp.value( list.at(i) ).toString();
                }
            }

        }
    }
    else
    {
        cout << "err = " << error.errorString();
    }

    return "";
}

// 通过读取文件, 得到文件类型, 存放在typeList
void Common::getFileTypeList()
{
    // QDir类使用相对或绝对文件路径来指向一个文件/目录。
    QDir dir(m_typePath);
    if(!dir.exists())
    {
        dir.mkpath(m_typePath);
        cout << m_typePath << "创建成功！！！";
    }

    /*
        QDir::Dirs      列出目录；
        QDir::AllDirs   列出所有目录，不对目录名进行过滤；
        QDir::Files     列出文件；
        QDir::Drives    列出逻辑驱动器名称，该枚举变量在Linux/Unix中将被忽略；
        QDir::NoSymLinks        不列出符号链接；
        QDir::NoDotAndDotDot    不列出文件系统中的特殊文件.及..；
        QDir::NoDot             不列出.文件，即指向当前目录的软链接
        QDir::NoDotDot          不列出..文件；
        QDir::AllEntries        其值为Dirs | Files | Drives，列出目录、文件、驱动器及软链接等所有文件；
        QDir::Readable      列出当前应用有读权限的文件或目录；
        QDir::Writable      列出当前应用有写权限的文件或目录；
        QDir::Executable    列出当前应用有执行权限的文件或目录；
        Readable、Writable及Executable均需要和Dirs或Files枚举值联合使用；
        QDir::Modified      列出已被修改的文件，该值在Linux/Unix系统中将被忽略；
        QDir::Hidden        列出隐藏文件；
        QDir::System        列出系统文件；
        QDir::CaseSensitive 设定过滤器为大小写敏感。
    */
    dir.setFilter(QDir::Files | QDir::NoDot |  QDir::NoDotDot | QDir::NoSymLinks); // 过滤文件
    dir.setSorting(QDir::Size | QDir::Reversed);   // 排序

    QFileInfoList list = dir.entryInfoList();

    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        m_typeList.append( fileInfo.fileName() );
    }
}

// 得到文件后缀，参数为文件类型，函数内部判断是否有此类型，如果有，使用此类型，没有，使用other.png
QString Common::getFileType(QString type)
{
    if(true == m_typeList.contains(type))
    {
        return m_typePath + "/" + type;
    }

    return m_typePath + "/other.png";
}

// 登录信息，写入配置文件
void Common::writeLoginInfo(QString user, QString pwd, bool isRemeber, QString path)
{
    // web_server信息
    QString ip = getCfgValue("web_server", "ip");
    QString port = getCfgValue("web_server", "port");

    QMap<QString, QVariant> web_server;
    web_server.insert("ip", ip);
    web_server.insert("port", port);

    // type_path信息
    QMap<QString, QVariant> type_path;
    type_path.insert("path", m_typePath);

    // login信息
    QMap<QString, QVariant> login;

    // 登陆信息加密
    int ret = 0;

    // 登陆用户加密
    unsigned char encUsr[1024] = {0};
    int encUsrLen;
    // toLocal8Bit(), 转换为本地字符集，如果windows则为gbk编码，如果linux则为utf-8编码
    ret = DesEnc((unsigned char *)user.toLocal8Bit().data(), user.toLocal8Bit().size(), encUsr, &encUsrLen);
    if(ret != 0)//加密失败
    {
        cout << "DesEnc err";
        return;
    }

    // 用户密码加密
    unsigned char encPwd[512] = {0};
    int encPwdLen;
    // toLocal8Bit(), 转换为本地字符集，如果windows则为gbk编码，如果linux则为utf-8编码
    ret = DesEnc((unsigned char *)pwd.toLocal8Bit().data(), pwd.toLocal8Bit().size(), encPwd, &encPwdLen);
    if(ret != 0)
    {
        cout << "DesEnc err";
        return;
    }

    // 再次加密
    // base64转码加密，目的将加密后的二进制转换为base64字符串
    login.insert("user",  QByteArray((char *)encUsr, encUsrLen).toBase64());
    login.insert("pwd", QByteArray((char *)encPwd, encPwdLen).toBase64() );
    if(isRemeber == true)
    {
         login.insert("remember", "yes");
    }
    else
    {
        login.insert("remember", "no");
    }

    // QVariant类作为一个最为普遍的Qt数据类型的联合
    // QVariant为一个万能的数据类型--可以作为许多类型互相之间进行自动转换。
    QMap<QString, QVariant> json;
    json.insert("web_server", web_server);
    json.insert("type_path", type_path);
    json.insert("login", login);


    QJsonDocument jsonDocument = QJsonDocument::fromVariant(json);
    if ( jsonDocument.isNull() == true)
    {
        cout << " QJsonDocument::fromVariant(json) err";
        return;
    }

    QFile file(path);

    if( false == file.open(QIODevice::WriteOnly) )
    {
        cout << "file open err";
        return;
    }

    //json内容写入文件
    file.write(jsonDocument.toJson());
    file.close();
}

// 服务器信息，写入配置文件
void Common::writeWebInfo(QString ip, QString port, QString path)
{
    // web_server信息
    QMap<QString, QVariant> web_server;
    web_server.insert("ip", ip);
    web_server.insert("port", port);

    // type_path信息
    QMap<QString, QVariant> type_path;
    type_path.insert("path", m_typePath);

    // login信息
    QString user = getCfgValue("login", "user");
    QString pwd = getCfgValue("login", "pwd");
    QString remember = getCfgValue("login", "remember");


    QMap<QString, QVariant> login;
    login.insert("user", user);
    login.insert("pwd", pwd);
    login.insert("remember", remember);


    // QVariant类作为一个最为普遍的Qt数据类型的联合
    // QVariant为一个万能的数据类型--可以作为许多类型互相之间进行自动转换。
    QMap<QString, QVariant> json;
    json.insert("web_server", web_server);
    json.insert("type_path", type_path);
    json.insert("login", login);


    QJsonDocument jsonDocument = QJsonDocument::fromVariant(json);
    if ( jsonDocument.isNull() == true)
    {
        cout << " QJsonDocument::fromVariant(json) err";
        return;
    }

    QFile file(path);

    if( false == file.open(QIODevice::WriteOnly) )
    {
        cout << "file open err";
        return;
    }

    file.write(jsonDocument.toJson());
    file.close();
}

// 获取某个文件的md5码
QString Common::getFileMd5(QString filePath)
{
    QFile localFile(filePath);

    if (!localFile.open(QFile::ReadOnly))
    {
        qDebug() << "file open error.";
        return 0;
    }

    QCryptographicHash ch(QCryptographicHash::Md5);

    quint64 totalBytes = 0;
    quint64 bytesWritten = 0;
    quint64 bytesToWrite = 0;
    quint64 loadSize = 1024 * 4;
    QByteArray buf;

    totalBytes = localFile.size();
    bytesToWrite = totalBytes;

    while (1)
    {
        if(bytesToWrite > 0)
        {
            buf = localFile.read(qMin(bytesToWrite, loadSize));
            ch.addData(buf);
            bytesWritten += buf.length();
            bytesToWrite -= buf.length();
            buf.resize(0);
        }
        else
        {
            break;
        }

        if(bytesWritten == totalBytes)
        {
            break;
        }
    }

    localFile.close();
    QByteArray md5 = ch.result();
    return md5.toHex();
}

// 将某个字符串加密成md5码
QString Common::getStrMd5(QString str)
{
    QByteArray array;
    //md5加密
    array = QCryptographicHash::hash ( str.toLocal8Bit(), QCryptographicHash::Md5 );

    return array.toHex();
}

// 产生分隔线
QString Common::getBoundary()
{
    // 随机种子
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    QString tmp;

    // 48~122, '0'~'A'~'z'
    for(int i = 0; i < 16; i++)
    {
        tmp[i] = qrand() % (122-48) + 48;
    }

    return QString("------WebKitFormBoundary%1").arg(tmp);
}

// 得到服务器回复的状态码， 返回值为 "000", 或 "001"
QString Common::getCode(QByteArray json)
{
    QJsonParseError error;

    // 将来源数据json转化为JsonDocument
    // 由QByteArray对象构造一个QJsonDocument对象，用于我们的读写操作
    QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    if (error.error == QJsonParseError::NoError)
    {
        if (doc.isNull() || doc.isEmpty())
        {
            cout << "doc.isNull() || doc.isEmpty()";
            return "";
        }

        if( doc.isObject() )
        {
            // 取得最外层这个大对象
            QJsonObject obj = doc.object();
            return obj.value( "code" ).toString();
        }

    }
    else
    {
        cout << "err = " << error.errorString();
    }

    return "";
}

// 传输数据记录到本地文件，user：操作用户，name：操作的文件, code: 操作码， path: 文件保存的路径
void Common::writeRecord(QString user, QString name, QString code, QString path)
{
    // 文件名字，登陆用户名则为文件名
    QString fileName = path + user;
    // 检查目录是否存在，如果不存在，则创建目录
    QDir dir(path);
    if(!dir.exists())
    {
        // 目录不存在, 创建
        if(dir.mkpath(path))
        {
            cout << path << "目录创建成功。。。";
        }
        else
        {
            cout << path << "目录创建失败。。。";
        }
    }
    cout << "fileName = " << fileName.toUtf8().data();
    QByteArray array;

    QFile file(fileName);

    // 如果文件存在， 先读取文件原来的内容
    if( true == file.exists() )
    {
        if( false == file.open(QIODevice::ReadOnly) )
        {
            cout << "file.open(QIODevice::ReadOnly) err";
            return;
        }
        //读取文件原来的内容
        array = file.readAll();
        file.close();
    }

    if( false == file.open(QIODevice::WriteOnly) )
    {
        cout << "file.open(QIODevice::WriteOnly) err";
        return;
    }

    // 记录包操作
    // xxx.jpg       2020年06月21日12:04:49       秒传成功
    // 获取当前时间
    QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间
    QString timeStr = time.toString("yyyy-MM-dd hh:mm:ss ddd"); //设置显示格式

    /*
       秒传文件：
            文件已存在：{"code":"005"}
            秒传成功：  {"code":"006"}
            秒传失败：  {"code":"007"}
        上传文件：
            成功：{"code":"008"}
            失败：{"code":"009"}
        下载文件：
            成功：{"code":"010"}
            失败：{"code":"011"}
    */
    QString actionStr;
    if(code == "005")
    {
        actionStr = "上传失败，文件已存在";
    }
    else if(code == "006")
    {
        actionStr = "秒传成功";
    }
    else if(code == "008")
    {
        actionStr = "上传成功";
    }
    else if(code == "009")
    {
        actionStr = "上传失败";
    }
    else if(code == "010")
    {
        actionStr = "下载成功";
    }
    else if(code == "011")
    {
        actionStr = "下载失败";
    }

    QString str = QString("[%1]\t%2\t[%3]\r\n").arg(name).arg(timeStr).arg(actionStr);
    cout << str.toUtf8().data();

    // toLocal8Bit(), 转换为本地字符集
    // 先写新内容
    file.write( str.toLocal8Bit() );

    if(array.isEmpty() == false)
    {
        // 再写原来的内容
        file.write(array);
    }

    file.close();

}

QNetworkAccessManager *Common::getNetManager()
{
    // 该对象一般一个应用程序中有一个就够了，无需new多个
    return  m_netManager;
}

Common::~Common()
{
}
