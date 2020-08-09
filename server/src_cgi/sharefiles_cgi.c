/**
 * @file sharefiles_cgi.c
 * @brief  共享文件列表展示CGI程序
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
#include "redis_keys.h"
#include "redis_op.h"
#include "cfg.h"
#include "cJSON.h"
#include <sys/time.h>

#define SHAREFILES_LOG_MODULE       "cgi"
#define SHAREFILES_LOG_PROC         "sharefiles"

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
    LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "mysql:[user=%s,pwd=%s,database=%s]", mysql_user, mysql_pwd, mysql_db);

    //读取redis配置信息
    get_cfg_value(CFG_PATH, "redis", "ip", redis_ip);
    get_cfg_value(CFG_PATH, "redis", "port", redis_port);
    LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "redis:[ip=%s,port=%s]\n", redis_ip, redis_port);
}

//获取共享文件个数
void get_share_files_count()
{
    char sql_cmd[SQL_MAX_LEN] = {0};
    MYSQL *conn = NULL;
    long line = 0;

    //connect the database
    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "msql_conn err\n");
        goto END;
    }

    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn, "set names utf8");

    sprintf(sql_cmd, "select count from user_file_count where user='%s'", "xxx_share_xxx_file_xxx_list_xxx_count_xxx");
    char tmp[512] = {0};
    int ret2 = process_result_one(conn, sql_cmd, tmp); //指向sql语句
    if(ret2 != 0)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "%s 操作失败\n", sql_cmd);
        goto END;
    }

    line = atol(tmp); //字符串转长整形

END:
    if(conn != NULL)
    {
        mysql_close(conn);
    }

    LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "line = %ld\n", line);
    printf("%ld", line); //给前端反馈的信息
}

//解析的json包
int get_fileslist_json_info(char *buf, int *p_start, int *p_count)
{
    int ret = 0;

    /*json数据如下
    {
        "start": 0
        "count": 10
    }
    */

    //解析json包
    //解析一个json字符串为cJSON对象
    cJSON * root = cJSON_Parse(buf);
    if(NULL == root)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "cJSON_Parse err\n");
        ret = -1;
        goto END;
    }

    //文件起点
    cJSON *child2 = cJSON_GetObjectItem(root, "start");
    if(NULL == child2)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    *p_start = child2->valueint;

    //文件请求个数
    cJSON *child3 = cJSON_GetObjectItem(root, "count");
    if(NULL == child3)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    *p_count = child3->valueint;

END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }

    return ret;
}

