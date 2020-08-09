/**
 * @file dealsharefile_cgi.c
 * @brief  共享文件pv字段处理、取消分享、转存文件cgi程序
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
#include "redis_keys.h"
#include "redis_op.h"
#include <sys/time.h>

#define DEALSHAREFILE_LOG_MODULE       "cgi"
#define DEALSHAREFILE_LOG_PROC         "dealsharefile"

//mysql 数据库配置信息 用户名， 密码， 数据库名称
static char mysql_user[128] = {0};
static char mysql_pwd[128] = {0};
static char mysql_db[128] = {0};

//redis 服务器ip、端口
static char redis_ip[30] = {0};
static char redis_port[10] = {0};

//读取配置信息
void read_cfg()
{
    //读取mysql数据库配置信息
    get_cfg_value(CFG_PATH, "mysql", "user", mysql_user);
    get_cfg_value(CFG_PATH, "mysql", "password", mysql_pwd);
    get_cfg_value(CFG_PATH, "mysql", "database", mysql_db);
    LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "mysql:[user=%s,pwd=%s,database=%s]", mysql_user, mysql_pwd, mysql_db);

    //读取redis配置信息
    get_cfg_value(CFG_PATH, "redis", "ip", redis_ip);
    get_cfg_value(CFG_PATH, "redis", "port", redis_port);
    LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "redis:[ip=%s,port=%s]\n", redis_ip, redis_port);
}

//解析的json包
int get_json_info(char *buf, char *user, char *md5, char *filename)
{
    int ret = 0;

    /*json数据如下
    {
    "user": "milo",
    "md5": "xxx",
    "filename": "xxx"
    }
    */

    //解析json包
    //解析一个json字符串为cJSON对象
    cJSON * root = cJSON_Parse(buf);
    if(NULL == root)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "cJSON_Parse err\n");
        ret = -1;
        goto END;
    }

    //返回指定字符串对应的json对象
    //用户
    cJSON *child1 = cJSON_GetObjectItem(root, "user");
    if(NULL == child1)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    //LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "child1->valuestring = %s\n", child1->valuestring);
    strcpy(user, child1->valuestring); //拷贝内容

    //文件md5码
    cJSON *child2 = cJSON_GetObjectItem(root, "md5");
    if(NULL == child2)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    strcpy(md5, child2->valuestring); //拷贝内容

    //文件名字
    cJSON *child3 = cJSON_GetObjectItem(root, "filename");
    if(NULL == child3)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    strcpy(filename, child3->valuestring); //拷贝内容


END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }

    return ret;
}

