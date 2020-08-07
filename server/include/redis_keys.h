/**
 * @brief  redis表 KEYS 值相关定义 
 */

#ifndef _REDIS_KEYS_H_
#define _REDIS_KEYS_H_

#define REDIS_SERVER_IP     "127.0.0.1"
#define REDIS_SERVER_PORT   "6379"


/*--------------------------------------------------------
| 共享用户文件有序集合 (ZSET)
| Key:     FILE_PUBLIC_LIST
| value:   md5文件名
| redis 语句
|   ZADD key score member 添加成员
|   ZREM key member 删除成员
|   ZREVRANGE key start stop [WITHSCORES] 降序查看
|   ZINCRBY key increment member 权重累加increment
|   ZCARD key 返回key的有序集元素个数
|   ZSCORE key member 获取某个成员的分数
|   ZREMRANGEBYRANK key start stop 删除指定范围的成员
|   zlexcount zset [member [member 判断某个成员是否存在，存在返回1，不存在返回0
`---------------------------------------------------------*/
#define FILE_PUBLIC_ZSET                  "FILE_PUBLIC_ZSET"

/*-------------------------------------------------------
| 文件标示和文件名对应表 (HASH)
| Key:    FILE_NAME_HASH
| field:  file_id(md5文件名)
| value:  file_name
| redis 语句
|    hset key field value
|    hget key field
`--------------------------------------------------------*/
#define FILE_NAME_HASH                "FILE_NAME_HASH"



#endif

