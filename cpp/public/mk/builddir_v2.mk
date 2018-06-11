#
#  Copyright (c)1998-2012, Chongqing Xunmei Technology
#  All Rights Reserved.
#
#	 Description:
#  $Id: builddir.mk 3877 2015-03-02 10:28:16Z shaoyikai $
#

DEBUG_SUFFIX = debug

ifneq ($(strip ${STATICLIB_NAME}), )
	COMPILE_PATH = ${PRJ_COMPILEDIR}/${PLATFORM}/${STATICLIB_NAME}/
	LIB_TARGET   = $(PRJ_LIBDIR)/$(PLATFORM)/$(STATICLIB_NAME)/lib$(STATICLIB_NAME).a
	LIB_TARGET_DBG = $(PRJ_LIBDIR)/$(PLATFORM)/$(STATICLIB_NAME)/lib$(STATICLIB_NAME)_$(DEBUG_SUFFIX).a
endif

ifneq ($(strip ${SHARDLIB_NAME}), )
	COMPILE_PATH = ${PRJ_COMPILEDIR}/${PLATFORM}/${SHARDLIB_NAME}/
	SHARDLIB_TARGET   = $(PRJ_LIBDIR)/$(PLATFORM)/$(SHARDLIB_NAME)/lib$(SHARDLIB_NAME).so
	SHARDLIB_TARGET_DBG = $(PRJ_LIBDIR)/$(PLATFORM)/$(SHARDLIB_NAME)/lib$(SHARDLIB_NAME)_$(DEBUG_SUFFIX).so
endif

ifneq ($(strip ${APP_NAME}), )
	COMPILE_PATH = ${PRJ_COMPILEDIR}/${PLATFORM}/${APP_NAME}/
	APP_TARGET = $(BINDIR)/$(PLATFORM)/$(APP_NAME)
	APP_TARGET_DBG = $(BINDIR)/$(PLATFORM)/$(APP_NAME)_$(DEBUG_SUFFIX)
endif

LIBDIR = ${PRJ_LIBDIR}/$(PLATFORM)
BINDIR = ${PRJ_PATH}/Bin


#build Dir
$(shell mkdir -p ${LIBDIR})
$(shell mkdir -p ${BINDIR})
$(foreach dir,$(SRCS_PATH),$(shell mkdir -p ${COMPILE_PATH}/${dir}))
$(foreach dir,$(APP_SRCS_PATH),$(shell mkdir -p ${COMPILE_PATH}/${dir}))

ifneq ($(strip ${LIB_TARGET}),)
$(shell mkdir -p $(dir ${LIB_TARGET}))
endif

ifneq ($(strip ${LIB_TARGET_DBG}),)
$(shell mkdir -p $(dir ${LIB_TARGET_DBG}))
endif

ifneq ($(strip ${SHARDLIB_TARGET}),)
$(shell mkdir -p $(dir ${SHARDLIB_TARGET}))
endif

ifneq ($(strip ${SHARDLIB_TARGET_DBG}),)
$(shell mkdir -p $(dir ${SHARDLIB_TARGET_DBG}))
endif

ifneq ($(strip ${APP_TARGET}),)
$(shell mkdir -p $(dir ${APP_TARGET}))
endif

ifneq ($(strip ${APP_TARGET_DBG}),)
$(shell mkdir -p $(dir ${APP_TARGET_DBG}))
endif

ifneq ($(strip ${INSTALL_DIR_LIB}),)
$(shell mkdir -p ${INSTALL_DIR_LIB})
endif

ifneq ($(strip ${INSTALL_DIR_LIBDBG}),)
$(shell mkdir -p ${INSTALL_DIR_LIBDBG})
endif

ifneq ($(strip ${INSTALL_DIR_HEADER}),)
$(shell mkdir -p ${INSTALL_DIR_HEADER})
endif

ifneq ($(strip ${INSTALL_DIR_SHARELIB}),)
$(shell mkdir -p ${INSTALL_DIR_SHARELIB})
endif

ifneq ($(strip ${INSTALL_DIR_SHARELIBDBG}),)
$(shell mkdir -p ${INSTALL_DIR_SHARELIBDBG})
endif

ifneq ($(strip ${INSTALL_DIR_APP}),)
$(shell mkdir -p ${INSTALL_DIR_APP})
endif


ifneq ($(strip ${INSTALL_DIR_APPDBG}),)
$(shell mkdir -p ${INSTALL_DIR_APPDBG})
endif
