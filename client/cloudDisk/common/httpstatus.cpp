#include "httpstatus.h"
#if _MSC_VER >=1600
#pragma execution_character_set("utf-8")
#endif

Response* createResponse(std::string code, std::string msg) {
    Response *response = new Response;
    response->code = code;
    response->msg = msg;
    return response;
}


HttpStatus::HttpStatus()
{
    //成功
    Response *response1_1 = createResponse(RES_SUCCESS, "成功");

    //存在
    Response *response2_1 = createResponse(RES_USER_EXIST, "用户已存在");
    Response *response2_2 = createResponse(RES_FILE_EXIST, "文件已存在");
    Response *response2_3 = createResponse(RES_ALREADY_SHARE_FILE, "别人已经分享此文件");

    //失败
    Response *response3_1 = createResponse(RES_REGISTER_FAILURE, "注册失败");
    Response *response3_2 = createResponse(RES_LOGIN_FAILURE, "登录失败");
    Response *response3_3 = createResponse(RES_FAST_UPLOAD_FAILURE, "秒传失败");
    Response *response3_4 = createResponse(RES_UPLOAD_FAILURE, "上传失败");
    Response *response3_5 = createResponse(RES_DOWNLOAD_FAILURE, "下载失败");
    Response *response3_6 = createResponse(RES_SHARE_FAILURE, "分享失败");
    Response *response3_7 = createResponse(RES_TOKEN_FAILURE, "token验证失败");
    Response *response3_8 = createResponse(RES_DELETE_FAILURE, "删除失败");
    Response *response3_9 = createResponse(RES_DOWNLOAD_FILE_PV_FAILURE, "下载文件pv字段处理失败");
    Response *response3_10 = createResponse(RES_CANCEL_FAILURE, "取消失败");
    Response *response3_11 = createResponse(RES_DEALSHARE_SAVE_FAILURE, "转存失败");
    Response *response3_12 = createResponse(RES_GET_SHARE_FILE_LIST_FAILURE, "获取共享文件列表失败");
    Response *response3_13 = createResponse(RES_GET_USER_FILE_LIST_FAILURE, "获取用户文件列表失败");


    m_responseMap.insert(std::pair<std::string, Response* >(RES_SUCCESS, response1_1));

    m_responseMap.insert(std::pair<std::string, Response* >(RES_USER_EXIST, response2_1));
    m_responseMap.insert(std::pair<std::string, Response* >(RES_FILE_EXIST, response2_2));
    m_responseMap.insert(std::pair<std::string, Response* >(RES_ALREADY_SHARE_FILE, response2_3));

    m_responseMap.insert(std::pair<std::string, Response* >(RES_REGISTER_FAILURE, response3_1));
    m_responseMap.insert(std::pair<std::string, Response* >(RES_LOGIN_FAILURE, response3_2));
    m_responseMap.insert(std::pair<std::string, Response* >(RES_FAST_UPLOAD_FAILURE, response3_3));
    m_responseMap.insert(std::pair<std::string, Response* >(RES_UPLOAD_FAILURE, response3_4));
    m_responseMap.insert(std::pair<std::string, Response* >(RES_DOWNLOAD_FAILURE, response3_5));
    m_responseMap.insert(std::pair<std::string, Response* >(RES_SHARE_FAILURE, response3_6));
    m_responseMap.insert(std::pair<std::string, Response* >(RES_TOKEN_FAILURE, response3_7));
    m_responseMap.insert(std::pair<std::string, Response* >(RES_DELETE_FAILURE, response3_8));
    m_responseMap.insert(std::pair<std::string, Response* >(RES_DOWNLOAD_FILE_PV_FAILURE, response3_9));
    m_responseMap.insert(std::pair<std::string, Response* >(RES_CANCEL_FAILURE, response3_10));
    m_responseMap.insert(std::pair<std::string, Response* >(RES_DEALSHARE_SAVE_FAILURE, response3_11));
    m_responseMap.insert(std::pair<std::string, Response* >(RES_GET_SHARE_FILE_LIST_FAILURE, response3_11));
    m_responseMap.insert(std::pair<std::string, Response* >(RES_GET_USER_FILE_LIST_FAILURE, response3_11));
}

HttpStatus HttpStatus::instance;

HttpStatus& HttpStatus::getInstance()
{
    return instance;
}

//获取Response信息
Response* HttpStatus::getResponse(std::string code)
{
    std::map<std::string, Response*>::iterator iter;

    for (iter = m_responseMap.begin(); iter != m_responseMap.end(); iter++) {
        //std::cout << iter->first << ' ' << iter->second << std::endl;
        if (iter->first == code) {
            return iter->second;
        }
    }

    return NULL;
}

/*
 * 成功 200
 *
 * 该用户已存在：  {"code":"003"}
 * 文件已存在：{"code":"005"}
别人已经分享此文件：{"code", "012"}
 *
 *  登录失败： 001
 *  注册失败： 004
 *  秒传失败： 007
 *  上传失败： 009
 *  下载失败： 011
    分享失败： 011
    token验证失败：111
    删除失败：014
    下载文件pv字段处理失败：017
    取消失败：019
    转存失败：022
  */





/*
        注册 - server端返回的json格式数据：
            成功:         {"code":"002"}
            该用户已存在：  {"code":"003"}
            失败:         {"code":"004"}
*/

/*
    登陆 - 服务器回写的json数据包格式：
        成功：{"code":"000"}
        失败：{"code":"001"}
*/



/*
   秒传文件：
        文件已存在：{"code":"005"}
        秒传成功：  {"code":"006"}
        秒传失败：  {"code":"007"}
    上传文件：
        成功：{"code":"008"}
        失败：{"code":"009"}
    下载文件：
        成功：{"code":"010"}
        失败：{"code":"011"}
*/




/*
    分享文件：
        成功：{"code":"010"}
        失败：{"code":"011"}
        别人已经分享此文件：{"code", "012"}

    token验证失败：{"code":"111"}

    */



/*
    删除文件：
        成功：{"code":"013"}
        失败：{"code":"014"}
    */




/*
    下载文件pv字段处理
        成功：{"code":"016"}
        失败：{"code":"017"}
    */


/*
   下载文件：
        成功：{"code":"009"}
        失败：{"code":"010"}
*/


/*
    取消分享：
        成功：{"code":"018"}
        失败：{"code":"019"}
*/



/*
   转存文件：
       成功：{"code":"020"}
       文件已存在：{"code":"021"}
       失败：{"code":"022"}
*/


/*
   下载文件：
        成功：{"code":"009"}
        失败：{"code":"010"}

    追加任务到下载队列
    参数：info：下载文件信息， filePathName：文件保存路径
    成功：0
    失败：
      -1: 下载的文件是否已经在下载队列中
      -2: 打开文件失败
*/




