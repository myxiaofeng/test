# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_LIB_SRCS 变量
include_directories(/home/gaofeng/gaofeng/md5/md5/include)
aux_source_directory(. DIR_LIB_SRCS)
# 生成链接库
add_library (checksum ${DIR_LIB_SRCS})