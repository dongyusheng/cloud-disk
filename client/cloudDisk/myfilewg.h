#ifndef MYFILEWG_H
#define MYFILEWG_H
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

#include <QWidget>
#include <QTimer>
#include "common/common.h"
#include "common/uploadtask.h"
#include "selfwidget/mymenu.h"

namespace Ui {
class MyFileWg;
}

class MyFileWg : public QWidget
{
    Q_OBJECT

public:
    explicit MyFileWg(QWidget *parent = 0);
    ~MyFileWg();

    // 初始化listWidget文件列表
    void initListWidget();
    // 添加右键菜单
    void addActionMenu();

    //==========>上传文件处理<==============
    // 添加需要上传的文件到上传任务列表
    void addUploadFiles();
    // 设置md5信息的json包
    QByteArray setMd5Json(QString user, QString token, QString md5, QString fileName);
    // 上传文件处理，取出上传任务列表的队首任务，上传完后，再取下一个任务
    void uploadFilesAction();
    // 上传真正的文件内容，不能秒传的前提下
    void uploadFile(UploadFileInfo *info);

    //==========>文件item展示<==============
    // 清空文件列表
    void clearFileList();
    // 清空所有item项目
    void clearItems();
    // 添加上传文件项目item
    void addUploadItem(QString iconPath=":/images/upload.png", QString name="上传文件");
    // 文件item展示
    void refreshFileItems();

    //==========>显示用户的文件列表<==============
    // desc是descend 降序意思
    // asc 是ascend 升序意思
    // Normal：普通用户列表，PvAsc：按下载量升序， PvDesc：按下载量降序
    enum Display{Normal, PvAsc, PvDesc};
    // 得到服务器json文件
    QStringList getCountStatus(QByteArray json);
    // 显示用户的文件列表
    void refreshFiles(Display cmd=Normal);
    // 设置json包
    QByteArray setGetCountJson(QString user, QString token);
    // 设置json包
    QByteArray setFilesListJson(QString user, QString token, int start, int count);
    // 获取用户文件列表
    void getUserFilesList(Display cmd=Normal);
    // 解析文件列表json信息，存放在文件列表中
    void getFileJsonInfo(QByteArray data);

    //==========>分享、删除文件<==============
    // 处理选中的文件
    void dealSelectdFile(QString cmd="分享");
    QByteArray setDealFileJson(QString user, QString token, QString md5, QString filename);//设置json包

    //==========>分享文件<==============
    void shareFile(FileInfo *info); //分享某个文件

    //==========>删除文件<==============
    void delFile(FileInfo *info); //删除某个文件

    //==========>获取文件属性<==============
    void getFileProperty(FileInfo *info); //获取属性信息

    //==========>下载文件处理<==============
    // 添加需要下载的文件到下载任务列表
    void addDownloadFiles();
    //下载文件处理，取出下载任务列表的队首任务，下载完后，再取下一个任务
    void downloadFilesAction();

    //==========>下载文件标志处理<==============
    void dealFilePv(QString md5, QString filename); //下载文件pv字段处理

    //清除上传下载任务
    void clearAllTask();
    // 定时检查处理任务队列中的任务
    void checkTaskList();


signals:
    void loginAgainSignal();
    void gotoTransfer(TransferStatus status);

private:
    // 右键菜单信号的槽函数
    void rightMenu(const QPoint &pos);


private:
    Ui::MyFileWg *ui;

    Common m_cm;
    QNetworkAccessManager* m_manager;
    MyMenu   *m_menu;           // 菜单1
    QAction *m_downloadAction; // 下载
    QAction *m_shareAction;    // 分享
    QAction *m_delAction;      // 删除
    QAction *m_propertyAction; // 属性

    MyMenu   *m_menuEmpty;          // 菜单2
    QAction *m_pvAscendingAction;  // 按下载量升序
    QAction *m_pvDescendingAction; // 按下载量降序
    QAction *m_refreshAction;      // 刷新
    QAction *m_uploadAction;       // 上传


    long m_userFilesCount;        //用户文件数目
    int m_start;                  //文件位置起点
    int m_count;                  //每次请求文件个数

    //定时器
    QTimer m_uploadFileTimer;       //定时检查上传队列是否有任务需要上传
    QTimer m_downloadTimer;         //定时检查下载队列是否有任务需要下载


    QList<FileInfo *> m_fileList;    //文件列表


};

#endif // MYFILEWG_H
