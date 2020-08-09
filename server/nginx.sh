#!/bin/bash

if [ $# -eq 0 ];then
    echo "please input the argument: "
    echo "  start    -----> start the nginx"
    echo "  stop     -----> stop  the nginx"
    echo "  reload   -----> reload the nginx"
    exit 1
fi

case $1 in
    start)
        sudo /usr/local/nginx/sbin/nginx 
        if [ $? -eq 0 ];then
            echo "nginx start success ..."
        else
            echo "nginx start fail ..."
        fi
        ;;
    stop)
        sudo /usr/local/nginx/sbin/nginx -s quit
        if [ $? -eq 0 ];then
            echo "nginx stop success ..."
        else
            echo "nginx stop fail ..."
        fi
        ;;
    reload)
        sudo /usr/local/nginx/sbin/nginx -s reload
        if [ $? -eq 0 ];then
            echo "nginx reload success ..."
        else
            echo "nginx reload fail ..."
        fi
        ;;
    *)
        echo "do nothing ..."
        ;;
esac

