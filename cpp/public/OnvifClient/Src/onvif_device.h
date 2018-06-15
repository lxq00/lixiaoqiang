/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2010-2014, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#ifndef _ONVIF_DEVICE_H_
#define _ONVIF_DEVICE_H_

#ifdef WIN32
#define __WIN32_OS__	1
#define __VXWORKS_OS__	0
#define __LINUX_OS__	0
#else
#define __WIN32_OS__	0
#define __VXWORKS_OS__	0
#define __LINUX_OS__	1
#endif
//#include "sys_inc.h"
/***************************************************************************************/
typedef int 			int32;
typedef unsigned int 	uint32;
typedef unsigned short 	uint16;
typedef unsigned char 	uint8;

typedef unsigned int	__u32;
typedef unsigned short	__u16;
typedef int				__s32;
typedef unsigned char 	__u8;

/***************************************************************************************/
#ifdef WIN32
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif
//#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#ifdef WIN32
#include <process.h>    /* _beginthread, _endthread */
#include <iphlpapi.h>
//#include <assert.h>
using namespace std;
#include "Winsock2.h"
//#include "ws2ipdef.h"

#define usleep(x) 		Sleep(x/1000)

#ifndef strcasecmp
#define strcasecmp 		stricmp
#endif // !strcasecmp

#ifndef snprintf
#define snprintf 		_snprintf
#endif

#ifndef strncasecmp
#define strncasecmp 	_memicmp
#endif

#define pthread_t 		DWORD

#define pthread_t 		DWORD
#define usleep(x) 		Sleep(x/1000)
#else
#define BOOL int
#endif

#define XMALLOC(x) 	xmalloc(x, __FILE__, __LINE__)
#define XFREE(x) 	xfree(x, __FILE__, __LINE__)


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// device type
#define ODT_UNKNOWN     0
#define ODT_NVT         1
#define ODT_NVD         2
#define ODT_NVS         3
#define ODT_NVA         4

// req cap
#define CAP_ALL         0
#define CAP_MEDIA       1
#define CAP_DEVICE      2
#define CAP_ANALYTICS   3
#define CAP_EVENTS      4
#define CAP_IMAGING     5
#define CAP_PTZ         6

#define FLAG_MANUAL		(1 << 0)

typedef struct
{
    int     port;           // onvif port
    char    host[24];       // ip of xaddrs
    char    url[128];       // /onvif/device_service
} onvif_xaddr;

typedef struct
{
    unsigned int    ip;             // msg recv from
    int             type;           // device type
    onvif_xaddr     xaddr;
} device_binfo;

typedef struct
{
    char Manufacturer[32];
    char Model[32];
    char FirmwareVersion[32];
    char SerialNumber[64];
    char HardwareId[32];
	char Name[128];
} device_info;

typedef struct
{
    onvif_xaddr xaddr;
    BOOL RTPMulticast;
    BOOL RTP_TCP;
    BOOL RTP_RTSP_TCP;
    BOOL support;
} device_media_cap;

typedef struct
{
    onvif_xaddr xaddr;
    BOOL        support;
} device_ptz_cap;

typedef struct
{
	onvif_xaddr xaddr;
	BOOL        support;
} device_event_cap;

typedef struct
{
	onvif_xaddr xaddr;
	BOOL        support;
} device_Message_cap;

typedef struct
{
    device_media_cap    media;
    device_ptz_cap      ptz;
	device_event_cap	events;
	device_Message_cap  message;
} onvif_dev_cap;


#define TOKEN_LEN   100
#define NAME_LEN    100
#define URI_LEN     512

typedef struct
{
    // Bounds   
    int  width;
    int  height;
    int  x;
    int  y;
    
    int  use_count;
    char token[TOKEN_LEN];
    char stream_name[NAME_LEN];
    char source_token[TOKEN_LEN]; 
} video_source;


#define VIDEO_ENCODING_JPEG     0 
#define VIDEO_ENCODING_MPEG4    1
#define VIDEO_ENCODING_H264     2
#define VIDEO_ENCODING_UNKNOWN  3


