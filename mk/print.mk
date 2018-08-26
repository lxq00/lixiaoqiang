#
#  Copyright (c)1998-2012, Chongqing Xunmei Technology
#  All Rights Reserved.
#
#	 Description:
#  $Id: print.mk 3877 2015-03-02 10:28:16Z shaoyikai $
#

Print:
	@echo 
	@echo Compile Target :${COMPILETARGET}
	@echo CompilePlatform: ${PLATFORM}
	@# Static Lib Info
ifneq ($(strip ${LIB_TARGET}),)
	@echo LibTarget Release: $(notdir ${LIB_TARGET})
ifneq ($(strip ${INSTALL_DIR_LIB}),)
	@echo LibTarget Release Install URL $(INSTALL_DIR_LIB)/$(notdir ${LIB_TARGET})
endif
endif
	@# Static Debug Lib Info
ifneq ($(strip ${LIB_TARGET_DBG}),)
	@echo LibTarget Debug: $(notdir ${LIB_TARGET_DBG})
ifneq ($(strip ${INSTALL_DIR_LIBDBG}),)
 ifeq ($(strip ${INSTALL_LIB_NAME_HOMONYMY}),YES)
	@echo LibTarget Debug: Install URL ${INSTALL_DIR_LIBDBG}/$(notdir ${LIB_TARGET})
 else
	@echo LibTarget Debug: Install URL ${INSTALL_DIR_LIBDBG}/$(notdir ${LIB_TARGET_DBG})
 endif
endif
endif
	@# Share Lib Info
ifneq ($(strip ${SHARDLIB_TARGET}),)
	@echo ShareLibTarget Release: $(notdir ${SHARDLIB_TARGET})
ifneq ($(strip ${INSTALL_DIR_SHARELIB}),)
	@echo ShareLibTarget Release Install URL $(INSTALL_DIR_SHARELIB)/$(notdir ${SHARDLIB_TARGET})
endif
endif
	@# Share Debug Lib Info
ifneq ($(strip ${SHARDLIB_TARGET_DBG}),)
	@echo ShareLibTarget Debug: $(notdir ${SHARDLIB_TARGET_DBG})
ifneq ($(strip ${INSTALL_DIR_SHARELIBDBG}),)
 ifeq ($(strip ${INSTALL_LIBSHARE_NAME_HOMONYMY}),YES)
	@echo ShareLibTarget Debug: Install URL ${INSTALL_DIR_SHARELIBDBG}/$(notdir ${SHARDLIB_TARGET})
 else
	@echo ShareLibTarget Debug: Install URL ${INSTALL_DIR_SHARELIBDBG}/$(notdir ${SHARDLIB_TARGET_DBG})
 endif
endif
endif
	@#App Info
ifneq ($(strip ${APP_TARGET}),)
	@echo AppTarget Release: ${APP_TARGET}
ifneq ($(strip ${INSTALL_DIR_APP}),)
	@echo AppTarget  Release Install URL: ${INSTALL_DIR_APP}/$(notdir ${APP_TARGET})
endif
endif
	@#App Debug Info
ifneq ($(strip ${APP_TARGET_DBG}),)	
	@echo AppTarget Debug: ${APP_TARGET_DBG} 
ifneq ($(strip ${INSTALL_DIR_APPDBG}),)
 ifeq ($(strip ${INSTALL_APP_NAME_HOMONYMY}),YES)
	@echo AppTarget Debug: Install URL  ${INSTALL_DIR_APPDBG}/$(notdir ${APP_TARGET})
 else
	@echo AppTarget Debug: Install URL ${INSTALL_DIR_APPDBG}/$(notdir ${APP_TARGET_DBG})
 endif
endif
endif
	@#Version Info
	@echo  Version:${R_MAJOR}.${R_MINOR}.${R_REVISION}
ifeq ($(strip ${GENSVNVER}), YES)	
	@echo  Svn Version include:${inclsvn}, Src: ${srcsvn}
endif
	@#Compile Time
	@echo CompileDate:$(shell date +%Y-%m-%d\ %H:%M:%S\)
	@#Flags
	@echo CFLAGS = ${CFLAGS}
	@echo CFLAGS_NDBG = ${CFLAGS_NDBG}
	@echo CFLAGS_DBG = ${CFLAGS_DBG}
	@echo CPPFLAGS = ${CPPFLAGS}
	@echo LDFLAGS = ${LDFLAGS}
	@echo AFLAGS = ${AFLAGS}
	
	
