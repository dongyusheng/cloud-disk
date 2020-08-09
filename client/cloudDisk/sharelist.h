#ifndef SHARELIST_H
#define SHARELIST_H
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

#include <QWidget>
#include <QTimer>
#include "common/common.h"
#include "selfwidget/mymenu.h"

namespace Ui {
class ShareList;
}

class ShareList : public QWidget
{
    Q_OBJECT

public:
    explicit ShareList(QWidget *parent = 0);
    ~ShareList();

    // 初始化ListWidget属性
    void initListWidget();
    // 添加动作菜单
    void addActionMenu();

    //==========>文件item展示<==============
    // 清空文件列表
    void clearshareFileList();
    // 清空所有item项目
    void clearItems();
    // 文件item展示
    void refreshFileItems();

    //==========>显示共享文件列表<==============
    void refreshFiles();//显示共享的文件列表
    QByteArray setFilesListJson(int start, int count);  //设置json包
    void getUserFilesList();                            //获取共享文件列表
    void getFileJsonInfo(QByteArray data);              //解析文件列表json信息，存放在文件列表中

    //==========>下载文件处理<==============
    // 添加需要下载的文件到下载任务列表
    void addDownloadFiles();
    // 下载文件处理，取出下载任务列表的队首任务，下载完后，再取下一个任务
    void downloadFilesAction();

    QByteArray setShareFileJson(QString user, QString md5, QString filename);//设置json包

    //==========>下载文件标志处理<==============
    void dealFilePv(QString md5, QString filename); //下载文件pv字段处理

    // 枚举，Property属性，Cancel取消分享，Save转存文件
    enum CMD{Property, Cancel, Save};
    void dealSelectdFile(CMD cmd=Property); //处理选中的文件

    //==========>获取文件属性<==============
    void getFileProperty(FileInfo *info); //获取属性信息

    //==========>取消已经分享的文件<==============
    void cancelShareFile(FileInfo *info);

    //==========>转存文件<==============
    void saveFileToMyList(FileInfo *info);

signals:
    void gotoTransfer(TransferStatus status);

private:
    Ui::ShareList *ui;

    Common  m_cm;
    MyMenu   *m_menuItem;          //菜单1
    QAction *m_downloadAction;    //下载
    QAction *m_propertyAction;    //属性
    QAction *m_cancelAction;      //取消分享
    QAction *m_saveAction;        //转存文件

    MyMenu *m_menuEmpty;           //菜单2
    QAction *m_refreshAction;     //刷新

    // QNetworkAccessManager类允许应用程序发送网络请求和接收网络应答。
    QNetworkAccessManager *m_manager;


    int m_start;            //文件位置起点
    int m_count;            //每次请求文件个数
    long m_userFilesCount;  // 用户文件数目

    QList<FileInfo *> m_shareFileList; //文件列表

    //定时器
    QTimer m_downloadTimer;   //定时检查下载队列是否有任务需要下载
};

#endif // SHARELIST_H
