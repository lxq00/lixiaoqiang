#
#  Copyright (c)1998-2012, Chongqing Xunmei Technology
#  All Rights Reserved.
#
#	 Description:
#  $Id: Base.mk 2055 2014-07-31 03:58:39Z lixiaoqiang $  
#

#可以自己写一个.platform 用于配置 平台，静态库拷贝的位置, 否则自己指定编译选项，参见.platform
-include .platform

# 生成库的设置
# 定义库的文件目录
	SRCS_PATH =  Src 
	
# 库的名称
   SHARDLIB_NAME = HTTP
   STATICLIB_NAME = 

# 应用程序选项
# 应用程序的代码路径
   APP_SRCS_PATH = 

# 应用程序名称
   APP_NAME = 

# 子目录编译需要的目录
   SUB_INCLUDE = 


# 应用程序依赖的库(除了本身的编译的库)
  LIBS = ${PRJ_PATH}/packages/openssl.1.0.1/build/native/lib/x86/libcrypto.a\
	${PRJ_PATH}/packages/openssl.1.0.1/build/native/lib/x86/libssl.a
  LIBS_DBG = -lcrypto -lssl

#这个为第三方自动生成的库，不能修改
#AutoAddOtherDefineStart
LDLIBS = -L $(PRJ_LIBDIR)/$(PLATFORM)/Base -lBase\
-L $(PRJ_LIBDIR)/$(PLATFORM)/Network -lNetwork\
-Wl,-rpath='$$ORIGIN' -Wl,-rpath-link=$(COMPILE_PATH)/.othersharedlib 

LDLIBS_DBG = -L $(PRJ_LIBDIR)/$(PLATFORM)/Base -lBase_debug\
-L $(PRJ_LIBDIR)/$(PLATFORM)/Network -lNetwork_debug\
-Wl,-rpath='$$ORIGIN' -Wl,-rpath-link=$(COMPILE_PATH)/.othersharedlib 

#AotoAddOtherDefineEnd


include ${PRJ_PATH}/mk/platform/x86
include ${PRJ_PATH}/mk/builddir_v2.mk


#UserDefined Option
include ${PRJ_PATH}//packages/ctemplate.5.0.1/build/native/ctemplate.mk
 include ${PRJ_PATH}//packages/curl.5.0.1/build/native/curl.mk
 include ${PRJ_PATH}//packages/JSON.5.0.0/build/native/JSON.mk
 include ${PRJ_PATH}//packages/openssl.1.0.1/build/native/openssl.mk
 include ${PRJ_PATH}//packages/shttpd.5.0.0/build/native/shttpd.mk
 include ${PRJ_PATH}//packages/sqlite3.5.0.3/build/native/sqlite3.mk
 -include userdefinedOption.mk

include ${PRJ_PATH}/mk/checksvn.mk					
include ${PRJ_PATH}/mk/print.mk
include ${PRJ_PATH}/mk/option.mk
include ${PRJ_PATH}/mk/rules_v2.mk