//文件下载标志处理
int pv_file(char *md5, char *filename)
{
    int ret = 0;
    char sql_cmd[SQL_MAX_LEN] = {0};
    MYSQL *conn = NULL;
    char *out = NULL;
    char tmp[512] = {0};
    int ret2 = 0;
    redisContext * redis_conn = NULL;
    char fileid[1024] = {0};

    //连接redis数据库
    redis_conn = rop_connectdb_nopwd(redis_ip, redis_port);
    if (redis_conn == NULL)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "redis connected error");
        ret = -1;
        goto END;
    }

    //文件标示，md5+文件名
    sprintf(fileid, "%s%s", md5, filename);

    //connect the database
    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "msql_conn err\n");
        ret = -1;
        goto END;
    }

    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn, "set names utf8");

    //===1、mysql的下载量+1(mysql操作)
    //sql语句
    //查看该共享文件的pv字段
    sprintf(sql_cmd, "select pv from share_file_list where md5 = '%s' and file_name = '%s'", md5, filename);
    LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "sql:%s\n", sql_cmd);

    ret2 = process_result_one(conn, sql_cmd, tmp); //执行sql语句
    int pv = 0;
    if(ret2 == 0)
    {
        pv = atoi(tmp); //pv字段
    }
    else
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "%s 操作失败\n", sql_cmd);
        ret = -1;
        goto END;
    }

    //更新该文件pv字段，+1
    sprintf(sql_cmd, "update share_file_list set pv = %d where md5 = '%s' and file_name = '%s'", pv+1, md5, filename);
    LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "sql:%s\n", sql_cmd);
    if (mysql_query(conn, sql_cmd) != 0)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "%s 操作失败\n", sql_cmd);
        ret = -1;
        goto END;
    }

    //===2、判断元素是否在集合中(redis操作)
    ret2 = rop_zset_exit(redis_conn, FILE_PUBLIC_ZSET, fileid);
    if(ret2 == 1) //存在
    {//===3、如果存在，有序集合score+1
        ret = rop_zset_increment(redis_conn, FILE_PUBLIC_ZSET, fileid);
        if(ret != 0)
        {
            LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "rop_zset_increment 操作失败\n");
        }
    }
    else if(ret2 == 0) //不存在
    {//===4、如果不存在，从mysql导入数据
        //===5、redis集合中增加一个元素(redis操作)
        rop_zset_add(redis_conn, FILE_PUBLIC_ZSET, pv+1, fileid);

        //===6、redis对应的hash也需要变化 (redis操作)
        //     fileid ------>  filename
        rop_hash_set(redis_conn, FILE_NAME_HASH, fileid, filename);

    }
    else//出错
    {
        ret = -1;
        goto END;
    }


END:
    /*
    下载文件pv字段处理
        成功：{"code":"016"}
        失败：{"code":"017"}
    */
    out = NULL;
    if(ret == 0)
    {
        out = return_status("016");
    }
    else
    {
        out = return_status("017");
    }

    if(out != NULL)
    {
        printf(out);//给前端反馈信息
        free(out);
    }

    if(redis_conn != NULL)
    {
        rop_disconnect(redis_conn);
    }


    if(conn != NULL)
    {
        mysql_close(conn);
    }

    return ret;
}

//取消分享文件
int cancel_share_file(char *user, char *md5, char *filename)
{
    int ret = 0;
    char sql_cmd[SQL_MAX_LEN] = {0};
    MYSQL *conn = NULL;
    redisContext * redis_conn = NULL;
    char *out = NULL;
    char fileid[1024] = {0};

    //连接redis数据库
    redis_conn = rop_connectdb_nopwd(redis_ip, redis_port);
    if (redis_conn == NULL)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "redis connected error");
        ret = -1;
        goto END;
    }

    //connect the database
    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "msql_conn err\n");
        ret = -1;
        goto END;
    }

    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn, "set names utf8");

    //文件标示，md5+文件名
    sprintf(fileid, "%s%s", md5, filename);

    //===1、mysql记录操作
    //sql语句
    sprintf(sql_cmd, "update user_file_list set shared_status = 0 where user = '%s' and md5 = '%s' and file_name = '%s'", user, md5, filename);

    if (mysql_query(conn, sql_cmd) != 0)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "%s 操作失败\n", sql_cmd);
        ret = -1;
        goto END;
    }

    //查询共享文件数量
    sprintf(sql_cmd, "select count from user_file_count where user = '%s'", "xxx_share_xxx_file_xxx_list_xxx_count_xxx");
    int count = 0;
    char tmp[512] = {0};
    int ret2 = process_result_one(conn, sql_cmd, tmp); //执行sql语句
    if(ret2 != 0)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "%s 操作失败\n", sql_cmd);
        ret = -1;
        goto END;
    }

    //更新用户文件数量count字段
    count = atoi(tmp);
    if(count == 1)
    {
        //删除数据
        sprintf(sql_cmd, "delete from user_file_count where user = '%s'", "xxx_share_xxx_file_xxx_list_xxx_count_xxx");
    }
    else
    {
        sprintf(sql_cmd, "update user_file_count set count = %d where user = '%s'", count-1, "xxx_share_xxx_file_xxx_list_xxx_count_xxx");
    }


    if(mysql_query(conn, sql_cmd) != 0)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "%s 操作失败\n", sql_cmd);
        ret = -1;
        goto END;
    }

    //删除在共享列表的数据
    sprintf(sql_cmd, "delete from share_file_list where user = '%s' and md5 = '%s' and file_name = '%s'", user, md5, filename);
    if(mysql_query(conn, sql_cmd) != 0)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "%s 操作失败\n", sql_cmd);
        ret = -1;
        goto END;
    }

    //===2、redis记录操作
    //有序集合删除指定成员
    ret = rop_zset_zrem(redis_conn, FILE_PUBLIC_ZSET, fileid);
    if(ret != 0)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "rop_zset_zrem 操作失败\n");
        goto END;
    }

    //从hash移除相应记录
    ret = rop_hash_del(redis_conn, FILE_NAME_HASH, fileid);
    if(ret != 0)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "rop_hash_del 操作失败\n");
        goto END;
    }


