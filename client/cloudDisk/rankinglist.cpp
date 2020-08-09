#include "rankinglist.h"
#include "ui_rankinglist.h"
#include "common/logininfoinstance.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

RankingList::RankingList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RankingList)
{
    ui->setupUi(this);

    // 设置TableWidget表头和一些属性
    initTableWidget();

    m_manager = Common::getNetManager();
}

RankingList::~RankingList()
{
    delete ui;
}

// 设置TableWidget表头和一些属性
void RankingList::initTableWidget()
{
    // 表头相关设置
    // 设置列数, 3列：排名、文件名、下载量
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(300); // 设列的宽度

    // 设置表头不可点击（默认点击后进行排序）
    ui->tableWidget->horizontalHeader()-> setSectionsClickable(false);


    // 设置表头内容
    QStringList header;
    header.append("排名");
    header.append("文件名");
    header.append("下载量");
    ui->tableWidget->setHorizontalHeaderLabels(header);

    // 设置字体
    QFont font = ui->tableWidget->horizontalHeader()->font(); // 获取表头原来的字体
    font.setBold(true);// 字体设置粗体
    ui->tableWidget->horizontalHeader()->setFont(font);

    ui->tableWidget->verticalHeader()->setDefaultSectionSize(40); // 设置处垂直方向高度
    // ui->tableWidget->setFrameShape(QFrame::NoFrame); // 设置无边框
    // ui->tableWidget->setShowGrid(false); // 设置不显示格子线
    ui->tableWidget->verticalHeader()->setVisible(false); // 设置垂直头不可见，不自动显示行号
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);   // 单行选择
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // 设置不可编辑
    // ui->tableWidget->horizontalHeader()->resizeSection(0, 150); // 设置表头第一列的宽度为150
    // ui->tableWidget->horizontalHeader()->setFixedHeight(40);    // 设置表头的高度

    // 通过样式表，设置表头背景色
    ui->tableWidget->horizontalHeader()->setStyleSheet(
                "QHeaderView::section{"
                "background: skyblue;"
                "font: 16pt \"新宋体\";"
                "height: 35px;"
                "border:1px solid #c7f0ea;"
                "}");

    // ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    // 设置第0列的宽度
    ui->tableWidget->setColumnWidth(0,100);

    // 设置列宽策略，使列自适应宽度，所有列平均分来填充空白部分
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
}

// 清空文件列表
void RankingList::clearshareFileList()
{
    int n = m_list.size();
    for(int i = 0; i < n; ++i)
    {
        RankingFileInfo *tmp = m_list.takeFirst();
        delete tmp;
    }
}

// 显示用户的文件列表
void RankingList::refreshFiles()
{
    // =========================>先获取共享文件数目<=========================
    m_userFilesCount = 0;

    // 获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance(); // 获取单例

    // 127.0.0.1:80/sharefiles&cmd=count		// 获取共享文件数目
    QString url = QString("http:// %1:%2/sharefiles?cmd=count").arg(login->getIp()).arg(login->getPort());

    // 发送get请求
    QNetworkReply * reply = m_manager->get( QNetworkRequest( QUrl(url)) );
    if(reply == NULL)
    {
        cout << "reply == NULL";
        return;
    }

    // 获取请求的数据完成时，就会发送信号SIGNAL(finished())
    connect(reply, &QNetworkReply::finished, [=]()
    {
        if (reply->error() != QNetworkReply::NoError) // 有错误
        {
            cout << reply->errorString();
            reply->deleteLater(); // 释放资源
            return;
        }

        // 服务器返回用户文件个数
        QByteArray array = reply->readAll();

        reply->deleteLater();

        // 转换为long
        m_userFilesCount = array.toLong();
        cout << "userFilesCount = " << m_userFilesCount;

        // 清空文件列表信息
        clearshareFileList();

        if(m_userFilesCount > 0)
        {
            // 说明共享列表有文件，获取文件列表
            m_start = 0;  // 从0开始
            m_count = 10; // 每次请求10个

            // 获取新的文件列表信息
            getUserFilesList();
        }
        else // 没有文件
        {
            // cout << "userFilesCount <= 0";

            // 更新排行版列表
            refreshList();
        }
    });
}

QByteArray RankingList::setFilesListJson(int start, int count)
{
    /*
        {
            "start": 0,
            "count": 10
        }
    */
    QMap<QString, QVariant> tmp;
    tmp.insert("start", start);
    tmp.insert("count", count);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if ( jsonDocument.isNull() )
    {
        cout << " jsonDocument.isNull() ";
        return "";
    }

    return jsonDocument.toJson();
}

