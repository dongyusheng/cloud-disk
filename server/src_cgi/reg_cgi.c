/**
 * @file reg_cgi.c
 * @brief  注册事件后CGI程序
 * @version 1.0
 * @date
 */

#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "make_log.h" //日志头文件
#include "util_cgi.h"
#include "deal_mysql.h"
#include "cfg.h"
#include "cJSON.h"
#include <sys/time.h>

#define REG_LOG_MODULE       "cgi"
#define REG_LOG_PROC         "reg"


//解析用户注册信息的json包
int get_reg_info(char *reg_buf, char *user, char *nick_name, char *pwd, char *tel, char *email)
{
    int ret = 0;

    /*json数据如下
    {
        userName:xxxx,
        nickName:xxx,
        firstPwd:xxx,
        phone:xxx,
        email:xxx
    }
    */

    //解析json包
    //解析一个json字符串为cJSON对象
    cJSON * root = cJSON_Parse(reg_buf);
    if(NULL == root)
    {
        LOG(REG_LOG_MODULE, REG_LOG_PROC, "cJSON_Parse err\n");
        ret = -1;
        goto END;
    }

    //返回指定字符串对应的json对象
    //用户
    cJSON *child1 = cJSON_GetObjectItem(root, "userName");
    if(NULL == child1)
    {
        LOG(REG_LOG_MODULE, REG_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    //LOG(REG_LOG_MODULE, REG_LOG_PROC, "child1->valuestring = %s\n", child1->valuestring);
    strcpy(user, child1->valuestring); //拷贝内容

    //昵称
    cJSON *child2 = cJSON_GetObjectItem(root, "nickName");
    if(NULL == child2)
    {
        LOG(REG_LOG_MODULE, REG_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }
    strcpy(nick_name, child2->valuestring); //拷贝内容

    //密码
    cJSON *child3 = cJSON_GetObjectItem(root, "firstPwd");
    if(NULL == child3)
    {
        LOG(REG_LOG_MODULE, REG_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }
    strcpy(pwd, child3->valuestring); //拷贝内容

    //电话
    cJSON *child4 = cJSON_GetObjectItem(root, "phone");
    if(NULL == child4)
    {
        LOG(REG_LOG_MODULE, REG_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }
    strcpy(tel, child4->valuestring); //拷贝内容

    //邮箱
    cJSON *child5 = cJSON_GetObjectItem(root, "email");
    if(NULL == child5)
    {
        LOG(REG_LOG_MODULE, REG_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }
    strcpy(email, child5->valuestring); //拷贝内容

END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }

    return ret;
}

//注册用户，成功返回0，失败返回-1, 该用户已存在返回-2
int user_register( char *reg_buf )
{
    int ret = 0;
    MYSQL *conn = NULL;

    //获取数据库用户名、用户密码、数据库标示等信息
    char mysql_user[256] = {0};
    char mysql_pwd[256] = {0};
    char mysql_db[256] = {0};
    ret = get_mysql_info(mysql_user, mysql_pwd,  mysql_db);
    if(ret != 0)
    {
        goto END;
    }
    LOG(REG_LOG_MODULE, REG_LOG_PROC, "mysql_user = %s, mysql_pwd = %s, mysql_db = %s\n", mysql_user, mysql_pwd, mysql_db);

    //获取注册用户的信息
    char user_name[128];
    char nick_name[128];
    char pwd[128];
    char tel[128];
    char email[128];
    ret = get_reg_info(reg_buf, user_name, nick_name, pwd, tel, email);
    if(ret != 0)
    {
        goto END;
    }
    LOG(REG_LOG_MODULE, REG_LOG_PROC, "user_name = %s, nick_name = %s, pwd = %s, tel = %s, email = %s\n", user_name, nick_name, pwd, tel, email);

    //connect the database
    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        LOG(REG_LOG_MODULE, REG_LOG_PROC, "msql_conn err\n");
        ret = -1;
        goto END;
    }

    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn, "set names utf8");

    char sql_cmd[SQL_MAX_LEN] = {0};

    sprintf(sql_cmd, "select * from user_info where user_name = '%s'", user_name);

    //查看该用户是否存在
    int ret2 = 0;
    //返回值： 0成功并保存记录集，1没有记录集，2有记录集但是没有保存，-1失败
    ret2 = process_result_one(conn, sql_cmd, NULL); //指向sql查询语句
    if(ret2 == 2) //如果存在
    {
        LOG(REG_LOG_MODULE, REG_LOG_PROC, "【%s】该用户已存在\n");
        ret = -2;
        goto END;
    }

    //当前时间戳
    struct timeval tv;
    struct tm* ptm;
    char time_str[128];

    //使用函数gettimeofday()函数来得到时间。它的精度可以达到微妙
    gettimeofday(&tv, NULL);
    ptm = localtime(&tv.tv_sec);//把从1970-1-1零点零分到当前时间系统所偏移的秒数时间转换为本地时间
    //strftime() 函数根据区域设置格式化本地时间/日期，函数的功能将时间格式化，或者说格式化一个时间字符串
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", ptm);

    //sql 语句, 插入注册信息
    sprintf(sql_cmd, "insert into user_info (user_name, nick_name, password, phone, email, create_time) values ('%s', '%s', '%s', '%s', '%s', '%s')", user_name, nick_name, pwd, tel, email, time_str);

    if(mysql_query(conn, sql_cmd) != 0)
    {
        LOG(REG_LOG_MODULE, REG_LOG_PROC, "%s 插入失败：%s\n", sql_cmd, mysql_error(conn));
        ret = -1;
        goto END;
    }

END:
    if(conn != NULL)
    {
        mysql_close(conn); //断开数据库连接
    }

    return ret;
}

int main()
{
    //阻塞等待用户连接
    while (FCGI_Accept() >= 0)
    {
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;

        printf("Content-type: text/html\r\n\r\n");

        if( contentLength == NULL )
        {
            len = 0;
        }
        else
        {
            len = atoi(contentLength); //字符串转整型
        }

        if (len <= 0)//没有登陆用户信息
        {
            printf("No data from standard input.<p>\n");
            LOG(REG_LOG_MODULE, REG_LOG_PROC, "len = 0, No data from standard input\n");
        }
        else //获取登陆用户信息
        {
            char buf[4*1024] = {0};
            int ret = 0;
            char *out = NULL;
            ret = fread(buf, 1, len, stdin); //从标准输入(web服务器)读取内容
            if(ret == 0)
            {
                LOG(REG_LOG_MODULE, REG_LOG_PROC, "fread(buf, 1, len, stdin) err\n");
                continue;
            }

            LOG(REG_LOG_MODULE, REG_LOG_PROC, "buf = %s\n", buf);

            //注册用户，成功返回0，失败返回-1, 该用户已存在返回-2
            /*
            注册：
                成功：{"code":"002"}
                该用户已存在：{"code":"003"}
                失败：{"code":"004"}
            */
            ret = user_register( buf );
            if (ret == 0) //登陆成功
            {
                //返回前端注册情况， 002代表成功
                out = return_status("002");//util_cgi.h
            }
            else if(ret == -1)
            {
                //返回前端注册情况， 004代表失败
                out = return_status("004");//util_cgi.h
            }
            else if(ret == -2)
            {
               out = return_status("003");//util_cgi.h
            }

            if(out != NULL)
            {
                printf(out); //给前端反馈信息
                free(out); //记得释放
            }
        }

    }

    return 0;
}

