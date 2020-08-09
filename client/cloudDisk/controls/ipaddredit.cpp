#include "ipaddredit.h"
#include <QPainter>
#include <QPen>
#include <QRegExp>
#include <QValidator>
#include <QKeyEvent>

IpPartLineEdit::IpPartLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    next_tab_ = NULL;

    this->setMaxLength(3);
    this->setFrame(false);
    this->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QValidator *validator = new QIntValidator(0, 255, this);
    this->setValidator(validator);

    connect(this, SIGNAL(textEdited(const QString&)), this, SLOT(text_edited(const QString&)));
}

IpPartLineEdit::~IpPartLineEdit(void)
{
}

void IpPartLineEdit::focusInEvent(QFocusEvent *e)
{
    this->selectAll();
    QLineEdit::focusInEvent(e);
}

void IpPartLineEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Period)
    {
        if (next_tab_)
        {
            next_tab_->setFocus();
            next_tab_->selectAll();
        }
    }
    QLineEdit::keyPressEvent(event);
}

void IpPartLineEdit::text_edited(const QString& text)
{
    QIntValidator v(0, 255, this);
    QString ipaddr = text;
    int pos = 0;
    QValidator::State state = v.validate(ipaddr, pos);
    if (state == QValidator::Acceptable)
    {
        if (ipaddr.size() > 1)
        {
            if (ipaddr.size() == 2)
            {
                int ipnum = ipaddr.toInt();

                if (ipnum > 25)
                {
                    if (next_tab_)
                    {
                        next_tab_->setFocus();
                        next_tab_->selectAll();
                    }
                }
            }
            else
            {
                if (next_tab_)
                {
                    next_tab_->setFocus();
                    next_tab_->selectAll();
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////
//////////////////////// Class: IpAddrEdit ///////////////////////////
///////////////////////////////////////////////////////////////////
#include <QHBoxLayout>
#include <QPainter>

IpAddrEdit::IpAddrEdit(QWidget* pParent /* = 0 */)
    : QWidget(pParent)
{
//    QWidget* widget = new QWidget(NULL);
    ip_part1_ = new IpPartLineEdit(this);
    ip_part2_ = new IpPartLineEdit(this);
    ip_part3_ = new IpPartLineEdit(this);
    ip_part4_ = new IpPartLineEdit(this);
    labeldot1_ = new QLabel(this);
    labeldot2_ = new QLabel(this);
    labeldot3_ = new QLabel(this);

    labeldot1_->setText(".");
    labeldot2_->setText(".");
    labeldot3_->setText(".");
    labeldot1_->setFixedWidth(1);
    labeldot2_->setFixedWidth(1);
    labeldot3_->setFixedWidth(1);

    QWidget::setTabOrder(ip_part1_, ip_part2_);
    QWidget::setTabOrder(ip_part2_, ip_part3_);
    QWidget::setTabOrder(ip_part3_, ip_part4_);
    ip_part1_->set_nexttab_edit(ip_part2_);
    ip_part2_->set_nexttab_edit(ip_part3_);
    ip_part3_->set_nexttab_edit(ip_part4_);

    //设置布局
    QHBoxLayout* h_layout = new QHBoxLayout;
    h_layout->addWidget(ip_part1_);
    h_layout->addWidget(labeldot1_);
    h_layout->addWidget(ip_part2_);
    h_layout->addWidget(labeldot2_);
    h_layout->addWidget(ip_part3_);
    h_layout->addWidget(labeldot3_);
    h_layout->addWidget(ip_part4_);
    h_layout->setContentsMargins(2, 2, 2, 2);
    h_layout->setSpacing(0);
    this->setLayout(h_layout);

//    QHBoxLayout* _layout = new QHBoxLayout;
//    _layout->addWidget(widget);
//    _layout->setContentsMargins(0, 0, 0, 0);
//    setLayout(_layout);

    connect(ip_part1_, SIGNAL(textChanged(const QString&)), this, SLOT(textchangedslot(const QString&)));
    connect(ip_part2_, SIGNAL(textChanged(const QString&)), this, SLOT(textchangedslot(const QString&)));
    connect(ip_part3_, SIGNAL(textChanged(const QString&)), this, SLOT(textchangedslot(const QString&)));
    connect(ip_part4_, SIGNAL(textChanged(const QString&)), this, SLOT(textchangedslot(const QString&)));

    connect(ip_part1_, SIGNAL(textEdited (const QString&)), this, SLOT(texteditedslot(const QString&)));
    connect(ip_part2_, SIGNAL(textEdited (const QString&)), this, SLOT(texteditedslot(const QString&)));
    connect(ip_part3_, SIGNAL(textEdited (const QString&)), this, SLOT(texteditedslot(const QString&)));
    connect(ip_part4_, SIGNAL(textEdited (const QString&)), this, SLOT(texteditedslot(const QString&)));

}

IpAddrEdit::~IpAddrEdit()
{

}

void IpAddrEdit::textchangedslot(const QString& /*text*/)
{
    QString ippart1, ippart2, ippart3, ippart4;
    ippart1 = ip_part1_->text();
    ippart2 = ip_part2_->text();
    ippart3 = ip_part3_->text();
    ippart4 = ip_part4_->text();

    QString ipaddr = QString("%1.%2.%3.%4")
                     .arg(ippart1)
                     .arg(ippart2)
                     .arg(ippart3)
                     .arg(ippart4);

    emit textchanged(ipaddr);
}

void IpAddrEdit::texteditedslot(const QString &/*text*/)
{
    QString ippart1, ippart2, ippart3, ippart4;
    ippart1 = ip_part1_->text();
    ippart2 = ip_part2_->text();
    ippart3 = ip_part3_->text();
    ippart4 = ip_part4_->text();

    QString ipaddr = QString("%1.%2.%3.%4")
        .arg(ippart1)
        .arg(ippart2)
        .arg(ippart3)
        .arg(ippart4);

    emit textedited(ipaddr);
}

void IpAddrEdit::settext(const QString &text)
{
    QString ippart1, ippart2, ippart3, ippart4;
    QString qstring_validate = text;

    // IP地址验证
    QRegExp regexp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    QRegExpValidator regexp_validator(regexp, this);
    int nPos = 0;
    QValidator::State state = regexp_validator.validate(qstring_validate, nPos);
    // IP合法
    if (state == QValidator::Acceptable)
    {
        QStringList ippartlist = text.split(".");

        int strcount = ippartlist.size();
        int index = 0;
        if (index < strcount)
        {
            ippart1 = ippartlist.at(index);
        }
        if (++index < strcount)
        {
            ippart2 = ippartlist.at(index);
        }
        if (++index < strcount)
        {
            ippart3 = ippartlist.at(index);
        }
        if (++index < strcount)
        {
            ippart4 = ippartlist.at(index);
        }
    }

    ip_part1_->setText(ippart1);
    ip_part2_->setText(ippart2);
    ip_part3_->setText(ippart3);
    ip_part4_->setText(ippart4);
}

QString IpAddrEdit::text()
{
    QString ippart1, ippart2, ippart3, ippart4;
    ippart1 = ip_part1_->text();
    ippart2 = ip_part2_->text();
    ippart3 = ip_part3_->text();
    ippart4 = ip_part4_->text();

    return QString("%1.%2.%3.%4")
        .arg(ippart1)
        .arg(ippart2)
        .arg(ippart3)
        .arg(ippart4);
}

void IpAddrEdit::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    QPen pen(QColor(209, 209, 209));
    pen.setWidth(4);
    painter.setPen(pen);
    painter.drawRoundedRect(0, 0, width(), height(), 2, 2);
}

void IpAddrEdit::clear()
{
    settext(QString());
}
