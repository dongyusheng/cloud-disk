/**
 * @file cfg.c
 * @brief  读取配置文件信息
 * @version 1.0
 * @date
 */

 #include "cJSON.h"
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "make_log.h"
 #include "cfg.h"
 #include "cJSON.h"

/* -------------------------------------------*/
/**
 * @brief  从配置文件中得到相对应的参数
 *
 * @param profile   配置文件路径
 * @param title      配置文件title名称[title]
 * @param key       key
 * @param value    (out)  得到的value
 *
 * @returns
 *      0 succ, -1 fail
 */
/* -------------------------------------------*/
int get_cfg_value(const char *profile, char *title, char *key, char *value)
{
    int ret = 0;
    char *buf = NULL;
    FILE *fp = NULL;

    //异常处理
    if(profile == NULL || title == NULL || key == NULL || value == NULL)
    {
        return -1;
    }

    //只读方式打开文件
    fp = fopen(profile, "rb");
    if(fp == NULL) //打开失败
    {
        perror("fopen");
        LOG(CFG_LOG_MODULE, CFG_LOG_PROC, "fopen err\n");
        ret = -1;
        goto END;
    }

    fseek(fp, 0, SEEK_END);//光标移动到末尾
    long size = ftell(fp); //获取文件大小
    fseek(fp, 0, SEEK_SET);//光标移动到开头


    buf = (char *)calloc(1, size+1); //动态分配空间
    if(buf == NULL)
    {
        perror("calloc");
        LOG(CFG_LOG_MODULE, CFG_LOG_PROC, "calloc err\n");
        ret = -1;
        goto END;
    }

    //读取文件内容
    fread(buf, 1, size, fp);

    //解析一个json字符串为cJSON对象
    cJSON * root = cJSON_Parse(buf);
    if(NULL == root)
    {
        LOG(CFG_LOG_MODULE, CFG_LOG_PROC, "root err\n");
        ret = -1;
        goto END;
    }

    //返回指定字符串对应的json对象
    cJSON * father = cJSON_GetObjectItem(root, title);
    if(NULL == father)
    {
        LOG(CFG_LOG_MODULE, CFG_LOG_PROC, "father err\n");
        ret = -1;
        goto END;
    }

    cJSON * son = cJSON_GetObjectItem(father, key);
    if(NULL == son)
    {
        LOG(CFG_LOG_MODULE, CFG_LOG_PROC, "son err\n");
        ret = -1;
        goto END;
    }

    //LOG(CFG_LOG_MODULE, CFG_LOG_PROC, "son->valuestring = %s\n", son->valuestring);
    strcpy(value, son->valuestring); //拷贝内容

    cJSON_Delete(root);//删除json对象

END:
    if(fp != NULL)
    {
        fclose(fp);
    }

    if(buf != NULL)
    {
        free(buf);
    }

    return ret;
}


// 从cfg.json配置文件中获取, 详情参阅CFG_PATH宏
// 获取数据库用户名、用户密码、数据库标示等信息
int get_mysql_info(char *mysql_user, char *mysql_pwd, char *mysql_db)
{
    // 调用get_cfg_value从配置文件中读取相应的数据
    if( -1 == get_cfg_value(CFG_PATH, "mysql", "user", mysql_user) )
    {
        LOG(CFG_LOG_MODULE, CFG_LOG_PROC, "mysql_user err\n");
        return -1;
    }

    if( -1 == get_cfg_value(CFG_PATH, "mysql", "password", mysql_pwd) )
    {
        LOG(CFG_LOG_MODULE, CFG_LOG_PROC, "mysql_pwd err\n");
        return -1;
    }

    if( -1 == get_cfg_value(CFG_PATH, "mysql", "database", mysql_db) )
    {
        LOG(CFG_LOG_MODULE, CFG_LOG_PROC, "mysql_db err\n");
        return -1;
    }

    return 0;
}