#define H264_PROFILE_Baseline   0
#define H264_PROFILE_Main       1 
#define H264_PROFILE_Extended   2 
#define H264_PROFILE_High       3


typedef struct
{
    float quality;
    int  width;
    int  height;
    int  use_count;
    int  session_timeout;
    
    char name[NAME_LEN];
    char token[TOKEN_LEN];
    
    int  encoding;
    
    int  framerate_limit;
	int  encoding_interval;
	int  bitrate_limit;
    
    /* H264Configuration */
	int  gov_len;
	int  h264_profile;
} video_encoder;

typedef struct
{
    int pan_tilt_x;
    int pan_tilt_y;
    int zoom;
} ptz_speed;

typedef struct
{
	float min;
	float max;
} range;

typedef struct
{
	int used;
	
	range absolute_pantilt_x;
	range absolute_pantilt_y;
	range absolute_zoom;
	
	range relative_pantilt_x;
	range relative_pantilt_y;
	range relative_zoom;

	range continuous_pantilt_x;
	range continuous_pantilt_y;
	range continuous_zoom;

	range pantilt_speed;
	range zoom_speed;
	range timeout;
} ptz_cfg_opt;

typedef struct
{
    int  use_count;
    int  def_timeout;
    
    char name[NAME_LEN];
    char token[TOKEN_LEN];
    char nodeToken[TOKEN_LEN];

    ptz_speed def_speed;

	range pantilt_x;
	range pantilt_y;
	range zoom;

	ptz_cfg_opt	opt;
} ptz_cfg;

typedef struct tag_onvif_profile
{
    struct tag_onvif_profile *next;
    
    video_source  *p_video_source;
    video_encoder *p_video_encoder;
    ptz_cfg       *p_ptz_cfg;

    char name[NAME_LEN];
    char token[TOKEN_LEN];
    char stream_uri[URI_LEN];
	char snap_uri[URI_LEN];
	char eventSubscription[URI_LEN];
    bool fixed;
    
} onvif_profile;


typedef struct
{
	int		width;
	int		height;
	int		fps;
	int		bitrate;
	char	name[32];
	char	token[32];
} video_encoder_cfg;

#define PTZ_CTRL_PAN    0
#define PTZ_CTRL_ZOOM   1

typedef struct
{
    int             ctrlType;
    double           panTiltX;
    double           panTiltY;
    float           zoom;
	int				duration;
} ptz_ctrl;

typedef struct
{
	int				flags;
	int				state;
	int				no_res_nums;
	
    device_binfo    binfo;
    device_info     devInfo;

    onvif_dev_cap   devCap;

    onvif_profile * profile;
	ptz_cfg			ptzcfg;
	

	// request
	char 			username[32];
	char 			password[32];
	
	ptz_ctrl        ptzcmd;        
    int             reqCap;

	char			lastMessage[64];
	bool			bMessageUpdate;
    
    
    onvif_profile * curProfile;
    
} onvif_device;

typedef struct PSN	//ppstack_node
{
	unsigned long		prev_node;
	unsigned long		next_node;
	unsigned long		node_flag;	//0:idle£»1:in FreeList 2:in UsedList 
}PPSN;

struct PPSN_CTX
{
	unsigned long		fl_base;	
	unsigned int		head_node;	
	unsigned int		tail_node;
	unsigned int		node_num;
	unsigned int		low_offset;
	unsigned int		high_offset;
	unsigned int		unit_size;
	void	*			ctx_mutex;
	unsigned int		pop_cnt;
	unsigned int		push_cnt;
};

/***************************************************************************************/
#define MAX_AVN				16	
#define MAX_AVDESCLEN		280	
#define MAX_USRL			64	
#define MAX_PWDL			32	
#define MAX_NUML			64
#define MAX_UA_ALT_NUM		8

/***************************************************************************************/
typedef struct header_value
{
	char	header[32];
	char *	value_string;
} HDRV;

typedef struct ua_address_info
{
	unsigned int	ip;
	unsigned short	port;
	unsigned short	type;	// UDP - 0; TCP-PASSIVE - 1; TCP-ACTIVE - 2
	char			user[64];
	char			passwd[64];
} UA_ADDRINFO;

