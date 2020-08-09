#ifndef BUTTONGROUP_H
#define BUTTONGROUP_H
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

#include <QWidget>
#include <QSignalMapper>
#include <QMap>

namespace Ui {
class ButtonGroup;
}

class QToolButton;
enum Page{MYFILE, SHARE, TRANKING, TRANSFER, SWITCHUSR};
class ButtonGroup : public QWidget
{
    Q_OBJECT

public:
    explicit ButtonGroup(QWidget *parent = 0);
    ~ButtonGroup();
    void defaulfPage();

public slots:
    // 按钮处理函数
    void slotButtonClick(Page cur);
    void slotButtonClick(QString text);
    void setParent(QWidget *parent);


protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);

signals:
    void sigMyFile();
    void sigShareList();
    void sigDownload();
    void sigTransform();
    void sigSwitchUser();
    void closeWindow();
    void minWindow();
    void maxWindow();

private:
    Ui::ButtonGroup *ui;

    QPoint m_pos;
    QWidget* m_parent;
    QSignalMapper* m_mapper;
    QToolButton* m_curBtn;
    QMap<QString, QToolButton*> m_btns;
    QMap<Page, QString> m_pages;
};

#endif // BUTTONGROUP_H
