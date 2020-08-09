
# 使用的编译器
CC=gcc
# 预处理参数
CPPLFAGS=-I./include					\
		 -I/usr/include/fastdfs			\
		 -I/usr/include/fastcommon		\
		 -I/usr/local/include/hiredis/  \
		 -I/usr/include/mysql/
# 选项
CFLAGS=-Wall
# 需要链接的动态库
LIBS=-lfdfsclient	\
	 -lfastcommon	\
	 -lhiredis		\
	 -lfcgi         \
	 -lm            \
	 -lmysqlclient  
# 目录路径
TEST_PATH=test
COMMON_PATH=common
CGI_BIN_PATH=bin_cgi
CGI_SRC_PATH=src_cgi

# 子目标, 因为是测试,所有需要单独生成很多子目标
# 测试用
main=main
redis=redis
# 项目用
login=$(CGI_BIN_PATH)/login
register=$(CGI_BIN_PATH)/register
upload=$(CGI_BIN_PATH)/upload
md5=$(CGI_BIN_PATH)/md5
myfiles=$(CGI_BIN_PATH)/myfiles
dealfile=$(CGI_BIN_PATH)/dealfile
sharefiles=$(CGI_BIN_PATH)/sharefiles
dealsharefile=$(CGI_BIN_PATH)/dealsharefile

# 最终目标
target=$(login)		\
	   $(register)	\
	   $(upload)	\
	   $(md5)		\
	   $(myfiles)	\
	   $(dealfile)	\
	   $(sharefiles)	\
	   $(dealsharefile)
ALL:$(target)

#######################################################################
#                        测试程序相关的规则
# 生成每一个子目标,
# main程序
$(main):$(TEST_PATH)/main.o $(TEST_PATH)/fdfs_api.o $(COMMON_PATH)/make_log.o
	$(CC) $^ $(LIBS) -o $@

# redis test 程序
$(redis):$(TEST_PATH)/myredis.o
	$(CC) $^ $(LIBS) -o $@
	#######################################################################

# =====================================================================
#							项目程序规则
# 登录
$(login):	$(CGI_SRC_PATH)/login_cgi.o \
			$(COMMON_PATH)/make_log.o  \
			$(COMMON_PATH)/cJSON.o \
			$(COMMON_PATH)/deal_mysql.o \
			$(COMMON_PATH)/redis_op.o  \
			$(COMMON_PATH)/cfg.o \
			$(COMMON_PATH)/util_cgi.o \
			$(COMMON_PATH)/des.o \
			$(COMMON_PATH)/base64.o \
			$(COMMON_PATH)/md5.o
	$(CC) $^ -o $@ $(LIBS)
# 注册
$(register):	$(CGI_SRC_PATH)/reg_cgi.o \
				$(COMMON_PATH)/make_log.o  \
				$(COMMON_PATH)/util_cgi.o \
				$(COMMON_PATH)/cJSON.o \
				$(COMMON_PATH)/deal_mysql.o \
				$(COMMON_PATH)/redis_op.o  \
				$(COMMON_PATH)/cfg.o
	$(CC) $^ -o $@ $(LIBS)
# 秒传
$(md5):		$(CGI_SRC_PATH)/md5_cgi.o \
			$(COMMON_PATH)/make_log.o  \
			$(COMMON_PATH)/util_cgi.o \
			$(COMMON_PATH)/cJSON.o \
			$(COMMON_PATH)/deal_mysql.o \
			$(COMMON_PATH)/redis_op.o  \
			$(COMMON_PATH)/cfg.o
	$(CC) $^ -o $@ $(LIBS)
# 上传
$(upload):$(CGI_SRC_PATH)/upload_cgi.o \
		  $(COMMON_PATH)/make_log.o  \
		  $(COMMON_PATH)/util_cgi.o \
		  $(COMMON_PATH)/cJSON.o \
		  $(COMMON_PATH)/deal_mysql.o \
		  $(COMMON_PATH)/redis_op.o  \
		  $(COMMON_PATH)/cfg.o
	$(CC) $^ -o $@ $(LIBS)
# 用户列表展示
$(myfiles):	$(CGI_SRC_PATH)/myfiles_cgi.o \
			$(COMMON_PATH)/make_log.o  \
			$(COMMON_PATH)/util_cgi.o \
			$(COMMON_PATH)/cJSON.o \
			$(COMMON_PATH)/deal_mysql.o \
			$(COMMON_PATH)/redis_op.o  \
			$(COMMON_PATH)/cfg.o
	$(CC) $^ -o $@ $(LIBS)
# 分享、删除文件、pv字段处理
$(dealfile):$(CGI_SRC_PATH)/dealfile_cgi.o \
			$(COMMON_PATH)/make_log.o  \
			$(COMMON_PATH)/util_cgi.o \
			$(COMMON_PATH)/cJSON.o \
			$(COMMON_PATH)/deal_mysql.o \
			$(COMMON_PATH)/redis_op.o  \
			$(COMMON_PATH)/cfg.o
	$(CC) $^ -o $@ $(LIBS)
# 共享文件列表展示
$(sharefiles):	$(CGI_SRC_PATH)/sharefiles_cgi.o \
			$(COMMON_PATH)/make_log.o  \
			$(COMMON_PATH)/util_cgi.o \
			$(COMMON_PATH)/cJSON.o \
			$(COMMON_PATH)/deal_mysql.o \
			$(COMMON_PATH)/redis_op.o  \
			$(COMMON_PATH)/cfg.o
	$(CC) $^ -o $@ $(LIBS)
# 共享文件pv字段处理、取消分享、转存文件
$(dealsharefile):	$(CGI_SRC_PATH)/dealsharefile_cgi.o \
			$(COMMON_PATH)/make_log.o  \
			$(COMMON_PATH)/util_cgi.o \
			$(COMMON_PATH)/cJSON.o \
			$(COMMON_PATH)/deal_mysql.o \
			$(COMMON_PATH)/redis_op.o  \
			$(COMMON_PATH)/cfg.o
	$(CC) $^ -o $@ $(LIBS)
# =====================================================================


#######################################################################
#                         所有程序都需要的规则
# 生成所有的.o 文件
%.o:%.c
	$(CC) -c $< -o $@ $(CPPLFAGS) $(CFLAGS)

# 项目清除
clean:
	-rm -rf *.o $(target) $(TEST_PATH)/*.o $(CGI_SRC_PATH)/*.o $(COMMON_PATH)/*.o

# 声明伪文件
.PHONY:clean ALL
#######################################################################