PrintApp:
ifeq ($(strip ${MAKECMDGOALS}),app)
	@echo 
	@echo Compile App Target : $(APP_TARGET) $(APP_TARGET_DBG)
	@echo CompilePlatform: ${PLATFORM}
	@echo  Version:${R_MAJOR}.${R_MINOR}.${R_REVISION}
ifeq ($(strip ${GENSVNVER}), YES)	
	@echo  Svn Version include:${inclsvn}, Src: ${srcsvn}
endif
	@echo CompileDate:$(shell date +%Y-%m-%d\ %H:%M:%S\)
	@echo CFLAGS = ${CFLAGS}
	@echo CFLAGS_NDBG = ${CFLAGS_NDBG}
	@echo CFLAGS_DBG = ${CFLAGS_DBG}
	@echo LDFLAGS = ${LDFLAGS}
	@echo AFLAGS = ${AFLAGS}
endif

PrintAppRelease:
ifeq ($(strip ${MAKECMDGOALS}),apprelease)
	@echo 
	@echo Compile App Target : $(APP_TARGET)
	@echo CompilePlatform: ${PLATFORM}
	@echo  Version:${R_MAJOR}.${R_MINOR}.${R_REVISION}
ifeq ($(strip ${GENSVNVER}), YES)	
	@echo  Svn Version include:${inclsvn}, Src: ${srcsvn}
endif
	@echo CompileDate:$(shell date +%Y-%m-%d\ %H:%M:%S\)
	@echo CFLAGS = ${CFLAGS}
	@echo CFLAGS_NDBG = ${CFLAGS_NDBG}
	@echo CFLAGS_DBG = ${CFLAGS_DBG}
	@echo LDFLAGS = ${LDFLAGS}
	@echo AFLAGS = ${AFLAGS}
endif

PrintAppDebug:
ifeq ($(strip ${MAKECMDGOALS}),appdebug)
	@echo 
	@echo Compile App Debug Target : $(APP_TARGET_DBG)
	@echo CompilePlatform: ${PLATFORM}
	@echo  Version:${R_MAJOR}.${R_MINOR}.${R_REVISION}
ifeq ($(strip ${GENSVNVER}), YES)	
	@echo  Svn Version include:${inclsvn}, Src: ${srcsvn}
endif
	@echo CompileDate:$(shell date +%Y-%m-%d\ %H:%M:%S\)
	@echo CFLAGS = ${CFLAGS}
	@echo CFLAGS_NDBG = ${CFLAGS_NDBG}
	@echo CFLAGS_DBG = ${CFLAGS_DBG}
	@echo LDFLAGS = ${LDFLAGS}
	@echo AFLAGS = ${AFLAGS}
endif

PrintLib:
ifeq ($(strip ${MAKECMDGOALS}),lib)
	@echo 
	@echo Compile Lib Target : ${LIB_TARGET} ${LIB_TARGET_DBG} $(SHARDLIB) $(SHARDLIB_DBG)
	@echo CompilePlatform: ${PLATFORM}
	@echo  Version:${R_MAJOR}.${R_MINOR}.${R_REVISION}
ifeq ($(strip ${GENSVNVER}), YES)	
	@echo  Svn Version include:${inclsvn}, Src: ${srcsvn}
endif	
	@echo CompileDate:$(shell date +%Y-%m-%d\ %H:%M:%S\)
	@echo CFLAGS = ${CFLAGS}
	@echo CFLAGS_NDBG = ${CFLAGS_NDBG}
	@echo CFLAGS_DBG = ${CFLAGS_DBG}
	@echo LDFLAGS = ${LDFLAGS}
	@echo AFLAGS = ${AFLAGS}
endif

PrintShare:
ifeq ($(strip ${MAKECMDGOALS}),share)
	@echo 
	@echo Compile Share Lib Target :  $(SHARDLIB_TARGET) $(SHARDLIB_TARGET_DBG)
	@echo CompilePlatform: ${PLATFORM}
	@echo  Version:${R_MAJOR}.${R_MINOR}.${R_REVISION}
ifeq ($(strip ${GENSVNVER}), YES)	
	@echo  Svn Version include:${inclsvn}, Src: ${srcsvn}
endif	
	@echo CompileDate:$(shell date +%Y-%m-%d\ %H:%M:%S\)
	@echo CFLAGS = ${CFLAGS}
	@echo CFLAGS_NDBG = ${CFLAGS_NDBG}
	@echo CFLAGS_DBG = ${CFLAGS_DBG}
	@echo LDFLAGS = ${LDFLAGS}
	@echo AFLAGS = ${AFLAGS}
endif