END:
    /*
    取消分享：
        成功：{"code":"018"}
        失败：{"code":"019"}
    */
    out = NULL;
    if(ret == 0)
    {
        out = return_status("018");
    }
    else
    {
        out = return_status("019");
    }

    if(out != NULL)
    {
        printf(out);//给前端反馈信息
        free(out);
    }

    if(redis_conn != NULL)
    {
        rop_disconnect(redis_conn);
    }


    if(conn != NULL)
    {
        mysql_close(conn);
    }

    return ret;
}

//转存文件
//返回值：0成功，-1转存失败，-2文件已存在
int save_file(char *user, char *md5, char *filename)
{
    int ret = 0;
    char sql_cmd[SQL_MAX_LEN] = {0};
    MYSQL *conn = NULL;
    char *out = NULL;
    int ret2 = 0;
    char tmp[512] = {0};

    //connect the database
    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "msql_conn err\n");
        ret = -1;
        goto END;
    }

    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn, "set names utf8");

    //查看此用户，文件名和md5是否存在，如果存在说明此文件存在
    sprintf(sql_cmd, "select * from user_file_list where user = '%s' and md5 = '%s' and file_name = '%s'", user, md5, filename);
    //返回值： 0成功并保存记录集，1没有记录集，2有记录集但是没有保存，-1失败
    ret2 = process_result_one(conn, sql_cmd, NULL); //执行sql语句, 最后一个参数为NULL
    if(ret2 == 2) //如果有结果，说明此用户已有此文件
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "%s[filename:%s, md5:%s]已存在\n", user, filename, md5);
        ret = -2; //返回-2错误码
        goto END;
    }

    //文件信息表，查找该文件的计数器
    sprintf(sql_cmd, "select count from file_info where md5 = '%s'", md5);
    ret2 = process_result_one(conn, sql_cmd, tmp); //执行sql语句
    if(ret2 != 0)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "%s 操作失败\n", sql_cmd);
        ret = -1;
        goto END;
    }

    int count = atoi(tmp); //字符串转整型，文件计数器
    //LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "count = %s\n", tmp);

    //1、修改file_info中的count字段，+1 （count 文件引用计数）
    sprintf(sql_cmd, "update file_info set count = %d where md5 = '%s'", count+1, md5);
    if(mysql_query(conn, sql_cmd) != 0)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "%s 操作失败\n", sql_cmd);
        ret = -1;
        goto END;
    }

    //2、user_file_list插入一条数据
    //当前时间戳
    struct timeval tv;
    struct tm* ptm;
    char time_str[128];

    //使用函数gettimeofday()函数来得到时间。它的精度可以达到微妙
    gettimeofday(&tv, NULL);
    ptm = localtime(&tv.tv_sec);//把从1970-1-1零点零分到当前时间系统所偏移的秒数时间转换为本地时间
    //strftime() 函数根据区域设置格式化本地时间/日期，函数的功能将时间格式化，或者说格式化一个时间字符串
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", ptm);

    //sql语句
    /*
    -- =============================================== 用户文件列表
    -- user	文件所属用户
    -- md5 文件md5
    -- create_time 文件创建时间
    -- file_name 文件名字
    -- shared_status 共享状态, 0为没有共享， 1为共享
    -- pv 文件下载量，默认值为0，下载一次加1
    */
    sprintf(sql_cmd, "insert into user_file_list(user, md5, create_time, file_name, shared_status, pv) values ('%s', '%s', '%s', '%s', %d, %d)", user, md5, time_str, filename, 0, 0);
    if(mysql_query(conn, sql_cmd) != 0)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "%s 操作失败\n", sql_cmd);
        ret = -1;
        goto END;
    }

    //3、查询用户文件数量，更新该字段
    sprintf(sql_cmd, "select count from user_file_count where user = '%s'", user);
    count = 0;
    //返回值： 0成功并保存记录集，1没有记录集，2有记录集但是没有保存，-1失败
    ret2 = process_result_one(conn, sql_cmd, tmp); //指向sql语句
    if(ret2 == 1) //没有记录
    {
        //插入记录
        sprintf(sql_cmd, " insert into user_file_count (user, count) values('%s', %d)", user, 1);
    }
    else if(ret2 == 0)
    {
        //更新用户文件数量count字段
        count = atoi(tmp);
        sprintf(sql_cmd, "update user_file_count set count = %d where user = '%s'", count+1, user);
    }

    if(mysql_query(conn, sql_cmd) != 0)
    {
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "%s 操作失败\n", sql_cmd);
        ret = -1;
        goto END;
    }