//获取共享文件列表
//获取用户文件信息 127.0.0.1:80/sharefiles&cmd=normal
int get_share_filelist(int start, int count)
{
    int ret = 0;
    char sql_cmd[SQL_MAX_LEN] = {0};
    MYSQL *conn = NULL;
    cJSON *root = NULL;
    cJSON *array =NULL;
    char *out = NULL;
    char *out2 = NULL;
    MYSQL_RES *res_set = NULL;

    //connect the database
    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "msql_conn err\n");
        ret = -1;
        goto END;
    }

    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn, "set names utf8");


    // 构造sql语句
    sprintf(sql_cmd, "select share_file_list.*, file_info.url, file_info.size, file_info.type from file_info, share_file_list where file_info.md5 = share_file_list.md5 limit %d, %d", start, count);

    LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "%s 在操作\n", sql_cmd);

    if (mysql_query(conn, sql_cmd) != 0)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "%s 操作失败: %s\n", sql_cmd, mysql_error(conn));
        ret = -1;
        goto END;
    }

    res_set = mysql_store_result(conn);/*生成结果集*/
    if (res_set == NULL)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "smysql_store_result error!\n");
        ret = -1;
        goto END;
    }

    ulong line = 0;
    //mysql_num_rows接受由mysql_store_result返回的结果结构集，并返回结构集中的行数
    line = mysql_num_rows(res_set);
    if (line == 0)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "mysql_num_rows(res_set) failed\n");
        ret = -1;
        goto END;
    }

    MYSQL_ROW row;

    root = cJSON_CreateObject();
    array = cJSON_CreateArray();
    // mysql_fetch_row从使用mysql_store_result得到的结果结构中提取一行，并把它放到一个行结构中。
    // 当数据用完或发生错误时返回NULL.
    while ((row = mysql_fetch_row(res_set)) != NULL)
    {
         //array[i]:
        cJSON* item = cJSON_CreateObject();

        //mysql_num_fields获取结果中列的个数
        /*for(i = 0; i < mysql_num_fields(res_set); i++)
        {
            if(row[i] != NULL) { }
        }*/

        /*
        {
        "user": "milo",
        "md5": "e8ea6031b779ac26c319ddf949ad9d8d",
        "create_time": "2020-06-21 21:35:25",
        "file_name": "test.mp4",
        "share_status": 1,
        "pv": 0,
        "url": "http://192.168.31.109:80/group1/M00/00/00/wKgfbViy2Z2AJ-FTAaM3As-g3Z0782.mp4",
        "size": 27473666,
         "type": "mp4"
        }
        */

        //column_index = 0;
        int column_index = 1;
        //-- user	文件所属用户
        if(row[column_index] != NULL)
        {
            cJSON_AddStringToObject(item, "user", row[column_index]);
        }

        column_index++;
        //-- md5 文件md5
        if(row[column_index] != NULL)
        {
            cJSON_AddStringToObject(item, "md5", row[column_index]);
        }

        column_index++;
        //-- filename 文件名字
        if(row[column_index] != NULL)
        {
            cJSON_AddStringToObject(item, "file_name", row[column_index]);
        }

        //-- shared_status 共享状态, 0为没有共享， 1为共享
        cJSON_AddNumberToObject(item, "share_status", 1);

        column_index++;
        //-- pv 文件下载量，默认值为0，下载一次加1
        if(row[column_index] != NULL)
        {
            cJSON_AddNumberToObject(item, "pv", atol( row[column_index] ));
        }

        column_index++;
        //-- createtime 文件创建时间
        if(row[column_index] != NULL)
        {
            cJSON_AddStringToObject(item, "create_time", row[column_index]);
        }

        column_index++;
        //-- url 文件url
        if(row[column_index] != NULL)
        {
            cJSON_AddStringToObject(item, "url", row[column_index]);
        }

        column_index++;
        //-- size 文件大小, 以字节为单位
        if(row[column_index] != NULL)
        {
            cJSON_AddNumberToObject(item, "size", atol( row[column_index] ));
        }

        column_index++;
        //-- type 文件类型： png, zip, mp4……
        if(row[column_index] != NULL)
        {
            cJSON_AddStringToObject(item, "type", row[column_index]);
        }

        cJSON_AddItemToArray(array, item);
    }

    cJSON_AddItemToObject(root, "files", array);

    out = cJSON_Print(root);

    LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "%s\n", out);

END:
    if(ret == 0)
    {
        printf("%s", out); //给前端反馈信息
    }
    else
    {   //失败
        out2 = NULL;
        out2 = return_status("015");
    }
    if(out2 != NULL)
    {
        printf(out2); //给前端反馈错误码
        free(out2);
    }

    if(res_set != NULL)
    {
        //完成所有对数据的操作后，调用mysql_free_result来善后处理
        mysql_free_result(res_set);
    }

    if(conn != NULL)
    {
        mysql_close(conn);
    }

    if(root != NULL)
    {
        cJSON_Delete(root);
    }

    if(out != NULL)
    {
        free(out);
    }

    return ret;
}

