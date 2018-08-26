#
#  Copyright (c)1998-2012, Chongqing Xunmei Technology
#  All Rights Reserved.
#
#	 Description:
#  $Id: rules.mk 8641 2016-03-29 04:50:18Z lixiaoqiang $
#

.PHONY: all app apprelease appdebug install uninstall clean cleanall cleandep share sharelib sharedebug static staticlib staticdebug lib\
	install_Lib install_Lib_Dbg install_ShareLib install_SharedLib_Dbg install_App install_App_Dbg  install_Header  pkt pkt_debug pkt_realse

DEBUG_SUFFIX = debug

LIB_SRCS_CPP := $(foreach dir,$(SRCS_PATH),$(wildcard $(dir)/*.cpp))
LIB_SRCS_CC := $(foreach dir,$(SRCS_PATH),$(wildcard $(dir)/*.cc))
LIB_SRCS_CXX := $(foreach dir,$(SRCS_PATH),$(wildcard $(dir)/*.cxx))
LIB_SRCS_C 	:= $(foreach dir,$(SRCS_PATH),$(wildcard $(dir)/*.c))
LIB_CPP_OBJS = $(patsubst %.cpp,${COMPILE_PATH}/%.o,${LIB_SRCS_CPP})
LIB_CPP_OBJS += $(patsubst %.cc,${COMPILE_PATH}/%.o,${LIB_SRCS_CC})
LIB_CPP_OBJS += $(patsubst %.cxx,${COMPILE_PATH}/%.o,${LIB_SRCS_CXX})
LIB_C_OBJS =$(patsubst %.c,${COMPILE_PATH}/%.o, ${LIB_SRCS_C})
LIB_OBJS = ${LIB_CPP_OBJS} ${LIB_C_OBJS}

LIB_OBJD = $(patsubst %.o, %.cpp.d, $(LIB_CPP_OBJS))
LIB_OBJD += $(patsubst %.o, %.cc.d, $(LIB_CPP_OBJS))
LIB_OBJD += $(patsubst %.o, %.cxx.d, $(LIB_CPP_OBJS))
LIB_OBJD +=	$(patsubst %.o, %.c.d, $(LIB_C_OBJS))
						
LIB_OBJS_DBG = $(patsubst %.cpp, ${COMPILE_PATH}/%.$(DEBUG_SUFFIX).o, $(LIB_SRCS_CPP))
LIB_OBJS_DBG += $(patsubst %.cc, ${COMPILE_PATH}/%.$(DEBUG_SUFFIX).o, $(LIB_SRCS_CC))
LIB_OBJS_DBG += $(patsubst %.cxx, ${COMPILE_PATH}/%.$(DEBUG_SUFFIX).o, $(LIB_SRCS_CXX))
LIB_OBJS_DBG +=$(patsubst %.c,${COMPILE_PATH}/%.$(DEBUG_SUFFIX).o, ${LIB_SRCS_C})

LIB_OBJD_DBG =$(patsubst %.o, %.cpp.d.$(DEBUG_SUFFIX), $(LIB_CPP_OBJS))
LIB_OBJD_DBG +=$(patsubst %.o, %.cc.d.$(DEBUG_SUFFIX), $(LIB_CPP_OBJS))
LIB_OBJD_DBG +=$(patsubst %.o, %.cxx.d.$(DEBUG_SUFFIX), $(LIB_CPP_OBJS))
LIB_OBJD_DBG	+= $(patsubst %.o, %.c.d.$(DEBUG_SUFFIX), $(LIB_C_OBJS))



APP_SRCS_CPP	=	$(foreach dir,$(APP_SRCS_PATH),$(wildcard $(dir)/*.cpp))
APP_SRCS_CPP	+=	$(foreach dir,$(APP_SRCS_PATH),$(wildcard $(dir)/*.cxx))
APP_SRCS_C	=	$(foreach dir,$(APP_SRCS_PATH),$(wildcard $(dir)/*.c))
APP_CPP_OBJS = $(patsubst %.cpp,${COMPILE_PATH}/%.o,$(APP_SRCS_CPP))
APP_C_OBJS = $(patsubst %.c,${COMPILE_PATH}/%.o,$(APP_SRCS_C))
APP_OBJS	=	${APP_CPP_OBJS} ${APP_C_OBJS}

APP_OBJD = $(patsubst %.o, %.cpp.d, $(APP_CPP_OBJS))
APP_OBJD +=	$(patsubst %.o, %.c.d, $(APP_C_OBJS))

APP_OBJS_DBG	=	$(patsubst %.cpp,${COMPILE_PATH}/%.$(DEBUG_SUFFIX).o,$(APP_SRCS_CPP))
APP_OBJS_DBG	+=	$(patsubst %.c,${COMPILE_PATH}/%.$(DEBUG_SUFFIX).o,$(APP_SRCS_C))
APP_OBJS_DBG	+=	$(patsubst %.cxx,${COMPILE_PATH}/%.$(DEBUG_SUFFIX).o,$(LIB_SRCS_CXX))

APP_OBJD_DBG =$(patsubst %.o, %.cpp.d.$(DEBUG_SUFFIX), $(APP_CPP_OBJS))
APP_OBJD_DBG += $(patsubst %.o, %.c.d.$(DEBUG_SUFFIX), $(APP_C_OBJS))

COMPILE_TIME = `date "+%Y%m%d %H:%M:%S"`
COMPILE_USER = `id -u -n`
#all: Print Ver install $(TARGET) 
all: Print install $(TARGET) 

Ver:
	@echo "#ifndef __${COMPILETARGET}__H__" >$(VERSION_REAL)
	@echo "#define __${COMPILETARGET}__H__" >>$(VERSION_REAL)
ifeq ($(strip ${GENSVNVER}), YES)
	@echo "static const char svnversion[] = \"`svnversion ${SVNVER_PATH}`\";">>$(VERSION_REAL)
endif
	@echo "static const int r_major = $(R_MAJOR);">>$(VERSION_REAL)	
	@echo "static const int r_minor = $(R_MINOR);">>$(VERSION_REAL)
	@echo "static const int r_revision = $(R_REVISION);">>$(VERSION_REAL)
	@echo "static const char compiletime[] = \"${COMPILE_TIME}\";" >>$(VERSION_REAL)
	@echo "static const char compileuser[] = \"${COMPILE_USER}\";" >>$(VERSION_REAL)
	@echo "#endif  //__${COMPILETARGET}__H__"  >>$(VERSION_REAL)
	@echo " " >>$(VERSION_REAL)


lib:PrintLib share static
share:PrintShare $(SHARDLIB_TARGET) $(SHARDLIB_TARGET_DBG)
sharelib:PrintShareRelease $(SHARDLIB_TARGET)
sharedebug:PrintShareDebug $(SHARDLIB_TARGET_DBG)
static:PrintStatic ${LIB_TARGET} ${LIB_TARGET_DBG}
staticlib:PrintStaticRelease ${LIB_TARGET}
staticdebug:PrintStaticDebug ${LIB_TARGET_DBG}



PACKETDIR=${BINDIR}/__packet_$(R_MAJOR).$(R_MINOR).$(R_REVISION).${srcsvn}
PUBLISHVER=$(R_MAJOR).$(R_MINOR).$(R_REVISION).${srcsvn}
PUBLISHTIME=$(shell date "+%Y-%m-%d")
SERVICEINFOXML=install/${APP_NAME}/Service.xml

pkt:pkt_debug pkt_realse
	
pkt_debug:
ifneq ($(strip ${APP_TARGET_DBG}),)
ifeq (install/${APP_NAME}, $(wildcard install/${APP_NAME}))
	@rm -rf ${APP_NAME}_Linux_D_$(R_MAJOR).$(R_MINOR).$(R_REVISION).${srcsvn}.zip
	@mkdir -p "${PACKETDIR}"
	@cp -rf ${APP_TARGET_DBG} ${PACKETDIR}/${APP_NAME}
ifneq ($(strip $(shell ls install/${APP_NAME}/x86)), )
	@cp -rf install/${APP_NAME}/x86/* ${PACKETDIR}
endif
	@cp -rf install/${APP_NAME}/common/* ${PACKETDIR}
	
	@sed -e 's/{PublishVersion}/${PUBLISHVER}/g' -e 's/{PublishTime}/${PUBLISHTIME}/g' -e 's/{Platform}/Linux/g' ${SERVICEINFOXML} > ${PACKETDIR}/ServerComponent.xml
	@cd ${PACKETDIR}/ && zip -q -r ../${APP_NAME}_Linux_D_$(R_MAJOR).$(R_MINOR).$(R_REVISION).${srcsvn}.zip ./*
	@rm -rf ${PACKETDIR}
	@echo Bulding Debug Packet ${BINDIR}/${APP_NAME}_Linux_D_$(R_MAJOR).$(R_MINOR).$(R_REVISION).${srcsvn}.zip Success!
else
	@echo noPath install/${APP_NAME}
endif
endif
	
pkt_realse:
ifneq ($(strip ${APP_TARGET}),)
ifeq (install/${APP_NAME}, $(wildcard install/${APP_NAME}))
	@rm -rf ${APP_NAME}_Linux_R_$(R_MAJOR).$(R_MINOR).$(R_REVISION).${srcsvn}.zip
	@mkdir -p "${PACKETDIR}"
	@cp -rf ${APP_TARGET} ${PACKETDIR}/${APP_NAME}
ifneq ($(strip $(shell ls install/${APP_NAME}/x86)), )
	@cp -rf install/${APP_NAME}/x86/* ${PACKETDIR}
endif
	@cp -rf install/${APP_NAME}/common/* ${PACKETDIR}
	@sed -e 's/{PublishVersion}/${PUBLISHVER}/g' -e 's/{PublishTime}/${PUBLISHTIME}/g' -e 's/{Platform}/Linux/g' ${SERVICEINFOXML} > ${PACKETDIR}/ServerComponent.xml
	@cd ${PACKETDIR}/ && zip -q -r ../${APP_NAME}_Linux_R_$(R_MAJOR).$(R_MINOR).$(R_REVISION).${srcsvn}.zip ./*
	@rm -rf ${PACKETDIR}
	@echo Bulding Realse Packet ${BINDIR}/${APP_NAME}_Linux_R_$(R_MAJOR).$(R_MINOR).$(R_REVISION).${srcsvn}.zip Success!
else
	@echo noPath install/${APP_NAME}
endif
endif

install:install_Lib install_Lib_Dbg install_ShareLib install_SharedLib_Dbg  install_App install_App_Dbg install_Header

install_Lib:${LIB_TARGET}
ifneq ($(strip ${LIB_TARGET}), )
ifneq ($(strip ${INSTALL_DIR_LIB}),)
	$(INSTALL) ${INSTALL_LIB_FLAGS} ${LIB_TARGET} $(INSTALL_DIR_LIB)/$(notdir ${LIB_TARGET})
endif
endif

install_Lib_Dbg: ${LIB_TARGET_DBG}
ifneq ($(strip ${LIB_TARGET_DBG}), )
ifneq ($(strip ${INSTALL_DIR_LIBDBG}),)
 ifeq ($(strip ${INSTALL_LIB_NAME_HOMONYMY}),YES)
	$(INSTALL) ${INSTALL_LIB_FLAGS} ${LIB_TARGET_DBG} $(INSTALL_DIR_LIBDBG)/$(notdir ${LIB_TARGET})
 else
	$(INSTALL) ${INSTALL_LIB_FLAGS} ${LIB_TARGET_DBG} $(INSTALL_DIR_LIBDBG)/$(notdir ${LIB_TARGET_DBG})
 endif
endif
endif

install_ShareLib: $(SHARDLIB_TARGET) 
ifneq ($(strip ${SHARDLIB_TARGET}), )
ifneq ($(strip ${INSTALL_DIR_SHARELIB}),)
	$(INSTALL) ${INSTALL_LIB_FLAGS} ${SHARDLIB_TARGET} $(INSTALL_DIR_SHARELIB)/$(notdir ${SHARDLIB_TARGET})
endif
endif

install_SharedLib_Dbg:$(SHARDLIB_TARGET_DBG)
ifneq ($(strip ${SHARDLIB_TARGET_DBG}), )
ifneq ($(strip ${INSTALL_DIR_SHARELIBDBG}),)
 ifeq ($(strip ${INSTALL_LIBSHARE_NAME_HOMONYMY}),YES)
	$(INSTALL) ${INSTALL_LIB_FLAGS} ${SHARDLIB_TARGET_DBG} $(INSTALL_DIR_SHARELIBDBG)/$(notdir ${SHARDLIB_TARGET})
 else
	$(INSTALL) ${INSTALL_LIB_FLAGS} ${SHARDLIB_TARGET_DBG} $(INSTALL_DIR_SHARELIBDBG)/$(notdir ${SHARDLIB_TARGET_DBG})
 endif
endif
endif

install_App:$(APP_TARGET)
ifneq ($(strip ${APP_TARGET}),)
ifneq ($(strip ${INSTALL_DIR_APP}),)
	$(INSTALL) ${INSTALL_APP_FLAGS} $(APP_TARGET) $(INSTALL_DIR_APP)/
endif
endif

install_App_Dbg:$(APP_TARGET_DBG)
ifneq ($(strip ${APP_TARGET_DBG}),)
ifneq ($(strip ${INSTALL_DIR_APPDBG}),)
ifneq ($(strip ${INSTALL_APP_NAME_HOMONYMY}), YES)
	$(INSTALL) ${INSTALL_APP_FLAGS}  $(APP_TARGET_DBG) $(INSTALL_DIR_APPDBG)/$(notdir ${APP_TARGET})
else
	$(INSTALL) ${INSTALL_APP_FLAGS}  $(APP_TARGET_DBG) $(INSTALL_DIR_APPDBG)/
endif
endif
endif

install_Header:${NEEDINSTALL_HEADER_DIR}
ifneq ($(strip ${NEEDINSTALL_HEADER_DIR}),)
ifneq ($(strip ${INSTALL_DIR_HEADER}),)
	$(COPY) ${NEEDINSTALL_HEADER_DIR} ${INSTALL_DIR_HEADER}
	@find ${INSTALL_DIR_HEADER} -type d -name ".svn" |xargs rm -rf
endif
endif

uninstall:
ifneq ($(strip ${LIB_TARGET}), )
ifneq ($(strip ${INSTALL_DIR_LIB}),)
	$(RM) $(INSTALL_DIR_LIB)/$(notdir ${LIB_TARGET})
endif
endif
ifneq ($(strip ${LIB_TARGET_DBG}), )
ifneq ($(strip ${INSTALL_DIR_LIBDBG}),)
 ifeq ($(strip ${INSTALL_LIB_NAME_HOMONYMY}),YES)
	$(RM) $(INSTALL_DIR_LIBDBG)/$(notdir ${LIB_TARGET})
 else
	$(RM) $(INSTALL_DIR_LIBDBG)/$(notdir ${LIB_TARGET_DBG})
 endif
endif
endif
ifneq ($(strip ${SHARDLIB_TARGET}), )
ifneq ($(strip ${INSTALL_DIR_SHARELIB}),)
	$(RM) ${INSTALL_LIB_FLAGS} ${SHARDLIB_TARGET} $(INSTALL_DIR_SHARELIB)/$(notdir ${LIB_TARGET})
endif
endif
ifneq ($(strip ${SHARDLIB_TARGET_DBG}), )
ifneq ($(strip ${INSTALL_DIR_SHARELIBDBG}),)
 ifeq ($(strip ${INSTALL_LIBSHARE_NAME_HOMONYMY}),YES)
	$(RM) $(INSTALL_DIR_SHARELIBDBG)/$(notdir ${SHARDLIB_TARGET})
 else
	$(RM) $(INSTALL_DIR_SHARELIBDBG)/$(notdir ${SHARDLIB_TARGET_DBG})
 endif
endif
endif
ifneq ($(strip ${APP_TARGET}),)
ifneq ($(strip ${INSTALL_DIR_APP}),)
	$(RM) $(INSTALL_DIR_APP)/$(notdir ${APP_TARGET})
endif
endif
ifneq ($(strip ${APP_TARGET_DBG}),)
ifneq ($(strip ${INSTALL_DIR_APPDBG}),)
ifneq ($(strip ${INSTALL_APP_NAME_HOMONYMY}), YES)
	$(RM) $(INSTALL_DIR_APPDBG)/$(notdir ${APP_TARGET})
else
	$(RM) $(INSTALL_DIR_APPDBG)/$(notdir $(APP_TARGET_DBG))
endif
endif
endif
ifneq ($(strip ${NEEDINSTALL_HEADER_DIR}),)
ifneq ($(strip ${INSTALL_DIR_HEADER}),)
	$(RMDIR) ${INSTALL_DIR_HEADER}
endif
endif

$(LIB_TARGET): $(LIB_OBJS)
	$(AR) $(AFLAGS) $@ $^ $(LIB_EXTERN) $(LIB_TARGETLINK)
	
$(LIB_TARGET_DBG): $(LIB_OBJS_DBG)
	$(AR) $(AFLAGS) $@ $^ $(LIB_EXTERN_DBG) $(LIB_TARGETLINK_DBG)

$(SHARDLIB_TARGET): $(LIB_C_OBJS) $(LIB_OBJS) 
ifneq ($(strip ${LIB_OBJS}), )
	$(CPP) -shared ${CFLAGS} $(CFLAGS_NDBG) -o $@ $^ $(SHARDLIB_EXTERN) $(SHARDLIB_TARGETLINK)
	$(STRIP) $@
else
	$(CC) -shared ${CFLAGS} $(CFLAGS_NDBG)  -o $@ $^ $(SHARDLIB_EXTERN) $(SHARDLIB_TARGETLINK)
	$(STRIP) $@
endif

	
	
$(SHARDLIB_TARGET_DBG): $(LIB_C_OBJS_DBG) $(LIB_OBJS_DBG)
ifneq ($(strip ${LIB_OBJS_DBG}), )
	$(CPP) -shared ${CFLAGS} $(CFLAGS_DBG) -o $@ $^ $(SHARDLIB_EXTERN_DBG) $(SHARDLIB_TARGETLINK_DBG)
else
	$(CC) -shared ${CFLAGS} $(CFLAGS_DBG) -o $@ $^ $(SHARDLIB_EXTERN_DBG) $(SHARDLIB_TARGETLINK_DBG)
endif
	
	
#compile
${COMPILE_PATH}/%.$(DEBUG_SUFFIX).o:%.cpp 
	$(CPP) -c $(CFLAGS) $(CFLAGS_DBG) $< -o $@

${COMPILE_PATH}/%.$(DEBUG_SUFFIX).o:%.cc
	$(CPP) -c $(CFLAGS) $(CFLAGS_DBG) $< -o $@

${COMPILE_PATH}/%.$(DEBUG_SUFFIX).o:%.cxx
	$(CPP) -c $(CFLAGS) $(CFLAGS_DBG) $< -o $@

${COMPILE_PATH}/%.$(DEBUG_SUFFIX).o:%.c 
	$(CC) -c $(CFLAGS) $(CFLAGS_DBG) $< -o $@
	
${COMPILE_PATH}/%.o:%.cpp
	$(CPP) -c $(CFLAGS) $(CFLAGS_NDBG) $< -o $@

${COMPILE_PATH}/%.o:%.cc
	$(CPP) -c $(CFLAGS) $(CFLAGS_NDBG) $< -o $@

${COMPILE_PATH}/%.o:%.cxx
	$(CPP) -c $(CFLAGS) $(CFLAGS_NDBG) $< -o $@

${COMPILE_PATH}/%.o:%.c
	$(CC) -c $(CFLAGS) $(CFLAGS_NDBG) $< -o $@

	
${COMPILE_PATH}/%.o:$(dir)/%.cpp
	$(CPP) -c $(CFLAGS) $(CFLAGS_NDBG) $< -o $@

${COMPILE_PATH}/%.o:$(dir)/%.cc
	$(CPP) -c $(CFLAGS) $(CFLAGS_NDBG) $< -o $@

${COMPILE_PATH}/%.o:$(dir)/%.cxx
	$(CPP) -c $(CFLAGS) $(CFLAGS_NDBG) $< -o $@

${COMPILE_PATH}/%.o:$(dir)/%.c
	$(CC) -c $(CFLAGS) $(CFLAGS_NDBG) $< -o $@

	
#depend 	
${COMPILE_PATH}/%.c.d:%.c
	$(CC) $(CFLAGS) ${CFLAGS_NDBG} -MM -MT '$(dir $@)$(subst .c.d,.o,$(notdir $@))'  -E $^ > $@
	
${COMPILE_PATH}/%.cpp.d:%.cpp
	$(CPP) $(CFLAGS) ${CFLAGS_NDBG} -MM -MT '$(dir $@)$(subst .cpp.d,.o,$(notdir $@))' -E $^ > $@

${COMPILE_PATH}/%.cc.d:%.cc
	$(CPP) $(CFLAGS) ${CFLAGS_NDBG} -MM -MT '$(dir $@)$(subst .cc.d,.o,$(notdir $@))' -E $^ > $@

${COMPILE_PATH}/%.cc.d:%.cxx
	$(CPP) $(CFLAGS) ${CFLAGS_NDBG} -MM -MT '$(dir $@)$(subst .cxx.d,.o,$(notdir $@))' -E $^ > $@

${COMPILE_PATH}/%.c.d.$(DEBUG_SUFFIX):%.c
	$(CC) $(CFLAGS) ${CFLAGS_DBG} -MM -MT '$(dir $@)$(subst .c.d.$(DEBUG_SUFFIX),.$(DEBUG_SUFFIX).o,$(notdir $@))'  -E $^ > $@
	
${COMPILE_PATH}/%.cpp.d.$(DEBUG_SUFFIX):%.cpp
	$(CPP) $(CFLAGS) ${CFLAGS_DBG} -MM -MT '$(dir $@)$(subst .cpp.d.$(DEBUG_SUFFIX),.$(DEBUG_SUFFIX).o,$(notdir $@))' -E $^ > $@

${COMPILE_PATH}/%.cc.d.$(DEBUG_SUFFIX):%.cc
	$(CPP) $(CFLAGS) ${CFLAGS_DBG} -MM -MT '$(dir $@)$(subst .cc.d.$(DEBUG_SUFFIX),.$(DEBUG_SUFFIX).o,$(notdir $@))' -E $^ > $@

${COMPILE_PATH}/%.cc.d.$(DEBUG_SUFFIX):%.cxx
	$(CPP) $(CFLAGS) ${CFLAGS_DBG} -MM -MT '$(dir $@)$(subst .cxx.d.$(DEBUG_SUFFIX),.$(DEBUG_SUFFIX).o,$(notdir $@))' -E $^ > $@

$(APP_TARGET):$(APP_OBJS) $(LIB_TARGET) $(LIBS) 
ifneq ($(strip ${APP_CPP_OBJS}), )
	$(CPP) ${LDFLAGS} $(APP_OBJS) -Xlinker --start-group ${LIBS} $(LIB_TARGET) ${LDLIBS} -Xlinker --end-group -o $@
	$(STRIP) $@
else
	$(CC) ${LDFLAGS} $(APP_OBJS) -Xlinker --start-group ${LIBS} $(LIB_TARGET) ${LDLIBS} -Xlinker --end-group -o $@
	$(STRIP) $@
endif

	
$(APP_TARGET_DBG):$(APP_OBJS_DBG) $(LIB_TARGET_DBG) $(LIBS_DBG)	
ifneq ($(strip ${APP_CPP_OBJS}), )
	$(CPP) ${LDFLAGS} $(APP_OBJS_DBG) -Xlinker --start-group  $(LIB_TARGET_DBG) $(LIBS_DBG)	$(LDLIBS_DBG) -Xlinker --end-group -o $@
else
	$(CC) ${LDFLAGS} $(APP_OBJS_DBG) -Xlinker --start-group  $(LIB_TARGET_DBG) $(LIBS_DBG)	$(LDLIBS_DBG) -Xlinker --end-group -o $@
endif


app:PrintApp $(APP_TARGET) $(APP_TARGET_DBG)
apprelease:PrintAppRelease $(APP_TARGET)
appdebug:PrintAppDebug $(APP_TARGET_DBG)

clean:
	$(RM) $(TARGET) $(LIB_OBJS) $(LIB_OBJS_DBG)  ${APP_OBJS} ${APP_OBJS_DBG} ${COMPILE_PATH}
cleandep:
	$(RM) ${LIB_OBJD} ${LIB_OBJD_DBG}  ${APP_OBJD} ${APP_OBJD_DBG} ${COMPILE_PATH}
cleanall:clean cleandep
ifneq ($(strip ${MAKECMDGOALS}),clean)
ifneq ($(strip ${MAKECMDGOALS}),cleanall)
ifneq ($(strip ${MAKECMDGOALS}),uninstall)
-include $(LIB_OBJD)
-include ${LIB_OBJD_DBG}
	ifneq ($(strip $(APP_TARGET)), )
	ifneq ($(strip $(APP_TARGET_DBG)), )	
	-include ${APP_OBJD}
	-include ${APP_OBJD_DBG}
	endif
	endif
endif
endif
endif
