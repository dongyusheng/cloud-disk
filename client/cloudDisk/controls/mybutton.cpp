#include "mybutton.h"

MyButton::MyButton(QWidget *parent) :
    QWidget(parent)
{
    // 初始化
    startxStep = 8;
    startyStep = 8;
    widthStep = 16;
    heightStep = 16;
}

MyButton::MyButton(const QString &img, QWidget *parent) :
    QWidget(parent)
{
    // 初始化
    imagePath = img;
    startxStep = 8;
    startyStep = 8;
    widthStep = 16;
    heightStep = 16;
}

/*
 * paint event - 刷新背景图片
*/
void MyButton::paintEvent(QPaintEvent *)
{
    QPainter paint(this); // create a painter
    paint.drawPixmap(0, 0, width(), height(), QPixmap(imagePath));
}

/*
 * 设置背景图片路径
*/
void MyButton::setImage(const QString &str)
{
    // save the image path
    this->imagePath = str;
    this->update();         // update the window
}


void MyButton::mousePressEvent(QMouseEvent *)
{
    // 按下鼠标左键按钮窗口放大显示
    this->setFixedSize(width()+widthStep, height()+heightStep);
    this->setGeometry(x()-startxStep, y()-startyStep, width()+widthStep, height()+heightStep);
    emit pressed();
}

void MyButton::mouseReleaseEvent(QMouseEvent *)
{
    // 释放鼠标键
    this->setFixedSize(width()-widthStep, height()-heightStep);
    this->setGeometry(x()+startxStep, y()+startyStep, width()-widthStep, height()-heightStep);
    emit released();
}
