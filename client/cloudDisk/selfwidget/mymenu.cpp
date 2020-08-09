#include "mymenu.h"
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

MyMenu::MyMenu(QWidget *parent) : QMenu(parent)
{
    setStyleSheet(
            "background-color:rgba(202, 245, 238, 80);"
            "color:rgb(255, 255, 0);"
            "font: 14pt \"新宋体\";"
    );
}
