#include "transfer.h"
#include "ui_transfer.h"
#include "common/downloadlayout.h"
#include "common/uploadlayout.h"
#include "common/logininfoinstance.h"
#include <QFile>
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

Transfer::Transfer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Transfer)
{
    ui->setupUi(this);

    // 设置上传布局实例
    UploadLayout *uploadLayout = UploadLayout::getInstance();
    uploadLayout->setUploadLayout(ui->upload_scroll);

    // 设置下载布局实例
    DownloadLayout *downloadLayout = DownloadLayout::getInstance();
    downloadLayout->setDownloadLayout(ui->download_scroll);

    ui->tabWidget->setCurrentIndex(0);

    // 切换tab页
    connect(ui->tabWidget, &QTabWidget::currentChanged, [=](int index)
    {
        if(index == 0) //上传
        {
            emit currentTabSignal("正在上传");
        }
        else if(index == 1)//下载
        {
             emit currentTabSignal("正在下载");
        }
        else //传输记录
        {
            emit currentTabSignal("传输记录");
            dispayDataRecord(); //显示数据传输记录
        }
    });
    // 设置样式 tabWidget
    ui->tabWidget->tabBar()->setStyleSheet(
        /*
       "QTabBar::tab{"
       "background-color: rgb(182, 202, 211);"
       "border-right: 1px solid gray;"
       "padding: 6px"
       "}"
       "QTabBar::tab:selected, QtabBar::tab:hover {"
       "background-color: rgb(20, 186, 248);"      
       "}"

       "QTabBar::tab{"
       "background-color: rgb(58, 189, 2);"
       "border-right: 1px solid gray;"
       "padding: 6px"
       "}"
       "QTabBar::tab:selected, QtabBar::tab:hover {"
       "background-color: rgb(28, 110, 217);"
       "}"
        */

       "QTabBar::tab{"
       "background-color: rgb(220, 220, 220));"
       "border-right: 1px solid gray;"
       "padding: 6px"
       "}"
       "QTabBar::tab:selected, QtabBar::tab:hover {"
       "background-color: rgb(173, 173, 173);"
       "}"
    );
    // 清空记录
    connect(ui->clearBtn, &QToolButton::clicked, [=]()
    {
        //获取登陆信息实例
        LoginInfoInstance *login = LoginInfoInstance::getInstance(); //获取单例

        //文件名字，登陆用户名则为文件名
        QString fileName = RECORDDIR + login->getUser();

        if( QFile::exists(fileName) ) //如果文件存在
        {
             QFile::remove(fileName); //删除文件
             ui->record_msg->clear();
        }
    });
}

Transfer::~Transfer()
{
    delete ui;
}

void Transfer::dispayDataRecord(QString path)
{
    //获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance(); //获取单例

    //文件名字，登陆用户名则为文件名
    QString fileName = path + login->getUser();
    QFile file(fileName);

    if( false == file.open(QIODevice::ReadOnly) ) //只读方式打开
    {
        cout << "file.open(QIODevice::ReadOnly) err";
        return;
    }

    QByteArray array = file.readAll();

    #ifdef _WIN32 //如果是windows平台
        //fromLocal8Bit(), 本地字符集转换为utf-8
        ui->record_msg->setText( QString::fromLocal8Bit(array) );
    #else //其它平台
        ui->record_msg->setText( array );
    #endif

        file.close();
}

void Transfer::showUpload()
{
    ui->tabWidget->setCurrentWidget(ui->upload);
}

void Transfer::showDownload()
{
    ui->tabWidget->setCurrentWidget(ui->download);
}
