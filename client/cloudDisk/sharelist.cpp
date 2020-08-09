#include "sharelist.h"
#include "ui_sharelist.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFileDialog>
#include "common/logininfoinstance.h"
#include "common/downloadtask.h"
#include "selfwidget/filepropertyinfo.h"
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

ShareList::ShareList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShareList)
{
    ui->setupUi(this);

    // 初始化ListWidget属性
    initListWidget();
    // 添加动作菜单
    addActionMenu();
    m_manager = Common::getNetManager();

    // 定时检查下载队列是否有任务需要下载
    connect(&m_downloadTimer, &QTimer::timeout, [=]()
    {
        // 上传文件处理，取出上传任务列表的队首任务，上传完后，再取下一个任务
        downloadFilesAction();
    });
    // 启动定时器，500毫秒间隔
    // 每个500毫秒，检测下载任务，每一次只能下载一个文件
    m_downloadTimer.start(500);
}

ShareList::~ShareList()
{
    // 清空文件列表
    clearshareFileList();
    // 清空所有item项目
    clearItems();

    delete ui;
}

// 初始化ListWidget属性
void ShareList::initListWidget()
{
    ui->listWidget->setViewMode(QListView::IconMode);   // 设置显示图标模式
    ui->listWidget->setIconSize(QSize(80, 80));         // 设置图标大小
    ui->listWidget->setGridSize(QSize(100, 120));       // 设置item大小

    // 设置QLisView大小改变时，图标的调整模式，默认是固定的，可以改成自动调整
    ui->listWidget->setResizeMode(QListView::Adjust);   // 自动适应布局

    // 设置列表可以拖动，如果想固定不能拖动，使用QListView::Static
    ui->listWidget->setMovement(QListView::Static);
    // 设置图标之间的间距, 当setGridSize()时，此选项无效
    ui->listWidget->setSpacing(30);
    ui->listWidget->setContextMenuPolicy( Qt::CustomContextMenu );
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, [=](const QPoint& pos)
    {
        QListWidgetItem *item = ui->listWidget->itemAt( pos );

        if( item == NULL ) // 没有点图标
        {
            // QPoint QMouseEvent::pos()   这个只是返回相对这个widget(重载了QMouseEvent的widget)的位置。
            // QPoint QMouseEvent::globalPos()  窗口坐标，这个是返回鼠标的全局坐标
            // QPoint QCursor::pos() [static] 返回相对显示器的全局坐标
            // QWidget::pos() : QPoint 这个属性获得的是当前目前控件在父窗口中的位置
            m_menuEmpty->exec( QCursor::pos() ); // 在鼠标点击的地方弹出菜单
        }
        else // 点图标
        {
            ui->listWidget->setCurrentItem(item);
            m_menuItem->exec( QCursor::pos() );
        }
    });
}

void ShareList::addActionMenu()
{
    // ==================菜单1===================
    m_menuItem = new MyMenu( this );

    // 动作1
    m_downloadAction = new QAction("下载",this);
    m_propertyAction = new QAction("属性",this);
    m_cancelAction = new QAction("取消分享",this);
    m_saveAction = new QAction("转存文件",this);

    // 动作1添加到菜单1
    m_menuItem->addAction(m_downloadAction);
    m_menuItem->addAction(m_propertyAction);
    m_menuItem->addAction(m_cancelAction);
    m_menuItem->addAction(m_saveAction);

    // ==================菜单2====================
    m_menuEmpty = new MyMenu( this );
    // 动作2
    m_refreshAction = new QAction("刷新", this);
    // 动作2添加到菜单2
    m_menuEmpty->addAction(m_refreshAction);

    // ====================信号与槽====================
    connect(m_downloadAction, &QAction::triggered, [=]()    // 下载
    {
        cout << "下载动作";
        // 添加需要下载的文件到下载任务列表
        addDownloadFiles();
    });

    connect(m_propertyAction, &QAction::triggered, [=]()    // 属性
    {
        cout << "属性动作";
        // 枚举，Property属性，Cancel取消分享，Save转存文件
        dealSelectdFile(Property); // 处理选中的文件，获取文件属性
    });

    connect(m_cancelAction, &QAction::triggered, [=]()     // 取消分享
    {
        cout << "取消分享";
        // 枚举，Property属性，Cancel取消分享，Save转存文件
        dealSelectdFile(Cancel);
    });

    connect(m_saveAction, &QAction::triggered, [=]()      // 转存文件
    {
        cout << "转存文件";
        // 枚举，Property属性，Cancel取消分享，Save转存文件
        dealSelectdFile(Save);
    });

    connect(m_refreshAction, &QAction::triggered, [=]()    // 刷新
    {
        cout << "刷新动作";
        refreshFiles();// 显示共享的文件列表
    });
}