END:
    /*
    返回值：0成功，-1转存失败，-2文件已存在
    转存文件：
        成功：{"code":"020"}
        文件已存在：{"code":"021"}
        失败：{"code":"022"}
    */
    out = NULL;
    if(ret == 0)
    {
        out = return_status("020");
    }
    else if(ret == -1)
    {
        out = return_status("022");
    }
    else if(ret == -2)
    {
        out = return_status("021");
    }

    if(out != NULL)
    {
        printf(out);//给前端反馈信息
        free(out);
    }


    if(conn != NULL)
    {
        mysql_close(conn);
    }

    return ret;
}


int main()
{
    char cmd[20];
    char user[USER_NAME_LEN];   //用户名
    char md5[MD5_LEN];          //文件md5码
    char filename[FILE_NAME_LEN]; //文件名字

    //读取数据库配置信息
    read_cfg();

    //阻塞等待用户连接
    while (FCGI_Accept() >= 0)
    {
         // 获取URL地址 "?" 后面的内容
        char *query = getenv("QUERY_STRING");

        //解析命令
        query_parse_key_value(query, "cmd", cmd, NULL);
        LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "cmd = %s\n", cmd);

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

        if (len <= 0)
        {
            printf("No data from standard input.<p>\n");
            LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "len = 0, No data from standard input\n");
        }
        else
        {
            char buf[4*1024] = {0};
            int ret = 0;
            ret = fread(buf, 1, len, stdin); //从标准输入(web服务器)读取内容
            if(ret == 0)
            {
                LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "fread(buf, 1, len, stdin) err\n");
                continue;
            }

            LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "buf = %s\n", buf);

            get_json_info(buf, user, md5, filename); //解析json信息
            LOG(DEALSHAREFILE_LOG_MODULE, DEALSHAREFILE_LOG_PROC, "user = %s, md5 = %s, file_name = %s\n", user, md5, filename);

            if(strcmp(cmd, "pv") == 0) //文件下载标志处理
            {
                pv_file(md5, filename);
            }
            else if(strcmp(cmd, "cancel") == 0) //取消分享文件
            {
                cancel_share_file(user, md5, filename);
            }
            else if(strcmp(cmd, "save") == 0) //转存文件
            {
                save_file(user, md5, filename);
            }

        }

    }

    return 0;
}