typedef struct ua_media_session_info
{
	int				ua_m_fd;				

	unsigned short	remote_a_port;			
	unsigned short	remote_r_port;			
	unsigned short	local_port;				

	unsigned int	rx_pkt_cnt;				
	unsigned int	tx_pkt_cnt;

	time_t			last_pkt_time;

	unsigned int	internet_local_ip;
	unsigned short	internet_local_port;

	UA_ADDRINFO		remote[MAX_UA_ALT_NUM];
	unsigned int	rip;					
	unsigned short	rport;					
	unsigned int	probe_try_cnt;			

	time_t			v3_echo_rx_time;		
	time_t			v3_echo_tx_time;		
} UA_MEDIA;

typedef struct ua_rtp_info
{
	char *			wav_buf;				
	int				rtp_offset;				
	int				rtp_len;				
	int				rtp_cnt;				
	int				cur_rtp_cnt;			
	unsigned int	rtp_ssrc;				
	unsigned int	rtp_ts;					
	unsigned char	rtp_pt;					
} UA_RTP_INFO;

typedef struct http_digest_auth_info
{
	char			auth_name[MAX_USRL];
	char			auth_pwd[32];
	char			auth_uri[256];			
	char			auth_qop[32];
	char			auth_nonce[64];
	char			auth_cnonce[128];
	char			auth_realm[128];
	int				auth_nc;
	char			auth_ncstr[12];
	char			auth_response[36];
} HD_AUTH_INFO;