// 清空文件列表
void ShareList::clearshareFileList()
{
    int n = m_shareFileList.size();
    for(int i = 0; i < n; ++i)
    {
        FileInfo *tmp = m_shareFileList.takeFirst();
        delete tmp;
    }
}

// 清空所有item项目
void ShareList::clearItems()
{
    // 使用QListWidget::count()来统计ListWidget中总共的item数目
    int n = ui->listWidget->count();
    for(int i = 0; i < n; ++i)
    {
        QListWidgetItem *item = ui->listWidget->takeItem(0); // 这里是0，不是i
        delete item;
    }
}

// 刷新listWidget列表
void ShareList::refreshFileItems()
{
    // 清空所有item项目
    clearItems();

    int n = m_shareFileList.size(); // 元素个数
    for(int i = 0; i < n; ++i)
    {
        FileInfo *tmp = m_shareFileList.at(i);
        QListWidgetItem *item = tmp->item;
        // list widget add item
        ui->listWidget->addItem(item);
    }
}

// 刷新用户文件列表
void ShareList::refreshFiles()
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

        // 清空文件列表信息
        clearshareFileList();

        // 转换为long
        m_userFilesCount = array.toLong();
        cout << "userFilesCount = " << m_userFilesCount;
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
            refreshFileItems(); // 更新item
        }
    });
}

QByteArray ShareList::setFilesListJson(int start, int count)
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

// 获取共享文件列表
void ShareList::getUserFilesList()
{
    // 遍历数目，结束条件处理
    if(m_userFilesCount <= 0) // 结束条件，这个条件很重要，函数递归的结束条件
    {
        cout << "获取共享文件列表条件结束";
        refreshFileItems(); // 更新item
        return;
    }
    else if(m_count > m_userFilesCount) // 如果请求文件数量大于共享的文件数目
    {
        m_count = m_userFilesCount;
    }

    QNetworkRequest request; // 请求对象
    // 获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance(); // 获取单例
    // 获取普通共享文件信息 127.0.0.1:80/sharefiles&cmd=normal
    QString url;
    url = QString("http:// %1:%2/sharefiles?cmd=normal").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); // 设置url
    // qt默认的请求头
    // request.setRawHeader("Content-Type","text/html");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

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
            getFileJsonInfo(array);       // 解析文件列表json信息，存放在文件列表中
            // 继续获取共享文件列表
            getUserFilesList();
        }
    });
}