//获取共享文件排行版
//按下载量降序127.0.0.1:80/sharefiles?cmd=pvdesc
int get_ranking_filelist(int start, int count)
{
    /*
    a) mysql共享文件数量和redis共享文件数量对比，判断是否相等
    b) 如果不相等，清空redis数据，从mysql中导入数据到redis (mysql和redis交互)
    c) 从redis读取数据，给前端反馈相应信息
    */

    int ret = 0;
    char sql_cmd[SQL_MAX_LEN] = {0};
    MYSQL *conn = NULL;
    cJSON *root = NULL;
    RVALUES value = NULL;
    cJSON *array =NULL;
    char *out = NULL;
    char *out2 = NULL;
    char tmp[512] = {0};
    int ret2 = 0;
    MYSQL_RES *res_set = NULL;
    redisContext * redis_conn = NULL;

    //连接redis数据库
    redis_conn = rop_connectdb_nopwd(redis_ip, redis_port);
    if (redis_conn == NULL)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "redis connected error");
        ret = -1;
        goto END;
    }

    //connect the database
    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "msql_conn err\n");
        ret = -1;
        goto END;
    }

    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn, "set names utf8");

    //===1、mysql共享文件数量
    sprintf(sql_cmd, "select count from user_file_count where user='%s'", "xxx_share_xxx_file_xxx_list_xxx_count_xxx");
    ret2 = process_result_one(conn, sql_cmd, tmp); //指向sql语句
    if(ret2 != 0)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "%s 操作失败\n", sql_cmd);
        ret = -1;
        goto END;
    }

    int sql_num = atoi(tmp); //字符串转长整形

    //===2、redis共享文件数量
    int redis_num = rop_zset_zcard(redis_conn, FILE_PUBLIC_ZSET);
    if(redis_num == -1)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "rop_zset_zcard 操作失败\n");
        ret = -1;
        goto END;
    }

    LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "sql_num = %d, redis_num = %d\n", sql_num, redis_num);

    //===3、mysql共享文件数量和redis共享文件数量对比，判断是否相等
    if(redis_num != sql_num)
    {//===4、如果不相等，清空redis数据，重新从mysql中导入数据到redis (mysql和redis交互)

        //a) 清空redis有序数据
        rop_del_key(redis_conn, FILE_PUBLIC_ZSET);
        rop_del_key(redis_conn, FILE_NAME_HASH);

        //b) 从mysql中导入数据到redis
        //sql语句
        strcpy(sql_cmd, "select md5, file_name, pv from share_file_list order by pv desc");

        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "%s 在操作\n", sql_cmd);

        if (mysql_query(conn, sql_cmd) != 0)
        {
            LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "%s 操作失败: %s\n", sql_cmd, mysql_error(conn));
            ret = -1;
            goto END;
        }

        res_set = mysql_store_result(conn);/*生成结果集*/
        if (res_set == NULL)
        {
            LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "smysql_store_result error!\n");
            ret = -1;
            goto END;
        }

        ulong line = 0;
        //mysql_num_rows接受由mysql_store_result返回的结果结构集，并返回结构集中的行数
        line = mysql_num_rows(res_set);
        if (line == 0)
        {
            LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "mysql_num_rows(res_set) failed\n");
            ret = -1;
            goto END;
        }

         MYSQL_ROW row;
        // mysql_fetch_row从使用mysql_store_result得到的结果结构中提取一行，并把它放到一个行结构中。
        // 当数据用完或发生错误时返回NULL.
        while ((row = mysql_fetch_row(res_set)) != NULL)
        {
            //md5, filename, pv
            if(row[0] == NULL || row[1] == NULL || row[2] == NULL)
            {
                LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "mysql_fetch_row(res_set)) failed\n");
                ret = -1;
                goto END;
            }

            char fileid[1024] = {0};
            sprintf(fileid, "%s%s", row[0], row[1]); //文件标示，md5+文件名

            //增加有序集合成员
            rop_zset_add(redis_conn, FILE_PUBLIC_ZSET, atoi(row[2]), fileid);

            //增加hash记录
            rop_hash_set(redis_conn, FILE_NAME_HASH, fileid, row[1]);
        }
    }

    //===5、从redis读取数据，给前端反馈相应信息
    //char value[count][1024];
    value  = (RVALUES)calloc(count, VALUES_ID_SIZE); //堆区请求空间
    if(value == NULL)
    {
        ret = -1;
        goto END;
    }

    int n = 0;
    int end = start + count - 1;//加载资源的结束位置
    //降序获取有序集合的元素
    ret = rop_zset_zrevrange(redis_conn, FILE_PUBLIC_ZSET, start, end, value, &n);
    if(ret != 0)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "rop_zset_zrevrange 操作失败\n");
        goto END;
    }

    root = cJSON_CreateObject();
    array = cJSON_CreateArray();
    //遍历元素个数
    for(int i = 0; i < n; ++i)
    {
        //array[i]:
        cJSON* item = cJSON_CreateObject();

        /*
        {
            "filename": "test.mp4",
            "pv": 0
        }
        */

        //-- filename 文件名字
        char filename[1024] = {0};
        ret = rop_hash_get(redis_conn, FILE_NAME_HASH, value[i], filename);
        if(ret != 0)
        {
            LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "rop_hash_get 操作失败\n");
            ret = -1;
            goto END;
        }
        cJSON_AddStringToObject(item, "filename", filename);


        //-- pv 文件下载量
        int score = rop_zset_get_score(redis_conn, FILE_PUBLIC_ZSET, value[i]);
        if(score == -1)
        {
            LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "rop_zset_get_score 操作失败\n");
            ret = -1;
            goto END;
        }
        cJSON_AddNumberToObject(item, "pv", score);


        cJSON_AddItemToArray(array, item);

    }

    cJSON_AddItemToObject(root, "files", array);

    out = cJSON_Print(root);

    LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "%s\n", out);

