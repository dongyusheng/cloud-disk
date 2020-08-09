#ifndef DOWNLOADLAYOUT_H
#define DOWNLOADLAYOUT_H
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

#include "common.h"
#include <QVBoxLayout>

//下载进度布局类，单例模式
class DownloadLayout
{
public:
    static DownloadLayout *getInstance(); //保证唯一一个实例
    void setDownloadLayout(QWidget *p); //设置布局
    QLayout *getDownloadLayout(); //获取布局

private:
    DownloadLayout()
    {

    }

    ~DownloadLayout()    //析构函数为私有
    {
    }

    //静态数据成员，类中声明，类外必须定义
    static DownloadLayout *instance;
    QLayout *m_layout;
    QWidget* m_wg;

    //它的唯一工作就是在析构函数中删除Singleton的实例
    class Garbo
    {
    public:
        ~Garbo()
        {
          if(NULL != DownloadLayout::instance)
          {
            delete DownloadLayout::instance;
            DownloadLayout::instance = NULL;
            cout << "instance is detele";
          }
        }
    };

    //定义一个静态成员变量，程序结束时，系统会自动调用它的析构函数
    //static类的析构函数在main()退出后调用
    static Garbo temp; //静态数据成员，类中声明，类外定义
};


#endif // DOWNLOADLAYOUT_H