pthread_t sys_os_create_thread(void * thread_func, void * argv);
char * sys_os_get_socket_error();
void * sys_os_create_mutex();
void initWinSock();
onvif_device * addDevice(onvif_device * pdevice);
/*extern "C" __declspec(dllexport) */void onvif_initSdk();
/*extern "C" __declspec(dllexport) */void onvif_uninitSdk();
typedef void (* onvif_probe_cb)(device_binfo * p_res, void * pdata);
//extern "C"
//{
	/*__declspec(dllexport)*/ BOOL onvif_GetDeviceInformation(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_GetCapabilities(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_GetScopes(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_GetProfiles(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_GetStreamUri(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_GetSnapUri(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_GetNetworkInterfaces(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_GetVideoEncoderConfigurations(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_ContinuousMove(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_AbsoluteMove(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_Stop(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_GetConfigurations(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_GetNode(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_GetConfigurationOptions(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_GetSystemDateAndTime(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_SetSystemDateAndTime(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_SystemReboot(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_get_dev_info(onvif_device * p_dev);
	/*__declspec(dllexport)*/ void onvif_free_profiles(onvif_device * p_dev);
	/*__declspec(dllexport)*/ void set_probe_cb(onvif_probe_cb cb, void * pdata);
	/*__declspec(dllexport)*/ int start_probe(int interval);
	/*__declspec(dllexport)*/ void stop_probe();
	/*__declspec(dllexport)*/ BOOL onvif_StartRecvAlarm(onvif_device * p_dev);
	/*__declspec(dllexport)*/ BOOL onvif_GetAlarm(onvif_device * p_dev);
//};

/***************************************************************************************/
void 	  * _pps_node_get_data(PPSN * p_node);
PPSN 	  * _pps_data_get_node(void * p_data);

PPSN_CTX  * pps_ctx_fl_init(unsigned long node_num,unsigned long content_size,BOOL bNeedMutex);
PPSN_CTX  * pps_ctx_fl_init_assign(unsigned long mem_addr, unsigned long mem_len, unsigned long node_num, unsigned long content_size, BOOL bNeedMutex);

void 		pps_fl_free(PPSN_CTX * fl_ctx);
void 		pps_fl_reinit(PPSN_CTX * fl_ctx);

void 		pps_ctx_show(PPSN_CTX * pps_ctx);
BOOL 		ppstack_fl_push(PPSN_CTX * pps_ctx,void * content_ptr);
BOOL 		ppstack_fl_push_tail(PPSN_CTX * pps_ctx,void * content_ptr);
void 	  * ppstack_fl_pop(PPSN_CTX * pps_ctx);

BOOL		pps_ctx_ul_init(PPSN_CTX * fl_ctx, BOOL bNeedMutex, PPSN_CTX*& ul_ctx);
BOOL 		pps_ctx_ul_init_assign(PPSN_CTX * ul_ctx, PPSN_CTX * fl_ctx,BOOL bNeedMutex);
BOOL 		pps_ctx_ul_init_nm(PPSN_CTX * fl_ctx,PPSN_CTX * ul_ctx);

void 		pps_ul_reinit(PPSN_CTX * ul_ctx);
void 		pps_ul_free(PPSN_CTX * ul_ctx);

BOOL 		pps_ctx_ul_del(PPSN_CTX * ul_ctx,void * content_ptr);

PPSN 	  * pps_ctx_ul_del_node_unlock(PPSN_CTX * ul_ctx,PPSN * p_node);
void 	  * pps_ctx_ul_del_unlock(PPSN_CTX * ul_ctx,void * content_ptr);

BOOL 		pps_ctx_ul_add(PPSN_CTX * ul_ctx,void * content_ptr);
BOOL 		pps_ctx_ul_add_head(PPSN_CTX * ul_ctx,void * content_ptr);

unsigned long pps_get_index(PPSN_CTX * pps_ctx,void * content_ptr);
void 	  * pps_get_node_by_index(PPSN_CTX * pps_ctx,unsigned long index);

PPSN 	  * _pps_node_head_start(PPSN_CTX * pps_ctx);
PPSN 	  * _pps_node_tail_start(PPSN_CTX * pps_ctx);
PPSN 	  * _pps_node_next(PPSN_CTX * pps_ctx, PPSN * p_node);
PPSN 	  * _pps_node_prev(PPSN_CTX * pps_ctx, PPSN * p_node);
void 		_pps_node_end(PPSN_CTX * pps_ctx);

/***************************************************************************************/
void 	  * pps_lookup_start(PPSN_CTX * pps_ctx);
void 	  * pps_lookup_next(PPSN_CTX * pps_ctx, void * ct_ptr);
void		pps_lookup_end(PPSN_CTX * pps_ctx);

void 	  * pps_lookback_start(PPSN_CTX * pps_ctx);
void 	  * pps_lookback_next(PPSN_CTX * pps_ctx, void * ct_ptr);
void 		pps_lookback_end(PPSN_CTX * pps_ctx);

void 		pps_wait_mutex(PPSN_CTX * pps_ctx);
void 		pps_post_mutex(PPSN_CTX * pps_ctx);

BOOL 		pps_safe_node(PPSN_CTX * pps_ctx,void * content_ptr);
BOOL 		pps_idle_node(PPSN_CTX * pps_ctx,void * content_ptr);
BOOL 		pps_exist_node(PPSN_CTX * pps_ctx,void * content_ptr);
BOOL 		pps_used_node(PPSN_CTX * pps_ctx,void * content_ptr);

/***************************************************************************************/
int 		pps_node_count(PPSN_CTX * pps_ctx);
void 	  * pps_get_head_node(PPSN_CTX * pps_ctx);
void 	  * pps_get_tail_node(PPSN_CTX * pps_ctx);
void 	  * pps_get_next_node(PPSN_CTX * pps_ctx, void * content_ptr);
void	  * pps_get_prev_node(PPSN_CTX * pps_ctx, void * content_ptr);


/***********************************************************************/
BOOL 		 net_buf_fl_init();
void 		 net_buf_fl_deinit();

char 	   * get_idle_net_buf();
void 		 free_net_buf(char * rbuf);
unsigned int idle_net_buf_num();

/***********************************************************************/
BOOL 		 hdrv_buf_fl_init(int num);
void 		 hdrv_buf_fl_deinit();

HDRV       * get_hdrv_buf();
void         free_hdrv_buf(HDRV * pHdrv);
unsigned int idle_hdrv_buf_num();

void         init_ul_hdrv_ctx(PPSN_CTX * ul_ctx);

void         free_ctx_hdrv(PPSN_CTX * p_ctx);

/***********************************************************************/
BOOL 		 sys_buf_init();
void         sys_buf_deinit();

#endif