END:
    if(ret == 0)
    {
        printf("%s", out); //给前端反馈信息
    }
    else
    {   //失败
        out2 = NULL;
        out2 = return_status("015");
    }
    if(out2 != NULL)
    {
        printf(out2); //给前端反馈错误码
        free(out2);
    }

    if(res_set != NULL)
    {
        //完成所有对数据的操作后，调用mysql_free_result来善后处理
        mysql_free_result(res_set);
    }

    if(redis_conn != NULL)
    {
        rop_disconnect(redis_conn);
    }

    if(conn != NULL)
    {
        mysql_close(conn);
    }

    if(value != NULL)
    {
        free(value);
    }

    if(root != NULL)
    {
        cJSON_Delete(root);
    }

    if(out != NULL)
    {
        free(out);
    }


    return ret;
}

int main()
{
    char cmd[20];

    //读取数据库配置信息
    read_cfg();

    //阻塞等待用户连接
    while (FCGI_Accept() >= 0)
    {

        // 获取URL地址 "?" 后面的内容
        char *query = getenv("QUERY_STRING");

        //解析命令
        query_parse_key_value(query, "cmd", cmd, NULL);
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "cmd = %s\n", cmd);

        printf("Content-type: text/html\r\n\r\n");

        if (strcmp(cmd, "count") == 0) //count 获取用户文件个数
        {
            get_share_files_count(); //获取共享文件个数
        }
        else
        {
            char *contentLength = getenv("CONTENT_LENGTH");
            int len;

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
                LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "len = 0, No data from standard input\n");
            }
            else
            {
                char buf[4*1024] = {0};
                int ret = 0;
                ret = fread(buf, 1, len, stdin); //从标准输入(web服务器)读取内容
                if(ret == 0)
                {
                    LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "fread(buf, 1, len, stdin) err\n");
                    continue;
                }

                LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "buf = %s\n", buf);

                //获取共享文件信息 127.0.0.1:80/sharefiles&cmd=normal
                //按下载量升序 127.0.0.1:80/sharefiles?cmd=pvasc
                //按下载量降序127.0.0.1:80/sharefiles?cmd=pvdesc

                int start; //文件起点
                int count; //文件个数
                get_fileslist_json_info(buf, &start, &count); //通过json包获取信息
                LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "start = %d, count = %d\n", start, count);
                 if (strcmp(cmd, "normal") == 0)
                 {
                    get_share_filelist(start, count); //获取共享文件列表
                 }
                 else if(strcmp(cmd, "pvdesc") == 0)
                 {
                    get_ranking_filelist(start, count);//获取共享文件排行版
                 }


            }
        }

    }

    return 0;
}