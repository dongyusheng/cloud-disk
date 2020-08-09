#include "uploadtask.h"
#include <QFileInfo>
#include "uploadlayout.h"
#include "common/common.h"
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

//静态数据成员，类中声明，类外必须定义
UploadTask * UploadTask::instance = new UploadTask;

//static类的析构函数在main()退出后调用
UploadTask::Garbo UploadTask::temp; //静态数据成员，类中声明，类外定义

UploadTask * UploadTask::getInstance()
{
    return instance;
}

//追加上传文件到上传列表中
//参数：path 上传文件路径
//失败：
//  -1: 文件大于30m
//  -2：上传的文件是否已经在上传队列中
//  -3: 打开文件失败
//  -4: 获取布局失败
int UploadTask::appendUploadList(QString path)
{
    qint64 size = QFileInfo( path ).size();
    if(size > 30*1024*1024) //最大文件只能是30M
    {
        cout << "file is to big\n";
        return -1;
    }


    //遍历查看一下，下载的文件是否已经在上传队列中
    for(int i = 0; i != list.size(); ++i)
    {
        if( list.at(i)->path == path) //list[i]->path
        {
            cout << QFileInfo( path ).fileName() << " 已经在上传队列中 ";
            return -2;
        }
    }

    QFile *file = new QFile(path); //文件指针分配空间

    if(!file->open(QIODevice::ReadOnly))
    {
        //如果打开文件失败，则删除 file，并使 file 指针为 0，然后返回
        cout << "file open error";

        delete file;
        file = NULL;
        return -3;
    }

    //获取文件属性信息
    QFileInfo info(path);

     //动态创建节点
    Common mc;
    UploadFileInfo *tmp = new UploadFileInfo;
    tmp->md5 = mc.getFileMd5(path); //获取文件的md5码, common.h
    tmp->file = file; //文件指针
    tmp->fileName = info.fileName();//文件名字
    tmp->size = size; //文件大小
    tmp->path = path; //文件路径
    tmp->isUpload = false;//当前文件没有被上传

    DataProgress *p = new DataProgress; //创建进度条
    p->setFileName(tmp->fileName); //设置文件名字
    tmp->dp = p;

    //获取布局
    UploadLayout *pUpload = UploadLayout::getInstance();
    if(pUpload == NULL)
    {
        cout << "UploadTask::getInstance() == NULL";
        return -4;
    }
    QVBoxLayout *layout = (QVBoxLayout*)pUpload->getUploadLayout();
    // 添加到布局, 最后一个是弹簧, 插入到弹簧上边
    layout->insertWidget(layout->count()-1, p);

    cout << tmp->fileName.toUtf8().data() << "已经放在上传列表";

    //插入节点
    list.append(tmp);

    return 0;
}

bool UploadTask::isEmpty() //判断上传队列释放为空
{
    return list.isEmpty();
}

bool UploadTask::isUpload() //是否有文件正在上传
{
    //遍历队列
    for(int i = 0; i != list.size(); ++i)
    {
        if( list.at(i)->isUpload == true) //说明有上传任务，不能添加新任务
        {

            return true;
        }
    }

    return false;
}

// 取出第0个上传任务，如果任务队列没有任务在上传，设置第0个任务上传
UploadFileInfo *UploadTask::takeTask()
{
    //取出第一个任务
    UploadFileInfo *tmp = list.at(0);
    list.at(0)->isUpload = true; //标志位，设置此文件在上传

    return tmp;
}

//删除上传完成的任务
void UploadTask::dealUploadTask()
{
    //遍历队列
    for(int i = 0; i != list.size(); ++i)
    {
        if( list.at(i)->isUpload == true) //说明有下载任务
        {
            //移除此文件，因为已经上传完成了
            UploadFileInfo *tmp = list.takeAt(i);

            //获取布局
            UploadLayout *pUpload = UploadLayout::getInstance();
            QLayout *layout = pUpload->getUploadLayout();
            layout->removeWidget(tmp->dp); //从布局中移除控件

            //关闭打开的文件指针
            QFile *file = tmp->file;
            file->close();
            delete file;

            delete tmp->dp;
            delete tmp; //释放空间

            return;
        }
    }
}

//清空上传列表
void UploadTask::clearList()
{
    int n = list.size();
    for(int i = 0; i < n; ++i)
    {
        UploadFileInfo *tmp = list.takeFirst();
        delete tmp;
    }
}
