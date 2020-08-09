#include "titlewidget.h"
#include "ui_titlewidget.h"
#include <QMouseEvent>
#include <QDebug>
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

TitleWidget::TitleWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TitleWidget)
{
    ui->setupUi(this);

    m_parent = parent;
    // logo
    ui->logo->setPixmap(QPixmap(":/images/logo.png").scaled(40, 40));

    // 按钮
    // 最小化
    connect(ui->min_btn, &QToolButton::clicked, [=]()
    {
        m_parent->showMinimized();
    });

    // 关闭
    connect(ui->close_btn, &QToolButton::clicked, [=]()
    {
        emit closeWindow();
    });

    // 设置
    connect(ui->set_btn, &QToolButton::clicked, [=]()
    {
        // 切换到设置窗口
        // 发信号给父窗口
        emit showSetWidget();
    });
}

TitleWidget::~TitleWidget()
{
    delete ui;
}

void TitleWidget::mousePressEvent(QMouseEvent *ev)
{
    // 如果是左键, 计算窗口左上角, 和当前按钮位置的距离
    if(ev->button() == Qt::LeftButton)
    {
        // 计算和窗口左上角的相对位置
        m_pos = ev->globalPos() - m_parent->geometry().topLeft();
    }
}

void TitleWidget::mouseMoveEvent(QMouseEvent *ev)
{
    // 移动是持续的状态, 需要使用buttons
    if(ev->buttons() & Qt::LeftButton)
    {
        QPoint pos = ev->globalPos() - m_pos;
        m_parent->move(pos);
    }
}
