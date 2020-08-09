#ifndef TRANSFER_H
#define TRANSFER_H
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

#include <QWidget>
#include "common/common.h"

namespace Ui {
class Transfer;
}

class Transfer : public QWidget
{
    Q_OBJECT

public:
    explicit Transfer(QWidget *parent = 0);
    ~Transfer();

    // 显示数据传输记录
    void dispayDataRecord(QString path=RECORDDIR);
    // 显示上传窗口
    void showUpload();
    // 显示下载窗口
    void showDownload();

signals:
    void currentTabSignal(QString); // 告诉主界面，当前是哪个tab

private:
    Ui::Transfer *ui;
};

#endif // TRANSFER_H
