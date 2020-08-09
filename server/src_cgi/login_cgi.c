/**
 * @file login_cgi.c
 * @brief   登陆后台CGI程序
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
#include "redis_op.h"
#include "cfg.h"
#include "cJSON.h"
#include "des.h"    //加密
#include "base64.h" //base64
#include "md5.h"    //md5
#include <time.h>

#define LOGIN_LOG_MODULE "cgi"
#define LOGIN_LOG_PROC   "login"

//解析用户登陆信息的json包login_buf
//用户名保存在username，密码保存在pwd
int get_login_info(char *login_buf, char *username, char *pwd)
{
    int ret = 0;

    //解析json包
    //解析一个json字符串为cJSON对象
    cJSON * root = cJSON_Parse(login_buf);
    if(NULL == root)
    {
        LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "cJSON_Parse err\n");
        ret = -1;
        goto END;
    }

    //返回指定字符串对应的json对象
    //用户
    cJSON *child1 = cJSON_GetObjectItem(root, "user");
    if(NULL == child1)
    {
        LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    //LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "child1->valuestring = %s\n", child1->valuestring);
    strcpy(username, child1->valuestring); //拷贝内容

    //密码
    cJSON *child2 = cJSON_GetObjectItem(root, "pwd");
    if(NULL == child2)
    {
        LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    //LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "child1->valuestring = %s\n", child1->valuestring);
    strcpy(pwd, child2->valuestring); //拷贝内容

END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }

    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  判断用户登陆情况
 *
 * @param username 		用户名
 * @param pwd 		密码
 *
 * @returns
 *      成功: 0
 *      失败：-1
 */
 /* -------------------------------------------*/
int check_user_pwd( char *username, char *pwd )
{
    char sql_cmd[SQL_MAX_LEN] = {0};
    int ret = 0;

    //获取数据库用户名、用户密码、数据库标示等信息
    char mysql_user[256] = {0};
    char mysql_pwd[256] = {0};
    char mysql_db[256] = {0};
    // 从配置文件中读取MySQL的配置信息, 此函数详情参阅cfg.c
    get_mysql_info(mysql_user, mysql_pwd,  mysql_db);
    LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "mysql_user = %s, mysql_pwd = %s, mysql_db = %s\n", mysql_user, mysql_pwd, mysql_db);

    //connect the database
    MYSQL *conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "msql_conn err\n");
        return -1;
    }

    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn, "set names utf8");

    // 构建sql语句，查找某个用户对应的密码
    sprintf(sql_cmd, "select password from user_info where user_name='%s'", username);

    //deal result
    char tmp[PWD_LEN] = {0};

    //返回值： 0成功并保存记录集，1没有记录集，2有记录集但是没有保存，-1失败
    process_result_one(conn, sql_cmd, tmp); //执行sql语句，结果集保存在tmp
    if(strcmp(tmp, pwd) == 0)
    {
        ret = 0;
    }
    else
    {
        ret = -1;
    }

    mysql_close(conn);


    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  生成token字符串, 保存redis数据库
 *
 * @param username 		用户名
 * @param token     生成的token字符串
 *
 * @returns
 *      成功: 0
 *      失败：-1
 */
 /* -------------------------------------------*/
