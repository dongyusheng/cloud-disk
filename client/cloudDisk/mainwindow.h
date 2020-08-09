#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

#include <QMainWindow>
#include "common/common.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // 显示主窗口
    void showMainWindow();
    // 处理信号
    void managerSignals();
    // 重新登陆
    void loginAgain();

signals:
    // 切换用户按钮信号
    void changeUser();

protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::MainWindow *ui;

    Common m_common;
};

#endif // MAINWINDOW_H
