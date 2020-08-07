#ifndef _UTIL_CGI_H_
#define _UTIL_CGI_H_

#define FILE_NAME_LEN       (256)	//文件名字长度
#define TEMP_BUF_MAX_LEN    (512)	//临时缓冲区大小
#define FILE_URL_LEN        (512)   //文件所存放storage的host_name长度
#define HOST_NAME_LEN       (30)	//主机ip地址长度
#define USER_NAME_LEN       (128)	//用户名字长度
#define TOKEN_LEN           (128)	//登陆token长度
#define MD5_LEN             (256)   //文件md5长度
#define PWD_LEN             (256)	//密码长度
#define TIME_STRING_LEN     (25)    //时间戳长度
#define SUFFIX_LEN          (8)     //后缀名长度
#define PIC_NAME_LEN        (10)    //图片资源名字长度
#define PIC_URL_LEN         (256)   //图片资源url名字长度

#define UTIL_LOG_MODULE     "cgi"
#define UTIL_LOG_PROC       "util"

/**
 * @brief  去掉一个字符串两边的空白字符
 *
 * @param inbuf确保inbuf可修改
 *
 * @returns   
 *      0 成功
 *      -1 失败
 */
int trim_space(char *inbuf);

/**
 * @brief  在字符串full_data中查找字符串substr第一次出现的位置
 *
 * @param full_data 	源字符串首地址
 * @param full_data_len 源字符串长度
 * @param substr        匹配字符串首地址
 *
 * @returns   
 *      成功: 匹配字符串首地址
 *      失败：NULL
 */
char* memstr(char* full_data, int full_data_len, char* substr);

/**
 * @brief  解析url query 类似 abc=123&bbb=456 字符串
 *          传入一个key,得到相应的value
 * @returns
 *          0 成功, -1 失败
 */
int query_parse_key_value(const char *query, const char *key, char *value, int *value_len_p);

//通过文件名file_name， 得到文件后缀字符串, 保存在suffix 如果非法文件后缀,返回"null"
int get_file_suffix(const char *file_name, char *suffix);

//字符串strSrc中的字串strFind，替换为strReplace
void str_replace(char* strSrc, char* strFind, char* strReplace);

//返回前端情况，NULL代表失败, 返回的指针不为空，则需要free
char * return_status(char *status_num);

//验证登陆token，成功返回0，失败-1
int verify_token(char *user, char *token);


#endif
