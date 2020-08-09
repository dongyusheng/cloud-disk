#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 去掉边框
    this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    // 给菜单窗口传参
    ui->btn_group->setParent(this);
    // 处理所有信号
    managerSignals();
    // 默认显示我的文件窗口
    ui->stackedWidget->setCurrentWidget(ui->myfiles_page);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showMainWindow()
{
    m_common.moveToCenter(this); //居中显示

    //默认页面
    ui->btn_group->defaulfPage();
    // 切换到我的文件页面
    ui->stackedWidget->setCurrentWidget(ui->myfiles_page);
    // 刷新用户文件列表
    ui->myfiles_page->refreshFiles();
}

// 处理信们
void MainWindow::managerSignals()
{
    // 关闭
    connect(ui->btn_group, &ButtonGroup::closeWindow, this, &MainWindow::close);
    // 最大化
    connect(ui->btn_group, &ButtonGroup::maxWindow, [=]()
    {
        static bool flag = false;
        if(!flag)
        {
            this->showMaximized();
            flag = true;
        }
        else
        {
            this->showNormal();
            flag = false;
        }
    });
    // 最小化
    connect(ui->btn_group, &ButtonGroup::minWindow, this, &MainWindow::showMinimized);
    // 栈窗口切换
    // 我的文件
    connect(ui->btn_group, &ButtonGroup::sigMyFile, [=]()
    {
        ui->stackedWidget->setCurrentWidget(ui->myfiles_page);
        // 刷新文件列表
        ui->myfiles_page->refreshFiles();
    });
    // 分享列表
    connect(ui->btn_group, &ButtonGroup::sigShareList, [=]()
    {
        ui->stackedWidget->setCurrentWidget(ui->sharefile_page);
        // 刷新分享列表
        ui->sharefile_page->refreshFiles();
    });
    // 下载榜
    connect(ui->btn_group, &ButtonGroup::sigDownload, [=]()
    {
        ui->stackedWidget->setCurrentWidget(ui->ranking_page);
        // 刷新下载榜列表
        ui->ranking_page->refreshFiles();
    });
    // 传输列表
    connect(ui->btn_group, &ButtonGroup::sigTransform, [=]()
    {
        ui->stackedWidget->setCurrentWidget(ui->transfer_page);
    });
    // 切换用户
    connect(ui->btn_group, &ButtonGroup::sigSwitchUser, [=]()
    {
        qDebug() << "bye bye...";
        loginAgain();
    });
    // stack窗口切换
    connect(ui->myfiles_page, &MyFileWg::gotoTransfer, [=](TransferStatus status)
    {
        ui->btn_group->slotButtonClick(Page::TRANSFER);
        if(status == TransferStatus::Uplaod)
        {
            ui->transfer_page->showUpload();
        }
        else if(status == TransferStatus::Download)
        {
            ui->transfer_page->showDownload();
        }
    });
    // 信号传递
    connect(ui->sharefile_page, &ShareList::gotoTransfer, ui->myfiles_page, &MyFileWg::gotoTransfer);
}

void MainWindow::loginAgain()
{
    // 发送信号，告诉登陆窗口，切换用户
    emit changeUser();
    // 清空上一个用户的上传或下载任务
    ui->myfiles_page->clearAllTask();

    // 清空上一个用户的一些文件显示信息
    ui->myfiles_page->clearFileList();
    ui->myfiles_page->clearItems();
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap bk(":/images/title_bk3.jpg");
    painter.drawPixmap(0, 0, width(), height(), bk);
}
