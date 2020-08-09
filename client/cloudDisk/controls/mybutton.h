#ifndef MYBUTTON_H
#define MYBUTTON_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QEvent>

/*
* 当按下或释放的时候, 窗口会变大或缩小
*/
class MyButton : public QWidget
{
    Q_OBJECT

public:
    explicit MyButton(QWidget *parent = 0);
    explicit MyButton(const QString &img, QWidget *parent = 0);
    void setImage(const QString &str = "");

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    QString imagePath;  // 保存图片路径
    int startxStep, startyStep, widthStep, heightStep;

signals:
    void pressed();
    void released();

public slots:

};

#endif // MYBUTTON_H