void RankingList::getUserFilesList()
{
    // 遍历数目，结束条件处理
    if(m_userFilesCount <= 0) // 结束条件，这个条件很重要，函数递归的结束条件
    {
        cout << "获取共享文件列表条件结束";
        // 更新排行版列表
        refreshList();

        return;
    }
    else if(m_count > m_userFilesCount) // 如果请求文件数量大于共享的文件数目
    {
        m_count = m_userFilesCount;
    }


    QNetworkRequest request; // 请求对象

    // 获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance(); // 获取单例

    // 按下载量降序127.0.0.1:80/sharefiles?cmd=pvdesc
    QString url;
    url = QString("http:// %1:%2/sharefiles?cmd=pvdesc").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); // 设置url

    // qt默认的请求头
    // request.setRawHeader("Content-Type","text/html");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    /*
    {
        "start": 0,
        "count": 10
    }
    */
    QByteArray data = setFilesListJson(m_start, m_count);

    // 改变文件起点位置
    m_start += m_count;
    m_userFilesCount -= m_count; // 文件数量递减

    // 发送post请求
    QNetworkReply * reply = m_manager->post( request, data );
    if(reply == NULL)
    {
        cout << "reply == NULL";
        return;
    }

    // 获取请求的数据完成时，就会发送信号SIGNAL(finished())
    connect(reply, &QNetworkReply::finished, [=]()
    {
        if (reply->error() != QNetworkReply::NoError) // 有错误
        {
            cout << reply->errorString();
            reply->deleteLater(); // 释放资源
            return;
        }

        // 服务器返回用户的数据
        QByteArray array = reply->readAll();

        reply->deleteLater();

        // 不是错误码就处理文件列表json信息
        if("015" != m_cm.getCode(array) ) // common.h
        {
            cout << array.data();
            getFileJsonInfo(array);// 解析文件列表json信息，存放在文件列表中

            // 继续获取共享文件列表
            getUserFilesList();

        }

    });
}

void RankingList::getFileJsonInfo(QByteArray data)
{
    QJsonParseError error;

    /*
    {
        "filename": "test.mp4",
        "pv": 0
    }
    */

    // -- filename 文件名字
    // -- pv 文件下载量，默认值为0，下载一次加1

    // 将来源数据json转化为JsonDocument
    // 由QByteArray对象构造一个QJsonDocument对象，用于我们的读写操作
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error == QJsonParseError::NoError) // 没有出错
    {
        if (doc.isNull() || doc.isEmpty())
        {
            cout << "doc.isNull() || doc.isEmpty()";
            return;
        }

        if( doc.isObject())
        {
            // QJsonObject json对象，描述json数据中{}括起来部分
            QJsonObject obj = doc.object();// 取得最外层这个大对象

            // 获取games所对应的数组
            // QJsonArray json数组，描述json数据中[]括起来部分
            QJsonArray array = obj.value("files").toArray();

            int size = array.size();   // 数组个数
            cout << "size = " << size;

            for(int i = 0; i < size; ++i)
            {
                QJsonObject tmp = array[i].toObject(); // 取第i个对象


                /*
                // 文件信息
                struct RankingFileInfo
                {
                    QString filename;   // 文件名字
                    int pv;             // 下载量
                };
                */

                RankingFileInfo *info = new RankingFileInfo;
                info->filename = tmp.value("filename").toString(); // 文件名字
                info->pv = tmp.value("pv").toInt(); // 下载量

                // list添加节点
                m_list.append(info);
            }
        }
    }
    else
    {
        cout << "err = " << error.errorString();
    }
}

void RankingList::refreshList()
{
    int rowCount = ui->tableWidget->rowCount(); // 获取表单行数
    for(int i = 0; i < rowCount; ++i)
    {
        // 参数为0，不是i，自动delete里面的item
        ui->tableWidget->removeRow(0);
    }

    int n = m_list.size(); // 元素个数
    // cout << "list.size() = " << n;
    rowCount = 0;
    for(int i = 0; i < n; ++i)
    {
        RankingFileInfo *tmp = m_list.at(i);
        ui->tableWidget->insertRow(rowCount); // 插入新行

        // 新建item
        QTableWidgetItem *item1 = new QTableWidgetItem;
        QTableWidgetItem *item2 = new QTableWidgetItem;
        QTableWidgetItem *item3 = new QTableWidgetItem;

        // 设置字体显示风格
        item1->setTextAlignment(Qt::AlignHCenter |  Qt::AlignVCenter);
        item2->setTextAlignment(Qt::AlignLeft |  Qt::AlignVCenter);
        item3->setTextAlignment(Qt::AlignHCenter |  Qt::AlignVCenter);

        // 排行
        // 字体大写
        QFont font;
        font.setPointSize(15); // 设置字体大小
        item1->setFont( font ); // 设置字体
        item1->setText(QString::number(i+1));

        // 文件名
        item2->setText(tmp->filename);

        // 下载量
        item3->setText(QString::number(tmp->pv));

        // 设置item
        ui->tableWidget->setItem(rowCount, 0, item1);
        ui->tableWidget->setItem(rowCount, 1, item2);
        ui->tableWidget->setItem(rowCount, 2, item3);

        rowCount++;// 行++
    }
}
