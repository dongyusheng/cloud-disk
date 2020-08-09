#ifndef DATAPROGRESS_H
#define DATAPROGRESS_H
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

#include <QWidget>

namespace Ui {
class DataProgress;
}

//上传，下载进度控件
class DataProgress : public QWidget
{
    Q_OBJECT

public:
    explicit DataProgress(QWidget *parent = 0);
    ~DataProgress();

    void setFileName(QString name = "测试"); //设置文件名字
    void setProgress(int value = 0, int max = 100); //设置进度条的当前值value, 最大值max

private:
    Ui::DataProgress *ui;
};

#endif // DATAPROGRESS_H