// 解析文件列表json信息，存放在文件列表中
void ShareList::getFileJsonInfo(QByteArray data)
{
    /*
        {
            "user": "milo",
            "md5": "e8ea6031b779ac26c319ddf949ad9d8d",
            "create_time": "2020-06-21 21:35:25",
            "file_name": "test.mp4",
            "share_status": 0,
            "pv": 0,
            "url": "http://192.168.52.139:80/group1/M00/00/00/wKgfbViy2Z2AJ-FTAaM3As-g3Z0782.mp4",
            "size": 27473666,
            "type": "mp4"
        }
            -- user	文件所属用户
            -- md5 文件md5
            -- create_time 文件创建时间
            -- file_name 文件名字
            -- shared_status 共享状态, 0为没有共享， 1为共享
            -- pv 文件下载量，默认值为0，下载一次加1
            -- url 文件url
            -- size 文件大小, 以字节为单位
            -- type 文件类型： png, zip, mp4……
    */
    // 将来源数据json转化为JsonDocument
    // 由QByteArray对象构造一个QJsonDocument对象，用于我们的读写操作
    QJsonParseError error;
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
                    文件信息
                    struct FileInfo
                    {
                        QString md5;            // 文件md5码
                        QString fileName;       // 文件名字
                        QString user;           // 用户
                        QString createTime;     // 上传时间
                        QString url;            // url
                        QString type;           // 文件类型
                        qint64 size;            // 文件大小
                        int shareStatus;        // 是否共享, 1共享， 0不共享
                        int pv;                 // 下载量
                        QListWidgetItem *item;  // list widget 的item
                    };

                    {
                        "user": "milo",
                        "md5": "e8ea6031b779ac26c319ddf949ad9d8d",
                        "create_time": "2020-06-21 21:35:25",
                        "file_name": "test.mp4",
                        "share_status": 0,
                        "pv": 0,
                        "url": "http://192.168.52.139:80/group1/M00/00/00/wKgfbViy2Z2AJ-FTAaM3As-g3Z0782.mp4",
                        "size": 27473666,
                        "type": "mp4"
                    }
                */
                FileInfo *info = new FileInfo;
                info->user = tmp.value("user").toString(); // 用户
                info->md5 = tmp.value("md5").toString(); // 文件md5
                info->createTime = tmp.value("create_time").toString(); // 上传时间
                info->fileName = tmp.value("file_name").toString(); // 文件名字
                info->shareStatus = tmp.value("share_status").toInt(); // 共享状态
                info->pv = tmp.value("pv").toInt(); // 下载量
                info->url = tmp.value("url").toString(); // url
                info->size = tmp.value("size").toInt(); // 文件大小，以字节为单位
                info->type = tmp.value("type").toString();// 文件后缀
                QString type = info->type + ".png";
                info->item = new QListWidgetItem(QIcon( m_cm.getFileType(type) ), info->fileName);
                // list添加节点
                m_shareFileList.append(info);
            }
        }
    }
    else
    {
        cout << "err = " << error.errorString();
    }
}

// 添加需要下载的文件到下载任务列表
void ShareList::addDownloadFiles()
{
    QListWidgetItem *item = ui->listWidget->currentItem();
    if(item == NULL)
    {
        cout << "item == NULL";
        return;
    }

    // 跳转到下载页面
    emit gotoTransfer(TransferStatus::Download);

    // 获取下载列表实例
    DownloadTask *p = DownloadTask::getInstance();
    if(p == NULL)
    {
        cout << "DownloadTask::getInstance() == NULL";
        return;
    }

    for(int i = 0; i < m_shareFileList.size(); ++i)
    {
        if(m_shareFileList.at(i)->item == item)
        {

            QString filePathName = QFileDialog::getSaveFileName(this, "选择保存文件路径", m_shareFileList.at(i)->fileName );
            if(filePathName.isEmpty())
            {
                cout << "filePathName.isEmpty()";
                return;
            }

            /*
               下载文件：
                    成功：{"code":"009"}
                    失败：{"code":"010"}

                追加任务到下载队列
                参数：info：下载文件信息， filePathName：文件保存路径
                成功：0
                失败：
                  -1: 下载的文件是否已经在下载队列中
                  -2: 打开文件失败
            */

            int res = p->appendDownloadList(m_shareFileList.at(i), filePathName, true); // 追加到下载列表
            if(res == -1)
            {
                QMessageBox::warning(this, "任务已存在", "任务已经在下载队列中！！！");
            }
            else if(res == -2) // 打开文件失败
            {
                LoginInfoInstance *login = LoginInfoInstance::getInstance(); // 获取单例
                m_cm.writeRecord(login->getUser(), m_shareFileList.at(i)->fileName, "010"); // 下载文件失败，记录
            }
            break; // 中断条件很重要
        }
    }
}

