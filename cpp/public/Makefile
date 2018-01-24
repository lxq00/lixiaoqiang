#
#  Copyright (c)1998-2012, Chongqing Xunmei Technology
#  All Rights Reserved.
#
#	 Description:
#  $Id: Makefile 2055 2014-07-31 03:58:39Z lixiaoqiang $  
#

.PHONY: all app apprelease appdebug install clean cleandep cleanall share sharelib \
		sharedebug static staticlib staticdebug lib
-include .platform

#交叉编译环境设置
#export CROSS=arm-hisiv100nptl-linux-

#编译文件 列表
ifneq ($(strip ${DISAPP}), YES)
	SubFile += Base/Base.mk 
	SubFile += Excel/Excel.mk 
	SubFile += Log/Log.mk 
	SubFile += Network/Network.mk 
	SubFile += RTSPClient/RTSPClient.mk 
	SubFile += Protocol/Protocol.mk 
	SubFile += ExtProtocol/ExtProtocol.mk 
	SubFile += XMQ/XMQ.mk 
	SubFile += XMQClient/XMQClient.mk 
	SubFile += XMS/XMS.mk 
	SubFile += GBStack/GBStack.mk
	SubFile += Configuration/Configuration.mk 	
	SubFile += MongoClient/MongoClient.mk 
	SubFile += MongoClient2/MongoClient2.mk 
	SubFile += HTTP/HTTP.mk 
	SubFile += DumpDownload/DumpDownload.mk 
	SubFile += StreamTarget/StreamTarget.mk 
	SubFile += StreamSource/StreamSource.mk 
	SubFile += sqlite3/sqlite3.mk	

	#SubFile += test/test.mk 
endif

#设置打包功能
PacketFile = 


#工作目录设置
CURRPATH = $(shell pwd)
export PRJ_PATH = ${CURRPATH}
export PRJ_LIBDIR = ${PRJ_PATH}/Lib/
export PRJ_INCDIR = ${PRJ_PATH}/Include/
export PRJ_COMPILEDIR=${PRJ_PATH}/__Compile/

#是否静默安装模式
ifeq ($(strip ${ENABLESILIENT}), YES)
	SILENT=-s
else
	SILENT=
endif


#生存编译环境
CURRPLATFROM = x86
ifneq (${CROSS}, )
	CURRPLATFROM=${subst - ,,${CROSS} }
endif

#编译环境
export PLATFORM = ${CURRPLATFROM}


all:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} all; cd ${PRJ_PATH};)
app:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} app; cd ${PRJ_PATH};)
apprelease:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} apprelease; cd ${PRJ_PATH};)
appdebug:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} appdebug; cd ${PRJ_PATH};)
install:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} install; cd ${PRJ_PATH};)
uninstall:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} uninstall; cd ${PRJ_PATH};)
clean:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} clean; cd ${PRJ_PATH};)
cleandep:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} cleandep; cd ${PRJ_PATH};)
cleanall:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} cleanall; cd ${PRJ_PATH};)
share:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} share; cd ${PRJ_PATH};)
sharelib:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} sharelib; cd ${PRJ_PATH};)
static:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} static; cd ${PRJ_PATH};)
staticlib:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} staticlib; cd ${PRJ_PATH};)
staticdebug:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} staticdebug; cd ${PRJ_PATH};)
lib:
	$(foreach file, ${SubFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} lib; cd ${PRJ_PATH};)

pkt:
	$(foreach file, ${PacketFile},cd $(shell dirname ${file}); ${MAKE} -f $(shell basename ${file}) ${SILENT} pkt; cd ${PRJ_PATH};)

sln:
	$(foreach file, ${SubFile},cd ${MKTOOLDIR};/bin/sh Publish.sh sln depend ${CURRPATH}/Makefile ${CURRPATH}/${file})