int set_token(char *username, char *token)
{
    int ret = 0;
    redisContext * redis_conn = NULL;

    //redis 服务器ip、端口
    char redis_ip[30] = {0};
    char redis_port[10] = {0};

    //读取redis配置信息
    get_cfg_value(CFG_PATH, "redis", "ip", redis_ip);
    get_cfg_value(CFG_PATH, "redis", "port", redis_port);
    LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "redis:[ip=%s,port=%s]\n", redis_ip, redis_port);

    //连接redis数据库
    redis_conn = rop_connectdb_nopwd(redis_ip, redis_port);
    if (redis_conn == NULL)
    {
        LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "redis connected error\n");
        ret = -1;
        goto END;
    }

    // 下面用随机数产生一个token

    //产生4个1000以内的随机数
    int rand_num[4] = {0};
    int i = 0;

    //设置随机种子
    srand((unsigned int)time(NULL));
    for(i = 0; i < 4; ++i)
    {
        rand_num[i] = rand()%1000;//随机数
    }

    char tmp[1024] = {0};
    sprintf(tmp, "%s%d%d%d%d", username, rand_num[0], rand_num[1], rand_num[2], rand_num[3]);
    LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "tmp = %s\n", tmp);

    //加密
    char enc_tmp[1024*2] = {0};
    int enc_len = 0;
    ret = DesEnc((unsigned char *)tmp, strlen(tmp), (unsigned char *)enc_tmp, &enc_len);
    if(ret != 0)
    {
        LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "DesEnc error\n");
        ret = -1;
        goto END;
    }

    //to base64
    char base64[1024*3] = {0};
    base64_encode((const unsigned char *)enc_tmp, enc_len, base64); //base64编码
    LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "base64 = %s\n", base64);

    //to md5
    MD5_CTX md5;
    MD5Init(&md5);
    unsigned char decrypt[16];
    MD5Update(&md5, (unsigned char *)base64, strlen(base64) );
    MD5Final(&md5, decrypt);


    char str[100] = { 0 };
    for (i = 0; i < 16; i++)
    {
        sprintf(str, "%02x", decrypt[i]);
        strcat(token, str);
    }

    // redis保存此字符串，用户名：token, 有效时间为24小时
    ret = rop_setex_string(redis_conn, username, 86400, token);
    //ret = rop_setex_string(redis_conn, user, 30, token); //30秒


END:
    if(redis_conn != NULL)
    {
        rop_disconnect(redis_conn);
    }

    return ret;

}

//返回前端情况
void return_login_status(char *status_num, char *token)
{
    char *out = NULL;
    cJSON *root = cJSON_CreateObject();  //创建json项目
    cJSON_AddStringToObject(root, "code", status_num);// {"code":"000"}
     cJSON_AddStringToObject(root, "token", token);// {"token":"token"}
    out = cJSON_Print(root);//cJSON to string(char *)

    cJSON_Delete(root);

    if(out != NULL)
    {
        printf(out); //给前端反馈信息
        free(out);   //记得释放
    }
}

int main()
{

    //阻塞等待用户连接
    while (FCGI_Accept() >= 0)
    {
        // HTTP请求主体内容的长度, 在这个代码里面就是登陆信息
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;
        char token[128] = {0};

        printf("Content-type: text/html\r\n\r\n");

        // 如果HTTP请求主题没有内容, 那么记录错误日志. 如果有的话
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
            LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "len = 0, No data from standard input\n");
        }
        else //获取登陆用户信息
        {
            char buf[4*1024] = {0};
            int ret = 0;
            ret = fread(buf, 1, len, stdin); //从标准输入(web服务器)读取内容, 读取的就是用户的登录信息
            if(ret == 0)
            {
                LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "fread(buf, 1, len, stdin) err\n");
                continue;
            }

            LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "buf = %s\n", buf);

            
            char username[512] = {0};
            char pwd[512] = {0};
            // 调用这个函数解析数据, 将登录的用户名和密码存放到变量中
            get_login_info(buf, username, pwd);
            LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "username = %s, pwd = %s\n", username, pwd);

            // 从MySQL中获取数据, 登陆判断，成功返回0，失败返回-1
            ret = check_user_pwd( username, pwd );
            if (ret == 0) //登陆成功
            {
                // 生成token字符串
                memset(token, 0, sizeof(token));
                // 将token保存在Redis中
                ret = set_token(username, token);
                LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "token = %s\n", token);

            }

            if(ret == 0)
            {
                //返回前端登陆情况， 000代表成功
                return_login_status("000", token);
            }
            else
            {
                //返回前端登陆情况， 001代表失败
                return_login_status("001", "fail");
            }
        }
    }

    return 0;
}