// 下载文件处理，取出下载任务列表的队首任务，下载完后，再取下一个任务
void ShareList::downloadFilesAction()
{
    DownloadTask *p = DownloadTask::getInstance();
    if(p == NULL)
    {
        cout << "DownloadTask::getInstance() == NULL";
        return;
    }

    if( p->isEmpty() ) // 如果队列为空，说明没有任务
    {
        return;
    }

    if( p->isDownload() ) // 当前时间没有任务在下载，才能下载，单任务
    {
        return;
    }

    // 看是否是共享文件下载任务，是才能往下执行, 如果不是共享文件任务，则中断程序
    if(p->isShareTask() == false)
    {
        return;
    }

    DownloadInfo *tmp = p->takeTask(); // 取下载任务
    QUrl url = tmp->url;
    QFile *file = tmp->file;
    QString md5 = tmp->md5;
    QString filename = tmp->fileName;
    DataProgress *dp = tmp->dp;

    // 发送get请求
    QNetworkReply * reply = m_manager->get( QNetworkRequest(url) );
    if(reply == NULL)
    {
        p->dealDownloadTask(); // 删除任务
        cout << "get err";
        return;
    }

    // 获取请求的数据完成时，就会发送信号SIGNAL(finished())
    connect(reply, &QNetworkReply::finished, [=]()
    {
        cout << "下载完成";
        reply->deleteLater();
        p->dealDownloadTask();// 删除下载任务

        // 获取登陆信息实例
        LoginInfoInstance *login = LoginInfoInstance::getInstance(); // 获取单例
        m_cm.writeRecord(login->getUser(), filename, "010"); // 下载文件成功，记录

        dealFilePv(md5, filename); // 下载文件pv字段处理
    });

    // 当有可用数据时，reply 就会发出readyRead()信号，我们这时就可以将可用的数据保存下来
    connect(reply, &QNetworkReply::readyRead, [=]()
    {
        // 如果文件存在，则写入文件
        if (file != NULL)
        {
            file->write(reply->readAll());
        }
    });

    // 有可用数据更新时
    connect(reply, &QNetworkReply::downloadProgress, [=](qint64 bytesRead, qint64 totalBytes)
    {
        dp->setProgress(bytesRead, totalBytes);// 设置进度
    });
}

QByteArray ShareList::setShareFileJson(QString user, QString md5, QString filename)
{
    /*
        {
            "user": "milo",
            "md5": "xxx",
            "filename": "xxx"
        }
    */
    QMap<QString, QVariant> tmp;
    tmp.insert("user", user);
    tmp.insert("md5", md5);
    tmp.insert("filename", filename);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if ( jsonDocument.isNull() )
    {
        cout << " jsonDocument.isNull() ";
        return "";
    }

    return jsonDocument.toJson();
}

// 下载文件 pv 字段处理
void ShareList::dealFilePv(QString md5, QString fileName)
{
    QNetworkRequest request; // 请求对象

    // 获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance(); // 获取单例

    // 127.0.0.1:80/dealsharefile?cmd=pv
    QString url = QString("http:// %1:%2/dealsharefile?cmd=pv").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); // 设置url
    cout << "url = " << url;

    // qt默认的请求头
    // request.setRawHeader("Content-Type","text/html");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    /*
    {
        "user": "milo",
        "md5": "xxx",
        "filename": "xxx"
    }
    */
    QByteArray data = setShareFileJson( login->getUser(),  md5, fileName); // 设置json包

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

        /*
            下载文件pv字段处理
                成功：{"code":"016"}
                失败：{"code":"017"}
        */
        if("016" == m_cm.getCode(array) ) // common.h
        {
            // 该文件pv字段+1
            for(int i = 0; i < m_shareFileList.size(); ++i)
            {
                FileInfo *info = m_shareFileList.at(i);
                if( info->md5 == md5 && info->fileName == fileName)
                {
                    int pv = info->pv;
                    info->pv = pv+1;
                    break; // 很重要的中断条件
                }
            }
        }
        else
        {
            cout << "下载文件pv字段处理失败";
        }
    });
}

