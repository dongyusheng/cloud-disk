/**
 * @file myfiles_cgi.c
 * @brief  用户列表展示CGI程序
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

#define MYFILES_LOG_MODULE       "cgi"
#define MYFILES_LOG_PROC         "myfiles"

//mysql 数据库配置信息 用户名， 密码， 数据库名称
static char mysql_user[128] = {0};
static char mysql_pwd[128] = {0};
static char mysql_db[128] = {0};

//redis 服务器ip、端口
//static char redis_ip[30] = {0};
//static char redis_port[10] = {0};

//读取配置信息
void read_cfg()
{
    //读取mysql数据库配置信息
    get_cfg_value(CFG_PATH, "mysql", "user", mysql_user);
    get_cfg_value(CFG_PATH, "mysql", "password", mysql_pwd);
    get_cfg_value(CFG_PATH, "mysql", "database", mysql_db);
    LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "mysql:[user=%s,pwd=%s,database=%s]", mysql_user, mysql_pwd, mysql_db);

    //读取redis配置信息
    //get_cfg_value(CFG_PATH, "redis", "ip", redis_ip);
    //get_cfg_value(CFG_PATH, "redis", "port", redis_port);
    //LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "redis:[ip=%s,port=%s]\n", redis_ip, redis_port);
}


//解析的json包, 登陆token
int get_count_json_info(char *buf, char *user, char *token)
{
    int ret = 0;

    /*json数据如下
    {
        "token": "9e894efc0b2a898a82765d0a7f2c94cb",
        user:xxxx
    }
    */

    //解析json包
    //解析一个json字符串为cJSON对象
    cJSON * root = cJSON_Parse(buf);
    if(NULL == root)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_Parse err\n");
        ret = -1;
        goto END;
    }

    //返回指定字符串对应的json对象
    //用户
    cJSON *child1 = cJSON_GetObjectItem(root, "user");
    if(NULL == child1)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    //LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "child1->valuestring = %s\n", child1->valuestring);
    strcpy(user, child1->valuestring); //拷贝内容

    //登陆token
    cJSON *child2 = cJSON_GetObjectItem(root, "token");
    if(NULL == child2)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    //LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "child2->valuestring = %s\n", child2->valuestring);
    strcpy(token, child2->valuestring); //拷贝内容

END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }

    return ret;
}

//返回前端情况
void return_login_status(long num, int token_flag)
{

    char *out = NULL;
    char *token;
    char num_buf[128] = {0};

    if(token_flag == 0)
    {
        token = "110"; //成功
    }
    else
    {
        token = "111"; //失败
    }

    //数字
    sprintf(num_buf, "%ld", num);

    cJSON *root = cJSON_CreateObject();  //创建json项目
    cJSON_AddStringToObject(root, "num", num_buf);// {"num":"1111"}
    cJSON_AddStringToObject(root, "code", token);// {"code":"110"}
    out = cJSON_Print(root);//cJSON to string(char *)

    cJSON_Delete(root);

    if(out != NULL)
    {
        printf(out); //给前端反馈信息
        free(out); //记得释放
    }
}

//获取用户文件个数
void get_user_files_count(char *user, int ret)
{
    char sql_cmd[SQL_MAX_LEN] = {0};
    MYSQL *conn = NULL;
    long line = 0;

    //connect the database
    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "msql_conn err\n");
        goto END;
    }

    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn, "set names utf8");

    // 构造sql语句, 查询用户文件的数量
    sprintf(sql_cmd, "select count from user_file_count where user='%s'", user);
    char tmp[512] = {0};
    //执行sql语句
    //返回值： 0成功并保存记录集，1没有记录集，2有记录集但是没有保存，-1失败
    int ret2 = process_result_one(conn, sql_cmd, tmp); 
    if(ret2 != 0)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "%s 操作失败\n", sql_cmd);
        goto END;
    }

    line = atol(tmp); //字符串转长整形

END:
    if(conn != NULL)
    {
        mysql_close(conn);
    }

    LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "line = %ld\n", line);

    //给前端反馈的信息
    return_login_status(line, ret);
}

//解析的json包
int get_fileslist_json_info(char *buf, char *user, char *token, int *p_start, int *p_count)
{
    int ret = 0;

    /*json数据如下
    {
        "user": "milo"
        "token": xxxx
        "start": 0
        "count": 10
    }
    */

    //解析json包
    //解析一个json字符串为cJSON对象
    cJSON * root = cJSON_Parse(buf);
    if(NULL == root)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_Parse err\n");
        ret = -1;
        goto END;
    }

    //返回指定字符串对应的json对象
    //用户
    cJSON *child1 = cJSON_GetObjectItem(root, "user");
    if(NULL == child1)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    //LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "child1->valuestring = %s\n", child1->valuestring);
    strcpy(user, child1->valuestring); //拷贝内容

    //token
    cJSON *child2 = cJSON_GetObjectItem(root, "token");
    if(NULL == child2)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    strcpy(token, child2->valuestring); //拷贝内容

    //文件起点
    cJSON *child3 = cJSON_GetObjectItem(root, "start");
    if(NULL == child3)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    *p_start = child3->valueint;

    //文件请求个数
    cJSON *child4 = cJSON_GetObjectItem(root, "count");
    if(NULL == child4)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    *p_count = child4->valueint;

END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }

    return ret;
}

