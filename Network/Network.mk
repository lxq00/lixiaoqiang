#
#  Copyright (c)1998-2012, Chongqing Xunmei Technology
#  All Rights Reserved.
#
#	 Description:
#  $Id: Base.mk 2055 2014-07-31 03:58:39Z lixiaoqiang $  
#

#�?以自己写一�?.platform 用于配置 平台，静态库拷贝的位�?, 否则�?己指定编译选项，参�?.platform
-include .platform

# 生成库的设置

# 定义库的文件�?�?
   SRCS_PATH =  Src
	
# 库的名称
   SHARDLIB_NAME = 
   STATICLIB_NAME = Network

# 应用程序选项
# 应用程序的代码路�?
   APP_SRCS_PATH = 

# 应用程序名称
   APP_NAME = 

# 子目录编译需要的�?�?
   SUB_INCLUDE = -I ${PRJ_PATH}/boost/Include


# 应用程序依赖的库(除了�?�?的编译的�?)
  LIBS = 
  LIBS_DBG = 

#这个为�??三方�?动生成的库，不能�?�?
#AutoAddOtherDefineStart
LDLIBS =
LDLIBS_DBG = 

#AotoAddOtherDefineEnd


include ${PRJ_PATH}/mk/platform/x86
include ${PRJ_PATH}/mk/builddir_v2.mk


#UserDefined Option
 -include userdefinedOption.mk

include ${PRJ_PATH}/mk/checksvn.mk					
include ${PRJ_PATH}/mk/print.mk
include ${PRJ_PATH}/mk/option.mk
include ${PRJ_PATH}/mk/rules_v2.mk



