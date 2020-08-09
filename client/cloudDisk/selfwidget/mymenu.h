#ifndef MYMENU_H
#define MYMENU_H
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

#include <QMenu>

class MyMenu : public QMenu
{
    Q_OBJECT
public:
    explicit MyMenu(QWidget *parent = 0);

signals:

public slots:
};

#endif // MYMENU_H
