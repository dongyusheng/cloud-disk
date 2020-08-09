#!/bin/bash

NAME=conf/redis
FILE=redis.pid
# 判断redis目录是否存在, 如果不存在则创建
is_directory()
{
    if [ ! -d $1 ]; then
        echo "$1 目录创建中..."
        mkdir $1
        if [ $? -ne 0 ];then
            echo "$1 目录创建失败, ~~~~(>_<)~~~~"
            exit 1
        fi
    fi
}

# 判断redis目录是否存在, 如果不存在则创建
is_regfile()
{
    if [ ! -f $1 ]; then
        #statements
        echo "$1 file not exist..."
        return 1
    fi
    return 0
}

# 根据参数设置redis状态
if [[ $1 = "" ]];then
    echo "please input argument:"
    echo "  start: start redis server"
    echo "  stop: stop redis server"
    echo "  status: show the redis server status"
    exit 1
fi

# 函数调用
is_directory $NAME

case $1 in
    start)
        # 判断 redis-server 进程是否已经启动...
        ps aux | grep "redis-server" | grep -v grep > /dev/null
        if [ $? -eq 0 ];then
            echo "Redis server is runing ..."
        else
            # 删除$FILE 文件
            unlink "$NAME/$FILE"

            echo "Redis starting ..."
            redis-server ./conf/redis/redis.conf
            if [ $? -eq 0 ];then
                echo "Redis server start success!!!"
                # 休眠1s, 等待pid文件被创建出来, 再进行后续判断
                sleep 1
                if is_regfile "$NAME/$FILE";then
                    printf "****** Redis server PID: [ %s ] ******\n" $(cat "$NAME/$FILE")
                    printf "****** Redis server PORT: [ %s ] ******\n" $(awk '/^port /{print $2}' "./conf/redis/redis.conf")
                fi
            fi
        fi
        ;;
    stop)
        # 判断 redis-server 进程是否已经启动...
        ps aux | grep "redis-server" | grep -v grep > /dev/null
        if [ $? -ne 0 ];then
            echo "Redis server is not runing..."
            exit 1
        fi
        echo "Redis stopping ..."
        # 判断pid文件是否存在
        if is_regfile "$NAME/$FILE"; then
            # 读进程文件
            echo "### 通过 redis.pid文件 方式关闭进程 ###"
            PID=$(cat "$NAME/$FILE")
        else
            # 查找进程ID
            echo "### 通过 查找进程ID 方式关闭进程 ###"
            PID=$(ps aux | grep "redis-server" | grep -v grep | awk '{print $2}')
        fi
        echo Redis server pid = $PID
        kill -9 $PID
        if [ $? -ne 0 ]; then
            echo "Redis server stop fail ..."
        else
            echo "Redis server stop success!!!"
        fi
        ;;
    status)
        ps aux | grep "redis-server" | grep -v grep > /dev/null
        if [ $? -eq 0 ];then
            echo "Redis server is running..."
        else
            echo "Redis server is not running ..."
        fi
        ;;
    *)
        echo "do nothing ..."
        ;;
esac