PrintShareRelease:
ifeq ($(strip ${MAKECMDGOALS}),sharelib)
	@echo 
	@echo Compile Share Release Lib Target :  $(SHARDLIB_TARGET)
	@echo CompilePlatform: ${PLATFORM}
	@echo  Version:${R_MAJOR}.${R_MINOR}.${R_REVISION}
ifeq ($(strip ${GENSVNVER}), YES)	
	@echo  Svn Version include:${inclsvn}, Src: ${srcsvn}
endif
	@echo CompileDate:$(shell date +%Y-%m-%d\ %H:%M:%S\)
	@echo CFLAGS = ${CFLAGS}
	@echo CFLAGS_NDBG = ${CFLAGS_NDBG}
	@echo CFLAGS_DBG = ${CFLAGS_DBG}
	@echo LDFLAGS = ${LDFLAGS}
	@echo AFLAGS = ${AFLAGS}
endif

PrintShareDebug:
ifeq ($(strip ${MAKECMDGOALS}),sharedebug)
	@echo 
	@echo Compile Share Debug Lib Target :  $(SHARDLIB_TARGET_DBG)
	@echo CompilePlatform: ${PLATFORM}
	@echo  Version:${R_MAJOR}.${R_MINOR}.${R_REVISION}
ifeq ($(strip ${GENSVNVER}), YES)	
	@echo  Svn Version include:${inclsvn}, Src: ${srcsvn}
endif
	@echo CompileDate:$(shell date +%Y-%m-%d\ %H:%M:%S\)
	@echo CFLAGS = ${CFLAGS}
	@echo CFLAGS_NDBG = ${CFLAGS_NDBG}
	@echo CFLAGS_DBG = ${CFLAGS_DBG}
	@echo LDFLAGS = ${LDFLAGS}
	@echo AFLAGS = ${AFLAGS}
endif

PrintStatic:
ifeq ($(strip ${MAKECMDGOALS}),static)
	@echo 
	@echo Compile Static Lib Target :   ${LIB_TARGET} ${LIB_TARGET_DBG}
	@echo CompilePlatform: ${PLATFORM}
	@echo  Version:${R_MAJOR}.${R_MINOR}.${R_REVISION}
ifeq ($(strip ${GENSVNVER}), YES)	
	@echo  Svn Version include:${inclsvn}, Src: ${srcsvn}
endif
	@echo CompileDate:$(shell date +%Y-%m-%d\ %H:%M:%S\)
	@echo CFLAGS = ${CFLAGS}
	@echo CFLAGS_NDBG = ${CFLAGS_NDBG}
	@echo CFLAGS_DBG = ${CFLAGS_DBG}
	@echo LDFLAGS = ${LDFLAGS}
	@echo AFLAGS = ${AFLAGS}
endif

PrintStaticRelease:
ifeq ($(strip ${MAKECMDGOALS}),staticlib)
	@echo 
	@echo Compile Static Release Lib Target :   ${LIB_TARGET} 
	@echo CompilePlatform: ${PLATFORM}
	@echo  Version:${R_MAJOR}.${R_MINOR}.${R_REVISION}
ifeq ($(strip ${GENSVNVER}), YES)	
	@echo  Svn Version include:${inclsvn}, Src: ${srcsvn}
endif
	@echo CompileDate:$(shell date +%Y-%m-%d\ %H:%M:%S\)
	@echo CFLAGS = ${CFLAGS}
	@echo CFLAGS_NDBG = ${CFLAGS_NDBG}
	@echo CFLAGS_DBG = ${CFLAGS_DBG}
	@echo LDFLAGS = ${LDFLAGS}
	@echo AFLAGS = ${AFLAGS}
endif

PrintStaticDebug:
ifeq ($(strip ${MAKECMDGOALS}),staticdebug)
	@echo 
	@echo Compile Static Debug Lib Target :   ${LIB_TARGET_DBG} 
	@echo CompilePlatform: ${PLATFORM}
	@echo  Version:${R_MAJOR}.${R_MINOR}.${R_REVISION}
ifeq ($(strip ${GENSVNVER}), YES)	
	@echo  Svn Version include:${inclsvn}, Src: ${srcsvn}
endif
	@echo CompileDate:$(shell date +%Y-%m-%d\ %H:%M:%S\)
	@echo CFLAGS = ${CFLAGS}
	@echo CFLAGS_NDBG = ${CFLAGS_NDBG}
	@echo CFLAGS_DBG = ${CFLAGS_DBG}
	@echo LDFLAGS = ${LDFLAGS}
	@echo AFLAGS = ${AFLAGS}
endif
