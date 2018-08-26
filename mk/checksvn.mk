#
#  Copyright (c)1998-2012, Chongqing Xunmei Technology
#  All Rights Reserved.
#
#	 Description:
#  $Id: checksvn.mk 3966 2015-03-05 09:08:34Z wangkewei $
#

#°æ±¾
ifneq (${TPL_VER_FILE}, )
ifeq (${TPL_VER_FILE}, $(wildcard ${TPL_VER_FILE}))
R_MAJOR = $(shell grep r_major ${TPL_VER_FILE} | sed -n 's/;//pg' |sed -n 's/^static const int r_major = //p')
R_MINOR = $(shell grep r_minor ${TPL_VER_FILE} | sed -n 's/;//pg' |sed -n 's/^static const int r_minor = //p')
R_REVISION = $(shell grep r_revision ${TPL_VER_FILE} | sed -n 's/;//pg' |sed -n 's/^static const int r_revision = //p')
endif
endif

ifeq ($(strip ${CHECKSVNVER}), YES)	
srcsvnTmp=$(shell svnversion ${SVNVER_PATH})
inclsvnTmp=$(shell svnversion ${SVNVER_PATH_INCL})

srcsvn = $(subst M,,${srcsvnTmp})
inclsvn = $(subst M,,${inclsvnTmp})

srcsvntmp=$(shell svnversion ${SVNVER_PATH}|sed "s/M//")
inclsvntmp=$(shell svnversion ${SVNVER_PATH_INCL} |sed "s/M//")
ifneq (${srcsvntmp}, ${inclsvntmp})
$(error Svn Version NotMatch)
endif
endif