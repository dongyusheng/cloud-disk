#include "filepropertyinfo.h"
#include "ui_filepropertyinfo.h"
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

FilePropertyInfo::FilePropertyInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilePropertyInfo)
{
    ui->setupUi(this);

    ui->url->setOpenExternalLinks(true); //label内容作为超链接内容
}

FilePropertyInfo::~FilePropertyInfo()
{
    delete ui;
}

//设置内容
void FilePropertyInfo::setInfo(FileInfo *info)
{
    ui->filename->setText(info->fileName);
    ui->user->setText(info->user);
    ui->time->setText(info->createTime);

    int size = info->size;
    if(size >= 1024 && size < 1024*1024)
    {
         ui->size->setText( QString("%1 KB").arg( size/1024.0 ) );
    }
    //else if(size >= 1024*1024 < size < 1024*1024*1024)
    else
    {
         ui->size->setText( QString("%1 MB").arg( size/1024.0/1024.0 ) );
    }

    ui->pv->setText(QString("被下载 %1 次").arg(info->pv));
    if(info->shareStatus == 0)
    {
        ui->share->setText("没有分享");
    }
    else
    {
        ui->share->setText("已经分享");
    }

    //"<a href="http://www.baidu.com">baidu</a>"
    QString tmp = QString("<a href=\"%1\">%2</a>").arg( info->url ).arg( info->url );
    ui->url->setText(tmp);
}
