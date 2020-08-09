#include "downloadlayout.h"
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

//静态数据成员，类中声明，类外必须定义
DownloadLayout * DownloadLayout::instance = new DownloadLayout;

//static类的析构函数在main()退出后调用
DownloadLayout::Garbo DownloadLayout::temp; //静态数据成员，类中声明，类外定义

DownloadLayout * DownloadLayout::getInstance()
{
    return instance;
}

//设置布局
void DownloadLayout::setDownloadLayout(QWidget *p)
{
    m_wg = new QWidget(p);
    QLayout* layout = p->layout();
    layout->addWidget(m_wg);
    layout->setContentsMargins(0, 0, 0, 0);
    QVBoxLayout* vlayout = new QVBoxLayout;
    // 布局设置给窗口
    m_wg->setLayout(vlayout);
    // 边界间隔
    vlayout->setContentsMargins(0, 0, 0, 0);
    m_layout = vlayout;

    vlayout->addStretch();
}

//获取布局
QLayout *DownloadLayout::getDownloadLayout()
{
    return m_layout;
}