// 处理选中的文件
// cmd取值，Property属性，Cancel取消分享，Save转存文件
void ShareList::dealSelectdFile(ShareList::CMD cmd)
{
    // 获取当前选中的item
    QListWidgetItem *item = ui->listWidget->currentItem();
    if(item == NULL)
    {
        return;
    }

    // 查找文件列表匹配的元素
    for(int i = 0; i < m_shareFileList.size(); ++i)
    {
        if(m_shareFileList.at(i)->item == item)
        {
            if(cmd == Property)// 文件属性
            {
                getFileProperty( m_shareFileList.at(i) ); // 获取属性信息
            }
            else if(cmd == Cancel)// 取消分享
            {
                cancelShareFile( m_shareFileList.at(i) );
            }
            else if(cmd == Save)// 转存文件
            {
                saveFileToMyList( m_shareFileList.at(i) );
            }

            break; // 跳出循环
        }
    }
}

// 获取文件属性信息
void ShareList::getFileProperty(FileInfo *info)
{
    FilePropertyInfo dlg; // 创建对话框
    dlg.setInfo(info);
    dlg.exec();           // 模态显示
}

// 取消分析的文件
void ShareList::cancelShareFile(FileInfo *info)
{
    // 获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance(); // 获取单例

    // 如果此文件不是本登陆用户分享，无法取消分享
    if(login->getUser() != info->user)
    {
        QMessageBox::warning(this, "操作失败", "此文件不是当前登陆用户分享，无法取消分享！！！");
        return;
    }

    // 取消分享操作
    QNetworkRequest request; // 请求对象

    // 取消分享文件
    // 127.0.0.1:80/dealsharefile?cmd=cancel
    QString url = QString("http:// %1:%2/dealsharefile?cmd=cancel").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); // 设置url
    cout << "url = " << url;

    // qt默认的请求头
    // request.setRawHeader("Content-Type","text/html");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    /*
    {
        "user": "milo",
        "md5": "xxx",
        "filename": "xxx"
    }
    */
    QByteArray data = setShareFileJson( login->getUser(),  info->md5, info->fileName); // 设置json包

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

        /*
            取消分享：
                成功：{"code":"018"}
                失败：{"code":"019"}
        */
        if("018" == m_cm.getCode(array) ) // common.h
        {
            // 从文件列表中移除该文件，移除列表视图中此item
            for(int i = 0; i < m_shareFileList.size(); ++i)
            {
                if( m_shareFileList.at(i) == info)
                {
                    QListWidgetItem *item = info->item;
                    // 从列表视图移除此item
                    ui->listWidget->removeItemWidget(item);
                    delete item;

                    m_shareFileList.removeAt(i);
                    delete info;
                    break;      // 很重要的中断条件
                }
            }

            QMessageBox::information(this, "操作成功", "此文件已取消分享！！！");
        }
        else
        {
            QMessageBox::warning(this, "操作失败", "取消分享文件操作失败！！！");
        }
    });
}

// 转存文件
void ShareList::saveFileToMyList(FileInfo *info)
{
    // 获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance(); // 获取单例

    // 如果此文件是本登陆用户分享，则说明是自己的文件，无需再转存
    if(login->getUser() == info->user)
    {
        QMessageBox::warning(this, "操作失败", "此文件本就属于该用户，无需转存！！！");
        return;
    }

    QNetworkRequest request; // 请求对象

    // 转存文件
    // 127.0.0.1:80/dealsharefile?cmd=save
    QString url = QString("http:// %1:%2/dealsharefile?cmd=save").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl( url )); // 设置url
    cout << "url = " << url;

    // qt默认的请求头
    // request.setRawHeader("Content-Type","text/html");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    /*
       {
           "user": "milo",
           "md5": "xxx",
           "filename": "xxx"
       }
   */
    QByteArray data = setShareFileJson( login->getUser(),  info->md5, info->fileName); // 设置json包

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

        /*
           转存文件：
               成功：{"code":"020"}
               文件已存在：{"code":"021"}
               失败：{"code":"022"}
        */
        if("020" == m_cm.getCode(array) ) // common.h
        {
            QMessageBox::information(this, "操作成功", "该文件已保存到该用户列表中！！！");
        }
        else if("021" == m_cm.getCode(array))
        {
            QMessageBox::warning(this, "操作失败", "此文件已存在，无需转存！！！");
        }
        else if("022" == m_cm.getCode(array))
        {
            QMessageBox::warning(this, "操作失败", "文件转存失败！！！");
        }
    });
}
