#ifndef FILEPROPERTYINFO_H
#define FILEPROPERTYINFO_H
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

#include <QDialog>
#include "common/common.h" //FileInfo

namespace Ui {
class FilePropertyInfo;
}

class FilePropertyInfo : public QDialog
{
    Q_OBJECT

public:
    explicit FilePropertyInfo(QWidget *parent = 0);
    ~FilePropertyInfo();

    //设置内容
    void setInfo(FileInfo *info);

private:
    Ui::FilePropertyInfo *ui;
};

#endif // FILEPROPERTYINFO_H
