#include <mysql/mysql.h> //数据库
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------*/
/**
 * @brief  打印操作数据库出错时的错误信息
 *
 * @param conn       (in)    连接数据库的句柄
 * @param title      (int)   用户错误提示信息
 *
 */
/* -------------------------------------------*/
void print_error(MYSQL *conn, const char *title)
{
    fprintf(stderr,"%s:\nError %u (%s)\n", title, mysql_errno(conn), mysql_error(conn));
}

/* -------------------------------------------*/
/**
 * @brief  连接数据库
 *
 * @param user_name	 (in)   数据库用户
 * @param passwd     (in)   数据库密码
 * @param db_name    (in)   数据库名称
 *
 * @returns   
 *          成功：连接数据库的局部
 *			失败：NULL
 */
/* -------------------------------------------*/
MYSQL* msql_conn(char *user_name, char* passwd, char *db_name)
{
    MYSQL *conn = NULL; //MYSQL对象句柄
                  

	//用来分配或者初始化一个MYSQL对象，用于连接mysql服务端
    conn = mysql_init(NULL);
    if (conn == NULL) 
	{
        fprintf(stderr, "mysql 初始化失败\n");
        return NULL;
    }

	//mysql_real_connect()尝试与运行在主机上的MySQL数据库引擎建立连接
    //conn: 是已有MYSQL结构的地址。调用mysql_real_connect()之前，必须调用mysql_init()来初始化MYSQL结构。
	//NULL: 值必须是主机名或IP地址。如果值是NULL或字符串"localhost"，连接将被视为与本地主机的连接。
	//user_name: 用户的MySQL登录ID
	//passwd: 参数包含用户的密码
	if ( mysql_real_connect(conn, NULL, user_name, passwd, db_name, 0, NULL, 0) == NULL)
	{
        fprintf(stderr, "mysql_conn 失败:Error %u(%s)\n", mysql_errno(conn), mysql_error(conn));
		
        mysql_close(conn);
        return NULL;
    }

    return conn;
}


/* -------------------------------------------*/
/**
 * @brief  处理数据库查询结果
 *
 * @param conn	     (in)   连接数据库的句柄
 * @param res_set    (in)   数据库查询后的结果集
 *
 */
/* -------------------------------------------*/
void process_result_test(MYSQL *conn, MYSQL_RES *res_set)
{
    MYSQL_ROW row;
    uint i;

	// mysql_fetch_row从使用mysql_store_result得到的结果结构中提取一行，并把它放到一个行结构中。当数据用完或发生错误时返回NULL.
    while ((row = mysql_fetch_row(res_set)) != NULL)
	{

		//mysql_num_fields获取结果中列的个数
        for(i = 0; i < mysql_num_fields(res_set); i++)
		{
            if (i > 0)
			{
				fputc('\t',stdout);
			}
            printf("%s",row[i] != NULL ? row[i] : "NULL");
        }

        fputc('\n',stdout);
    }

    if( mysql_errno(conn) != 0 ) 
	{
        print_error(conn,"mysql_fetch_row() failed");
    }
    else 
	{
		//mysql_num_rows接受由mysql_store_result返回的结果结构集，并返回结构集中的行数 
        printf("%lu rows returned \n", (ulong)mysql_num_rows(res_set));
    }
}

// 处理数据库查询结果，结果集保存在buf，只处理一条记录，一个字段, 如果buf为NULL，无需保存结果集，只做判断有没有此记录
// 返回值： 0成功并保存记录集，1没有记录集，2有记录集但是没有保存，-1失败
int process_result_one(MYSQL *conn, char *sql_cmd, char *buf)
{
    int ret = 0;
    MYSQL_RES *res_set = NULL;  //结果集结构的指针

    if (mysql_query(conn, sql_cmd)!= 0) //执行sql语句，执行成功返回0
    {
        print_error(conn, "mysql_query error!\n");
        ret = -1;
        goto END;
    }

    res_set = mysql_store_result(conn);//生成结果集
    if (res_set == NULL)
    {
        print_error(conn, "smysql_store_result error!\n");
        ret = -1;
        goto END;
    }

    MYSQL_ROW row;
    ulong line = 0;

    //mysql_num_rows接受由mysql_store_result返回的结果结构集，并返回结构集中的行数
    line = mysql_num_rows(res_set);
    if (line == 0)
    {
        ret = 1; //1没有记录集
        goto END;
    }
    else if(line > 0 && buf == NULL) //如果buf为NULL，无需保存结果集，只做判断有没有此记录
    {
        ret = 2; //2有记录集但是没有保存
        goto END;
    }

    // mysql_fetch_row从结果结构中提取一行，并把它放到一个行结构中。当数据用完或发生错误时返回NULL.
    if (( row = mysql_fetch_row(res_set) ) != NULL)
    {
        if (row[0] != NULL)
        {
            strcpy(buf, row[0]);
        }
    }

END:
    if(res_set != NULL)
    {
        //完成所有对数据的操作后，调用mysql_free_result来善后处理
        mysql_free_result(res_set);
    }

    return ret;
}
