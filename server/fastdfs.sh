#!/bin/bash

# 关闭tracker 和 storage服务
tracker_start()
{
    ps aux | grep fdfs_trackerd | grep -v grep > /dev/null
    if [ $? -eq 0 ];then
        echo "fdfs_trackerd 已经在运行中, 无需启动..."
    else
        sudo fdfs_trackerd  /etc/fdfs/tracker.conf
        if [ $? -ne 0 ];then
            echo "tracker start failed ..."
        else
            echo "tracker start success ..."
        fi
    fi
}

storage_start()
{
    ps aux | grep fdfs_storaged | grep -v grep > /dev/null
    if [ $? -eq 0 ];then
        echo "fdfs_storaged 已经在运行中, 无需启动..."
    else
        sudo fdfs_storaged  /etc/fdfs/storage.conf
        if [ $? -ne 0 ];then
            echo "storage start failed ..."
        else
            echo "storage start success ..."
        fi
    fi
}

if [ $# -eq 0 ];then
    echo "Operation:"
    echo "  start storage please input argument: storage"
    echo "  start tracker please input argument: tracker"
    echo "  start storage && tracker please input argument: all"
    echo "  stop storage && tracker input argument: stop"
    exit 0
fi


case $1 in
    storage)
        storage_start
        ;;
    tracker)
        tracker_start
        ;;
    all)
        storage_start
        tracker_start
        ;;
    stop)
        sudo fdfs_trackerd /etc/fdfs/tracker.conf stop
        sudo fdfs_storaged /etc/fdfs/storage.conf stop
        ;;
    *)
        echo "nothing ......"
esac
