#使用数据库
use `cloud_disk`;

#取消安全模式
SET SQL_SAFE_UPDATES = 0;

DELETE FROM `file_info`;
DELETE FROM `share_file_list`;
DELETE FROM `user_file_count`;
DELETE FROM `user_file_list`;
DELETE FROM `user_info`;

#设置安全模式
SET SQL_SAFE_UPDATES = 1;
