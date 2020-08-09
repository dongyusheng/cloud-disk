#ifndef UPLOADTASK_H
#define UPLOADTASK_H
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

#include "common.h"
#include <QVBoxLayout>
#include <QFile>
#include "selfwidget/dataprogress.h"

//上传文件信息
struct UploadFileInfo
{
    QString md5;        //文件md5码
    QFile *file;        //文件指针
    QString fileName;   //文件名字
    qint64 size;        //文件大小
    QString path;       //文件路径
    bool isUpload;      //是否已经在上传
    DataProgress *dp;   //上传进度控件
};

//上传任务列表类，单例模式，一个程序只能有一个上传任务列表
class UploadTask
{
public:
    static UploadTask *getInstance(); //保证唯一一个实例


    //追加上传文件到上传列表中
    //参数：path 上传文件路径
    //返回值：成功为0
    //失败：
    //  -1: 文件大于30m
    //  -2：上传的文件是否已经在上传队列中
    //  -3: 打开文件失败
    //  -4: 获取布局失败
    int appendUploadList(QString path);

    bool isEmpty(); //判断上传队列释放为空
    bool isUpload(); //是否有文件正在上传

    //取出第0个上传任务，如果任务队列没有任务在上传，设置第0个任务上传
    UploadFileInfo * takeTask();
    //删除上传完成的任务
    void dealUploadTask();

    void clearList(); //清空上传列表

private:
    UploadTask()    //构造函数为私有
    {
    }

    ~UploadTask()    //析构函数为私有
    {
    }

    //静态数据成员，类中声明，类外必须定义
    static UploadTask *instance;

    //它的唯一工作就是在析构函数中删除Singleton的实例
    class Garbo
    {
    public:
        ~Garbo()
        {
          if(NULL != UploadTask::instance)
          {
            UploadTask::instance->clearList();

            delete UploadTask::instance;
            UploadTask::instance = NULL;
            cout << "instance is detele";
          }
        }
    };

    //定义一个静态成员变量，程序结束时，系统会自动调用它的析构函数
    //static类的析构函数在main()退出后调用
    static Garbo temp; //静态数据成员，类中声明，类外定义

    QList <UploadFileInfo *> list; //上传任务列表
};

#endif // UPLOADTASK_H
