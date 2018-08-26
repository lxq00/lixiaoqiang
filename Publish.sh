#!/bin/sh

function checkMountSharedDirFlag()
{
	if [ ! -d "$1" ]; then
		mkdir -p $1
	fi

	if [ ! -f "$2" ]; then
		/bin/mount -t cifs -o noatime -o nodiratime -o username="cd.xunmei.com/lixiaoqiang",password='123abc!!',rw,uid=0,dir_mode=0777,file_mode=0777 $3 $1
	fi
}




checkMountSharedDirFlag /tmp/.vgsii_shared_pend /tmp/.vgsii_shared_pend/_depend_flag	//192.168.0.10/vgsii_depends
checkMountSharedDirFlag /tmp/.vgsii_shared_buildlog /tmp/.vgsii_shared_buildlog/_depend_flag //192.168.0.10/个人目录/l李小强/_buildlogs

cd `dirname %0`

mkdir -p mk/platform 2>/dev/null
cp /tmp/.vgsii_shared_pend/__VGSII__GIT_Tool/vgsii_pkt_tools_2.0/mk/* mk/ 2>/dev/null
cp /tmp/.vgsii_shared_pend/__VGSII__GIT_Tool/vgsii_pkt_tools_2.0/mk/platform/* mk/platform/ 2>/dev/null

node /tmp/.vgsii_shared_pend/__VGSII__GIT_Tool/vgsii_pkt_tools_2.0/mk/pkt/pkt_v3.js $1 $2 $3 $4