#ifndef IPADDREDIT_H
#define IPADDREDIT_H

#include <QLineEdit>
#include <QLabel>

class QWidget;
class QFocusEvent;
class QKeyEvent;

class IpPartLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    IpPartLineEdit(QWidget *parent = 0);
    ~IpPartLineEdit(void);

    void set_nexttab_edit(QLineEdit *nexttab) { next_tab_ = nexttab; }

protected:
    virtual void focusInEvent(QFocusEvent *e);
    virtual void keyPressEvent(QKeyEvent *event);

private slots:
    void text_edited(const QString& text);

private:
    QLineEdit *next_tab_;
};

//////////////////////////////////////////////////////
////////////////////  Class: IpAddrEdit  /////////////////
//////////////////////////////////////////////////////
class IpAddrEdit : public QWidget
{
    Q_OBJECT
public:
    IpAddrEdit(QWidget* pParent = 0);
    ~IpAddrEdit();

    void settext(const QString &text);
    QString text();
    void clear();

signals:
    void textchanged(const QString& text);
    void textedited(const QString &text);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void textchangedslot(const QString& text);
    void texteditedslot(const QString &text);

private:
    IpPartLineEdit *ip_part1_;
    IpPartLineEdit *ip_part2_;
    IpPartLineEdit *ip_part3_;
    IpPartLineEdit *ip_part4_;

    QLabel *labeldot1_;
    QLabel *labeldot2_;
    QLabel *labeldot3_;
};

#endif // IPADDREDIT_H
