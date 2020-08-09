#ifndef HTTPSTATUS_H
#define HTTPSTATUS_H
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

#include <iostream>
#include <vector>
#include <map>

using namespace std;

// Response状态码
#define RES_SUCCESS                         "200"       //成功

#define RES_USER_EXIST                      "901"       //该用户已存在：
#define RES_FILE_EXIST                      "902"       //文件已存在：
#define RES_ALREADY_SHARE_FILE              "903"       //别人已经分享此文件

#define RES_REGISTER_FAILURE                "001"       //注册失败
#define RES_LOGIN_FAILURE                   "002"       //登录失败
#define RES_FAST_UPLOAD_FAILURE             "003"       //秒传失败
#define RES_UPLOAD_FAILURE                  "004"       //上传失败
#define RES_DOWNLOAD_FAILURE                "005"       //下载失败
#define RES_SHARE_FAILURE                   "006"       //分享失败
#define RES_TOKEN_FAILURE                   "007"       //Token验证失败
#define RES_DELETE_FAILURE                  "008"       //删除失败
#define RES_DOWNLOAD_FILE_PV_FAILURE        "009"      //下载文件pv字段处理失败
#define RES_CANCEL_FAILURE                  "010"       //取消失败
#define RES_DEALSHARE_SAVE_FAILURE          "011"       //转存失败
#define RES_GET_SHARE_FILE_LIST_FAILURE     "012"       //获取共享文件列表失败
#define RES_GET_USER_FILE_LIST_FAILURE      "013"       //获取用户文件列表失败

// Response返回结果
struct Response
{
    std::string code;           // 状态码
    std::string msg;            // 消息
};


/**
 * Http状态码
 */
class HttpStatus
{
public:
    //获取Response信息
    Response* getResponse(std::string code);
    static HttpStatus& getInstance();

private:
    std::map<std::string, Response* > m_responseMap;

    HttpStatus();
    static HttpStatus instance;
};

#endif // HTTPSTATUS_H