//获取用户文件列表
//获取用户文件信息 127.0.0.1:80/myfiles&cmd=normal
//按下载量升序 127.0.0.1:80/myfiles?cmd=pvasc
//按下载量降序127.0.0.1:80/myfiles?cmd=pvdesc
int get_user_filelist(char *cmd, char *user, int start, int count)
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
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "msql_conn err\n");
        ret = -1;
        goto END;
    }

    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn, "set names utf8");

    // 多表指定行范围查询
    // 获取用户文件信息
    if(strcmp(cmd, "normal") == 0) 
    {
        //sql语句
        sprintf(sql_cmd, "select user_file_list.*, file_info.url, file_info.size, file_info.type from file_info, user_file_list where user = '%s' and file_info.md5 = user_file_list.md5 limit %d, %d", user, start, count);
    }
    //按下载量升序
    else if(strcmp(cmd, "pvasc") == 0) 
    {
        //sql语句
        sprintf(sql_cmd, "select user_file_list.*, file_info.url, file_info.size, file_info.type from file_info, user_file_list where user = '%s' and file_info.md5 = user_file_list.md5  order by pv asc limit %d, %d", user, start, count);
    }
    //按下载量降序
    else if(strcmp(cmd, "pvdesc") == 0) 
    {
        //sql语句
        sprintf(sql_cmd, "select user_file_list.*, file_info.url, file_info.size, file_info.type from file_info, user_file_list where user = '%s' and file_info.md5 = user_file_list.md5 order by pv desc limit %d, %d", user, start, count);
    }

    LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "%s 在操作\n", sql_cmd);

    if (mysql_query(conn, sql_cmd) != 0)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "%s 操作失败：%s\n", sql_cmd, mysql_error(conn));
        ret = -1;
        goto END;
    }

    // 生成结果集
    res_set = mysql_store_result(conn);
    if (res_set == NULL)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "smysql_store_result error: %s!\n", mysql_error(conn));
        ret = -1;
        goto END;
    }

    ulong line = 0;
    //mysql_num_rows接受由mysql_store_result返回的结果结构集，并返回结构集中的行数
    line = mysql_num_rows(res_set);
    if (line == 0)//没有结果
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "mysql_num_rows(res_set) failed：%s\n", mysql_error(conn));
        ret = -1;
        goto END;
    }

    // 将MySQL中查询的结果设置为JSON格式, 然后返回给客户端
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
            if(row[i] != NULL){ }
        }*/

        /*
        {
        "user": "milo",
        "md5": "e8ea6031b779ac26c319ddf949ad9d8d",
        "create_time": "2020-06-21 21:35:25",
        "file_name": "test.mp4",
        "share_status": 0,
        "pv": 0,
        "url": "http://192.168.31.109:80/group1/M00/00/00/wKgfbViy2Z2AJ-FTAaM3As-g3Z0782.mp4",
        "size": 27473666,
         "type": "mp4"
        }

        */
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
        //-- createtime 文件创建时间
        if(row[column_index] != NULL)
        {
            cJSON_AddStringToObject(item, "create_time", row[column_index]);
        }

        column_index++;
        //-- filename 文件名字
        if(row[column_index] != NULL)
        {
            cJSON_AddStringToObject(item, "file_name", row[column_index]);
        }

        column_index++;
        //-- shared_status 共享状态, 0为没有共享， 1为共享
        if(row[column_index] != NULL)
        {
            cJSON_AddNumberToObject(item, "share_status", atoi( row[column_index] ));
        }

        column_index++;
        //-- pv 文件下载量，默认值为0，下载一次加1
        if(row[column_index] != NULL)
        {
            cJSON_AddNumberToObject(item, "pv", atol( row[column_index] ));
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

    LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "%s\n", out);

END:
    if(ret == 0)
    {
        printf("%s", out); //给前端反馈信息
    }
    else
    {   //失败
        /*
        获取用户文件列表：
            成功：文件列表json
            失败：{"code": "015"}
        */
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

int main()
{
    //count 获取用户文件个数
    //display 获取用户文件信息，展示到前端
    char cmd[20];
    char user[USER_NAME_LEN];
    char token[TOKEN_LEN];

     //读取数据库配置信息
    read_cfg();

    //阻塞等待用户连接
    while (FCGI_Accept() >= 0)
    {

        // 获取URL地址 "?" 后面的内容
        char *query = getenv("QUERY_STRING");

        // 解析命令
        query_parse_key_value(query, "cmd", cmd, NULL);
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cmd = %s\n", cmd);

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
            LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "len = 0, No data from standard input\n");
        }
        else
        {
            char buf[4*1024] = {0};
            int ret = 0;
            ret = fread(buf, 1, len, stdin); //从标准输入(web服务器)读取内容
            if(ret == 0)
            {
                LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "fread(buf, 1, len, stdin) err\n");
                continue;
            }

            LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "buf = %s\n", buf);

            // count: 获取用户文件个数
            if (strcmp(cmd, "count") == 0) 
            {
                // 通过json包获取用户名, token
                get_count_json_info(buf, user, token); 

                // 验证登陆token，成功返回0，失败-1
                ret = verify_token(user, token); //util_cgi.h

                // 从MySQL中获取用户文件个数
                get_user_files_count(user, ret);
            }
            //获取用户文件信息 127.0.0.1:80/myfiles&cmd=normal
            //按下载量升序 127.0.0.1:80/myfiles?cmd=pvasc
            //按下载量降序127.0.0.1:80/myfiles?cmd=pvdesc
            else
            {
                int start; //文件起点
                int count; //文件个数
                get_fileslist_json_info(buf, user, token, &start, &count); //通过json包获取信息
                LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "user = %s, token = %s, start = %d, count = %d\n", user, token, start, count);

                //验证登陆token，成功返回0，失败-1
                ret = verify_token(user, token); //util_cgi.h
                if(ret == 0)
                {
                     get_user_filelist(cmd, user, start, count); //获取用户文件列表
                }
                else
                {
                    char *out = return_status("111"); //token验证失败错误码
                    if(out != NULL)
                    {
                        printf(out); //给前端反馈错误码
                        free(out);
                    }
                }

            }

        }

    }

    return 0;
}
