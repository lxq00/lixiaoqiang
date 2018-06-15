#include "OnvifClient/onvif_device.h"
#include <stdint.h>
#include "bm/sha1.h"
#include "bm/strCoding.h"
#ifndef WIN32
#include <stdarg.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netdb.h>
#define closesocket close
#define SOCKET int
#define SOCKET_ERROR -1
#else
#define socklen_t int
#endif
#include "Network/Guid.h"
using namespace Xunmei::Network;
PPSN_CTX * m_dev_ul = NULL;
PPSN_CTX * m_dev_fl = NULL;
PPSN_CTX * hdrv_buf_fl = NULL;

typedef enum http_request_msg_type
{
	HTTP_MT_NULL = 0,
	HTTP_MT_GET,
	HTTP_MT_HEAD,
	HTTP_MT_MPOST,
	HTTP_MT_MSEARCH,
	HTTP_MT_NOTIFY,
	HTTP_MT_POST,
	HTTP_MT_SUBSCRIBE,
	HTTP_MT_UNSUBSCRIBE,
}HTTP_MT;

/*************************************************************************/
typedef enum http_content_type
{
	CTT_NULL = 0,
	CTT_SDP,
	CTT_TXT,
	CTT_HTM,
	CTT_XML
}HTTPCTT;

typedef struct _http_msg_content
{
	unsigned int	msg_type;
	unsigned int	msg_sub_type;
	HDRV 			first_line;

	PPSN_CTX		hdr_ctx;
	PPSN_CTX		ctt_ctx;

	int				hdr_len;
	int				ctt_len;

	HTTPCTT			ctt_type;

	char *			msg_buf;
	int				buf_offset;

	unsigned long	remote_ip;
	unsigned short	remote_port;
	unsigned short	local_port;
}HTTPMSG;

/*************************************************************************/
typedef struct http_client
{
	int				cfd;
	unsigned int	rip;
	unsigned int	rport;

	unsigned int	guid;

	char			rcv_buf[2048];
	char *			dyn_recv_buf;
	int				rcv_dlen;
	int				hdr_len;
	int				ctt_len;
	char *			p_rbuf;				// --> rcv_buf or dyn_recv_buf
	int				mlen;				// = sizeof(rcv_buf) or size of dyn_recv_buf
}HTTPCLN;

typedef struct http_req
{

	int				cfd;
	onvif_device *  p_dev;

	unsigned int	port;
	char			host[256];
	char			url[256];

	char			rcv_buf[2048];
	char *			dyn_recv_buf;
	int				rcv_dlen;
	int				hdr_len;
	int				ctt_len;
	char *			p_rbuf;				// --> rcv_buf or dyn_recv_buf
	int				mlen;				// = sizeof(rcv_buf) or size of dyn_recv_buf

	HTTPMSG *		rx_msg;
}HTTPREQ;

/*************************************************************************/
typedef enum onvif_action_e
{
	eActionNull = 0,
	eGetDeviceInformation,
	eGetCapabilities,
	eGetProfiles,
	eGetStreamUri,
	eGetNetworkInterfaces,
	eGetVideoEncoderConfigurations,	
	eGetPresets,
	eSetPreset,
	eGotoPreset,
	eGotoHomePosition,
	eSetHomePosition,
	eContinuousMove,
	eStop,
	eGetConfigurations,
	eGetConfigurationOptions,
	eGetSystemDateAndTime,
	eSetSystemDateAndTime,
	eSystemReboot,
	eStartRecvAlarm,
	eRecvAlarm,
	eStopRecvAlarm,
	eGetSnapUri,
	eGetScopes,
	eGetNode,
	eAbsoluteMove,
	eActionMax,
}eOnvifAction;

typedef struct onvif_action_s
{
	eOnvifAction	type;
	char			action_url[256];
}OVFACTS;

typedef struct request_message_type_value
{
	HTTP_MT	msg_type;
	char	msg_str[32];
	int		msg_len;
}REQMTV;

static const REQMTV req_mtvs[]={
	{HTTP_MT_GET,			"GET",			3},
	{HTTP_MT_HEAD,			"HEAD",			4},
	{HTTP_MT_MPOST,			"M-POST",		6},
	{HTTP_MT_MSEARCH,		"M-SEARCH",		8},
	{HTTP_MT_NOTIFY,		"NOTIFY",		6},
	{HTTP_MT_POST,			"POST",			4},
	{HTTP_MT_SUBSCRIBE,		"SUBSCRIBE",	9},
	{HTTP_MT_UNSUBSCRIBE,	"UNSUBSCRIBE",	11}
};

/************************************************************************************/
typedef struct LINKED_NODE
{
	struct LINKED_NODE * p_next;
	struct LINKED_NODE * p_previous;
	void* p_data;
} LINKED_NODE;

/************************************************************************************/
typedef struct LINKED_LIST
{
	LINKED_NODE *		p_first_node;
	LINKED_NODE *		p_last_node;
	void	*			list_semMutex;
} LINKED_LIST;

#define LTXML_MAX_STACK_DEPTH	1024
#define LTXML_MAX_ATTR_NUM	128

typedef struct ltxd_xmlparser 
{
	char *	xmlstart;
	char *	xmlend;
	char *	ptr;		// pointer to current character
	int		xmlsize;

	char *	e_stack[LTXML_MAX_STACK_DEPTH];
	int		e_stack_index;					

	const char *	attr[LTXML_MAX_ATTR_NUM];

	void *	userdata;
	void (*startElement)(void * userdata, const char * name, const char ** attr);
	void (*endElement)(void * userdata, const char * name);
	void (*charData)(void * userdata, const char * str, int len);
}LTXMLPRS;

/************************************************************************/
#define NTYPE_TAG		0
#define NTYPE_ATTRIB	1
#define NTYPE_CDATA		2

#define NTYPE_LAST		2
#define NTYPE_UNDEF		-1

typedef struct XMLN
{
	const char *	name;
	unsigned int	type;
	const char *	data;
	int				dlen;
	int				finish;
	struct XMLN *	parent;
	struct XMLN *	f_child;
	struct XMLN *	l_child;
	struct XMLN *	prev;
	struct XMLN *	next;
	struct XMLN *	f_attrib;
	struct XMLN *	l_attrib;
}XMLN;

/***********************************************************/
#define	HQ_PUT_WAIT		0x00000001
#define	HQ_GET_WAIT		0x00000002
#define	HQ_NO_EVENT		0x00000004

/***********************************************************/
typedef struct h_queue
{
	unsigned int	queue_mode;
	unsigned int	unit_num;
	unsigned int	unit_size;
	unsigned int	front;
	unsigned int	rear;
	unsigned int	queue_buffer;
	unsigned int	count_put_full;

	void *			queue_putMutex;	
	void *			queue_nnulEvent;
	void *			queue_nfulEvent;
}HQUEUE;

typedef enum word_type
{
	WORD_TYPE_NULL = 0,
	WORD_TYPE_STRING,
	WORD_TYPE_NUM,
	WORD_TYPE_SEPARATOR
}WORD_TYPE;

/***********************************************************/
HQUEUE * hqCreate(unsigned int unit_num,unsigned int unit_size,unsigned int queue_mode);
void 	 hqDelete(HQUEUE * phq);
BOOL 	 hqBufPut(HQUEUE * phq,char * buf);
BOOL 	 hqBufGet(HQUEUE * phq,char * buf);
BOOL 	 hqBufIsEmpty(HQUEUE * phq);
char   * hqBufGetWait(HQUEUE * phq);
void 	 hqBufGetWaitPost(HQUEUE * phq);
BOOL 	 hqBufPeek(HQUEUE * phq,char * buf);
void xml_node_del(XMLN * p_node);
int hxml_parse(LTXMLPRS * parse);
int  dq_string_get(char *word_buf,unsigned int word_buf_len, char *input_buf,unsigned int *offset);
BOOL is_char(char ch);
BOOL is_num(char ch);
BOOL is_separator(char ch);
BOOL is_ip_address(const char * address);
BOOL is_integer(char * p_str);
BOOL GetLineText(char *buf,int cur_line_offset,int max_len, int * len,int * next_line_offset);
BOOL GetSipLine(char *p_buf, int max_len, int * len, BOOL * bHaveNextLine);
BOOL GetLineWord(char *line, int cur_word_offset, int line_max_len, char *word_buf, int buf_len, int *next_word_offset, WORD_TYPE w_t);
BOOL GetNameValuePair(char * text_buf, int text_len, const char * name, char *value, int value_len);

/*************************************************************************/
int 			get_if_nums();
unsigned int 	get_if_ip(int index);
unsigned int 	get_route_if_ip(unsigned int dst_ip);
unsigned int 	get_default_if_ip();
int 			get_default_if_mac(unsigned char * mac);
unsigned int 	get_address_by_name(char *host_name);
/*************************************************************************/
char          * lowercase(char *str);
char          * uppercase(char *str);
int 			unicode(char **dst, char *src);

char          * printmem(char *src, size_t len, int bitwidth);
char 		  * scanmem(char *src, int bitwidth);
/*************************************************************************/
int 			base64_encode(unsigned char *source, unsigned int sourcelen, char *target, unsigned int targetlen);
void 			str_b64decode(char* str);
int 			base64_decode(const char *source, unsigned char *target, unsigned int targetlen);
/*************************************************************************/
time_t 			get_time_by_string(char * p_time_str);
int 			tcp_connect_timeout(unsigned int rip, int port, int timeout);

void _base64_encode_triple(unsigned char triple[3], char result[4]);
int base64_encode(unsigned char *source, unsigned int sourcelen, char *target, unsigned int targetlen);
void str_b64decode(char* str);
int base64_decode(const char *source, unsigned char *target, unsigned int targetlen) ;

void * xmalloc(size_t size, const char * pFileName, int line)
{
	void * ptr = malloc(size);
	// mem_//log_print("+++0X%p, %u, file[%s],line[%d]\r\n", ptr, size, pFileName, line);
	return ptr;
}

void xfree(void * ptr, const char * pFileName, int line)
{
	// mem_//log_print("---0X%p, file[%s],line[%d]\r\n", ptr, pFileName, line);
	free(ptr);
}

BOOL is_http_msg(char * msg_buf)
{
	unsigned int i;
	for(i=0; i<sizeof(req_mtvs)/sizeof(REQMTV);i++)
	{
		if(memcmp(msg_buf, req_mtvs[i].msg_str, req_mtvs[i].msg_len) == 0)
		{
			return TRUE;
		}
	}

	if(memcmp(msg_buf,"HTTP/1.1",strlen("HTTP/1.1")) == 0 || memcmp(msg_buf,"HTTP/1.0",strlen("HTTP/1.0")) == 0)
		return TRUE;

	return FALSE;
}

void http_headl_parse(char * pline, int llen, HTTPMSG * p_msg)
{
	char	word_buf[256];
	int		word_len;
	int		next_word_offset;
	BOOL	bHaveNextWord;

	bHaveNextWord = GetLineWord(pline,0,llen,word_buf,
		sizeof(word_buf),&next_word_offset,WORD_TYPE_STRING);
	word_len = strlen(word_buf);
	if(word_len > 0 && word_len < 31)
	{
		memcpy(p_msg->first_line.header,pline,word_len);
		p_msg->first_line.header[word_len] = '\0';

		while(pline[next_word_offset] == ' ')next_word_offset++;

		p_msg->first_line.value_string = pline+next_word_offset;

		//		if(strcasecmp(word_buf,"HTTP/1.1") == 0 || strcasecmp(word_buf,"HTTP/1.0") == 0)
		if(strcmp(word_buf,"HTTP/1.1") == 0 || strcmp(word_buf,"HTTP/1.0") == 0)
		{
			if(bHaveNextWord)
			{
				word_len = sizeof(word_buf);
				bHaveNextWord = GetLineWord(pline,next_word_offset,llen,
					word_buf,sizeof(word_buf),&next_word_offset,WORD_TYPE_NUM);
				word_len = strlen(word_buf);
				if(word_len > 0)
				{
					p_msg->msg_type = 1;
					p_msg->msg_sub_type = atoi(word_buf);
				}
			}
		}
		else
		{
			p_msg->msg_type = 0;
			unsigned int i;
			for(i=0; i<sizeof(req_mtvs)/sizeof(REQMTV);i++)
			{
				//				if(strcasecmp(word_buf,(char *)(req_mtvs[i].msg_str)) == 0)
				if(strcmp(word_buf,(char *)(req_mtvs[i].msg_str)) == 0)
				{
					p_msg->msg_sub_type = req_mtvs[i].msg_type;
					break;
				}
			}
		}
	}
}

int http_line_parse(char * p_buf, int max_len, char sep_char, PPSN_CTX * p_ctx)
{
	char word_buf[256];
	BOOL bHaveNextLine = TRUE;
	int line_len = 0;
	int parse_len = 0;

	char * ptr = p_buf;

	do{
		if(GetSipLine(ptr, max_len, &line_len, &bHaveNextLine) == FALSE)
		{
			//log_print("http_line_parse::get sip line error!!!\r\n");
			return -1;
		}

		if(line_len == 2)
		{
			return(parse_len + 2);
		}

		int	next_word_offset = 0;
		GetLineWord(ptr,0,line_len-2,word_buf,sizeof(word_buf),&next_word_offset,WORD_TYPE_STRING);
		char nchar = *(ptr + next_word_offset);
		if(nchar != sep_char) // SIP is ':',SDP is '='
		{
			//log_print("http_line_parse::format error!!!\r\n");
			return -1;
		}

		next_word_offset++;
		while(ptr[next_word_offset] == ' ') next_word_offset++;

		HDRV * pHdrV = get_hdrv_buf();
		if(pHdrV == NULL)
		{
			//log_print("http_line_parse::get_hdrv_buf return NULL!!!\r\n");
			return -1;
		}

		strncpy(pHdrV->header,word_buf,32);
		pHdrV->value_string = ptr+next_word_offset;
		pps_ctx_ul_add(p_ctx, pHdrV);

		ptr += line_len;
		max_len -= line_len;
		parse_len += line_len;

	}while(bHaveNextLine);

	return parse_len;
}

int http_ctt_parse(HTTPMSG * p_msg)
{
	int flag = 0;
	HTTPCTT w_ctx_type;

	HDRV * pHdrV = (HDRV *)pps_lookup_start(&(p_msg->hdr_ctx));
	while (pHdrV != NULL)
	{		
		// 		if(strcasecmp(pHdrV->header,"Content-Length") == 0)
		// 		{
		// 			p_msg->ctt_len = atol(pHdrV->value_string);
		// 			flag++;
		// 		}
		// 		else if(strcasecmp(pHdrV->header,"Content-Type") == 0)
		// 		{
		// 			char type_word[64];
		// 			int  next_tmp;
		// 			GetLineWord(pHdrV->value_string,0,strlen(pHdrV->value_string),type_word,sizeof(type_word),&next_tmp,WORD_TYPE_STRING);
		// 
		// 			if(strcasecmp(type_word,"application/sdp") == 0)
		// 				w_ctx_type = CTT_SDP;
		// 			else if(strcasecmp(type_word,"application/soap+xml") == 0)
		// 				w_ctx_type = CTT_XML;
		// 			else if(strcasecmp(type_word,"text/plain") == 0)
		// 				w_ctx_type = CTT_TXT;
		// 			else if(strcasecmp(type_word,"text/html") == 0)
		// 				w_ctx_type = CTT_HTM;
		// 			else
		// 				w_ctx_type = CTT_NULL;
		// 
		// 			p_msg->ctt_type = w_ctx_type;
		// 			flag++;
		// 		}
		if(strcmp(pHdrV->header,"Content-Length") == 0)
		{
			p_msg->ctt_len = atol(pHdrV->value_string);
			flag++;
		}
		else if(strcmp(pHdrV->header,"Content-Type") == 0)
		{
			char type_word[64];
			int  next_tmp;
			GetLineWord(pHdrV->value_string,0,strlen(pHdrV->value_string),type_word,sizeof(type_word),&next_tmp,WORD_TYPE_STRING);

			if(strcmp(type_word,"application/sdp") == 0)
				w_ctx_type = CTT_SDP;
			else if(strcmp(type_word,"application/soap+xml") == 0)
				w_ctx_type = CTT_XML;
			else if(strcmp(type_word,"text/plain") == 0)
				w_ctx_type = CTT_TXT;
			else if(strcmp(type_word,"text/html") == 0)
				w_ctx_type = CTT_HTM;
			else
				w_ctx_type = CTT_NULL;

			p_msg->ctt_type = w_ctx_type;
			flag++;
		}
		pHdrV = (HDRV *)pps_lookup_next(&(p_msg->hdr_ctx),pHdrV);
	}
	pps_lookup_end(&(p_msg->hdr_ctx));

	if(p_msg->ctt_type && p_msg->ctt_len)
		return 1;

	return 0;
}

int http_msg_parse(char * msg_buf,int msg_buf_len,HTTPMSG * msg)
{
	BOOL bHaveNextLine;
	int line_len = 0;
	char * p_buf = msg_buf;

	msg->msg_type = (unsigned int)-1;

	if(GetSipLine(p_buf, msg_buf_len,&line_len,&bHaveNextLine) == FALSE)
		return -1;
	if(line_len > 0)
		http_headl_parse(p_buf, line_len-2, msg);
	if(msg->msg_type == (unsigned int)-1)
		return -1;

	p_buf += line_len;
	msg->hdr_len = http_line_parse(p_buf, msg_buf_len-line_len, ':', &(msg->hdr_ctx));
	if(msg->hdr_len <= 0)
		return -1;

	p_buf += msg->hdr_len;
	if(http_ctt_parse(msg) == 1 && msg->ctt_len > 0)
	{
		HDRV * pHdrV = get_hdrv_buf();
		if(pHdrV == NULL)
		{
			//log_print("sip_msg_parse::get_hdrv_buf return NULL!!!\r\n");
			return -1;
		}

		strcpy(pHdrV->header,"");
		pHdrV->value_string = p_buf;
		pps_ctx_ul_add(&(msg->ctt_ctx), pHdrV);

		int slen = strlen(p_buf);
		if(slen != msg->ctt_len)
		{
			//log_print("sip_msg_parse::text xml strlen[%d] != ctx len[%d]!!!\r\n", slen, msg->ctt_len);
		}
	}

	return (line_len + msg->hdr_len + msg->ctt_len);
}

int http_msg_parse_part1(char * p_buf,int buf_len,HTTPMSG * msg)
{
	BOOL bHaveNextLine;
	int line_len = 0;

	msg->msg_type = -1;

	if(GetSipLine(p_buf, buf_len,&line_len,&bHaveNextLine) == FALSE)
		return -1;
	if(line_len > 0)
		http_headl_parse(p_buf, line_len-2, msg);
	if(msg->msg_type == (unsigned int)-1)
		return -1;

	p_buf += line_len;
	msg->hdr_len = http_line_parse(p_buf, buf_len-line_len, ':', &(msg->hdr_ctx));
	if(msg->hdr_len <= 0)
		return -1;

	http_ctt_parse(msg);

	return (line_len + msg->hdr_len);
}

int http_msg_parse_part2(char * p_buf,int buf_len,HTTPMSG * msg)
{
	HDRV * pHdrV = get_hdrv_buf();
	if(pHdrV == NULL)
	{
		//log_print("sip_msg_parse::get_hdrv_buf return NULL!!!\r\n");
		return -1;
	}

	strcpy(pHdrV->header,"");
	pHdrV->value_string = p_buf;
	pps_ctx_ul_add(&(msg->ctt_ctx), pHdrV);

	int slen = strlen(p_buf);
	if(slen != msg->ctt_len)
	{
		//log_print("http_msg_parse_part2::text xml strlen[%d] != ctx len[%d]!!!\r\n", slen, msg->ctt_len);
	}

	return slen;
}

HDRV * find_http_headline(HTTPMSG * msg, const char * head)
{
	if(msg == NULL || head == NULL)
		return NULL;

	HDRV * line = (HDRV *)pps_lookup_start(&(msg->hdr_ctx));
	while (line != NULL)
	{		
		//		if(strcasecmp(line->header,head) == 0)
		if(strcmp(line->header,head) == 0)
		{
			pps_lookup_end(&(msg->hdr_ctx));
			return line;
		}

		line = (HDRV *)pps_lookup_next(&(msg->hdr_ctx),line);
	}
	pps_lookup_end(&(msg->hdr_ctx));

	return NULL;
}

char * get_http_headline(HTTPMSG * msg, const char * head)
{
	HDRV * p_hdrv = find_http_headline(msg, head);
	if(p_hdrv == NULL)
		return NULL;
	return p_hdrv->value_string;
}

void add_tx_http_line(HTTPMSG * tx_msg, const char * msg_hdr, const char * msg_fmt,...)
{
	va_list argptr;
	int slen;

	if(tx_msg == NULL || tx_msg->msg_buf == NULL)
		return;

	HDRV *pHdrV = get_hdrv_buf();
	if(pHdrV == NULL)
	{
		//log_print("add_tx_msg_line::get_hdrv_buf return NULL!!!\r\n");
		return;
	}

	pHdrV->value_string = tx_msg->msg_buf + tx_msg->buf_offset;

	strncpy(pHdrV->header,msg_hdr,31);

	va_start(argptr,msg_fmt);
#if	__LINUX_OS__
	slen = vsnprintf(pHdrV->value_string,1600-tx_msg->buf_offset,msg_fmt,argptr);
#else
	slen = vsprintf(pHdrV->value_string,msg_fmt,argptr);
#endif
	va_end(argptr);

	if(slen < 0)
	{
		//log_print("add_tx_msg_line::vsnprintf return %d !!!\r\n",slen);
		free_hdrv_buf(pHdrV);
		return;
	}

	pHdrV->value_string[slen] = '\0';
	tx_msg->buf_offset += slen + 1;

	pps_ctx_ul_add(&(tx_msg->hdr_ctx),pHdrV);
}

HDRV * find_ctt_headline(HTTPMSG * msg, const char * head)
{
	if(msg == NULL || head == NULL)
		return NULL;

	HDRV * line = (HDRV *)pps_lookup_start(&(msg->ctt_ctx));
	while (line != NULL)
	{
		//		if(strcasecmp(line->header,head) == 0)
		if(strcmp(line->header,head) == 0)
		{
			pps_lookup_end(&(msg->ctt_ctx));
			return line;
		}

		line = (HDRV *)pps_lookup_next(&(msg->ctt_ctx),line);
	}
	pps_lookup_end(&(msg->ctt_ctx));

	return NULL;
}

char * get_http_ctt(HTTPMSG * msg)
{
	if(msg == NULL)
		return NULL;

	HDRV * line = (HDRV *)pps_lookup_start(&(msg->ctt_ctx));
	pps_lookup_end(&(msg->ctt_ctx));
	if(line)
		return line->value_string;

	return NULL;
}
/***********************************************************************/
static PPSN_CTX * msg_buf_fl = NULL;

BOOL http_msg_buf_fl_init(int num)
{
	msg_buf_fl = pps_ctx_fl_init(num,sizeof(HTTPMSG),TRUE);
	if(msg_buf_fl == NULL)
		return FALSE;

	return TRUE;
}

void http_msg_ctx_init(HTTPMSG * msg)
{
	pps_ctx_ul_init_nm(hdrv_buf_fl,&(msg->hdr_ctx));
	pps_ctx_ul_init_nm(hdrv_buf_fl,&(msg->ctt_ctx));
}

void http_free_msg_buf(HTTPMSG * msg)
{
	ppstack_fl_push(msg_buf_fl,msg);
}

void http_msg_buf_fl_deinit()
{
	if(msg_buf_fl)
	{
		pps_fl_free(msg_buf_fl);
		msg_buf_fl = NULL;
	}
}

HTTPMSG * http_get_msg_buf()
{
	HTTPMSG * tx_msg = (HTTPMSG *)ppstack_fl_pop(msg_buf_fl);
	if(tx_msg == NULL)
	{
		//log_print("get_msg_buf pop null!!!\r\n");
		return NULL;
	}

	memset(tx_msg,0,sizeof(HTTPMSG));
	tx_msg->msg_buf = get_idle_net_buf();
	if(tx_msg->msg_buf == NULL)
	{
		http_free_msg_buf(tx_msg);
		return NULL;
	}

	http_msg_ctx_init(tx_msg);

	return tx_msg;
}

unsigned int http_idle_msg_buf_num()
{
	return msg_buf_fl->node_num;
}

HTTPMSG * http_get_msg_large_buf(int size)
{
	HTTPMSG * tx_msg = (HTTPMSG *)ppstack_fl_pop(msg_buf_fl);
	if(tx_msg == NULL)
	{
		//log_print("get_msg_large_buf pop null!!!\r\n");
		return NULL;
	}

	memset(tx_msg,0,sizeof(HTTPMSG));
	tx_msg->msg_buf = (char *)malloc(size);
	if(tx_msg->msg_buf == NULL)
	{
		http_free_msg_buf(tx_msg);
		return NULL;
	}

	http_msg_ctx_init(tx_msg);

	return tx_msg;
}

void free_http_msg_ctx(HTTPMSG * msg,int type)	//0:sip list; 1:sdp list;
{
	PPSN_CTX * p_free_ctx = NULL;

	switch(type)
	{
	case 0:
		p_free_ctx = &(msg->hdr_ctx);
		break;

	case 1:
		p_free_ctx = &(msg->ctt_ctx);
		break;
	}

	if(p_free_ctx == NULL)
		return;

	free_ctx_hdrv(p_free_ctx);
}

void free_http_msg_content(HTTPMSG * msg)
{
	if(msg == NULL)	return;

	free_http_msg_ctx(msg,0);
	free_http_msg_ctx(msg,1);

	free_net_buf(msg->msg_buf);
}

void free_http_msg(HTTPMSG * msg)
{
	if(msg == NULL)return;

	free_http_msg_content(msg);
	http_free_msg_buf(msg);
}

/***************************************************************************************/
void * sys_os_create_mutex()
{
	void * p_mutex = NULL;

#if	__WIN32_OS__

	p_mutex  = CreateMutex(NULL, FALSE, NULL);

#elif (__LINUX_OS__ || __VXWORKS_OS__)

	p_mutex = (sem_t *)XMALLOC(sizeof(sem_t));
	int ret = sem_init((sem_t *)p_mutex,0,1);
	if (ret != 0)
	{
		XFREE(p_mutex);
		return NULL;
	}

#endif

	return p_mutex;
}

void * sys_os_create_sig()
{
	void * p_sig = NULL;

#if	__WIN32_OS__

	p_sig = CreateEvent(NULL, FALSE, FALSE, NULL);

#elif (__LINUX_OS__ || __VXWORKS_OS__)

	p_sig = XMALLOC(sizeof(sem_t));
	int ret = sem_init((sem_t *)p_sig,0,0);
	if (ret != 0)
	{
		XFREE(p_sig);
		return NULL;
	}

#endif

	return p_sig;
}

void sys_os_destroy_sig_mutx(void * ptr)
{
	if (ptr == NULL)
		return;

#if	__WIN32_OS__

	CloseHandle(ptr);

#elif (__LINUX_OS__ || __VXWORKS_OS__)

	sem_destroy((sem_t *)ptr);
	XFREE(ptr);

#endif
}

int sys_os_mutex_enter(void * p_sem)
{
	if (p_sem == NULL)
		return -1;

#if	(__VXWORKS_OS__ || __LINUX_OS__)
	int ret = sem_wait((sem_t *)p_sem);
	if (ret != 0)
		return -1;

#elif __WIN32_OS__

	DWORD ret = WaitForSingleObject(p_sem,INFINITE);
	if (ret == WAIT_FAILED)
		return -1;

#endif

	return 0;
}

void sys_os_mutex_leave(void * p_sem)
{
	if (p_sem == NULL)
		return;

#if	(__VXWORKS_OS__ || __LINUX_OS__)

	sem_post((sem_t *)p_sem);

#elif __WIN32_OS__

	ReleaseMutex(p_sem);

#endif
}

int sys_os_sig_wait(void * p_sig)
{
	if (p_sig == NULL)
		return -1;

#if	(__VXWORKS_OS__ || __LINUX_OS__)

	int ret = sem_wait((sem_t *)p_sig);
	if (ret != 0)
		return -1;

#elif __WIN32_OS__

	DWORD ret = WaitForSingleObject(p_sig,INFINITE);
	if (ret == WAIT_FAILED)
		return -1;

#endif

	return 0;
}

int sys_os_sig_wait_timeout(void * p_sig, unsigned int ms)
{
	if (p_sig == NULL) return -1;

#if	(__VXWORKS_OS__ || __LINUX_OS__)

	struct timespec ts;
	struct timeval tt;
	gettimeofday(&tt,NULL);

	tt.tv_sec = tt.tv_sec + ms / 1000;
	tt.tv_usec = tt.tv_usec + (ms % 1000) * 1000;
	tt.tv_sec += tt.tv_usec / (1000 * 1000);
	tt.tv_usec = tt.tv_usec % (1000 * 1000);

	ts.tv_sec = tt.tv_sec;
	ts.tv_nsec = tt.tv_usec * 1000;

	int ret = sem_timedwait((sem_t *)p_sig, &ts);
	if (ret == -1 && errno == ETIMEDOUT)
		return -1;
	else
		return 0;

#elif __WIN32_OS__

	DWORD ret = WaitForSingleObject(p_sig, ms);
	if (ret == WAIT_FAILED)
		return -1;
	else if (ret == WAIT_TIMEOUT)
		return -1;

#endif

	return 0;
}

void sys_os_sig_sign(void * p_sig)
{
	if (p_sig == NULL)
		return;

#if	(__VXWORKS_OS__ || __LINUX_OS__)

	sem_post((sem_t *)p_sig);

#elif __WIN32_OS__

	SetEvent(p_sig);

#endif
}

pthread_t sys_os_create_thread(void * thread_func, void * argv)
{
	pthread_t tid = 0;

#if	(__VXWORKS_OS__ || __LINUX_OS__)

	int ret = pthread_create(&tid,NULL,(void *(*)(void *))thread_func,argv);
	if (ret != 0)
	{
		perror("sys_os_create_thread::pthread_create failed!!!");
		exit(EXIT_FAILURE);
	}

#elif __WIN32_OS__

	HANDLE hret = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_func, argv, 0, &tid);
	if (hret == NULL || tid == 0)
	{
		DWORD err = GetLastError();
		//log_print("sys_os_create_thread::CreateThread hret=%u, tid=%u, err=%u\r\n", hret, tid, err);
	}

#endif

	return tid;
}

unsigned int sys_os_get_ms()
{
	unsigned int ms = 0;

#if __LINUX_OS__

	struct timeval tv;
	gettimeofday(&tv, NULL);
	ms = tv.tv_sec * 1000 + tv.tv_usec/1000;

#elif __WIN32_OS__

	ms = GetTickCount();

#endif

	return ms;
}

char * sys_os_get_socket_error()
{
	char * p_estr = NULL;

#if __LINUX_OS__

	p_estr = strerror(errno);

#elif __WIN32_OS__

	int err = WSAGetLastError();
	static char serr_code_buf[24];
	sprintf(serr_code_buf, "WSAE-%d", err);
	p_estr = serr_code_buf;

#endif

	return p_estr;
}

static FILE * pMemLogFile = NULL;
static void * logMemFileMutex = NULL;

void mem_log_quit()
{
	if (pMemLogFile)
	{
		fclose(pMemLogFile);
		pMemLogFile = NULL;
	}

	if (logMemFileMutex)
	{
		sys_os_destroy_sig_mutx(logMemFileMutex);
		logMemFileMutex = NULL;
	}
}

int _mem_log_print(char *fmt, va_list argptr)
{
	int slen = 0;

	if (pMemLogFile == NULL)
	{
		pMemLogFile = fopen("./mem_log.txt", "wb+");
		logMemFileMutex = sys_os_create_mutex();

		if (logMemFileMutex == NULL)
		{
			printf("log init sem_init failed[%s]\r\n",strerror(errno));
		}
	}

	if (pMemLogFile == NULL)
		return 0;

	sys_os_mutex_enter(logMemFileMutex);

	slen = vfprintf(pMemLogFile,fmt,argptr);
	fflush(pMemLogFile);

	sys_os_mutex_leave(logMemFileMutex);

	return slen;
}

int mem_log_print(char * fmt,...)
{
	va_list argptr;

	va_start(argptr,fmt);

	int slen = _mem_log_print(fmt,argptr);

	va_end(argptr);

	return slen;
}

void onvif_initSdk()
{
	sys_buf_init();
	http_msg_buf_fl_init(100);
	initWinSock();
	m_dev_fl = pps_ctx_fl_init(100, sizeof(onvif_device), TRUE);
	pps_ctx_ul_init(m_dev_fl, TRUE, m_dev_ul);
}

void onvif_uninitSdk()
{
	sys_buf_deinit();
	http_msg_buf_fl_deinit();
	pps_ul_free(m_dev_ul);
	pps_fl_free(m_dev_fl);
}

onvif_device * addDevice(onvif_device * pdevice)
{
	onvif_device * p_dev = (onvif_device *) ppstack_fl_pop(m_dev_fl);
	if (p_dev)
	{
		memcpy(p_dev, pdevice, sizeof(onvif_device));

		pps_ctx_ul_add(m_dev_ul, p_dev);
	}

	return p_dev;
}

void initWinSock()
{
#ifdef WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}


/***************************************************************************************/
PPSN_CTX * pps_ctx_fl_init_assign(unsigned long mem_addr,unsigned long mem_len,unsigned long node_num,unsigned long content_size,BOOL bNeedMutex)
{
	unsigned long unit_len = content_size + sizeof(PPSN);
	unsigned long content_len = node_num * unit_len;

	if(mem_len < (content_len + sizeof(PPSN_CTX)))
	{
		//log_print("pps_ctx_fl_init_assign:: assign mem len too short!!!\r\n");
		return NULL;
	}

	PPSN_CTX * ctx_ptr = (PPSN_CTX *)mem_addr;
	memset(ctx_ptr,0,sizeof(PPSN_CTX));

	memset((char *)(mem_addr+sizeof(PPSN_CTX)),0,content_len);

	unsigned int i=0;
	for(; i<node_num; i++)
	{
		unsigned int offset = sizeof(PPSN_CTX) + unit_len * i;
		PPSN * p_node = (PPSN *)(mem_addr + offset);
		if(ctx_ptr->head_node == 0)
		{
			ctx_ptr->head_node = offset;
			ctx_ptr->tail_node = offset;
		}
		else
		{
			PPSN * p_prev_node = (PPSN *)(mem_addr + ctx_ptr->tail_node);
			p_prev_node->next_node = offset;
			p_node->prev_node = ctx_ptr->tail_node;
			ctx_ptr->tail_node = offset;
		}

		p_node->node_flag = 1;

		(ctx_ptr->node_num)++;
	}

	if(bNeedMutex)
		ctx_ptr->ctx_mutex = sys_os_create_mutex();
	else
		ctx_ptr->ctx_mutex = 0;

	ctx_ptr->fl_base = (unsigned long)ctx_ptr;
	ctx_ptr->low_offset = sizeof(PPSN_CTX) + sizeof(PPSN);
	ctx_ptr->high_offset = sizeof(PPSN_CTX) + content_len - unit_len + sizeof(PPSN);
	ctx_ptr->unit_size = unit_len;

	return ctx_ptr;
}

PPSN_CTX * pps_ctx_fl_init(unsigned long node_num,unsigned long content_size,BOOL bNeedMutex)
{
	unsigned long unit_len = content_size + sizeof(PPSN);
	unsigned long content_len = node_num * unit_len;

	char * content_ptr = (char *)malloc(content_len + sizeof(PPSN_CTX));
	if(content_ptr == NULL)
	{
		//log_print("pps_ctx_fl_init::memory XMALLOC failed,len = %d\r\n", content_len);
		return NULL;
	}

	PPSN_CTX * ctx_ptr = pps_ctx_fl_init_assign(
		(unsigned long)content_ptr,content_len+sizeof(PPSN_CTX),
		node_num,content_size,bNeedMutex);

	return ctx_ptr;
}

void pps_fl_free(PPSN_CTX * fl_ctx)
{
	if(fl_ctx == NULL) return;

	if(fl_ctx->ctx_mutex)
	{
		sys_os_destroy_sig_mutx(fl_ctx->ctx_mutex);
	}

	free(fl_ctx);
}

/***************************************************************************************/
void pps_fl_reinit(PPSN_CTX * fl_ctx)
{
	if(fl_ctx == NULL) return;

	unsigned long mem_addr = (unsigned long)fl_ctx;

	pps_wait_mutex(fl_ctx);

	char * content_start = (char *)(mem_addr + fl_ctx->low_offset - sizeof(PPSN));
	char * content_end = (char *)(mem_addr + fl_ctx->high_offset - sizeof(PPSN) + fl_ctx->unit_size);

	unsigned int content_len = content_end - content_start;
	fl_ctx->node_num = content_len / fl_ctx->unit_size;
	fl_ctx->head_node = 0;
	fl_ctx->tail_node = 0;

	memset(content_start,0,content_len);

	unsigned int i=0;
	for(; i<fl_ctx->node_num; i++)
	{
		unsigned int offset = sizeof(PPSN_CTX) + fl_ctx->unit_size * i;
		PPSN * p_node = (PPSN *)(mem_addr + offset);
		if(fl_ctx->head_node == 0)
		{
			fl_ctx->head_node = offset;
			fl_ctx->tail_node = offset;
		}
		else
		{
			PPSN * p_prev_node = (PPSN *)(mem_addr + fl_ctx->tail_node);
			p_prev_node->next_node = offset;
			p_node->prev_node = fl_ctx->tail_node;
			fl_ctx->tail_node = offset;
		}

		p_node->node_flag = 1;	
	}

	pps_post_mutex(fl_ctx);
}

BOOL ppstack_fl_push(PPSN_CTX * pps_ctx,void * content_ptr)
{
	if(pps_ctx == NULL || content_ptr == NULL)
		return FALSE;

	if(pps_safe_node(pps_ctx, content_ptr) == FALSE)
	{
		//log_print("ppstack_push::unit ptr error!!!\r\n");
		return FALSE;
	}

	PPSN * p_node = (PPSN *)(((unsigned long)content_ptr) - sizeof(PPSN));

	unsigned long offset = (unsigned long)p_node - pps_ctx->fl_base;

	pps_wait_mutex(pps_ctx);

	if(p_node->node_flag == 1)
	{
		//log_print("ppstack_push::unit node %d already in freelist !!!\r\n",pps_get_index(pps_ctx, content_ptr));
		pps_post_mutex(pps_ctx);
		return FALSE;
	}

	p_node->prev_node = 0;
	p_node->node_flag = 1;

	if(pps_ctx->head_node == 0)
	{
		pps_ctx->head_node = offset;
		pps_ctx->tail_node = offset;
		p_node->next_node = 0;
	}
	else
	{
		PPSN * p_prev = (PPSN *)(pps_ctx->head_node + pps_ctx->fl_base);
		p_prev->prev_node = offset;
		p_node->next_node = pps_ctx->head_node;
		pps_ctx->head_node = offset;
	}

	pps_ctx->node_num++;
	pps_ctx->push_cnt++;

	pps_post_mutex(pps_ctx);

	return TRUE;
}

BOOL ppstack_fl_push_tail(PPSN_CTX * pps_ctx,void * content_ptr)
{
	if(pps_ctx == NULL || content_ptr == NULL)
		return FALSE;

	if(pps_safe_node(pps_ctx, content_ptr) == FALSE)
	{
		//log_print("ppstack_fl_push_tail::unit ptr error!!!\r\n");
		return FALSE;
	}

	PPSN * p_node = (PPSN *)(((unsigned long)content_ptr) - sizeof(PPSN));

	unsigned long offset = (unsigned long)p_node - pps_ctx->fl_base;

	pps_wait_mutex(pps_ctx);

	if(p_node->node_flag == 1)
	{
		//log_print("ppstack_fl_push_tail::unit node %d already in freelist !!!\r\n",pps_get_index(pps_ctx, content_ptr));
		pps_post_mutex(pps_ctx);
		return FALSE;
	}

	p_node->prev_node = 0;
	p_node->next_node = 0;
	p_node->node_flag = 1;

	if(pps_ctx->tail_node == 0)
	{
		pps_ctx->head_node = offset;
		pps_ctx->tail_node = offset;
	}
	else
	{
		p_node->prev_node = pps_ctx->tail_node;
		PPSN * p_prev = (PPSN *)(pps_ctx->tail_node + (unsigned long)pps_ctx);
		p_prev->next_node = offset;
		pps_ctx->tail_node = offset;
	}

	pps_ctx->node_num++;
	pps_ctx->push_cnt++;

	pps_post_mutex(pps_ctx);

	return TRUE;
}

void * ppstack_fl_pop(PPSN_CTX * pps_ctx)
{
	if(pps_ctx == NULL)
		return NULL;

	pps_wait_mutex(pps_ctx);

	if(pps_ctx->head_node == 0)
	{
		pps_post_mutex(pps_ctx);
		return NULL;
	}

	PPSN * p_node = (PPSN *)(pps_ctx->fl_base + pps_ctx->head_node);

	pps_ctx->head_node = p_node->next_node;

	if(pps_ctx->head_node == 0)
		pps_ctx->tail_node = 0;
	else
	{
		PPSN * p_new_head = (PPSN *)(pps_ctx->fl_base + pps_ctx->head_node);
		p_new_head->prev_node = 0;
	}

	(pps_ctx->node_num)--;
	(pps_ctx->pop_cnt)++;

	pps_post_mutex(pps_ctx);

	memset(p_node,0,sizeof(PPSN));	

	void * ret_ptr = (void *)(((unsigned long)p_node) + sizeof(PPSN));

	return ret_ptr;
}

void pps_ctx_fl_show(PPSN_CTX * pps_ctx)
{
	if(pps_ctx == NULL)	return;

	pps_wait_mutex(pps_ctx);

	//log_print("PPSN_CTX[0x%p]::unit size = %d,unit num = %d,head = %d,tail = %d\r\n",
	//	pps_ctx->fl_base, pps_ctx->unit_size, pps_ctx->node_num, pps_ctx->head_node, pps_ctx->tail_node);

	unsigned long ctx_count = 0;
	unsigned int offset = pps_ctx->head_node;
	while(offset != 0)
	{
		PPSN * p_node = (PPSN *)(pps_ctx->fl_base + offset);
		//log_print("0x%p == FLAG: %d  next: 0x%08x  prev: 0x%08x\r\n",
		//	p_node, p_node->node_flag, p_node->next_node, p_node->prev_node);

		ctx_count++;

		if(ctx_count > pps_ctx->node_num)
		{
			//log_print("\r\n!!!FreeList Error,Linked item count[%u] > real item count[%u]\r\n",ctx_count,pps_ctx->node_num);
			break;
		}

		offset = p_node->next_node;
	}

	//log_print("\r\nFreeList Linked item count[%d]\r\n",ctx_count);

	pps_post_mutex(pps_ctx);
}

/***************************************************************************************/
BOOL pps_ctx_ul_init_assign(PPSN_CTX * ul_ctx, PPSN_CTX * fl_ctx,BOOL bNeedMutex)
{
	if(ul_ctx == NULL || fl_ctx == NULL)
		return FALSE;

	memset(ul_ctx,0,sizeof(PPSN_CTX));

	ul_ctx->fl_base = fl_ctx->fl_base;
	ul_ctx->high_offset = fl_ctx->high_offset;
	ul_ctx->low_offset = fl_ctx->low_offset;
	ul_ctx->unit_size = fl_ctx->unit_size;

	if(bNeedMutex)
		ul_ctx->ctx_mutex = sys_os_create_mutex();
	else
		ul_ctx->ctx_mutex = 0;

	return TRUE;
}

BOOL pps_ctx_ul_init(PPSN_CTX * fl_ctx, BOOL bNeedMutex, PPSN_CTX*& ul_ctx)
{
	if(fl_ctx == NULL)
		return FALSE;

	ul_ctx = (PPSN_CTX *)malloc(sizeof(PPSN_CTX));
	if(ul_ctx == NULL)
	{
		return FALSE;
	}

	memset(ul_ctx,0,sizeof(PPSN_CTX));

	ul_ctx->fl_base = fl_ctx->fl_base;
	ul_ctx->high_offset = fl_ctx->high_offset;	// + fl_ctx->fl_base;
	ul_ctx->low_offset = fl_ctx->low_offset;	// + fl_ctx->fl_base;
	ul_ctx->unit_size = fl_ctx->unit_size;

	if(bNeedMutex)
		ul_ctx->ctx_mutex = sys_os_create_mutex();
	else
		ul_ctx->ctx_mutex = 0;

	return TRUE;
}

BOOL pps_ctx_ul_init_nm(PPSN_CTX * fl_ctx,PPSN_CTX * ul_ctx)
{
	return pps_ctx_ul_init_assign(ul_ctx, fl_ctx, FALSE);
}

/***************************************************************************************/
void pps_ul_reinit(PPSN_CTX * ul_ctx)
{
	if(ul_ctx == NULL) return;

	ul_ctx->node_num = 0;
	ul_ctx->head_node = 0;
	ul_ctx->tail_node = 0;

	pps_wait_mutex(ul_ctx);
	pps_post_mutex(ul_ctx);

	if(ul_ctx->ctx_mutex)
	{
		sys_os_destroy_sig_mutx(ul_ctx->ctx_mutex);
	}	
}

void pps_ul_free(PPSN_CTX * ul_ctx)
{
	if(ul_ctx == NULL) return;

	if(ul_ctx->ctx_mutex)
	{
		sys_os_destroy_sig_mutx(ul_ctx->ctx_mutex);
	}

	free(ul_ctx);
}

BOOL pps_ctx_ul_del(PPSN_CTX * ul_ctx,void * content_ptr)
{
	if(pps_used_node(ul_ctx, content_ptr) == FALSE)
		return FALSE;

	PPSN * p_node = (PPSN *)(((unsigned long)content_ptr) - sizeof(PPSN));

	pps_wait_mutex(ul_ctx);

	if(p_node->prev_node == 0)
		ul_ctx->head_node = p_node->next_node;
	else
		((PPSN *)(ul_ctx->fl_base + p_node->prev_node))->next_node = p_node->next_node;

	if(p_node->next_node == 0)
		ul_ctx->tail_node = p_node->prev_node;
	else
		((PPSN *)(ul_ctx->fl_base + p_node->next_node))->prev_node = p_node->prev_node;

	(ul_ctx->node_num)--;

	pps_post_mutex(ul_ctx);

	memset(p_node,0,sizeof(PPSN));

	return TRUE;
}

PPSN * pps_ctx_ul_del_node_unlock(PPSN_CTX * ul_ctx,PPSN * p_node)
{
	if(p_node->node_flag != 2)
	{
		//log_print("pps_ctx_ul_del_node_unlock::unit not in used list!!!\r\n");
		return NULL;
	}

	if(ul_ctx->head_node == 0)
	{
		//log_print("pps_ctx_ul_del_node_unlock::used list is empty!!!\r\n");
		return NULL;
	}

	if(p_node->prev_node == 0)
		ul_ctx->head_node = p_node->next_node;
	else
		((PPSN *)(ul_ctx->fl_base + p_node->prev_node))->next_node = p_node->next_node;

	if(p_node->next_node == 0)
		ul_ctx->tail_node = p_node->prev_node;
	else
		((PPSN *)(ul_ctx->fl_base + p_node->next_node))->prev_node = p_node->prev_node;

	(ul_ctx->node_num)--;

	if(p_node->next_node == 0)
		return NULL;
	else
		return (PPSN *)(ul_ctx->fl_base + p_node->next_node);
}

void * pps_ctx_ul_del_unlock(PPSN_CTX * ul_ctx,void * content_ptr)
{
	if(pps_used_node(ul_ctx, content_ptr) == FALSE)
		return FALSE;

	PPSN * p_node = (PPSN *)(((unsigned long)content_ptr) - sizeof(PPSN));

	PPSN * p_ret = pps_ctx_ul_del_node_unlock(ul_ctx, p_node);
	//	memset(p_node,0,sizeof(PPSN));
	if(p_ret == NULL)
		return NULL;
	else
	{
		void * ret_ptr = (void *)(((unsigned long)p_ret) + sizeof(PPSN));
		return ret_ptr;
	}
}

BOOL pps_ctx_ul_add(PPSN_CTX * ul_ctx,void * content_ptr)
{
	if(pps_safe_node(ul_ctx, content_ptr) == FALSE)
		return FALSE;

	PPSN * p_node = (PPSN *)(((unsigned long)content_ptr) - sizeof(PPSN));
	if(p_node->node_flag != 0)
		return FALSE;

	pps_wait_mutex(ul_ctx);

	p_node->next_node = 0;
	p_node->node_flag = 2;

	unsigned int offset = ((unsigned long)p_node) - ul_ctx->fl_base;
	if(ul_ctx->tail_node == 0)
	{
		ul_ctx->tail_node = offset;
		ul_ctx->head_node = offset;
		p_node->prev_node = 0;
	}
	else
	{
		PPSN * p_tail = (PPSN *)(ul_ctx->fl_base + ul_ctx->tail_node);
		p_tail->next_node = offset;
		p_node->prev_node = ul_ctx->tail_node;
		ul_ctx->tail_node = offset;
	}

	(ul_ctx->node_num)++;

	pps_post_mutex(ul_ctx);

	return TRUE;
}

BOOL pps_ctx_ul_add_head(PPSN_CTX * ul_ctx,void * content_ptr)
{
	if(pps_safe_node(ul_ctx, content_ptr) == FALSE)
		return FALSE;

	PPSN * p_node = (PPSN *)(((unsigned long)content_ptr) - sizeof(PPSN));
	if(p_node->node_flag != 0)
		return FALSE;

	pps_wait_mutex(ul_ctx);

	unsigned int offset = ((unsigned long)p_node) - ul_ctx->fl_base;
	p_node->node_flag = 2;
	p_node->prev_node = 0;

	if(ul_ctx->head_node == 0)
	{
		ul_ctx->tail_node = offset;
		ul_ctx->head_node = offset;
		p_node->next_node = 0;
	}
	else
	{
		PPSN * p_head = (PPSN *)(ul_ctx->fl_base + ul_ctx->head_node);
		p_head->prev_node = offset;
		p_node->next_node = ul_ctx->head_node;
		ul_ctx->head_node = offset;
	}

	(ul_ctx->node_num)++;

	pps_post_mutex(ul_ctx);

	return TRUE;
}


PPSN * _pps_node_head_start(PPSN_CTX * pps_ctx)
{
	if(pps_ctx == NULL) return NULL;

	pps_wait_mutex(pps_ctx);

	if(pps_ctx->head_node == 0)
		return NULL;
	else
		return (PPSN *)(pps_ctx->fl_base + pps_ctx->head_node);
}

PPSN * _pps_node_tail_start(PPSN_CTX * pps_ctx)
{
	if(pps_ctx == NULL) return NULL;

	pps_wait_mutex(pps_ctx);

	if(pps_ctx->tail_node == 0)
		return NULL;
	else
		return (PPSN *)(pps_ctx->fl_base + pps_ctx->tail_node);
}

PPSN * _pps_node_next(PPSN_CTX * pps_ctx, PPSN * p_node)
{
	if(pps_ctx == NULL || p_node == NULL) return NULL;

	unsigned long ctx_ptr = ((unsigned long)p_node) + sizeof(PPSN);

	if((unsigned long)ctx_ptr < (pps_ctx->fl_base + pps_ctx->low_offset) ||
		(unsigned long)ctx_ptr > (pps_ctx->fl_base + pps_ctx->high_offset))
	{
		//log_print("pps_lookup_next::unit ptr error!!!!!!\r\n");
		return NULL;
	}

	if(p_node->next_node == 0)
		return NULL;
	else
		return (PPSN *)(p_node->next_node + pps_ctx->fl_base);
}

PPSN * _pps_node_prev(PPSN_CTX * pps_ctx, PPSN * p_node)
{
	if(pps_ctx == NULL || p_node == NULL) return NULL;

	unsigned long ctx_ptr = ((unsigned long)p_node) + sizeof(PPSN);

	if((unsigned long)ctx_ptr < (pps_ctx->low_offset+pps_ctx->fl_base) ||
		(unsigned long)ctx_ptr > (pps_ctx->high_offset+pps_ctx->fl_base))
	{
		//log_print("pps_lookup_next::unit ptr error!!!!!!\r\n");	
		return NULL;
	}

	if(p_node->prev_node == 0)
		return NULL;
	else
		return (PPSN *)(pps_ctx->fl_base + p_node->prev_node);
}

void _pps_node_end(PPSN_CTX * pps_ctx)
{
	pps_post_mutex(pps_ctx);
}

void * pps_lookup_start(PPSN_CTX * pps_ctx)
{
	if(pps_ctx == NULL) return NULL;

	pps_wait_mutex(pps_ctx);

	if(pps_ctx->head_node)
	{
		void * ret_ptr = (void *)(pps_ctx->fl_base + pps_ctx->head_node + sizeof(PPSN));
		return ret_ptr;
	}

	return NULL;
}

void * pps_lookup_next(PPSN_CTX * pps_ctx, void * ctx_ptr)
{
	if(pps_ctx == NULL || ctx_ptr == NULL) return NULL;

	if((unsigned long)ctx_ptr < (pps_ctx->fl_base + pps_ctx->low_offset) ||
		(unsigned long)ctx_ptr > (pps_ctx->fl_base + pps_ctx->high_offset))
	{
		//log_print("pps_lookup_next::unit ptr error!!!\r\n");
		return NULL;
	}

	PPSN * p_node = (PPSN *)(((unsigned long)ctx_ptr) - sizeof(PPSN));

	if(p_node->next_node == 0)
		return NULL;
	else
	{
		void * ret_ptr = (void *)(pps_ctx->fl_base + p_node->next_node + sizeof(PPSN));
		return ret_ptr;
	}
}

void pps_lookup_end(PPSN_CTX * pps_ctx)
{
	pps_post_mutex(pps_ctx);
}

void * pps_lookback_start(PPSN_CTX * pps_ctx)
{
	if(pps_ctx == NULL) return NULL;

	pps_wait_mutex(pps_ctx);

	if(pps_ctx->tail_node)
	{
		void * ret_ptr = (void *)(pps_ctx->tail_node + sizeof(PPSN)+pps_ctx->fl_base);
		return ret_ptr;
	}

	return NULL;
}

void * pps_lookback_next(PPSN_CTX * pps_ctx, void * ctx_ptr)
{
	if(pps_ctx == NULL || ctx_ptr == NULL) return NULL;

	if((unsigned long)ctx_ptr < (pps_ctx->low_offset+pps_ctx->fl_base) ||
		(unsigned long)ctx_ptr > (pps_ctx->high_offset+pps_ctx->fl_base))
	{
		//log_print("pps_lookup_next::unit ptr error!!!\r\n");
		return NULL;
	}

	PPSN * p_node = (PPSN *)(((unsigned long)ctx_ptr) - sizeof(PPSN));

	if(p_node->prev_node == 0)
		return NULL;
	else
	{
		void * ret_ptr = (void *)(p_node->prev_node + sizeof(PPSN)+pps_ctx->fl_base);
		return ret_ptr;
	}
}

void pps_lookback_end(PPSN_CTX * pps_ctx)
{
	pps_post_mutex(pps_ctx);
}

unsigned long pps_get_index(PPSN_CTX * pps_ctx,void * content_ptr)
{
	if(pps_ctx == NULL || content_ptr == NULL) return 0xFFFFFFFF;

	if((unsigned long)content_ptr < (pps_ctx->low_offset+pps_ctx->fl_base) ||
		(unsigned long)content_ptr > (pps_ctx->high_offset+pps_ctx->fl_base))
	{
		//log_print("pps_get_index::unit ptr error!!!\r\n");
		return 0xFFFFFFFF;
	}

	unsigned long index = (unsigned long)content_ptr - pps_ctx->low_offset - pps_ctx->fl_base;
	unsigned long offset = index % pps_ctx->unit_size;
	if(offset != 0)
	{
		index = index /pps_ctx->unit_size;

		//log_print("pps_get_index::unit ptr error,pps_ctx[0x%08x],ptr[0x%08x],low_offset[0x%08x],offset[0x%08x],like entry[%u]\r\n",
		//	pps_ctx,content_ptr,pps_ctx->low_offset,offset,index);
		return 0xFFFFFFFF;
	}

	index = index /pps_ctx->unit_size;

	return index;
}

void * pps_get_node_by_index(PPSN_CTX * pps_ctx,unsigned long index)
{
	if(pps_ctx == NULL) return NULL;

	unsigned long content_offset = pps_ctx->low_offset + index * pps_ctx->unit_size;
	if(content_offset > pps_ctx->high_offset)
	{
		if(index != 0xFFFFFFFF)
			//log_print("pps_get_node_by_index::index [%u]error!!!\r\n",index);
		return NULL;
	}

	return (void *)(content_offset + pps_ctx->fl_base);
}

void pps_wait_mutex(PPSN_CTX * pps_ctx)
{
	if(pps_ctx == NULL)
	{
		//log_print("pps_wait_mutex::pps_ctx == NULL!!!\r\n");
		return;
	}

	if(pps_ctx->ctx_mutex)
	{
		sys_os_mutex_enter (pps_ctx->ctx_mutex);
	}
}

void pps_post_mutex(PPSN_CTX * pps_ctx)
{
	if(pps_ctx == NULL)
	{
		//log_print("pps_post_mutex::pps_ctx == NULL!!!\r\n");
		return;
	}

	if(pps_ctx->ctx_mutex)
	{
		sys_os_mutex_leave (pps_ctx->ctx_mutex);
	}
}

BOOL pps_safe_node(PPSN_CTX * pps_ctx,void * content_ptr)
{
	if(pps_ctx == NULL || content_ptr == NULL) return FALSE;

	if((unsigned long)content_ptr < (pps_ctx->low_offset + pps_ctx->fl_base) ||
		(unsigned long)content_ptr > (pps_ctx->high_offset + pps_ctx->fl_base))
	{
		// //log_print("pps_safe_node::unit ptr error!!!\r\n");
		return FALSE;
	}

	unsigned int index = (unsigned long)content_ptr - pps_ctx->low_offset - pps_ctx->fl_base;
	unsigned int offset = index % pps_ctx->unit_size;
	if(offset != 0)
	{
		index = index /pps_ctx->unit_size;

		//log_print("pps_safe_node::unit ptr error,pps_ctx[0x%08x],ptr[0x%08x],low_offset[0x%08x],offset[0x%08x],like entry[%u]\r\n",
		//	pps_ctx,content_ptr,pps_ctx->low_offset,offset,index);
		return FALSE;
	}

	return TRUE;
}

BOOL pps_idle_node(PPSN_CTX * pps_ctx,void * content_ptr)
{
	if(pps_safe_node(pps_ctx, content_ptr) == FALSE)
		return FALSE;

	PPSN * p_node = (PPSN *)(((unsigned long)content_ptr) - sizeof(PPSN));
	return (p_node->node_flag == 1);
}

BOOL pps_exist_node(PPSN_CTX * pps_ctx,void * content_ptr)
{
	if(pps_safe_node(pps_ctx, content_ptr) == FALSE)
		return FALSE;

	PPSN * p_node = (PPSN *)(((unsigned long)content_ptr) - sizeof(PPSN));
	return (p_node->node_flag != 1);
}

BOOL pps_used_node(PPSN_CTX * pps_ctx,void * content_ptr)
{
	if(pps_safe_node(pps_ctx, content_ptr) == FALSE)
		return FALSE;

	if(pps_ctx->head_node == 0)	
	{
		//log_print("pps_used_node::used list is empty!!!\r\n");
		return FALSE;
	}

	PPSN * p_node = (PPSN *)(((unsigned long)content_ptr) - sizeof(PPSN));
	return (p_node->node_flag == 2);
}

void * _pps_node_get_data(PPSN * p_node){
	if(p_node == NULL) return NULL;
	return (void *)(((unsigned long)p_node) + sizeof(PPSN));
}

PPSN * _pps_data_get_node(void * p_data){
	if(p_data == NULL) return NULL;
	PPSN * p_node = (PPSN *)(((unsigned long)p_data) - sizeof(PPSN));
	return p_node;
}

int pps_node_count(PPSN_CTX * pps_ctx)
{
	if (pps_ctx == NULL) return 0;
	return pps_ctx->node_num;
}

void * pps_get_head_node(PPSN_CTX * pps_ctx)
{
	if(pps_ctx == NULL) return NULL;

	//pps_wait_mutex(pps_ctx);

	if(pps_ctx->head_node)
	{
		void * ret_ptr = (void *)(pps_ctx->head_node + sizeof(PPSN));
		return ret_ptr;
	}

	return NULL;
}

void * pps_get_tail_node(PPSN_CTX * pps_ctx)
{
	if(pps_ctx == NULL) return NULL;

	//pps_wait_mutex(pps_ctx);

	if(pps_ctx->tail_node)
	{
		void * ret_ptr = (void *)(pps_ctx->tail_node + sizeof(PPSN));
		return ret_ptr;
	}

	return NULL;
}

void * pps_get_next_node(PPSN_CTX * pps_ctx, void * content_ptr)
{
	PPSN * p_node = _pps_data_get_node(content_ptr);
	p_node = _pps_node_next(pps_ctx, p_node);
	return _pps_node_get_data(p_node);
}

void * pps_get_prev_node(PPSN_CTX * pps_ctx, void * content_ptr)
{
	PPSN * p_node = _pps_data_get_node(content_ptr);
	p_node = _pps_node_prev(pps_ctx, p_node);
	return _pps_node_get_data(p_node);
}


/***************************************************************************************/
PPSN_CTX * net_buf_fl = NULL;


/***************************************************************************************/
BOOL net_buf_fl_init()
{
	net_buf_fl = pps_ctx_fl_init(128,2048,TRUE);
	if (net_buf_fl == NULL)
		return FALSE;

	//log_print("net_buf_fl_init::num = %lu\r\n",net_buf_fl->node_num);

	return TRUE;
}

char * get_idle_net_buf()
{
	//	//log_print("get_idle_sip_buf::num = %d\r\n",sip_buf_fl->node_num);
	return (char *)ppstack_fl_pop(net_buf_fl);
}

void free_net_buf(char * rbuf)
{
	if (rbuf == NULL)
		return;

	if (pps_safe_node(net_buf_fl, rbuf))
		ppstack_fl_push_tail(net_buf_fl,rbuf);	
	else
		free(rbuf);
}

unsigned int idle_net_buf_num()
{
	return net_buf_fl->node_num;
}

void net_buf_fl_deinit()
{
	if (net_buf_fl)
	{
		pps_fl_free(net_buf_fl);
		net_buf_fl = NULL;
	}
}

BOOL hdrv_buf_fl_init(int num)
{
	hdrv_buf_fl = pps_ctx_fl_init(num,sizeof(HDRV),TRUE);
	if (hdrv_buf_fl == NULL)
		return FALSE;

	//log_print("hdrv_buf_fl_init::num = %lu\r\n",hdrv_buf_fl->node_num);
	//	pps_ctx_fl_show(hdrv_buf_fl);

	return TRUE;
}

void hdrv_buf_fl_deinit()
{
	if (hdrv_buf_fl)
	{
		pps_fl_free(hdrv_buf_fl);
		hdrv_buf_fl = NULL;
	}
}

HDRV * get_hdrv_buf()
{
	HDRV * p_ret = (HDRV *)ppstack_fl_pop(hdrv_buf_fl);
	//	//log_print("get_hdrv_buf::num = %d\r\n",hdrv_buf_fl->node_num);
	//	pps_ctx_fl_show(hdrv_buf_fl);
	return p_ret;
}

void free_hdrv_buf(HDRV * pHdrv)
{
	if (pHdrv == NULL)
		return;

	pHdrv->header[0] = '\0';
	pHdrv->value_string = NULL;
	ppstack_fl_push(hdrv_buf_fl,pHdrv);
	//	//log_print("free_hdrv_buf::num = %d,pHdrv=0x%08x\r\n",hdrv_buf_fl->node_num,pHdrv);
	//	pps_ctx_fl_show(hdrv_buf_fl);
}

unsigned int idle_hdrv_buf_num()
{
	return hdrv_buf_fl->node_num;
}

void init_ul_hdrv_ctx(PPSN_CTX * ul_ctx)
{
	pps_ctx_ul_init_nm(hdrv_buf_fl,ul_ctx);
}

void free_ctx_hdrv(PPSN_CTX * p_ctx)
{
	if (p_ctx == NULL)
		return;

	HDRV * p_free = (HDRV *)pps_lookup_start(p_ctx);
	while (p_free != NULL) 
	{
		HDRV * p_next = (HDRV *)pps_lookup_next(p_ctx,p_free);
		pps_ctx_ul_del(p_ctx,p_free);
		free_hdrv_buf(p_free);

		p_free = p_next;		
	}
	pps_lookup_end(p_ctx);
}

BOOL sys_buf_init()
{
	if (net_buf_fl_init() == FALSE)
	{
		//log_print("sys_buf_init::net_buf_fl_init failed!!!\r\n");
		return FALSE;
	}

	if (hdrv_buf_fl_init(1024) == FALSE)
	{
		//log_print("sys_buf_init::hdrv_buf_fl_init failed!!!\r\n");
		return FALSE;
	}

	return TRUE;
}

void sys_buf_deinit()
{
	net_buf_fl_deinit();
	hdrv_buf_fl_deinit();
}

int http_pkt_find_end(char * p_buf)
{
	int end_off = 0;
	int http_pkt_finish = 0; 

	while(p_buf[end_off] != '\0')
	{
		if((p_buf[end_off] == '\r' && p_buf[end_off+1] == '\n') &&
			(p_buf[end_off+2] == '\r' && p_buf[end_off+3] == '\n'))
		{
			http_pkt_finish = 1;
			break;
		}

		end_off++;
	}

	if(http_pkt_finish)
		return(end_off + 4);
	return 0;
}

BOOL http_tcp_rx(HTTPREQ * p_user)
{
	if(p_user->p_rbuf == NULL)
	{
		p_user->p_rbuf = p_user->rcv_buf;
		p_user->mlen = sizeof(p_user->rcv_buf)-1;
		p_user->rcv_dlen = 0;
		p_user->ctt_len = 0;
		p_user->hdr_len = 0;
	}

	int rlen = recv(p_user->cfd,p_user->p_rbuf+p_user->rcv_dlen,p_user->mlen-p_user->rcv_dlen,0);
	if(rlen <= 0)
	{
		//log_print("http_tcp_rx::recv return = %d, dlen[%d], mlen[%d]\r\n",rlen,p_user->rcv_dlen,p_user->mlen);	
		closesocket(p_user->cfd);
		p_user->cfd = 0;
		return FALSE;
	}

	p_user->rcv_dlen += rlen;
	p_user->p_rbuf[p_user->rcv_dlen] = '\0';

	if(p_user->rcv_dlen < 16)
		return TRUE;

	if(is_http_msg(p_user->p_rbuf) == FALSE)	
		return FALSE;

	HTTPMSG * rx_msg = NULL;

	if(p_user->hdr_len == 0)
	{
		int http_pkt_len = http_pkt_find_end(p_user->p_rbuf);
		if(http_pkt_len == 0) 
		{
			return TRUE;
		}
		p_user->hdr_len = http_pkt_len;

		rx_msg = http_get_msg_buf();
		if(rx_msg == NULL)
		{
			//log_print("http_tcp_rx::get_msg_buf ret null!!!\r\n");
			return FALSE;
		}

		memcpy(rx_msg->msg_buf,p_user->p_rbuf,http_pkt_len);
		rx_msg->msg_buf[http_pkt_len] = '\0';
		////log_print("%s\r\n\r\n\r\n",rx_msg->msg_buf);

		int parse_len = http_msg_parse_part1(rx_msg->msg_buf,http_pkt_len,rx_msg);
		if(parse_len != http_pkt_len)
		{
			//log_print("http_tcp_rx::http_msg_parse_part1=%d, sip_pkt_len=%d!!!\r\n",parse_len,http_pkt_len);
			free_http_msg(rx_msg);
			return FALSE;
		}
		p_user->ctt_len = rx_msg->ctt_len;
	}

	//	if((p_user->ctt_len + p_user->hdr_len) > sizeof(p_user->rcv_buf))
	if((p_user->ctt_len + p_user->hdr_len) > p_user->mlen)
	{
		if(p_user->dyn_recv_buf)
		{
			free(p_user->dyn_recv_buf);
		}

		p_user->dyn_recv_buf = (char *)malloc(p_user->ctt_len + p_user->hdr_len + 1);
		memcpy(p_user->dyn_recv_buf, p_user->rcv_buf, p_user->rcv_dlen);
		p_user->p_rbuf = p_user->dyn_recv_buf;
		p_user->mlen = p_user->ctt_len + p_user->hdr_len;

		if(rx_msg) free_http_msg(rx_msg);
		return TRUE;
	}

	if(p_user->rcv_dlen >= (p_user->ctt_len + p_user->hdr_len))
	{
		if(rx_msg == NULL)
		{
			int nlen = p_user->ctt_len + p_user->hdr_len;
			if(nlen > 2048)
			{
				rx_msg = http_get_msg_large_buf(nlen+1);
				if(rx_msg == NULL)
					return FALSE;
			}
			else
			{
				rx_msg = http_get_msg_buf();
				if(rx_msg == NULL)
					return FALSE;
			}

			memcpy(rx_msg->msg_buf,p_user->p_rbuf,p_user->hdr_len);
			rx_msg->msg_buf[p_user->hdr_len] = '\0';
			////log_print("%s\r\n\r\n\r\n",rx_msg->msg_buf);

			int parse_len = http_msg_parse_part1(rx_msg->msg_buf,p_user->hdr_len,rx_msg);
			if(parse_len != p_user->hdr_len)
			{
				//log_print("http_tcp_rx::http_msg_parse_part1=%d, sip_pkt_len=%d!!!\r\n",parse_len,p_user->hdr_len);
				free_http_msg(rx_msg);
				return FALSE;
			}
		}

		if(p_user->ctt_len > 0)
		{
			memcpy(rx_msg->msg_buf+p_user->hdr_len, p_user->p_rbuf+p_user->hdr_len, p_user->ctt_len);
			rx_msg->msg_buf[p_user->hdr_len + p_user->ctt_len] = '\0';
			////log_print("%s",rx_msg->msg_buf+p_user->hdr_len);

			int parse_len = http_msg_parse_part2(rx_msg->msg_buf+p_user->hdr_len,p_user->ctt_len,rx_msg);
			if(parse_len != p_user->ctt_len)
			{
				//log_print("http_tcp_rx::http_msg_parse_part2=%d, sdp_pkt_len=%d!!!\r\n",parse_len,p_user->ctt_len);
				free_http_msg(rx_msg);
				return FALSE;
			}
		}

		p_user->rx_msg = rx_msg;
	}

	return TRUE;
}

BOOL http_tcp_tx(HTTPREQ * p_user, char * p_data, int len)
{
	if(p_user->cfd <= 0)
		return FALSE;

	int slen = send(p_user->cfd, p_data, len, 0);
	if(slen != len)
		return FALSE;

	return TRUE;
}

BOOL http_onvif_req(HTTPREQ * p_user, char * action, char * p_xml, int len)
{
	char bufs[1024 * 4];
	if(len > 3072)
		return FALSE;

	int offset = 0;
	offset += sprintf(bufs+offset, "POST %s HTTP/1.1\r\n", p_user->url);
	offset += sprintf(bufs+offset, "Content-Type: application/soap+xml; charset=utf-8; action=\"%s\"\r\n", action);
	offset += sprintf(bufs+offset, "Host: %s\r\n", p_user->host);
	offset += sprintf(bufs+offset, "Content-Length: %d\r\n", len);
	offset += sprintf(bufs+offset, "Accept-Encoding: gzip, deflate\r\n");
	offset += sprintf(bufs+offset, "User-Agent: ltxd/1.0\r\n");
	offset += sprintf(bufs+offset, "Connection: close\r\n\r\n");

	memcpy(bufs+offset, p_xml, len);
	offset += len;
	bufs[offset] = '\0';

	return http_tcp_tx(p_user, bufs, offset);
}

/*************************************************************************/
OVFACTS g_onvif_acts[] = 
{
	{eGetDeviceInformation, "http://www.onvif.org/ver10/device/wsdl/GetDeviceInformation"},
	{eGetCapabilities, "http://www.onvif.org/ver10/device/wsdl/GetCapabilities"},
	{eGetProfiles, "http://www.onvif.org/ver10/media/wsdl/GetProfiles"},
	{eGetStreamUri, "http://www.onvif.org/ver10/media/wsdl/GetStreamUri"},
	{eGetNetworkInterfaces, "http://www.onvif.org/ver10/device/wsdl/GetNetworkInterfaces"},
	{eGetVideoEncoderConfigurations, "http://www.onvif.org/ver10/media/wsdl/GetVideoEncoderConfigurations"},
	{eGetPresets, "http://www.onvif.org/ver20/ptz/wsdl/GetPresets"},
	{eSetPreset, "http://www.onvif.org/ver20/ptz/wsdl/SetPreset"},
	{eGotoPreset, "http://www.onvif.org/ver20/ptz/wsdl/GotoPreset"},
	{eGotoHomePosition, "http://www.onvif.org/ver20/ptz/wsdl/GotoHomePosition"},
	{eSetHomePosition, "http://www.onvif.org/ver20/ptz/wsdl/SetHomePosition"},
	{eContinuousMove, "http://www.onvif.org/ver20/ptz/wsdl/ContinuousMove"},	
	{eStop, "http://www.onvif.org/ver20/ptz/wsdl/Stop"},
	{eGetConfigurations, "http://www.onvif.org/ver20/ptz/wsdl/GetConfigurations"},
	{eGetConfigurationOptions, "http://www.onvif.org/ver20/ptz/wsdl/GetConfigurationOptions"},
	{eGetSystemDateAndTime, "http://www.onvif.org/ver10/device/wsdl/GetSystemDateAndTime"},
	{eSetSystemDateAndTime, "http://www.onvif.org/ver10/device/wsdl/SetSystemDateAndTime"},
	{eSystemReboot, "http://www.onvif.org/ver10/device/wsdl/SystemReboot"},
	{eStartRecvAlarm, "http://www.onvif.org/ver10/events/wsdl/EventPortType/CreatePullPointSubscriptionRequest"},
	{eRecvAlarm, "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesRequest"},
	{eStopRecvAlarm, "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesRequest"},
	{eGetSnapUri, "http://www.onvif.org/ver10/media/wsdl/GetSnapshotUri"},
	{eGetScopes, "http://www.onvif.org/ver10/device/wsdl/GetScopes"},
	{eGetNode, "http://www.onvif.org/ver20/ptz/wsdl/GetNode"},
	{eAbsoluteMove, "http://www.onvif.org/ver20/ptz/wsdl/AbsoluteMove"},
};

OVFACTS * onvif_find_action_by_type(eOnvifAction type)
{
	if(type < eActionNull || type >= eActionMax)
		return NULL;
	for(unsigned int i=0; i<(sizeof(g_onvif_acts)/sizeof(OVFACTS)); i++)
	{
		if(g_onvif_acts[i].type == type)
			return &g_onvif_acts[i];
	}

	return NULL;
}

char buf[128] = {0};
void GetGUID()
{
	Guid guid = Guid::createGuid();
	strcpy(buf, (char*)guid.getBitStream());
}


static const char * onvif_xml_ns = 
	"xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" " 
	"xmlns:enc=\"http://www.w3.org/2003/05/soap-encoding\" " 
	"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" " 
	"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" " 
	"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" " 
	"xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\" " 
	"xmlns:xenc=\"http://www.w3.org/2001/04/xmlenc#\" " 
	"xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\" " 
	"xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\" " 
	"xmlns:xmime=\"http://tempuri.org/xmime.xsd\" " 
	"xmlns:tt=\"http://www.onvif.org/ver10/schema\" " 
	"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" "
	"xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\" " 
	"xmlns:ter=\"http://www.onvif.org/ver10/error\"";

static const char * onvif_xml_header = 
	"<s:Header>"
	"<wsse:Security "
	"xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\" "
	"xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">"
	"<wsse:UsernameToken>"
	"<wsse:Username>%s</wsse:Username>"
	"<wsse:Password "
	"Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">"
	"%s</wsse:Password>"
	"<wsse:Nonce>%s</wsse:Nonce>"
	"<wsu:Created>%s</wsu:Created>"
	"</wsse:UsernameToken></wsse:Security>"
	"</s:Header>";

// static const char * onvif_xml_header = 
// 	"<s:Header>"
// 	"<Security s:mustUnderstand=\"1\""
// 	"xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
// 	"<UsernameToken>"
// 	"<Username>%s</Username>"
// 	"<Password "
// 	"Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">"
// 	"%s</Password>"
// 	"<Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0\">%s</Nonce>"
// 	"<Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">%s</Created>"
// 	"</UsernameToken></Security>"
// 	"</s:Header>";

static const char * onvif_xml_headerCreatePullPointSubscriptionRequest = 
	"<s:Header>"
	"<a:Action s:mustUnderstand=\"1\">http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesRequest</a:Action>"
	"<a:MessageID>urn:uuid:%s</a:MessageID>"
	"<a:ReplyTo>"
	"<a:Address>http://www.w3.org/2005/08/addressing/anonymous</a:Address>"
	"</a:ReplyTo>"
	"<Security s:mustUnderstand=\"1\""
	"xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
	"<UsernameToken>"
	"<Username>%s</Username>"
	"<Password "
	"Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">""%s</Password>"
	"<Nonce>%s</Nonce>"
	"<Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">%s</Created>"
	"</UsernameToken></Security>"
	"<a:To s:mustUnderstand=\"1\">http://%s%s</a:To>"
	"</s:Header>";

static const char * onvif_xml_headerPullMessage = 
	"<s:Header>"
	"<a:Action s:mustUnderstand=\"1\">http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesReques</a:Action>"
	"<a:MessageID>urn:uuid:%s</a:MessageID>"
	"<a:ReplyTo>"
	"<a:Address>http://www.w3.org/2005/08/addressing/anonymous</a:Address>"
	"</a:ReplyTo>"
	"<Security s:mustUnderstand=\"1\""
	"xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
	"<UsernameToken>"
	"<Username>%s</Username>"
	"<Password "
	"Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">""%s</Password>"
	"<Nonce>%s</Nonce>"
	"<Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">%s</Created>"
	"</UsernameToken></Security>"
	"<a:To s:mustUnderstand=\"1\">http://%s%s</a:To>"
	"</s:Header>";

#define NONCELEN	20
#define SHA1_SIZE	20

void onvif_calc_nonce(char nonce[NONCELEN])
{ 
	static unsigned short count = 0xCA53;
	char buf[NONCELEN + 1];
	/* we could have used raw binary instead of hex as below */
	sprintf(buf, "%8.8x%4.4hx%8.8x", (int)time(NULL), count++, (int)rand());
	memcpy(nonce, buf, NONCELEN);
}

const char * onvif_datetime(time_t n)
{
	struct tm *pT;
	static char buff[100];

	memset(buff, 0, sizeof(buff));

	if ((pT = gmtime(&n)))
	{ 
		strftime(buff, sizeof(buff), "%Y-%m-%dT%H:%M:%SZ", pT);
	}

	return buff;
}

void onvif_calc_digest(const char *created, const char *nonce, int noncelen, const char *password, char hash[SHA1_SIZE])
{
	sha1_context ctx;

	sha1_starts(&ctx);
	sha1_update(&ctx, (unsigned char *)nonce, noncelen);
	sha1_update(&ctx, (unsigned char *)created, strlen(created));
	sha1_update(&ctx, (unsigned char *)password, strlen(password));
	sha1_finish(&ctx, (unsigned char *)hash);
}


int build_onvif_req_header(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;

	if (p_dev->username[0] != '\0' && p_dev->password[0] != '\0')
	{
		char HA[SHA1_SIZE], HABase64[100];
		char nonce[NONCELEN], nonceBase64[100];

		onvif_calc_nonce(nonce);

		const char *created = onvif_datetime(time(NULL));

		base64_encode((unsigned char*)nonce, NONCELEN, nonceBase64, sizeof(nonceBase64));

		onvif_calc_digest(created, nonce, NONCELEN, p_dev->password, HA);

		base64_encode((unsigned char*)HA, SHA1_SIZE, HABase64, sizeof(HABase64));

		offset += snprintf(xml_bufs+offset, mlen-offset, onvif_xml_header, 
			p_dev->username, HABase64, nonceBase64, created);
	}

	return offset;
}

int build_onvif_req_headerEx(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;

	if (p_dev->username[0] != '\0' && p_dev->password[0] != '\0')
	{
		char HA[SHA1_SIZE], HABase64[100];
		char nonce[NONCELEN], nonceBase64[100];

		onvif_calc_nonce(nonce);

		const char *created = onvif_datetime(time(NULL));

		base64_encode((unsigned char*)nonce, NONCELEN, nonceBase64, sizeof(nonceBase64));

		onvif_calc_digest(created, nonce, NONCELEN, p_dev->password, HA);

		base64_encode((unsigned char*)HA, SHA1_SIZE, HABase64, sizeof(HABase64));
		GetGUID();
		offset += snprintf(xml_bufs+offset, mlen-offset, onvif_xml_headerCreatePullPointSubscriptionRequest, 
			buf, p_dev->username, HABase64, nonceBase64, created, p_dev->devCap.events.xaddr.host, p_dev->devCap.events.xaddr.url);
	}

	return offset;
}

int build_onvif_req_headerExMeaasge(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;

	if (p_dev->username[0] != '\0' && p_dev->password[0] != '\0')
	{
		char HA[SHA1_SIZE], HABase64[100];
		char nonce[NONCELEN], nonceBase64[100];

		onvif_calc_nonce(nonce);

		const char *created = onvif_datetime(time(NULL));

		base64_encode((unsigned char*)nonce, NONCELEN, nonceBase64, sizeof(nonceBase64));

		onvif_calc_digest(created, nonce, NONCELEN, p_dev->password, HA);

		base64_encode((unsigned char*)HA, SHA1_SIZE, HABase64, sizeof(HABase64));
		GetGUID();
		offset += snprintf(xml_bufs+offset, mlen-offset, onvif_xml_headerPullMessage, 
			buf, p_dev->username, HABase64, nonceBase64, created, p_dev->devCap.message.xaddr.host, p_dev->devCap.message.xaddr.url);
	}

	return offset;
}

const char * onvif_get_cap_str(int cap)
{
	switch (cap)
	{
	case CAP_ALL:
		return "All";
		break;

	case CAP_ANALYTICS:
		return "Analytics";
		break;

	case CAP_DEVICE:
		return "Device";
		break;

	case CAP_EVENTS:
		return "Events";
		break;

	case CAP_IMAGING:
		return "Imaging";
		break;

	case CAP_MEDIA:
		return "Media";
		break;

	case CAP_PTZ:
		return "PTZ";
		break;    
	}  

	return NULL;
}

int build_onvif_req_xml_GetDeviceInformation(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<tds:GetDeviceInformation></tds:GetDeviceInformation>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");	
	return offset;
}

int build_onvif_req_xml_GetCapabilities(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	//offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", "xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\"");
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<GetCapabilities xmlns=\"http://www.onvif.org/ver10/device/wsdl\"><Category>%s</Category></GetCapabilities>", 
		onvif_get_cap_str(p_dev->reqCap));
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_GetProfiles(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<trt:GetProfiles></trt:GetProfiles>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_GetStreamUri(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<trt:GetStreamUri>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<trt:StreamSetup>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<tt:Stream>RTP-Unicast</tt:Stream>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<tt:Transport>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<tt:Protocol>RTSP</tt:Protocol>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</tt:Transport>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</trt:StreamSetup>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<trt:ProfileToken>%s</trt:ProfileToken>", p_dev->curProfile->token);
	offset += snprintf(xml_bufs+offset, mlen-offset, "</trt:GetStreamUri>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_GetSnapUri(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<GetSnapshotUri xmlns=\"http://www.onvif.org/ver10/media/wsdl\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<trt:ProfileToken>%s</trt:ProfileToken>", p_dev->curProfile->token);
	offset += snprintf(xml_bufs+offset, mlen-offset, "</GetSnapshotUri>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_GetNetworkInterfaces(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<tds:GetNetworkInterfaces></tds:GetNetworkInterfaces>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_GetVideoEncoderConfigurations(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<trt:GetVideoEncoderConfigurations></trt:GetVideoEncoderConfigurations>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_GetPresets(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<GetPresets xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\" />");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_SetPreset(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<SetPreset xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\" />");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_GotoPreset(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<GotoPreset xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\" />");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_GotoHomePosition(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<GotoHomePosition xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\" />");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_SetHomePosition(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<SetHomePosition xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\" />");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_ContinuousMove(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<ContinuousMove xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<ProfileToken>%s</ProfileToken>", p_dev->curProfile->token);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Velocity>");
	if (p_dev->ptzcmd.ctrlType == PTZ_CTRL_PAN)
	{
		offset += snprintf(xml_bufs+offset, mlen-offset, "<PanTilt x=\"%0.1f\" y=\"%0.1f\" ", 
			p_dev->ptzcmd.panTiltX, p_dev->ptzcmd.panTiltY);
	}
	else if(p_dev->ptzcmd.ctrlType == PTZ_CTRL_ZOOM)
	{
		offset += snprintf(xml_bufs+offset, mlen-offset, "<Zoom x=\"%0.1f\" ", p_dev->ptzcmd.zoom);    	
	}

	offset += snprintf(xml_bufs+offset, mlen-offset, "xmlns=\"http://www.onvif.org/ver10/schema\" />");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</Velocity>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Timeout>PT%dS</Timeout>", p_dev->ptzcmd.duration);
	offset += snprintf(xml_bufs+offset, mlen-offset, "</ContinuousMove>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_AbsoluteMove(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<AbsoluteMove xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<ProfileToken>%s</ProfileToken>", p_dev->curProfile->token);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Position>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<PanTilt x=\"%f\" y=\"%f\" ", p_dev->ptzcmd.panTiltX, p_dev->ptzcmd.panTiltY);
	offset += snprintf(xml_bufs+offset, mlen-offset, "xmlns=\"http://www.onvif.org/ver10/schema\" />");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Zoom x=\"%f\" ", p_dev->ptzcmd.zoom);    	
	offset += snprintf(xml_bufs+offset, mlen-offset, "xmlns=\"http://www.onvif.org/ver10/schema\" />");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</Position>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</AbsoluteMove>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_Stop(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Stop xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<ProfileToken>%s</ProfileToken>", p_dev->curProfile->token);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<PanTilt>%s</PanTilt>", 
		(p_dev->ptzcmd.ctrlType == PTZ_CTRL_PAN) ? "true" : "false");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Zoom>%s</Zoom>", 
		(p_dev->ptzcmd.ctrlType == PTZ_CTRL_ZOOM) ? "true" : "false");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</Stop>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_GetConfigurations(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<GetConfigurations xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\" />");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_GetConfigurationOptions(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<GetConfigurationOptions xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<ConfigurationToken>%s</ConfigurationToken>", p_dev->ptzcfg.token);
	offset += snprintf(xml_bufs+offset, mlen-offset, "</GetConfigurationOptions>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_GetSystemDateAndTime(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<GetSystemDateAndTime xmlns=\"http://www.onvif.org/ver10/device/wsdl\" />");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_SetSystemDateAndTime(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	time_t nowtime;
	struct tm * tmtime;

	time(&nowtime);
	tmtime = gmtime(&nowtime);

	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<SetSystemDateAndTime xmlns=\"http://www.onvif.org/ver10/device/wsdl\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<DateTimeType>Manual</DateTimeType>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<DaylightSavings>false</DaylightSavings>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<UTCDateTime>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Time xmlns=\"http://www.onvif.org/ver10/schema\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Hour>%d</Hour>", tmtime->tm_hour);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Minute>%d</Minute>", tmtime->tm_min);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Second>%d</Second>", tmtime->tm_sec);
	offset += snprintf(xml_bufs+offset, mlen-offset, "</Time>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Date xmlns=\"http://www.onvif.org/ver10/schema\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Year>%d</Year>", tmtime->tm_year+1900);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Month>%d</Month>", tmtime->tm_mon+1);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Day>%d</Day>", tmtime->tm_mday);
	offset += snprintf(xml_bufs+offset, mlen-offset, "</Date>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</UTCDateTime>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</SetSystemDateAndTime>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_SystemReboot(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<tds:SystemReboot/>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_StartRecvAlarm(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	//offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://www.w3.org/2005/08/addressing\">");
	
	offset += build_onvif_req_headerEx(xml_bufs+offset, mlen-offset, p_dev);
	 
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">");
	//offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<CreatePullPointSubscription xmlns=\"http://www.onvif.org/ver10/events/wsdl\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<InitialTerminationTime>PT60S</InitialTerminationTime>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</CreatePullPointSubscription>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_GetAlarm(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://www.w3.org/2005/08/addressing\">");
	offset += build_onvif_req_headerExMeaasge(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<PullMessages xmlns=\"http://www.onvif.org/ver10/events/wsdl\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<Timeout>PT50S</Timeout>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<MessageLimit>1024</MessageLimit>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</PullMessages>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_StopRecvAlarm(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<tds:SystemReboot/>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_GetScopes(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<GetScopes xmlns=\"http://www.onvif.org/ver10/device/wsdl\"/>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml_GetNode(char * xml_bufs, int mlen, onvif_device * p_dev)
{
	int offset = 0;
	offset += snprintf(xml_bufs+offset, mlen-offset, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Envelope %s>", onvif_xml_ns);
	offset += build_onvif_req_header(xml_bufs+offset, mlen-offset, p_dev);
	offset += snprintf(xml_bufs+offset, mlen-offset, "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<GetNode xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">");
	offset += snprintf(xml_bufs+offset, mlen-offset, "<NodeToken>%s</NodeToken>", p_dev->ptzcfg.nodeToken);
	offset += snprintf(xml_bufs+offset, mlen-offset, "</GetNode>");
	offset += snprintf(xml_bufs+offset, mlen-offset, "</s:Body></s:Envelope>");
	return offset;
}

int build_onvif_req_xml(char * xml_bufs, int mlen, eOnvifAction type, onvif_device * p_dev)
{
	int rlen = 0;

	switch(type)
	{
	case eGetDeviceInformation:
		rlen = build_onvif_req_xml_GetDeviceInformation(xml_bufs, mlen, p_dev);
		break;

	case eGetCapabilities:
		rlen = build_onvif_req_xml_GetCapabilities(xml_bufs, mlen, p_dev);
		break;

	case eGetProfiles:
		rlen = build_onvif_req_xml_GetProfiles(xml_bufs, mlen, p_dev);
		break;   

	case eGetStreamUri:
		rlen = build_onvif_req_xml_GetStreamUri(xml_bufs, mlen, p_dev);
		break;

	case eGetNetworkInterfaces:
		rlen = build_onvif_req_xml_GetNetworkInterfaces(xml_bufs, mlen, p_dev);
		break;

	case eGetVideoEncoderConfigurations:
		rlen = build_onvif_req_xml_GetVideoEncoderConfigurations(xml_bufs, mlen, p_dev);
		break;

	case eGetPresets:
		rlen = build_onvif_req_xml_GetPresets(xml_bufs, mlen, p_dev);
		break;

	case eSetPreset:
		rlen = build_onvif_req_xml_SetPreset(xml_bufs, mlen, p_dev);
		break;

	case eGotoPreset:
		rlen = build_onvif_req_xml_GotoPreset(xml_bufs, mlen, p_dev);
		break;

	case eGotoHomePosition:
		rlen = build_onvif_req_xml_GotoHomePosition(xml_bufs, mlen, p_dev);
		break;

	case eSetHomePosition:
		rlen = build_onvif_req_xml_SetHomePosition(xml_bufs, mlen, p_dev);
		break;

	case eContinuousMove:
		rlen = build_onvif_req_xml_ContinuousMove(xml_bufs, mlen, p_dev);
		break;

	case eAbsoluteMove:
		rlen = build_onvif_req_xml_AbsoluteMove(xml_bufs, mlen, p_dev);
		break;

	case eStop:
		rlen = build_onvif_req_xml_Stop(xml_bufs, mlen, p_dev);
		break;

	case eGetConfigurations:
		rlen = build_onvif_req_xml_GetConfigurations(xml_bufs, mlen, p_dev);
		break;

	case eGetConfigurationOptions:
		rlen = build_onvif_req_xml_GetConfigurationOptions(xml_bufs, mlen, p_dev);
		break;

	case eGetSystemDateAndTime:
		rlen = build_onvif_req_xml_GetSystemDateAndTime(xml_bufs, mlen, p_dev);
		break;

	case eSetSystemDateAndTime:
		rlen = build_onvif_req_xml_SetSystemDateAndTime(xml_bufs, mlen, p_dev);
		break;

	case eSystemReboot:
		rlen = build_onvif_req_xml_SystemReboot(xml_bufs, mlen, p_dev);
		break;

	case eGetSnapUri:
		rlen = build_onvif_req_xml_GetSnapUri(xml_bufs, mlen, p_dev);
		break;

	case eStartRecvAlarm:
		rlen = build_onvif_req_xml_StartRecvAlarm(xml_bufs, mlen, p_dev);
		break;

	case eRecvAlarm:
		rlen = build_onvif_req_xml_GetAlarm(xml_bufs, mlen, p_dev);
		break;

	case eStopRecvAlarm:
		rlen = build_onvif_req_xml_StopRecvAlarm(xml_bufs, mlen, p_dev);
		break;

	case eGetScopes:
		rlen = build_onvif_req_xml_GetScopes(xml_bufs, mlen, p_dev);
		break;

	case eGetNode:
		rlen = build_onvif_req_xml_GetNode(xml_bufs, mlen, p_dev);

	default:
		break;
	}

	return rlen;
}


/*---------------------------------------------------------------------*/
XMLN * xml_attr_add(XMLN * p_node, const char * name, const char * value)
{
	if(p_node == NULL || name == NULL || value == NULL)
		return NULL;

	XMLN * p_attr = (XMLN *)malloc(sizeof(XMLN));
	if(p_attr == NULL)
	{
		//log_print("xml_attr_add::memory alloc fail!!!\r\n");
		return NULL;
	}

	memset(p_attr, 0, sizeof(XMLN));

	p_attr->type = NTYPE_ATTRIB;
	p_attr->name = name;	//strdup(name);
	p_attr->data = value;	//strdup(value);
	p_attr->dlen = strlen(value);

	if(p_node->f_attrib == NULL)
	{
		p_node->f_attrib = p_attr;
		p_node->l_attrib = p_attr;
	}
	else
	{
		p_attr->prev = p_node->l_attrib;
		p_node->l_attrib->next = p_attr;
		p_node->l_attrib = p_attr;
	}

	return p_attr;
}

void xml_attr_del(XMLN * p_node, const char * name)
{
	if(p_node == NULL || name == NULL)
		return;

	XMLN * p_attr = p_node->f_attrib;
	while(p_attr != NULL)
	{
		//		if(strcasecmp(p_attr->name,name) == 0)
		if(strcmp(p_attr->name,name) == 0)
		{
			xml_node_del(p_attr);
			return;
		}

		p_attr = p_attr->next;
	}
}

const char * xml_attr_get(XMLN * p_node, const char * name)
{
	if(p_node == NULL || name == NULL)
		return NULL;

	XMLN * p_attr = p_node->f_attrib;
	while(p_attr != NULL)
	{
		//		if ((NTYPE_ATTRIB == p_attr->type) && (0 == strcasecmp(p_attr->name,name)))
		if ((NTYPE_ATTRIB == p_attr->type) && (0 == strcmp(p_attr->name,name)))
			return p_attr->data;

		p_attr = p_attr->next;
	}

	return NULL;
}

XMLN * xml_attr_node_get(XMLN * p_node, const char * name)
{
	if(p_node == NULL || name == NULL)
		return NULL;

	XMLN * p_attr = p_node->f_attrib;
	while(p_attr != NULL)
	{
		//		if ((NTYPE_ATTRIB == p_attr->type) && (0 == strcasecmp(p_attr->name,name)))
		if ((NTYPE_ATTRIB == p_attr->type) && (0 == strcmp(p_attr->name,name)))
			return p_attr;

		p_attr = p_attr->next;
	}

	return NULL;
}

/*---------------------------------------------------------------------*/
void xml_cdata_set(XMLN * p_node, const char * value, int len)
{
	if(p_node == NULL || value == NULL || len <= 0)
		return;

	p_node->data = value;
	p_node->dlen = len;
}

/*---------------------------------------------------------------------*/
int xml_calc_buf_len(XMLN * p_node)
{
	int xml_len = 0;
	xml_len += 1 + strlen(p_node->name);	//sprintf(xml_buf+xml_len, "<%s",p_node->name);

	XMLN * p_attr = p_node->f_attrib;
	while(p_attr)
	{
		if(p_attr->type == NTYPE_ATTRIB)
			xml_len += strlen(p_attr->name) + 4 + strlen(p_attr->data);	//sprintf(xml_buf+xml_len," %s=\"%s\"",p_attr->name,p_attr->data);
		else if(p_attr->type == NTYPE_CDATA)
		{
			xml_len += 1 + strlen(p_attr->data) + 2 + strlen(p_node->name) + 1;	//sprintf(xml_buf+xml_len,">%s</%s>",p_attr->data,p_node->name);
			return xml_len;
		}
		else
			;

		p_attr = p_attr->next;
	}

	if(p_node->f_child)
	{
		xml_len += 1;	//sprintf(xml_buf+xml_len, ">");
		XMLN * p_child = p_node->f_child;
		while(p_child)
		{
			xml_len += xml_calc_buf_len(p_child);	//xml_write_buf(p_child,xml_buf+xml_len);
			p_child = p_child->next;
		}

		xml_len += 2 + strlen(p_node->name) + 1;	//sprintf(xml_buf+xml_len, "</%s>",p_node->name);
	}
	else
	{
		xml_len += 2;	//sprintf(xml_buf+xml_len, "/>");
	}

	return xml_len;
}

/*---------------------------------------------------------------------*/
int xml_write_buf(XMLN * p_node, char * xml_buf)
{
	int xml_len = 0;

	xml_len += sprintf(xml_buf+xml_len, "<%s",p_node->name);

	XMLN * p_attr = p_node->f_attrib;
	while(p_attr)
	{
		if(p_attr->type == NTYPE_ATTRIB)
			xml_len += sprintf(xml_buf+xml_len," %s=\"%s\"",p_attr->name,p_attr->data);
		else if(p_attr->type == NTYPE_CDATA)
		{
			xml_len += sprintf(xml_buf+xml_len,">%s</%s>",p_attr->data,p_node->name);
			return xml_len;
		}
		else
			;

		p_attr = p_attr->next;
	}

	if(p_node->f_child)
	{
		xml_len += sprintf(xml_buf+xml_len, ">");
		XMLN * p_child = p_node->f_child;
		while(p_child)
		{
			xml_len += xml_write_buf(p_child,xml_buf+xml_len);
			p_child = p_child->next;
		}

		xml_len += sprintf(xml_buf+xml_len, "</%s>",p_node->name);
	}
	else
	{
		xml_len += sprintf(xml_buf+xml_len, "/>");
	}

	return xml_len;
}

/*---------------------------------------------------------------------*/
int xml_nwrite_buf(XMLN * p_node, char * xml_buf, int buf_len)
{
	int xml_len = 0;

	if ((NULL == p_node) || (NULL == p_node->name))
		return -1;

	if (strlen(p_node->name) >= (unsigned int)buf_len)
		return -1;

	xml_len += sprintf(xml_buf+xml_len, "<%s",p_node->name);

	XMLN * p_attr = p_node->f_attrib;
	while(p_attr)
	{
		if(p_attr->type == NTYPE_ATTRIB)
		{
			if ((strlen(p_attr->name) + strlen(p_attr->data) + xml_len) > (unsigned int)buf_len)
				return -1;
			xml_len += sprintf(xml_buf+xml_len," %s=\"%s\"",p_attr->name,p_attr->data);
		}
		else if(p_attr->type == NTYPE_CDATA)
		{
			if (0x0a == (*p_attr->data))
			{
				p_attr = p_attr->next;
				continue;
			}
			if ((strlen(p_attr->data) + strlen(p_node->name) + xml_len) >= (unsigned int)buf_len)
				return -1;
			xml_len += sprintf(xml_buf+xml_len,">%s</%s>",p_attr->data,p_node->name);
			return xml_len;
		}
		else
			;

		p_attr = p_attr->next;
	}

	int ret = 0;
	if(p_node->f_child)
	{
		xml_len += sprintf(xml_buf+xml_len, ">");
		XMLN * p_child = p_node->f_child;
		while(p_child)
		{
			ret = xml_nwrite_buf(p_child, xml_buf+xml_len, buf_len-xml_len);
			if (ret < 0)
				return ret;
			xml_len += ret;
			p_child = p_child->next;
		}

		xml_len += sprintf(xml_buf+xml_len, "</%s>",p_node->name);
	}
	else
	{
		xml_len += sprintf(xml_buf+xml_len, "/>");
	}

	return xml_len;
}

/***********************************************************************/
XMLN * xml_node_add(XMLN * parent, char * name)
{
	XMLN * p_node = (XMLN *)malloc(sizeof(XMLN));
	if(p_node == NULL)
	{
		//log_print("xml_node_add::memory alloc fail!!!\r\n");
		return NULL;
	}

	memset(p_node, 0, sizeof(XMLN));

	p_node->type = NTYPE_TAG;
	p_node->name = name;	//strdup(name);

	if(parent != NULL)
	{
		p_node->parent = parent;
		if(parent->f_child == NULL)
		{
			parent->f_child = p_node;
			parent->l_child = p_node;
		}
		else
		{
			parent->l_child->next = p_node;
			p_node->prev = parent->l_child;
			parent->l_child = p_node;
		}
	}

	return p_node;
}

XMLN * xml_node_add_by_struct_at_last(XMLN * parent, XMLN * pdest)
{
	if((parent == NULL)||(pdest == NULL))
	{
		//log_print("xml_node_add_by_struct_at_last:: parent or pdest is NULL\r\n");
		return NULL;
	}
	pdest->parent = parent;
	if(parent->f_child == NULL)
	{
		parent->f_child = pdest;
		parent->l_child = pdest;
	}
	else
	{
		parent->l_child->next = pdest;
		pdest->prev = parent->l_child;
		parent->l_child = pdest;
	}
	return pdest;
}

void xml_node_del(XMLN * p_node)
{
	if(p_node == NULL)return;

	XMLN * p_attr = p_node->f_attrib;
	while(p_attr)
	{
		XMLN * p_next = p_attr->next;
		//	if(p_attr->data) free(p_attr->data);
		//	if(p_attr->name) free(p_attr->name);

		free(p_attr);

		p_attr = p_next;
	}

	XMLN * p_child = p_node->f_child;
	while(p_child)
	{
		XMLN * p_next = p_child->next;
		xml_node_del(p_child);
		p_child = p_next;
	}

	if(p_node->prev) p_node->prev->next = p_node->next;
	if(p_node->next) p_node->next->prev = p_node->prev;

	if(p_node->parent)
	{
		if(p_node->parent->f_child == p_node)
			p_node->parent->f_child = p_node->next;
		if(p_node->parent->l_child == p_node)
			p_node->parent->l_child = p_node->prev;
	}

	//	if(p_node->name) free(p_node->name);
	//	if(p_node->data) free(p_node->data);

	free(p_node);
}

XMLN * xml_node_get(XMLN * parent, const char * name)
{
	if(parent == NULL || name == NULL)
		return NULL;

	XMLN * p_node = parent->f_child;
	while(p_node != NULL)
	{
		//		if(strcasecmp(p_node->name,name) == 0)
		if(strcmp(p_node->name,name) == 0)
			return p_node;

		p_node = p_node->next;
	}

	return NULL;
}

int soap_strcmp(const char * str1, const char * str2)
{
	// 	if(strcasecmp(str1, str2) == 0)
	// 		return 0;
	// 
	// 	const char * ptr1 = strchr(str1, ':');
	// 	const char * ptr2 = strchr(str2, ':');
	// 	if(ptr1 && ptr2)
	// 		return strcasecmp(ptr1+1, ptr2+1);
	// 	else if(ptr1)
	// 		return strcasecmp(ptr1+1, str2);
	// 	else if(ptr2)
	// 		return strcasecmp(str1, ptr2+1);
	// 	else
	// 		return -1;
	if(strcmp(str1, str2) == 0)
		return 0;

	const char * ptr1 = strchr(str1, ':');
	const char * ptr2 = strchr(str2, ':');
	if(ptr1 && ptr2)
		return strcmp(ptr1+1, ptr2+1);
	else if(ptr1)
		return strcmp(ptr1+1, str2);
	else if(ptr2)
		return strcmp(str1, ptr2+1);
	else
		return -1;
}

XMLN * xml_node_soap_get(XMLN * parent, const char * name)
{
	if(parent == NULL || name == NULL)
		return NULL;

	XMLN * p_node = parent->f_child;
	while(p_node != NULL)
	{
		if(soap_strcmp(p_node->name,name) == 0)
			return p_node;

		p_node = p_node->next;
	}

	return NULL;
}

BOOL onvif_rly_GetDeviceInformation(XMLN * p_xml, onvif_device * p_dev)
{
	XMLN * p_res = xml_node_soap_get(p_xml, "tds:GetDeviceInformationResponse");
	if (NULL == p_res)
	{
		return FALSE;
	}

	XMLN * p_manu = xml_node_soap_get(p_res, "tds:Manufacturer");
	if (p_manu && p_manu->data)
	{   
		strncpy(p_dev->devInfo.Manufacturer, p_manu->data, sizeof(p_dev->devInfo.Manufacturer)-1);
	}

	XMLN * p_model = xml_node_soap_get(p_res, "tds:Model");
	if (p_model && p_model->data)
	{   
		strncpy(p_dev->devInfo.Model, p_model->data, sizeof(p_dev->devInfo.Model)-1);
	}

	XMLN * p_fmv = xml_node_soap_get(p_res, "tds:FirmwareVersion");
	if (p_fmv && p_fmv->data)
	{   
		strncpy(p_dev->devInfo.FirmwareVersion, p_fmv->data, sizeof(p_dev->devInfo.FirmwareVersion)-1);
	}

	XMLN * p_sn = xml_node_soap_get(p_res, "tds:SerialNumber");
	if (p_sn && p_sn->data)
	{   
		strncpy(p_dev->devInfo.SerialNumber, p_sn->data, sizeof(p_dev->devInfo.SerialNumber)-1);
	}

	XMLN * p_hd = xml_node_soap_get(p_res, "tds:HardwareId");
	if (p_hd && p_hd->data)
	{   
		strncpy(p_dev->devInfo.HardwareId, p_hd->data, sizeof(p_dev->devInfo.HardwareId)-1);
	}

	return TRUE;
}

int onvif_parse_type(const char * pdata)
{
	if (soap_strcmp(pdata, "dn:NetworkVideoTransmitter") == 0)
	{
		return ODT_NVT;
	}
	else if (soap_strcmp(pdata, "dn:NetworkVideoDisplay") == 0)
	{
		return ODT_NVD;
	}
	else if (soap_strcmp(pdata, "dn:NetworkVideoStorage") == 0)
	{
		return ODT_NVS;
	}
	else if (soap_strcmp(pdata, "dn:NetworkVideoAnalytics") == 0)
	{
		return ODT_NVA;
	}

	return ODT_NVT;
}

// int onvif_parse_xaddr(const char * pdata, onvif_xaddr * p_xaddr)
// {
// 	const char *p1, *p2;
// 	int len = strlen(pdata);
// 
// 	p_xaddr->port = 80;
// 
// 	if (len > 7) // skip "http://"
// 	{
// 		p1 = strchr(pdata+7, ':');
// 		if (p1)
// 		{
// 			strncpy(p_xaddr->host, pdata+7, p1-pdata-7);
// 
// 			char buff[100];
// 			memset(buff, 0, 100);
// 
// 			p2 = strchr(p1, '/');
// 			if (p2)
// 			{
// 				strncpy(p_xaddr->url, p2, len - (p2 - pdata));
// 
// 				len = p2 - p1 - 1;
// 				strncpy(buff, p1+1, len);                
// 			}
// 			else
// 			{
// 				len = len - (p1 - pdata);
// 				strncpy(buff, p1+1, len);
// 			}  
// 
// 			p_xaddr->port = atoi(buff);
// 		}
// 		else
// 		{
// 			p2 = strchr(pdata+7, '/');
// 			if (p2)
// 			{
// 				strncpy(p_xaddr->url, p2, len - (p2 - pdata));
// 
// 				len = p2 - pdata - 7;
// 				strncpy(p_xaddr->host, pdata+7, len);
// 			}
// 			else
// 			{
// 				len = len - 7;
// 				strncpy(p_xaddr->host, pdata+7, len);
// 			}
// 		}
// 	}
// 
// 	return 1;
// }

int onvif_parse_xaddr(const char * pdata, onvif_xaddr * p_xaddr)
{
	const char *p2;
	int len = strlen(pdata);

	p_xaddr->port = 80;

	if (len > 7) // skip "http://"
	{
		p2 = strchr(pdata+7, '/');
		if (p2)
		{
			char szHost[24] = {0};
			strncpy(szHost, pdata+7, (p2 - pdata - 7));
			char *p3 = strchr(szHost, ':');
			if (p3)
			{
				strncpy(p_xaddr->host, szHost, p3 - szHost);
				p_xaddr->port = atoi(p3+1);
			}
			else
			{
				strncpy(p_xaddr->host, szHost, (p2 - pdata - 7));
			}
			strncpy(p_xaddr->url, p2, len - (p2- pdata));

// 			len = p2 - pdata - 7;
// 			strncpy(p_xaddr->host, pdata+7, len);
		}
		else
		{
			len = len - 7;
			strncpy(p_xaddr->host, pdata+7, len);
		}

// 		p1 = strchr(pdata+7, ':');
// 		if (p1)
// 		{
// 			strncpy(p_xaddr->host, pdata+7, p1-pdata-7);
// 
// 			char buff[100];
// 			memset(buff, 0, 100);
// 
// 			p2 = strchr(p1, '/');
// 			if (p2)
// 			{
// 				strncpy(p_xaddr->url, p2, len - (p2 - pdata));
// 
// 				len = p2 - p1 - 1;
// 				strncpy(buff, p1+1, len);                
// 			}
// 			else
// 			{
// 				len = len - (p1 - pdata);
// 				strncpy(buff, p1+1, len);
// 			}  
// 
// 			p_xaddr->port = atoi(buff);
// 		}
// 		else
// 		{
// 	
// 		}
	}

	return 1;
}

BOOL onvif_parse_bool(const char * pdata)
{
	if (NULL == pdata)
	{
		return FALSE;
	}

	//    if (strcasecmp(pdata, "true") == 0)
	if (strcmp(pdata, "true") == 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


int onvif_parse_encoding(const char * pdata)
{
	//     if (strcasecmp(pdata, "H264") == 0)
	//     {
	//         return VIDEO_ENCODING_H264;
	//     }
	//     else if (strcasecmp(pdata, "JPEG") == 0)
	//     {
	//         return VIDEO_ENCODING_JPEG;
	//     }
	//     else if (strcasecmp(pdata, "MPEG4") == 0)
	//     {
	//         return VIDEO_ENCODING_MPEG4;
	//     }
	if (strcmp(pdata, "H264") == 0)
	{
		return VIDEO_ENCODING_H264;
	}
	else if (strcmp(pdata, "JPEG") == 0)
	{
		return VIDEO_ENCODING_JPEG;
	}
	else if (strcmp(pdata, "MPEG4") == 0)
	{
		return VIDEO_ENCODING_MPEG4;
	}
	return VIDEO_ENCODING_UNKNOWN;
}

int onvif_parse_h264_profile(const char * pdata)
{
	//     if (strcasecmp(pdata, "Baseline") == 0)
	//     {
	//         return H264_PROFILE_Baseline;
	//     }
	//     else if (strcasecmp(pdata, "High") == 0)
	//     {
	//         return H264_PROFILE_High;
	//     }
	//     else if (strcasecmp(pdata, "Main") == 0)
	//     {
	//         return H264_PROFILE_Main;
	//     }
	//     else if (strcasecmp(pdata, "Extended") == 0)
	//     {
	//         return H264_PROFILE_Extended;
	//     }
	if (strcmp(pdata, "Baseline") == 0)
	{
		return H264_PROFILE_Baseline;
	}
	else if (strcmp(pdata, "High") == 0)
	{
		return H264_PROFILE_High;
	}
	else if (strcmp(pdata, "Main") == 0)
	{
		return H264_PROFILE_Main;
	}
	else if (strcmp(pdata, "Extended") == 0)
	{
		return H264_PROFILE_Extended;
	}
	return H264_PROFILE_Baseline;
}

int onvif_parse_time(const char * pdata)
{

	return atoi(pdata+2);
}

BOOL onvif_parse_media_cap(XMLN * p_media, onvif_device * p_dev)
{
	XMLN * p_xaddr = xml_node_soap_get(p_media, "tt:XAddr");
	if (p_xaddr && p_xaddr->data)
	{
		onvif_parse_xaddr(p_xaddr->data, &p_dev->devCap.media.xaddr);
	}
	else
	{
		return FALSE;
	}

	XMLN * p_cap = xml_node_soap_get(p_media, "tt:StreamingCapabilities");
	if (NULL == p_cap)
	{
		return FALSE;
	}

	XMLN * p_mc = xml_node_soap_get(p_cap, "tt:RTPMulticast");
	if (p_mc && p_mc->data)
	{
		p_dev->devCap.media.RTPMulticast = onvif_parse_bool(p_mc->data);
	}

	XMLN * p_rtp_tcp = xml_node_soap_get(p_cap, "tt:RTP_TCP");
	if (p_rtp_tcp && p_rtp_tcp->data)
	{
		p_dev->devCap.media.RTP_TCP = onvif_parse_bool(p_rtp_tcp->data);;
	}

	XMLN * p_rtp_rtsp_tcp = xml_node_soap_get(p_cap, "RTP_RTSP_TCP");
	if (p_rtp_rtsp_tcp && p_rtp_rtsp_tcp->data)
	{
		p_dev->devCap.media.RTP_RTSP_TCP = onvif_parse_bool(p_rtp_rtsp_tcp->data);;
	}

	return TRUE;
}

BOOL onvif_parse_ptz_cap(XMLN * p_ptz, onvif_device * p_dev)
{
	XMLN * p_xaddr = xml_node_soap_get(p_ptz, "tt:XAddr");
	if (p_xaddr && p_xaddr->data)
	{
		onvif_parse_xaddr(p_xaddr->data, &p_dev->devCap.ptz.xaddr);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL onvif_parse_events_cap(XMLN * p_events, onvif_device * p_dev)
{
	XMLN * p_xaddr = xml_node_soap_get(p_events, "tt:XAddr");
	if (p_xaddr && p_xaddr->data)
	{
		onvif_parse_xaddr(p_xaddr->data, &p_dev->devCap.events.xaddr);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL onvif_rly_GetCapabilities(XMLN * p_xml, onvif_device * p_dev)
{
	XMLN * p_res = xml_node_soap_get(p_xml, "tds:GetCapabilitiesResponse");
	if (NULL == p_res)
	{
		return FALSE;
	}

	XMLN * p_cap = xml_node_soap_get(p_res, "tds:Capabilities");
	if (NULL == p_cap)
	{
		return FALSE;
	}

	XMLN * p_media = xml_node_soap_get(p_cap, "tt:Media");
	if (p_media)
	{
		p_dev->devCap.media.support = onvif_parse_media_cap(p_media, p_dev);
	}

	XMLN * p_ptz = xml_node_soap_get(p_cap, "tt:PTZ");
	if (p_ptz)
	{
		p_dev->devCap.ptz.support = onvif_parse_ptz_cap(p_ptz, p_dev);
	}

	XMLN* p_events = xml_node_soap_get(p_cap, "tt:Events");
	if (p_events)
	{
		p_dev->devCap.events.support = onvif_parse_events_cap(p_events, p_dev);
	}
	return TRUE;
}

BOOL onvif_parse_video_source(XMLN * p_node, video_source * p_video_src)
{
	XMLN * p_name = xml_node_soap_get(p_node, "tt:Name");
	if (p_name && p_name->data)
	{
		strncpy(p_video_src->stream_name, p_name->data, NAME_LEN);
	}
	else
	{
		return FALSE;
	}

	XMLN * p_usecount = xml_node_soap_get(p_node, "tt:UseCount");
	if (p_usecount && p_usecount->data)
	{
		p_video_src->use_count = atoi(p_usecount->data);
	}

	XMLN * p_token = xml_node_soap_get(p_node, "tt:SourceToken");
	if (p_token && p_token->data)
	{
		strncpy(p_video_src->source_token, p_name->data, TOKEN_LEN);
	}
	else
	{
		return FALSE;
	}

	XMLN * p_bounds = xml_node_soap_get(p_node, "tt:Bounds");
	if (p_bounds)
	{
		p_video_src->height = atoi(xml_attr_get(p_bounds, "height"));
		p_video_src->width = atoi(xml_attr_get(p_bounds, "width"));
		p_video_src->x = atoi(xml_attr_get(p_bounds, "x"));
		p_video_src->y = atoi(xml_attr_get(p_bounds, "y"));
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL onvif_parse_video_encoder(XMLN * p_node, video_encoder * p_video_enc)
{
	XMLN * p_name = xml_node_soap_get(p_node, "tt:Name");
	if (p_name && p_name->data)
	{
		strncpy(p_video_enc->name, p_name->data, NAME_LEN);
	}
	else
	{
		return FALSE;
	}

	XMLN * p_usecount = xml_node_soap_get(p_node, "tt:UseCount");
	if (p_usecount && p_usecount->data)
	{
		p_video_enc->use_count = atoi(p_usecount->data);
	}

	XMLN * p_encoding = xml_node_soap_get(p_node, "tt:Encoding");
	if (p_encoding && p_encoding->data)
	{
		p_video_enc->encoding = onvif_parse_encoding(p_encoding->data);
	}
	else
	{
		return FALSE;
	}

	XMLN * p_resolution = xml_node_soap_get(p_node, "tt:Resolution");
	if (p_resolution)
	{
		XMLN * p_width = xml_node_soap_get(p_resolution, "tt:Width");
		if (p_width && p_width->data)
		{
			p_video_enc->width = atoi(p_width->data);
		}

		XMLN * p_height = xml_node_soap_get(p_resolution, "tt:Height");
		if (p_height && p_height->data)
		{
			p_video_enc->height = atoi(p_height->data);
		}
	}

	XMLN * p_quality = xml_node_soap_get(p_node, "tt:Quality");
	if (p_quality && p_quality->data)
	{
		p_video_enc->quality = (float)atoi(p_quality->data);
	}

	XMLN * p_rate_ctl = xml_node_soap_get(p_node, "tt:RateControl");
	if (p_rate_ctl)
	{
		XMLN * p_fr_limit = xml_node_soap_get(p_rate_ctl, "tt:FrameRateLimit");
		if (p_fr_limit && p_fr_limit->data)
		{
			p_video_enc->framerate_limit = atoi(p_fr_limit->data);
		}

		XMLN * p_en_int = xml_node_soap_get(p_rate_ctl, "tt:EncodingInterval");
		if (p_en_int && p_en_int->data)
		{
			p_video_enc->encoding_interval = atoi(p_en_int->data);
		}

		XMLN * p_bt_limit = xml_node_soap_get(p_rate_ctl, "tt:BitrateLimit");
		if (p_bt_limit && p_bt_limit->data)
		{
			p_video_enc->bitrate_limit = atoi(p_bt_limit->data);
		}
	}

	if (p_video_enc->encoding == VIDEO_ENCODING_H264)
	{
		XMLN * p_h264 = xml_node_soap_get(p_node, "tt:H264");
		if (p_h264)
		{
			XMLN * p_gov_len = xml_node_soap_get(p_h264, "tt:GovLength");
			if (p_gov_len && p_gov_len->data)
			{
				p_video_enc->gov_len = atoi(p_gov_len->data);
			}

			XMLN * p_h264_profile = xml_node_soap_get(p_h264, "tt:H264Profile");
			if (p_h264_profile && p_h264_profile->data)
			{
				p_video_enc->h264_profile = onvif_parse_h264_profile(p_h264_profile->data);
			}
		}    
	}

	XMLN * p_time = xml_node_soap_get(p_node, "tt:SessionTimeout");
	if (p_time && p_time->data)
	{
		p_video_enc->session_timeout = onvif_parse_time(p_time->data);
	}

	return TRUE;
}

BOOL onvif_parse_ptz_cfg(XMLN * p_node, ptz_cfg * p_ptz_cfg)
{
	XMLN * p_name = xml_node_soap_get(p_node, "tt:Name");
	if (p_name && p_name->data)
	{
		strncpy(p_ptz_cfg->name, p_name->data, NAME_LEN);
	}
	else
	{
		return FALSE;
	}

	XMLN * p_usecount = xml_node_soap_get(p_node, "tt:UseCount");
	if (p_usecount && p_usecount->data)
	{
		p_ptz_cfg->use_count = atoi(p_usecount->data);
	}

	XMLN * p_node_token = xml_node_soap_get(p_node, "tt:NodeToken");
	if (p_node_token && p_node_token->data)
	{
		strncpy(p_ptz_cfg->nodeToken, p_node_token->data, TOKEN_LEN);
	}

	XMLN * p_def_speed = xml_node_soap_get(p_node, "tt:DefaultPTZSpeed");
	if (p_def_speed)
	{
		XMLN * p_pantilt = xml_node_soap_get(p_def_speed, "tt:PanTilt");
		if (p_pantilt)
		{
			p_ptz_cfg->def_speed.pan_tilt_x = atoi(xml_attr_get(p_pantilt, "x"));
			p_ptz_cfg->def_speed.pan_tilt_y = atoi(xml_attr_get(p_pantilt, "y"));
		}

		XMLN * p_zoom = xml_node_soap_get(p_def_speed, "tt:Zoom");
		if (p_zoom)
		{
			p_ptz_cfg->def_speed.zoom = atoi(xml_attr_get(p_zoom, "x"));
		}
	}

	XMLN * p_time = xml_node_soap_get(p_node, "tt:DefaultPTZTimeout");
	if (p_time && p_time->data)
	{
		p_ptz_cfg->def_timeout = onvif_parse_time(p_time->data);
	}

	return TRUE;
}

BOOL onvif_parse_profiles(XMLN * p_profiles, onvif_profile * profile)
{
	XMLN * p_name = xml_node_soap_get(p_profiles, "tt:Name");
	if (p_name && p_name->data)
	{
		strncpy(profile->name, p_name->data, NAME_LEN);
	}
	else
	{
		return FALSE;
	}    

	XMLN * p_video_src = xml_node_soap_get(p_profiles, "tt:VideoSourceConfiguration");
	if (p_video_src)
	{
		profile->p_video_source = (video_source *) malloc(sizeof(video_source));
		memset(profile->p_video_source, 0, sizeof(video_source));

		strncpy(profile->p_video_source->token, xml_attr_get(p_video_src, "token"), TOKEN_LEN);

		if (!onvif_parse_video_source(p_video_src, profile->p_video_source))
		{
			free(profile->p_video_source);
			profile->p_video_source = NULL;
		}
	}

	XMLN * p_video_enc = xml_node_soap_get(p_profiles, "tt:VideoEncoderConfiguration");
	if (p_video_enc)
	{
		profile->p_video_encoder = (video_encoder *) malloc(sizeof(video_encoder));
		memset(profile->p_video_encoder, 0, sizeof(video_encoder));

		strncpy(profile->p_video_encoder->token, xml_attr_get(p_video_enc, "token"), TOKEN_LEN);

		if (!onvif_parse_video_encoder(p_video_enc, profile->p_video_encoder))
		{
			free(profile->p_video_encoder);
			profile->p_video_encoder = NULL;            
		}
	}

	XMLN * p_ptz_cfg = xml_node_soap_get(p_profiles, "tt:PTZConfiguration");
	if (p_ptz_cfg)
	{
		profile->p_ptz_cfg = (ptz_cfg *) malloc(sizeof(ptz_cfg));
		memset(profile->p_ptz_cfg, 0, sizeof(ptz_cfg));

		strncpy(profile->p_ptz_cfg->token, xml_attr_get(p_ptz_cfg, "token"), TOKEN_LEN);

		if (!onvif_parse_ptz_cfg(p_ptz_cfg, profile->p_ptz_cfg))
		{
			free(profile->p_ptz_cfg);
			profile->p_ptz_cfg = NULL;            
		}
	}

	return TRUE;
}

BOOL onvif_rly_GetProfiles(XMLN * p_xml, onvif_device * p_dev)
{
	XMLN * p_res = xml_node_soap_get(p_xml, "trt:GetProfilesResponse");
	if (NULL == p_res)
	{
		return FALSE;
	}

	XMLN * p_profiles = xml_node_soap_get(p_res, "trt:Profiles");
	while (p_profiles)
	{
		onvif_profile * profile = (onvif_profile *) malloc(sizeof(onvif_profile));
		if (NULL == profile)
		{
			return FALSE;
		}
		memset(profile, 0, sizeof(onvif_profile));

		profile->fixed = onvif_parse_bool(xml_attr_get(p_profiles, "fixed"))==TRUE;
		strncpy(profile->token, xml_attr_get(p_profiles, "token"), TOKEN_LEN);

		if (onvif_parse_profiles(p_profiles, profile))
		{
			if (NULL == p_dev->profile)
			{
				p_dev->profile = profile;
			}
			else
			{
				onvif_profile * p = p_dev->profile;
				while (p && p->next)
				{
					p = p->next;
				}

				p->next = profile;
			}
		}
		else
		{
			free(profile);
		}

		p_profiles = p_profiles->next;
	}

	return TRUE;
}

BOOL onvif_rly_GetStreamUri(XMLN * p_xml, onvif_device * p_dev)
{
	XMLN * p_res = xml_node_soap_get(p_xml, "trt:GetStreamUriResponse");
	if (NULL == p_res)
	{
		return FALSE;
	}

	XMLN * p_media_uri = xml_node_soap_get(p_res, "trt:MediaUri");
	if (p_media_uri)
	{
		XMLN * p_uri = xml_node_soap_get(p_media_uri, "trt:Uri");
		if (p_uri && p_uri->data && p_dev->curProfile)
		{
			strncpy(p_dev->curProfile->stream_uri, p_uri->data, URI_LEN);
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL onvif_rly_GetSnapUri(XMLN * p_xml, onvif_device * p_dev)
{
	XMLN * p_res = xml_node_soap_get(p_xml, "trt:GetSnapshotUriResponse");
	if (NULL == p_res)
	{
		return FALSE;
	}

	XMLN * p_media_uri = xml_node_soap_get(p_res, "trt:MediaUri");
	if (p_media_uri)
	{
		XMLN * p_uri = xml_node_soap_get(p_media_uri, "trt:Uri");
		if (p_uri && p_uri->data && p_dev->curProfile)
		{
			strncpy(p_dev->curProfile->snap_uri, p_uri->data, URI_LEN);
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL onvif_rly_CreatePullPointSubscriptionRequest(XMLN * p_xml, onvif_device * p_dev)
{
	XMLN * p_res = xml_node_soap_get(p_xml, "tev:CreatePullPointSubscriptionResponse");
	if (NULL == p_res)
	{
		return FALSE;
	}

	XMLN * p_media_uri = xml_node_soap_get(p_res, "tev:SubscriptionReference");
	if (p_media_uri)
	{
		XMLN * p_uri = xml_node_soap_get(p_media_uri, "wsa5:Address");
		if (p_uri && p_uri->data)
		{
			onvif_parse_xaddr(p_uri->data, &p_dev->devCap.message.xaddr);
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL onvif_rly_GetNode(XMLN * p_xml, onvif_device * p_dev, const char* pBuf)
{
	XMLN * p_res = xml_node_soap_get(p_xml, "tptz:GetNodeResponse");
	if (NULL == p_res)
	{
		return FALSE;
	}

	XMLN * PTZNode = xml_node_soap_get(p_res, "tptz:PTZNode");
	if (PTZNode)
	{
		XMLN * p_Space = xml_node_soap_get(PTZNode, "tt:SupportedPTZSpaces");
		if (p_Space)
		{
			XMLN * p_AbsolutePanTiltPositionSpace = xml_node_soap_get(p_Space, "tt:AbsolutePanTiltPositionSpace");
			if (p_AbsolutePanTiltPositionSpace)
			{
				XMLN * p_XRange = xml_node_soap_get(p_AbsolutePanTiltPositionSpace, "tt:XRange");
				if (p_XRange)
				{
					XMLN * p_XMin = xml_node_soap_get(p_XRange, "tt:Min");
					if (p_XMin)
					{
						p_dev->ptzcfg.pantilt_x.min = (float)atof(p_XMin->data);
					}
					XMLN * p_XMax = xml_node_soap_get(p_XRange, "tt:Max");
					if (p_XMax)
					{
						p_dev->ptzcfg.pantilt_x.max = (float)atof(p_XMax->data);
					}
				}

				XMLN * p_YRange = xml_node_soap_get(p_AbsolutePanTiltPositionSpace, "tt:YRange");
				if (p_YRange)
				{
					XMLN * p_XMin = xml_node_soap_get(p_YRange, "tt:Min");
					if (p_XMin)
					{
						p_dev->ptzcfg.pantilt_y.min = (float)atof(p_XMin->data);
					}
					XMLN * p_XMax = xml_node_soap_get(p_YRange, "tt:Max");
					if (p_XMax)
					{
						p_dev->ptzcfg.pantilt_y.max = (float)atof(p_XMax->data);
					}
				}
			}
			XMLN * p_AbsoluteZoomPositionSpace = xml_node_soap_get(p_Space, "tt:AbsoluteZoomPositionSpace");
			if (p_AbsoluteZoomPositionSpace)
			{
				XMLN * p_XRange = xml_node_soap_get(p_AbsoluteZoomPositionSpace, "tt:XRange");
				if (p_XRange)
				{
					XMLN * p_XMin = xml_node_soap_get(p_XRange, "tt:Min");
					if (p_XMin)
					{
						p_dev->ptzcfg.zoom.min = (float)atof(p_XMin->data);
					}
					XMLN * p_XMax = xml_node_soap_get(p_XRange, "tt:Max");
					if (p_XMax)
					{
						p_dev->ptzcfg.zoom.max = (float)atof(p_XMax->data);
					}
				}
			}
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	
	}

	return TRUE;
}

BOOL onvif_rly_GetScopes(XMLN * p_xml, onvif_device * p_dev)
{
	XMLN * p_res = xml_node_soap_get(p_xml, "tds:GetScopesResponse");
	if (NULL == p_res)
	{
		return FALSE;
	}
	/*const char *p_res21 = */xml_attr_get(p_xml, "onvif://www.onvif.org/name");
	

	while (true)
	{
		XMLN * p_cfg = xml_node_soap_get(p_res, "tds:Scopes");
		if (NULL == p_res)
		{
			return FALSE;
		}
		else
		{
// 			XMLN * p_cfg = xml_node_soap_get(p_res, "tt:ScopeDef");
// 			if (NULL == p_res)
// 			{
// 				return FALSE;
// 			}
			XMLN * p_cfg1 = xml_node_soap_get(p_cfg, "tt:ScopeItem");
			if (NULL == p_res)
			{
				return FALSE;
			}
			if (p_cfg && p_cfg->data)
			{
				if (0 == strncmp("onvif://www.onvif.org/name", p_cfg->data, 26))
				{
				}
			}
			p_res = p_cfg1;
		}
	}

	return TRUE;
}

BOOL onvif_rly_GetNetworkInterfaces(XMLN * p_xml, onvif_device * p_dev)
{
	return TRUE;
}

BOOL onvif_rly_GetVideoEncoderConfigurations(XMLN * p_xml, onvif_device * p_dev)
{
	return TRUE;
}

BOOL onvif_rly_GetPresets(XMLN * p_xml, onvif_device * p_dev)
{
	return TRUE;
}

BOOL onvif_rly_SetPreset(XMLN * p_xml, onvif_device * p_dev)
{
	return TRUE;
}

BOOL onvif_rly_GotoPreset(XMLN * p_xml, onvif_device * p_dev)
{	
	return TRUE;
}

BOOL onvif_rly_GotoHomePosition(XMLN * p_xml, onvif_device * p_dev)
{
	return TRUE;
}

BOOL onvif_rly_SetHomePosition(XMLN * p_xml, onvif_device * p_dev)
{
	return TRUE;
}

BOOL onvif_rly_ContinuousMove(XMLN * p_xml, onvif_device * p_dev)
{
	return TRUE;
}

BOOL onvif_rly_AbsoluteMove(XMLN * p_xml, onvif_device * p_dev)
{
	return TRUE;
}

BOOL onvif_rly_Stop(XMLN * p_xml, onvif_device * p_dev)
{
	return TRUE;
}

BOOL onvif_rly_GetConfigurations(XMLN * p_xml, onvif_device * p_dev)
{	
	XMLN * p_res = xml_node_soap_get(p_xml, "tptz:GetConfigurationsResponse");
	if (NULL == p_res)
	{
		return FALSE;
	}

	XMLN * p_cfg = xml_node_soap_get(p_res, "tptz:PTZConfiguration");
	if (p_cfg)
	{
		strncpy(p_dev->ptzcfg.token, xml_attr_get(p_cfg, "token"), TOKEN_LEN);
	}
	else
	{
		return FALSE;
	}

	XMLN * p_name = xml_node_soap_get(p_cfg, "tt:Name");
	if (p_name && p_name->data)
	{
		strncpy(p_dev->ptzcfg.name, p_name->data, NAME_LEN);
	}

	XMLN * p_use_count = xml_node_soap_get(p_cfg, "tt:UseCount");
	if (p_use_count && p_use_count->data)
	{
		p_dev->ptzcfg.use_count = atoi(p_use_count->data);
	}

	XMLN * p_node_token = xml_node_soap_get(p_cfg, "tt:NodeToken");
	if (p_node_token && p_node_token->data)
	{
		strncpy(p_dev->ptzcfg.nodeToken, p_node_token->data, NAME_LEN);
	}

	XMLN * p_pantilt_limits = xml_node_soap_get(p_cfg, "tt:PanTiltLimits");
	if (p_pantilt_limits)
	{
		XMLN * p_range = xml_node_soap_get(p_pantilt_limits, "tt:Range");
		if (p_range)
		{
			XMLN * p_xrange = xml_node_soap_get(p_range, "tt:XRange");
			if (p_xrange)
			{
				XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
				if (p_min && p_min->data)
				{
					p_dev->ptzcfg.pantilt_x.min = (float)atoi(p_min->data);
				}	

				XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
				if (p_max && p_max->data)
				{
					p_dev->ptzcfg.pantilt_x.max = (float)atoi(p_max->data);
				}	
			}

			XMLN * p_yrange = xml_node_soap_get(p_range, "tt:YRange");
			if (p_yrange)
			{
				XMLN * p_min = xml_node_soap_get(p_yrange, "tt:Min");
				if (p_min && p_min->data)
				{
					p_dev->ptzcfg.pantilt_y.min = (float)atoi(p_min->data);
				}	

				XMLN * p_max = xml_node_soap_get(p_yrange, "tt:Max");
				if (p_max && p_max->data)
				{
					p_dev->ptzcfg.pantilt_y.min = (float)atoi(p_max->data);
				}	
			}
		}		
	}

	XMLN * p_zoom_limits = xml_node_soap_get(p_cfg, "tt:ZoomLimits");
	if (p_zoom_limits)
	{
		XMLN * p_range = xml_node_soap_get(p_zoom_limits, "tt:Range");
		if (p_range)
		{
			XMLN * p_xrange = xml_node_soap_get(p_range, "tt:XRange");
			if (p_xrange)
			{
				XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
				if (p_min && p_min->data)
				{
					p_dev->ptzcfg.zoom.min = (float)atoi(p_min->data);
				}	

				XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
				if (p_max && p_max->data)
				{
					p_dev->ptzcfg.zoom.max = (float)atoi(p_max->data);
				}	
			}
		}
	}

	return TRUE;
} 

BOOL onvif_rly_GetConfigurationOptions(XMLN * p_xml, onvif_device * p_dev)
{
	XMLN * p_res = xml_node_soap_get(p_xml, "tptz:GetConfigurationOptionsResponse");
	if (NULL == p_res)
	{
		return FALSE;
	}

	XMLN * p_opt = xml_node_soap_get(p_res, "tptz:PTZConfigurationOptions");
	if (NULL == p_opt)
	{
		return FALSE;
	}

	XMLN * p_spaces = xml_node_soap_get(p_opt, "tt:Spaces");
	if (NULL == p_spaces)
	{
		return FALSE;
	}

	XMLN * p_abs_pan_space = xml_node_soap_get(p_spaces, "tt:AbsolutePanTiltPositionSpace");
	if (p_abs_pan_space)
	{
		XMLN * p_xrange = xml_node_soap_get(p_abs_pan_space, "tt:XRange");
		if (p_xrange)
		{
			XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
			if (p_min && p_min->data)
			{
				p_dev->ptzcfg.opt.absolute_pantilt_x.min = (float)atoi(p_min->data);
			}

			XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
			if (p_max && p_max->data)
			{
				p_dev->ptzcfg.opt.absolute_pantilt_x.max = (float)atoi(p_max->data);
			}
		}

		XMLN * p_yrange = xml_node_soap_get(p_abs_pan_space, "tt:YRange");
		if (p_yrange)
		{
			XMLN * p_min = xml_node_soap_get(p_yrange, "tt:Min");
			if (p_min && p_min->data)
			{
				p_dev->ptzcfg.opt.absolute_pantilt_y.min = (float)atoi(p_min->data);
			}

			XMLN * p_max = xml_node_soap_get(p_yrange, "tt:Max");
			if (p_max && p_max->data)
			{
				p_dev->ptzcfg.opt.absolute_pantilt_y.max = (float)atoi(p_max->data);
			}
		}
	}

	XMLN * p_abs_zoom_space = xml_node_soap_get(p_spaces, "tt:AbsoluteZoomPositionSpace");
	if (p_abs_zoom_space)
	{
		XMLN * p_xrange = xml_node_soap_get(p_abs_zoom_space, "tt:XRange");
		if (p_xrange)
		{
			XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
			if (p_min && p_min->data)
			{
				p_dev->ptzcfg.opt.absolute_zoom.min = (float)atoi(p_min->data);
			}

			XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
			if (p_max && p_max->data)
			{
				p_dev->ptzcfg.opt.absolute_zoom.max = (float)atoi(p_max->data);
			}
		}
	}

	XMLN * p_rel_pan_space = xml_node_soap_get(p_spaces, "tt:RelativePanTiltTranslationSpace");
	if (p_rel_pan_space)
	{
		XMLN * p_xrange = xml_node_soap_get(p_rel_pan_space, "tt:XRange");
		if (p_xrange)
		{
			XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
			if (p_min && p_min->data)
			{
				p_dev->ptzcfg.opt.relative_pantilt_x.min = (float)atoi(p_min->data);
			}

			XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
			if (p_max && p_max->data)
			{
				p_dev->ptzcfg.opt.relative_pantilt_x.max = (float)atoi(p_max->data);
			}
		}

		XMLN * p_yrange = xml_node_soap_get(p_rel_pan_space, "tt:YRange");
		if (p_yrange)
		{
			XMLN * p_min = xml_node_soap_get(p_yrange, "tt:Min");
			if (p_min && p_min->data)
			{
				p_dev->ptzcfg.opt.relative_pantilt_y.min = (float)atoi(p_min->data);
			}

			XMLN * p_max = xml_node_soap_get(p_yrange, "tt:Max");
			if (p_max && p_max->data)
			{
				p_dev->ptzcfg.opt.relative_pantilt_y.max = (float)atoi(p_max->data);
			}
		}
	}

	XMLN * p_rel_zoom_space = xml_node_soap_get(p_spaces, "tt:RelativeZoomTranslationSpace");
	if (p_rel_zoom_space)
	{
		XMLN * p_xrange = xml_node_soap_get(p_rel_zoom_space, "tt:XRange");
		if (p_xrange)
		{
			XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
			if (p_min && p_min->data)
			{
				p_dev->ptzcfg.opt.relative_zoom.min = (float)atoi(p_min->data);
			}

			XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
			if (p_max && p_max->data)
			{
				p_dev->ptzcfg.opt.relative_zoom.max = (float)atoi(p_max->data);
			}
		}
	}

	XMLN * p_con_pan_space = xml_node_soap_get(p_spaces, "tt:ContinuousPanTiltVelocitySpace");
	if (p_con_pan_space)
	{
		XMLN * p_xrange = xml_node_soap_get(p_con_pan_space, "tt:XRange");
		if (p_xrange)
		{
			XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
			if (p_min && p_min->data)
			{
				p_dev->ptzcfg.opt.continuous_pantilt_x.min = (float)atoi(p_min->data);
			}

			XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
			if (p_max && p_max->data)
			{
				p_dev->ptzcfg.opt.continuous_pantilt_x.max = (float)atoi(p_max->data);
			}
		}

		XMLN * p_yrange = xml_node_soap_get(p_con_pan_space, "tt:YRange");
		if (p_yrange)
		{
			XMLN * p_min = xml_node_soap_get(p_yrange, "tt:Min");
			if (p_min && p_min->data)
			{
				p_dev->ptzcfg.opt.continuous_pantilt_y.min = (float)atoi(p_min->data);
			}

			XMLN * p_max = xml_node_soap_get(p_yrange, "tt:Max");
			if (p_max && p_max->data)
			{
				p_dev->ptzcfg.opt.continuous_pantilt_y.max = (float)atoi(p_max->data);
			}
		}
	}

	XMLN * p_con_zoom_space = xml_node_soap_get(p_spaces, "tt:ContinuousZoomVelocitySpace");
	if (p_con_zoom_space)
	{
		XMLN * p_xrange = xml_node_soap_get(p_rel_zoom_space, "tt:XRange");
		if (p_xrange)
		{
			XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
			if (p_min && p_min->data)
			{
				p_dev->ptzcfg.opt.continuous_zoom.min = (float)atoi(p_min->data);
			}

			XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
			if (p_max && p_max->data)
			{
				p_dev->ptzcfg.opt.continuous_zoom.max = (float)atoi(p_max->data);
			}
		}
	}

	XMLN * p_pan_speed_space = xml_node_soap_get(p_spaces, "tt:PanTiltSpeedSpace");
	if (p_pan_speed_space)
	{
		XMLN * p_xrange = xml_node_soap_get(p_pan_speed_space, "tt:XRange");
		if (p_xrange)
		{
			XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
			if (p_min && p_min->data)
			{
				p_dev->ptzcfg.opt.pantilt_speed.min = (float)atoi(p_min->data);
			}

			XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
			if (p_max && p_max->data)
			{
				p_dev->ptzcfg.opt.pantilt_speed.max = (float)atoi(p_max->data);
			}
		}
	}

	XMLN * p_zoom_speed_space = xml_node_soap_get(p_spaces, "tt:ZoomSpeedSpace");
	if (p_zoom_speed_space)
	{
		XMLN * p_xrange = xml_node_soap_get(p_zoom_speed_space, "tt:XRange");
		if (p_xrange)
		{
			XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
			if (p_min && p_min->data)
			{
				p_dev->ptzcfg.opt.zoom_speed.min = (float)atoi(p_min->data);
			}

			XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
			if (p_max && p_max->data)
			{
				p_dev->ptzcfg.opt.zoom_speed.max = (float)atoi(p_max->data);
			}
		}
	}

	XMLN * p_timeout = xml_node_soap_get(p_opt, "tt:PTZTimeout");
	if (p_timeout)
	{
		XMLN * p_min = xml_node_soap_get(p_timeout, "tt:Min");
		if (p_min && p_min->data)
		{
			p_dev->ptzcfg.opt.timeout.min = (float)onvif_parse_time(p_min->data);
		}

		XMLN * p_max = xml_node_soap_get(p_timeout, "tt:Max");
		if (p_max && p_max->data)
		{
			p_dev->ptzcfg.opt.timeout.max = (float)onvif_parse_time(p_max->data);
		}
	}

	p_dev->ptzcfg.opt.used = 1;

	return TRUE;
}

BOOL onvif_rly_GetSystemDateAndTime(XMLN * p_xml, onvif_device * p_dev)
{
	return TRUE;
}

BOOL onvif_rly_SetSystemDateAndTime(XMLN * p_xml, onvif_device * p_dev)
{
	return TRUE;
}

BOOL onvif_rly_SystemReboot(XMLN * p_xml, onvif_device * p_dev)
{
	return TRUE;
}

BOOL onvif_rly_GetEvents(XMLN * p_xml, onvif_device * p_dev, const char* pBuf)
{
	XMLN * p_res = xml_node_soap_get(p_xml, "tev:PullMessagesResponse");
	if (NULL == p_res)
	{
		return FALSE;
	}

	std::string strAlarmInfo = "";
	const char *pKeyAlarmIn = strstr(pBuf, "AlarmIn");
	if (NULL != pKeyAlarmIn)
	{
		p_dev->bMessageUpdate = true;
		strAlarmInfo = "_AlarmIn";
	}
	const char *pKeyDigitalInput = strstr(pBuf, "DigitalInput");
	if (NULL != pKeyDigitalInput)
	{
		p_dev->bMessageUpdate = true;
		strAlarmInfo = "_AlarmIn";
	}
	const char *pKeyMaskAlarm = strstr(pBuf, "MaskAlarm");
	if (NULL != pKeyMaskAlarm)
	{
		p_dev->bMessageUpdate = true;
		strAlarmInfo += "_MaskAlarm";
	}
	const char *pKeyTamperAlarm = strstr(pBuf, "Tamper");
	if (NULL != pKeyTamperAlarm)
	{
		p_dev->bMessageUpdate = true;
		strAlarmInfo += "_MaskAlarm";
	}
	const char *pKeyMotionAlarm = strstr(pBuf, "MotionAlarm");
	if (NULL != pKeyMotionAlarm)
	{
		p_dev->bMessageUpdate = true;
		strAlarmInfo += "_MotionAlarm";
	}

	strcpy(p_dev->lastMessage, strAlarmInfo.c_str());
	if (!p_dev->bMessageUpdate)
	{
		return TRUE;
	}

// 	XMLN * p_media_uri = xml_node_soap_get(p_res->f_child, "wsnt:NotificationMessage");
// 	if (p_media_uri)
// 	{
// 		XMLN * p_uri = xml_node_soap_get(p_media_uri, "wsnt:Topic");
// 		if (p_uri && p_uri->data)
// 		{
// 			//onvif_parse_xaddr(p_uri->data, &p_dev->devCap.message.xaddr);
// 			strcpy(p_dev->lastMessage, p_uri->data);
// 			p_dev->bMessageUpdate = true;
// 		}
// 		else
// 		{
// 			return FALSE;
// 		}
// 	}
// 	else
// 	{
// 		return FALSE;
// 	}
	return TRUE;
}


BOOL onvif_rly_handler(XMLN * p_xml, eOnvifAction act, onvif_device * p_dev, const char* pBuf)
{
	BOOL ret = FALSE;

	switch(act)
	{
	case eGetDeviceInformation:
		ret = onvif_rly_GetDeviceInformation(p_xml, p_dev);
		break;

	case eGetCapabilities:
		ret = onvif_rly_GetCapabilities(p_xml, p_dev);
		break;

	case eGetProfiles:
		ret = onvif_rly_GetProfiles(p_xml, p_dev);
		break;   

	case eGetStreamUri:
		ret = onvif_rly_GetStreamUri(p_xml, p_dev);
		break;

	case eGetNetworkInterfaces:
		ret = onvif_rly_GetNetworkInterfaces(p_xml, p_dev);
		break;

	case eGetVideoEncoderConfigurations:
		ret = onvif_rly_GetVideoEncoderConfigurations(p_xml, p_dev);
		break;

	case eGetPresets:
		ret = onvif_rly_GetPresets(p_xml, p_dev);
		break;

	case eSetPreset:
		ret = onvif_rly_SetPreset(p_xml, p_dev);
		break;

	case eGotoPreset:
		ret = onvif_rly_GotoPreset(p_xml, p_dev);
		break;

	case eGotoHomePosition:
		ret = onvif_rly_GotoHomePosition(p_xml, p_dev);
		break;

	case eSetHomePosition:
		ret = onvif_rly_SetHomePosition(p_xml, p_dev);
		break;

	case eContinuousMove:
		ret = onvif_rly_ContinuousMove(p_xml, p_dev);
		break;

	case eAbsoluteMove:
		ret = onvif_rly_ContinuousMove(p_xml, p_dev);
		break;

	case eStop:
		ret = onvif_rly_Stop(p_xml, p_dev);
		break;

	case eGetConfigurations:
		ret = onvif_rly_GetConfigurations(p_xml, p_dev);
		break;

	case eGetConfigurationOptions:
		ret = onvif_rly_GetConfigurationOptions(p_xml, p_dev);
		break;

	case eGetSystemDateAndTime:
		ret = onvif_rly_GetSystemDateAndTime(p_xml, p_dev);
		break;

	case eSetSystemDateAndTime:
		ret = onvif_rly_SetSystemDateAndTime(p_xml, p_dev);
		break;

	case eSystemReboot:
		ret = onvif_rly_SystemReboot(p_xml, p_dev);
		break;

	case eRecvAlarm:
		ret = onvif_rly_GetEvents(p_xml, p_dev, pBuf);
		break;

	case eGetSnapUri:
		ret = onvif_rly_GetSnapUri(p_xml, p_dev);
		break;

	case eStartRecvAlarm:
		ret = onvif_rly_CreatePullPointSubscriptionRequest(p_xml, p_dev);
		break;

	case eGetNode:
		ret = onvif_rly_GetNode(p_xml, p_dev, pBuf);
		break;

	case eGetScopes:
		//暂时不解析GetScopes，太复杂，目前没需求
		ret = TRUE;//onvif_rly_GetScopes(p_xml, p_dev);
		break;

	default:
		break;
	}

	return ret;
}

void stream_startElement(void * userdata, const char * name, const char ** atts)
{
	XMLN ** pp_node = (XMLN **)userdata;
	if(pp_node == NULL)
	{
		return;
	}

	XMLN * parent = *pp_node;
	XMLN * p_node = xml_node_add(parent,(char *)name);
	if(atts)
	{
		int i=0;
		while(atts[i] != NULL)
		{
			if(atts[i+1] == NULL)
				break;

			/*XMLN * p_attr = */xml_attr_add(p_node,atts[i],atts[i+1]);	//(XMLN *)malloc(sizeof(XMLN));

			i += 2;
		}
	}

	*pp_node = p_node;
}

void stream_endElement(void * userdata, const char * name)
{
	XMLN ** pp_node = (XMLN **)userdata;
	if(pp_node == NULL)
	{
		return;
	}

	XMLN * p_node = *pp_node;
	if(p_node == NULL)
	{
		return;
	}

	p_node->finish = 1;

	if(p_node->type == NTYPE_TAG && p_node->parent == NULL)
	{
		// parse finish
	}
	else
	{
		*pp_node = p_node->parent;	// back up a level
	}
}

void stream_charData(void* userdata, const char* s, int len)
{
	XMLN ** pp_node = (XMLN **)userdata;
	if(pp_node == NULL)
	{
		return;
	}

//	const char *pchar = NULL;
	XMLN * p_node = *pp_node;
	if(p_node == NULL)
	{
		//	log_user_print((void *)p_user,"stream_charData::cur_node is null,s'addr=0x%x,len=%d!!!\r\n",s,len);
		return;
	}
	p_node->data = s;
	p_node->dlen = len;
}

XMLN * xxx_hxml_parse(char * p_xml, int len)
{
	XMLN * p_root = NULL;

	LTXMLPRS parse;
	memset(&parse, 0, sizeof(parse));

	parse.userdata = &p_root;
	parse.startElement = stream_startElement;
	parse.endElement = stream_endElement;
	parse.charData = stream_charData;

	parse.xmlstart = p_xml;
	parse.xmlend = p_xml + len;
	parse.ptr = parse.xmlstart;

	int status = hxml_parse(&parse);
	if(status < 0)
	{
		//log_print("xxx_hxml_parse::err[%d]\r\n", status);

		xml_node_del(p_root);
		p_root = NULL;
	}

	return p_root;
}

BOOL http_onvif_trans(HTTPREQ * p_req, int timeout, eOnvifAction act)
{
	OVFACTS * p_ent = onvif_find_action_by_type(act);
	if(p_ent == NULL)
		return FALSE;

	// build xml bufs
	char xml_bufs[1024 * 2];
	int xml_len = build_onvif_req_xml(xml_bufs, sizeof(xml_bufs), act, p_req->p_dev);

	// connect onvif device
	p_req->cfd = tcp_connect_timeout(inet_addr(p_req->host), p_req->port, timeout);
	if(p_req->cfd <= 0)
		return FALSE;

	if (FALSE == http_onvif_req(p_req, p_ent->action_url, xml_bufs, xml_len))
	{
		closesocket(p_req->cfd);
		free_http_msg(p_req->rx_msg);
		return FALSE;
	}

	int count = 0;

	while(1)
	{
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100 * 1000;

		fd_set fdr;
		FD_ZERO(&fdr);
		FD_SET(p_req->cfd, &fdr);

		int sret = select(p_req->cfd+1, &fdr,NULL,NULL,&tv);
		if(sret == 0)
		{
			count++;
			if (count >= timeout / 100)
			{
				//log_print("http_onvif_trans::timeout!!!\r\n");
				break;
			}
			continue;
		}
		else if(sret < 0)
		{
			//log_print("http_rx_thread::select err[%s], sret[%d]!!!\r\n", sys_os_get_socket_error(), sret);
			break;
		}

		if(FD_ISSET(p_req->cfd, &fdr))
		{
			if(http_tcp_rx(p_req) == FALSE)
				break;
			else if(p_req->rx_msg != NULL)
				break;
		}
	}

	if(p_req->rx_msg == NULL)
	{
		closesocket(p_req->cfd);
		free_http_msg(p_req->rx_msg);
		if (NULL != p_req->dyn_recv_buf)
		{
			free(p_req->dyn_recv_buf);
			p_req->dyn_recv_buf = NULL;
		}
		return FALSE;
	}

	if(p_req->rx_msg->msg_sub_type != 200)
	{
		closesocket(p_req->cfd);
		free_http_msg(p_req->rx_msg);
		if (NULL != p_req->dyn_recv_buf)
		{
			free(p_req->dyn_recv_buf);
			p_req->dyn_recv_buf = NULL;
		}
		return FALSE;
	}

	if(p_req->rx_msg->ctt_type != CTT_XML)
	{
		closesocket(p_req->cfd);
		free_http_msg(p_req->rx_msg);
		if (NULL != p_req->dyn_recv_buf)
		{
			free(p_req->dyn_recv_buf);
			p_req->dyn_recv_buf = NULL;
		}
		return FALSE;
	}

	char * rx_xml = get_http_ctt(p_req->rx_msg);
	if (NULL == rx_xml)
	{
		closesocket(p_req->cfd);
		free_http_msg(p_req->rx_msg);
		if (NULL != p_req->dyn_recv_buf)
		{
			free(p_req->dyn_recv_buf);
			p_req->dyn_recv_buf = NULL;
		}
		return FALSE;
	}

	if (act == eGetScopes)
	{
		char *pName = strstr(p_req->p_rbuf, "onvif://www.onvif.org/name");
		if (NULL != pName)
		{
			char *pEndName = strstr(pName, "<");
			if (NULL != pEndName)
			{
				int nSkepLen = strlen("onvif://www.onvif.org/name") + 1;
				
				char szTempBuf[128] = {0};
				strncpy(szTempBuf, pName + nSkepLen, pEndName - pName - nSkepLen);
				strCoding cd;
				string stName = cd.UrlUTF8Decode(szTempBuf);
				strcpy(p_req->p_dev->devInfo.Name, stName.c_str());
			}
		}
	}

	int rlen = p_req->rx_msg->ctt_len;

	XMLN * p_node = xxx_hxml_parse(rx_xml, rlen);
	if(p_node == NULL || p_node->name == NULL)
	{
		//log_print("http_onvif_trans::xxx_hxml_parse ret null!!!\r\n");
		closesocket(p_req->cfd);
		free_http_msg(p_req->rx_msg);
		if (NULL != p_req->dyn_recv_buf)
		{
			free(p_req->dyn_recv_buf);
			p_req->dyn_recv_buf = NULL;
		}
		return FALSE;
	}

	if(soap_strcmp(p_node->name, "s:Envelope") != 0)
	{	
		//log_print("http_onvif_trans::node name[%s] != [s:Envelope]!!!\r\n", p_node->name);
		closesocket(p_req->cfd);
		xml_node_del(p_node);		
		free_http_msg(p_req->rx_msg);
		if (NULL != p_req->dyn_recv_buf)
		{
			free(p_req->dyn_recv_buf);
			p_req->dyn_recv_buf = NULL;
		}
		return FALSE;
	}

	XMLN * p_body = xml_node_soap_get(p_node, "s:Body");
	if(p_body == NULL)
	{
		//log_print("http_onvif_trans::xml_node_soap_get[s:Body] ret null!!!\r\n");
		closesocket(p_req->cfd);
		xml_node_del(p_node);		
		free_http_msg(p_req->rx_msg);
		if (NULL != p_req->dyn_recv_buf)
		{
			free(p_req->dyn_recv_buf);
			p_req->dyn_recv_buf = NULL;
		}
		return FALSE;
	}

	if (!onvif_rly_handler(p_body, act, p_req->p_dev, p_req->p_rbuf))
	{
		closesocket(p_req->cfd);
		xml_node_del(p_node);
		free_http_msg(p_req->rx_msg);
		if (NULL != p_req->dyn_recv_buf)
		{
			free(p_req->dyn_recv_buf);
			p_req->dyn_recv_buf = NULL;
		}
		return FALSE;
	}

	closesocket(p_req->cfd);
	xml_node_del(p_node);
	free_http_msg(p_req->rx_msg);
	if (NULL != p_req->dyn_recv_buf)
	{
		free(p_req->dyn_recv_buf);
		p_req->dyn_recv_buf = NULL;
	}
	return TRUE;
}

void onvif_init_req(HTTPREQ * p_req, onvif_xaddr * p_xaddr, onvif_device * p_dev)
{
	memset(p_req, 0, sizeof(HTTPREQ));

	strcpy(p_req->host, p_xaddr->host);
	p_req->port = p_xaddr->port;
	p_req->p_dev = p_dev;

	if (p_xaddr->url[0] != '\0')
	{
		strcpy(p_req->url, p_xaddr->url);
	}
	else
	{
		strcpy(p_req->url, "/onvif/device_service");
	}
}

BOOL onvif_GetDeviceInformation(onvif_device * p_dev)
{
	HTTPREQ http_req;
	onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);

	return http_onvif_trans(&http_req, 6000, eGetDeviceInformation);
}

BOOL onvif_GetCapabilities(onvif_device * p_dev)
{
	HTTPREQ http_req;
	onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);

	return http_onvif_trans(&http_req, 6000, eGetCapabilities);
}

BOOL onvif_GetScopes(onvif_device * p_dev)
{
	HTTPREQ http_req;
	onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);

	return http_onvif_trans(&http_req, 6000, eGetScopes);
}

BOOL onvif_GetProfiles(onvif_device * p_dev)
{
	HTTPREQ http_req;

	if (p_dev->devCap.media.support)
	{
		onvif_init_req(&http_req, &p_dev->devCap.media.xaddr, p_dev);
	}
	else
	{
		onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);
	}

	return http_onvif_trans(&http_req, 6000, eGetProfiles);    
}

BOOL onvif_GetStreamUri(onvif_device * p_dev)
{
	HTTPREQ http_req;

	if (p_dev->devCap.media.support)
	{
		onvif_init_req(&http_req, &p_dev->devCap.media.xaddr, p_dev);
	}
	else
	{
		onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);
	}

	return http_onvif_trans(&http_req, 6000, eGetStreamUri);
}

BOOL onvif_GetSnapUri(onvif_device * p_dev)
{
	HTTPREQ http_req;

	if (p_dev->devCap.media.support)
	{
		onvif_init_req(&http_req, &p_dev->devCap.media.xaddr, p_dev);
	}
	else
	{
		onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);
	}

	return http_onvif_trans(&http_req, 6000, eGetSnapUri);
}

BOOL onvif_GetNetworkInterfaces(onvif_device * p_dev)
{
	HTTPREQ http_req;
	onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);

	return http_onvif_trans(&http_req, 6000, eGetNetworkInterfaces);
}

BOOL onvif_GetVideoEncoderConfigurations(onvif_device * p_dev)
{
	HTTPREQ http_req;
	onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);

	return http_onvif_trans(&http_req, 6000, eGetVideoEncoderConfigurations);
}

BOOL onvif_ContinuousMove(onvif_device * p_dev)
{
	HTTPREQ http_req;

	if (p_dev->devCap.ptz.support)
	{
		onvif_init_req(&http_req, &p_dev->devCap.ptz.xaddr, p_dev);
	}
	else
	{
		onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);
	}

	return http_onvif_trans(&http_req, 6000, eContinuousMove);
}

BOOL onvif_AbsoluteMove(onvif_device * p_dev)
{
	HTTPREQ http_req;

	if (p_dev->devCap.ptz.support)
	{
		onvif_init_req(&http_req, &p_dev->devCap.ptz.xaddr, p_dev);
	}
	else
	{
		onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);
	}

	return http_onvif_trans(&http_req, 6000, eAbsoluteMove);
}

BOOL onvif_Stop(onvif_device * p_dev)
{
	HTTPREQ http_req;

	if (p_dev->devCap.ptz.support)
	{
		onvif_init_req(&http_req, &p_dev->devCap.ptz.xaddr, p_dev);
	}
	else
	{
		onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);
	}

	return http_onvif_trans(&http_req, 6000, eStop);
}

BOOL onvif_GetConfigurations(onvif_device * p_dev)
{
	HTTPREQ http_req;

	if (p_dev->devCap.ptz.support)
	{
		onvif_init_req(&http_req, &p_dev->devCap.ptz.xaddr, p_dev);
	}
	else
	{
		onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);
	}

	return http_onvif_trans(&http_req, 6000, eGetConfigurations);
}

BOOL onvif_GetNode(onvif_device * p_dev)
{
	HTTPREQ http_req;

	if (p_dev->devCap.ptz.support)
	{
		onvif_init_req(&http_req, &p_dev->devCap.ptz.xaddr, p_dev);
	}
	else
	{
		onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);
	}

	 return http_onvif_trans(&http_req, 6000, eGetNode);
}

BOOL onvif_GetConfigurationOptions(onvif_device * p_dev)
{
	HTTPREQ http_req;

	if (p_dev->devCap.ptz.support)
	{
		onvif_init_req(&http_req, &p_dev->devCap.ptz.xaddr, p_dev);
	}
	else
	{
		onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);
	}

	return http_onvif_trans(&http_req, 6000, eGetConfigurationOptions);
}

BOOL onvif_GetSystemDateAndTime(onvif_device * p_dev)
{
	HTTPREQ http_req;
	onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);

	return http_onvif_trans(&http_req, 6000, eGetSystemDateAndTime);
}

BOOL onvif_SetSystemDateAndTime(onvif_device * p_dev)
{
	HTTPREQ http_req;
	onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);

	return http_onvif_trans(&http_req, 6000, eSetSystemDateAndTime);
}

BOOL onvif_SystemReboot(onvif_device * p_dev)
{
	HTTPREQ http_req;
	onvif_init_req(&http_req, &p_dev->binfo.xaddr, p_dev);

	return http_onvif_trans(&http_req, 6000, eSystemReboot);
}

void onvif_free_profiles(onvif_device * p_dev)
{
	onvif_profile * next;
	onvif_profile * profile = p_dev->profile;

	while (profile)
	{
		next = profile->next;

		free(profile);
		profile = next;
	}

	p_dev->profile = NULL;
}

BOOL onvif_get_dev_info(onvif_device * p_dev)
{
	if (!onvif_GetDeviceInformation(p_dev))
	{
		return FALSE;
	}

	if (!onvif_GetCapabilities(p_dev))
	{
		return FALSE;
	}

	onvif_free_profiles(p_dev);

	if (!onvif_GetProfiles(p_dev))
	{
		return FALSE;
	}

	onvif_profile * profile = p_dev->profile;
	while (profile)
	{
		p_dev->curProfile = profile;

		onvif_GetStreamUri(p_dev);

		profile = profile->next;
	}

	p_dev->curProfile = p_dev->profile;

	if (p_dev->devCap.ptz.support)
	{
		if (onvif_GetConfigurations(p_dev))
		{
			onvif_GetConfigurationOptions(p_dev);
		}
	}

	return TRUE;
}

/************************************************************************************/
LINKED_LIST* h_list_create (BOOL bNeedMutex)
{
	LINKED_LIST* p_linked_list;

	p_linked_list = (LINKED_LIST*) malloc (sizeof (LINKED_LIST));

	if (p_linked_list == NULL)
	{
		return NULL;
	}

	p_linked_list->p_first_node = NULL;
	p_linked_list->p_last_node = NULL;

	if(bNeedMutex)
		p_linked_list->list_semMutex = sys_os_create_mutex();
	else
		p_linked_list->list_semMutex = NULL;

	return p_linked_list;
}
/************************************************************************************/
void get_ownership(LINKED_LIST* p_linked_list)
{
	if(p_linked_list->list_semMutex)
	{
		sys_os_mutex_enter(p_linked_list->list_semMutex);
	}
}
/************************************************************************************/
void giveup_ownership(LINKED_LIST* p_linked_list)
{
	if(p_linked_list->list_semMutex)
	{
		sys_os_mutex_leave(p_linked_list->list_semMutex);
	}
}
/************************************************************************************/
void h_list_free_container (LINKED_LIST* p_linked_list)
{
	LINKED_NODE* p_node;
	LINKED_NODE* p_next_node;

	if (p_linked_list == NULL)	return;

	get_ownership(p_linked_list);

	p_node = p_linked_list->p_first_node;

	while (p_node != NULL) 
	{
		p_next_node = p_node->p_next;

		void * p_free = p_node->p_data;

		if(p_free != NULL)
			free(p_free);

		free (p_node);

		p_node = p_next_node;		
	}

	giveup_ownership(p_linked_list);

	if(p_linked_list->list_semMutex)
	{
		sys_os_destroy_sig_mutx(p_linked_list->list_semMutex);
	}

	free (p_linked_list);
}

/************************************************************************************/
void h_list_free_all_node (LINKED_LIST* p_linked_list)
{
	LINKED_NODE* p_node;
	LINKED_NODE* p_next_node;

	if (p_linked_list == NULL)	return;

	get_ownership(p_linked_list);

	p_node = p_linked_list->p_first_node;

	while (p_node != NULL) 
	{
		p_next_node = p_node->p_next;

		if(p_node->p_data != NULL)
			free(p_node->p_data);

		free (p_node);

		p_node = p_next_node;		
	}

	p_linked_list->p_first_node = NULL;
	p_linked_list->p_last_node = NULL;

	giveup_ownership(p_linked_list);	
}

/************************************************************************************/
BOOL h_list_add_at_front (LINKED_LIST* p_linked_list, void* p_item)
{
	LINKED_NODE* p_node;
	LINKED_NODE* p_next_node;

	if (p_linked_list == NULL)	return (FALSE);

	if (p_item == NULL)	return (FALSE);

	p_node = (LINKED_NODE*) malloc (sizeof (LINKED_NODE));

	if (p_node == NULL)	return (FALSE);

	p_node->p_next = NULL;
	p_node->p_previous = NULL;
	p_node->p_data = p_item;

	get_ownership(p_linked_list);

	if (p_linked_list->p_first_node == NULL)
	{
		p_linked_list->p_first_node = p_node;
		p_linked_list->p_last_node = p_node;

		p_node->p_previous = NULL;
		p_node->p_next = NULL;
	}
	else
	{
		p_next_node = p_linked_list->p_first_node;

		p_node->p_next = p_next_node;
		p_node->p_previous = NULL;

		p_next_node->p_previous = p_node;
		p_linked_list->p_first_node = p_node;
	}

	giveup_ownership(p_linked_list);
	return (TRUE);
}
/************************************************************************************/
void h_list_remove_from_front (LINKED_LIST* p_linked_list)
{
	LINKED_NODE* p_node_to_remove;

	if (p_linked_list == NULL)	return;

	get_ownership(p_linked_list);

	p_node_to_remove = p_linked_list->p_first_node;

	if (p_node_to_remove == NULL) 
	{
		giveup_ownership(p_linked_list);
		return;
	}

	if (p_linked_list->p_first_node == p_linked_list->p_last_node)
	{
		p_linked_list->p_first_node = NULL;
		p_linked_list->p_last_node = NULL;
	}
	else
	{
		p_linked_list->p_first_node = p_node_to_remove->p_next;
		p_linked_list->p_first_node->p_previous = NULL;
	}

	free (p_node_to_remove);
	giveup_ownership(p_linked_list);
}
/************************************************************************************/
void h_list_remove_from_front_no_lock (LINKED_LIST* p_linked_list)
{
	LINKED_NODE* p_node_to_remove;

	if (p_linked_list == NULL)	return;

	p_node_to_remove = p_linked_list->p_first_node;

	if (p_node_to_remove == NULL) 
	{
		return;
	}

	if (p_linked_list->p_first_node == p_linked_list->p_last_node)
	{
		p_linked_list->p_first_node = NULL;
		p_linked_list->p_last_node = NULL;
	}
	else
	{
		p_linked_list->p_first_node = p_node_to_remove->p_next;
		p_linked_list->p_first_node->p_previous = NULL;
	}

	free (p_node_to_remove);
}
/************************************************************************************/
BOOL h_list_add_at_back (LINKED_LIST* p_linked_list, void* p_item)
{
	LINKED_NODE* p_node;
	LINKED_NODE* p_previous_node;

	if (p_linked_list == NULL)	return (FALSE);

	if (p_item == NULL)	return (FALSE);

	p_node = (LINKED_NODE*) malloc (sizeof (LINKED_NODE));

	if (p_node == NULL)	return (FALSE);

	p_node->p_next = NULL;
	p_node->p_previous = NULL;
	p_node->p_data = (void*) p_item;

	get_ownership(p_linked_list);

	if (p_linked_list->p_last_node == NULL)
	{
		p_linked_list->p_last_node = p_node;
		p_linked_list->p_first_node = p_node;

		p_node->p_next = NULL;
		p_node->p_previous = NULL;
	}
	else
	{
		p_previous_node = p_linked_list->p_last_node;

		p_node->p_next = NULL;
		p_node->p_previous = p_previous_node;

		p_previous_node->p_next = p_node;

		p_linked_list->p_last_node = p_node;
	}

	giveup_ownership(p_linked_list);

	return (TRUE);
}
/************************************************************************************/
void h_list_remove_from_back (LINKED_LIST* p_linked_list)
{
	LINKED_NODE* p_node_to_remove;

	p_linked_list = p_linked_list;

	if (p_linked_list == NULL) return;

	get_ownership(p_linked_list);

	if (p_linked_list->p_last_node == NULL)	
	{
		giveup_ownership(p_linked_list);
		return;
	}

	if (p_linked_list->p_first_node == p_linked_list->p_last_node)
	{
		p_node_to_remove = p_linked_list->p_first_node;

		p_linked_list->p_first_node = NULL;
		p_linked_list->p_last_node = NULL;

		free (p_node_to_remove);

		giveup_ownership(p_linked_list);
		return;
	}

	p_node_to_remove = p_linked_list->p_last_node;

	p_linked_list->p_last_node = p_node_to_remove->p_previous;

	p_linked_list->p_last_node->p_next = NULL;

	free (p_node_to_remove);

	p_node_to_remove = NULL;

	giveup_ownership(p_linked_list);
}
/************************************************************************************/
BOOL h_list_remove(LINKED_LIST* p_linked_list,LINKED_NODE * p_node)
{
	LINKED_NODE* p_previous_node;
	LINKED_NODE* p_next_node;

	if ((p_linked_list == NULL) || (p_node == NULL))
	{
		return (FALSE);
	}

	p_previous_node = p_node->p_previous;

	p_next_node = p_node->p_next;

	if (p_previous_node != NULL)
		p_previous_node->p_next = p_next_node;
	else
		p_linked_list->p_first_node = p_next_node;


	if (p_next_node != NULL)
		p_next_node->p_previous = p_previous_node;
	else
		p_linked_list->p_last_node = p_previous_node;

	free (p_node);

	return (TRUE);				
}
/************************************************************************************/
BOOL h_list_remove_data(LINKED_LIST* p_linked_list,void * p_data)
{
	LINKED_NODE* p_previous_node;
	LINKED_NODE* p_next_node;
	LINKED_NODE* p_node;

	if ((p_linked_list == NULL) || (p_data == NULL))
		return (FALSE);

	get_ownership(p_linked_list);

	p_node = p_linked_list->p_first_node;

	while(p_node != NULL)
	{
		if(p_data == p_node->p_data)break;
		p_node = p_node->p_next;
	}

	if(p_node == NULL)
	{
		giveup_ownership(p_linked_list);
		return FALSE;
	}

	p_previous_node = p_node->p_previous;

	p_next_node = p_node->p_next;

	if (p_previous_node != NULL)
		p_previous_node->p_next = p_next_node;
	else
		p_linked_list->p_first_node = p_next_node;


	if (p_next_node != NULL)
		p_next_node->p_previous = p_previous_node;
	else
		p_linked_list->p_last_node = p_previous_node;

	free (p_node);

	giveup_ownership(p_linked_list);

	return (TRUE);				
}
/************************************************************************************/
unsigned int h_list_get_number_of_nodes (LINKED_LIST* p_linked_list)
{
	unsigned int number_of_nodes;
	LINKED_NODE* p_node;

	get_ownership(p_linked_list);

	p_node = p_linked_list->p_first_node;

	number_of_nodes = 0;
	while (p_node !=  NULL)
	{
		++ number_of_nodes;
		p_node = p_node->p_next;
	}

	giveup_ownership(p_linked_list);

	return (number_of_nodes);
}

/************************************************************************************/
BOOL h_list_insert(LINKED_LIST * p_linked_list,LINKED_NODE * p_pre_node,void *p_item)
{
	if((p_linked_list == NULL) || (p_item == NULL))
		return FALSE;

	if(p_pre_node == NULL)
		h_list_add_at_front(p_linked_list, p_item);
	else
	{
		if(p_pre_node->p_next == NULL)
			h_list_add_at_back(p_linked_list, p_item);
		else
		{
			LINKED_NODE * p_node = (LINKED_NODE *)malloc(sizeof(LINKED_NODE));

			get_ownership(p_linked_list);

			p_node->p_data = p_item;
			p_node->p_next = p_pre_node->p_next;
			p_node->p_previous = p_pre_node;
			p_pre_node->p_next->p_previous = p_node;
			p_pre_node->p_next = p_node;

			giveup_ownership(p_linked_list);
		}
	}

	return TRUE;
}
/***********************************************************************\
\***********************************************************************/
LINKED_NODE * h_list_lookup_start(LINKED_LIST * p_linked_list)
{
	if(p_linked_list == NULL)
		return NULL;

	get_ownership(p_linked_list);

	if(p_linked_list->p_first_node)
	{
		return p_linked_list->p_first_node;
	}

	return NULL;
}

LINKED_NODE * h_list_lookup_next(LINKED_LIST * p_linked_list, LINKED_NODE * p_node)
{
	if(p_node == NULL)
		return NULL;

	return p_node->p_next;
}

void h_list_lookup_end(LINKED_LIST * p_linked_list)
{
	if(p_linked_list == NULL)
		return;

	giveup_ownership(p_linked_list);
}

LINKED_NODE * h_list_get_from_front(LINKED_LIST * p_list)
{
	return p_list->p_first_node;
}

LINKED_NODE * h_list_get_from_back(LINKED_LIST * p_list)
{
	return p_list->p_last_node;
}

BOOL h_list_is_empty(LINKED_LIST* p_list)
{
	return (p_list->p_first_node == NULL);
}

#define IS_WHITE_SPACE(c) ((c==' ') || (c=='\t') || (c=='\r') || (c=='\n'))

#define IS_XMLH_START(ptr) (*ptr == '<' && *(ptr+1) == '?')
#define IS_XMLH_END(ptr) (*ptr == '?' && *(ptr+1) == '>')

// <?xml version="1.0" encoding="UTF-8"?>
int hxml_parse_header(LTXMLPRS * parse)
{
	char * ptr = parse->ptr;
	char * xmlend = parse->xmlend;

	while(IS_WHITE_SPACE(*ptr) && (ptr != xmlend)) ptr++;
	if(ptr == xmlend) return -1;

	if(!IS_XMLH_START(ptr))
		return -1;
	ptr+=2;
	while((!IS_XMLH_END(ptr)) && (ptr != xmlend)) ptr++;
	if(ptr == xmlend) return -1;
	ptr+=2;
	parse->ptr = ptr;

	return 0;
}

#define IS_ELEMS_END(ptr) (*ptr == '>' || (*ptr == '/' && *(ptr+1) == '>'))
#define IS_ELEME_START(ptr) (*ptr == '<' && *(ptr+1) == '/')

#define RF_NO_END		0
#define RF_ELES_END		2
#define RF_ELEE_END		3

int hxml_parse_attr(LTXMLPRS * parse)
{
	char * ptr = parse->ptr;
	char * xmlend = parse->xmlend;
	int ret = RF_NO_END;
	int cnt = 0;

	while(1)
	{
		ret = RF_NO_END;

		while(IS_WHITE_SPACE(*ptr) && (ptr != xmlend)) ptr++;
		if(ptr == xmlend) return -1;

		if(*ptr == '>')
		{
			*ptr = '\0'; ptr++;
			ret = RF_ELES_END; // node start finish
			break;
		}
		else if(*ptr == '/' && *(ptr+1) == '>')
		{
			*ptr = '\0'; ptr += 2;
			ret = RF_ELEE_END;	// node end finish
			break;
		}

		char * attr_name = ptr;
		while(*ptr != '=' && (!IS_ELEMS_END(ptr)) && ptr != xmlend) ptr++;
		if(ptr == xmlend) return -1;
		if(IS_ELEMS_END(ptr))
		{
			if(*ptr == '>')
			{
				ret = RF_ELES_END;
				*ptr = '\0'; ptr++;
			}
			else if(*ptr == '/' && *(ptr+1) == '>')
			{
				ret = RF_ELEE_END;
				*ptr = '\0'; ptr+=2;
			}
			break;
		}

		*ptr = '\0';	// '=' --> '\0'
		ptr++;

		char * attr_value = ptr;
		if(*ptr == '"')
		{
			attr_value++;
			ptr++;
			while(*ptr != '"' && ptr != xmlend) ptr++;
			if(ptr == xmlend) return -1;
			*ptr = '\0'; // '"' --> '\0'

			ptr++;
		}
		else
		{
			while((!IS_WHITE_SPACE(*ptr)) && (!IS_ELEMS_END(ptr)) && ptr != xmlend) ptr++;
			if(ptr == xmlend) return -1;

			if(IS_WHITE_SPACE(*ptr))
			{
				*ptr = '\0';
				ptr++;
			}
			else
			{
				if(*ptr == '>')
				{
					ret = RF_ELES_END;
					*ptr = '\0'; ptr++;
				}
				else if(*ptr == '/' && *(ptr+1) == '>')
				{
					ret = RF_ELEE_END;
					*ptr = '\0'; ptr+=2;
				}
			}
		}

		int index = cnt << 1;
		parse->attr[index] = attr_name;
		parse->attr[index+1] = attr_value;
		cnt++;
		if(ret > RF_NO_END)
			break;
	}

	parse->ptr = ptr;
	return ret;
}

#define CHECK_XML_STACK_RET(parse) \
	do{ if(parse->e_stack_index >= LTXML_MAX_STACK_DEPTH || parse->e_stack_index < 0) return -1;}while(0)

int hxml_parse_element_end(LTXMLPRS * parse)
{
	char * stack_name = parse->e_stack[parse->e_stack_index];
	if(stack_name == NULL)
		return -1;

	char * xmlend = parse->xmlend;

	while(IS_WHITE_SPACE(*(parse->ptr)) && ((parse->ptr) != xmlend)) (parse->ptr)++;
	if((parse->ptr) == xmlend) return -1;

	char * end_name = (parse->ptr);
	while((!IS_WHITE_SPACE(*(parse->ptr))) && ((parse->ptr) != xmlend) && (*(parse->ptr) != '>')) (parse->ptr)++;
	if((parse->ptr) == xmlend) return -1;
	if(IS_WHITE_SPACE(*(parse->ptr)))
	{
		*(parse->ptr) = '\0'; (parse->ptr)++;
		while(IS_WHITE_SPACE(*(parse->ptr)) && ((parse->ptr) != xmlend)) (parse->ptr)++;
		if((parse->ptr) == xmlend) return -1;
	}

	if(*(parse->ptr) != '>') return -1;
	*(parse->ptr) = '\0';
	(parse->ptr)++;

	//	if(strcasecmp(end_name, stack_name) != 0)
	if(strcmp(end_name, stack_name) != 0)
	{
		//log_print("hxml_parse_element_end::cur name[%s] != stack name[%s]!!!\r\n", end_name, stack_name);
		return -1;
	}

	if(parse->endElement)
		parse->endElement(parse->userdata, end_name);

	parse->e_stack[parse->e_stack_index] = NULL; parse->e_stack_index--;
	CHECK_XML_STACK_RET(parse);

	return 0;
}

int hxml_parse_element_start(LTXMLPRS * parse)
{
	char * xmlend = parse->xmlend;

	while(IS_WHITE_SPACE(*(parse->ptr)) && ((parse->ptr) != xmlend)) (parse->ptr)++;
	if((parse->ptr) == xmlend) return -1;

	char * element_name = (parse->ptr);
	while((!IS_WHITE_SPACE(*(parse->ptr))) && ((parse->ptr) != xmlend) && (!IS_ELEMS_END((parse->ptr)))) (parse->ptr)++;
	if((parse->ptr) == xmlend) return -1;

	parse->e_stack_index++; parse->e_stack[parse->e_stack_index] = element_name;
	CHECK_XML_STACK_RET(parse);

	if(*(parse->ptr) == '>')
	{
		*(parse->ptr) = '\0'; (parse->ptr)++;
		if(parse->startElement)
			parse->startElement(parse->userdata, element_name, parse->attr);
		return RF_ELES_END;
	}
	else if(*(parse->ptr) == '/' && *((parse->ptr)+1) == '>')
	{
		*(parse->ptr) = '\0'; (parse->ptr)+=2;
		if(parse->startElement)
			parse->startElement(parse->userdata, element_name, parse->attr);
		if(parse->endElement)
			parse->endElement(parse->userdata, element_name);

		parse->e_stack[parse->e_stack_index] = NULL; parse->e_stack_index--;
		CHECK_XML_STACK_RET(parse);
		return RF_ELEE_END;
	}
	else
	{
		*(parse->ptr) = '\0'; (parse->ptr)++;

		int ret = hxml_parse_attr(parse); if(ret < 0) return -1;

		if(parse->startElement)
			parse->startElement(parse->userdata, element_name, parse->attr);

		memset(parse->attr, 0, sizeof(parse->attr));

		if(ret == RF_ELEE_END)
		{
			if(parse->endElement)
				parse->endElement(parse->userdata, element_name);

			parse->e_stack[parse->e_stack_index] = NULL; parse->e_stack_index--;
			CHECK_XML_STACK_RET(parse);
		}

		return ret;
	}
}

#define CUR_PARSE_START		1
#define CUR_PARSE_END		2

int hxml_parse_element(LTXMLPRS * parse)
{
	char * xmlend = parse->xmlend;
	int parse_type = CUR_PARSE_START;
	while(1)
	{
		int ret = RF_NO_END;

xml_parse_type:
		while(IS_WHITE_SPACE(*(parse->ptr)) && ((parse->ptr) != xmlend)) (parse->ptr)++;
		if((parse->ptr) == xmlend)
		{
			if(parse->e_stack_index == 0)
				return 0;
			return -1;
		}


		if(*(parse->ptr) == '<' && *((parse->ptr)+1) == '/')
		{
			(parse->ptr)+=2;
			parse_type = CUR_PARSE_END;
		}
		else if(*(parse->ptr) == '<')
		{
			(parse->ptr)++;
			parse_type = CUR_PARSE_START;
		}
		else
		{
			return -1;
		}

		//xml_parse_point:

		while(IS_WHITE_SPACE(*(parse->ptr)) && ((parse->ptr) != xmlend)) (parse->ptr)++;
		if((parse->ptr) == xmlend)
		{
			if(parse->e_stack_index == 0)
				return 0;
			return -1;
		}

		if(parse_type == CUR_PARSE_END)
		{
			ret = hxml_parse_element_end(parse);
			if(ret < 0)
				return -1;

			if(parse->e_stack_index == 0)
				return 0;

			parse_type = CUR_PARSE_START;
		}
		else // CUR_PARSE_START
		{
			ret = hxml_parse_element_start(parse);
			if(ret < 0)
				return -1;
			if(ret == RF_ELEE_END)
			{
				if(parse->e_stack_index == 0)
					return 0;

				parse_type = CUR_PARSE_START;
				goto xml_parse_type;
			}

			while(IS_WHITE_SPACE(*(parse->ptr)) && ((parse->ptr) != xmlend)) (parse->ptr)++; 
			if((parse->ptr) == xmlend) return -1;

			if(*(parse->ptr) == '<') goto xml_parse_type; 

			char * cdata_ptr = (parse->ptr);
			while(*(parse->ptr) != '<' && (parse->ptr) != xmlend) (parse->ptr)++;
			if((parse->ptr) == xmlend) return -1;

			int len = (parse->ptr) - cdata_ptr;
			if(len > 0)	
			{
				*(parse->ptr) = '\0'; (parse->ptr)++;
				if(parse->charData)
					parse->charData(parse->userdata, cdata_ptr, len);

				if(*(parse->ptr) != '/')
					return -1;

				(parse->ptr)++;

				if(hxml_parse_element_end(parse) < 0)
					return -1;
			}

			goto xml_parse_type;
		}
	}

	return 0;
}

int hxml_parse(LTXMLPRS * parse)
{
	// //log_print("hxml parse header start...\r\n");
	int ret =  hxml_parse_header(parse);
	if(ret < 0)
	{
		// //log_print("hxml parse xml header failed!!!\r\n");
		// return -1;
	}

	// //log_print("hxml parse element start...\r\n");
	return hxml_parse_element(parse);
}

void xml_startElement(void * userdata, const char * name, const char ** attr)
{
	// //log_print("startElement[%s].\r\n", name);
	int i;
	for(i=0; ;i++)
	{
		if(attr[i] && attr[i+1])
		{
			//log_print("\tattr[%s]=[%s].\r\n", attr[i], attr[i+1]);
		}
		else
			break;
	}
}

void xml_endElement(void * userdata, const char * name)
{
	//log_print("endElement[%s].\r\n", name);
}

void xml_charData(void * userdata, const char * str, int len)
{
	//log_print("charData[%s].\r\n", str);
}

onvif_probe_cb g_probe_cb = 0;
void * g_probe_cb_data = 0;
pthread_t g_probe_thread = 0;
int g_probe_fd = 0;
int g_probe_interval = 30;
bool g_probe_running = false;


const char * get_local_ip()
{
	struct in_addr addr;
	addr.s_addr = get_default_if_ip();

	const char * p_ip = NULL;

	if (addr.s_addr != 0)
	{
		p_ip = inet_ntoa(addr);
	}
	else
	{
		int nums = get_if_nums();
		for (int i = 0; i < nums; i++)
		{
			addr.s_addr = get_if_ip(i);
			if (addr.s_addr != 0 && addr.s_addr != inet_addr("127.0.0.1"))
			{
				p_ip = inet_ntoa(addr);
			}
		}
	}

	//log_print("local ip %s\r\n", p_ip);

	return p_ip;
}


int onvif_probe_init()
{	
	SOCKET fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
	{
		//log_print("socket SOCK_DGRAM error!\n");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = INADDR_ANY; //inet_addr(get_local_ip());

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
	}

	BOOL flag = TRUE;

	if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char*) &flag, sizeof(TRUE)) == SOCKET_ERROR)
	{
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*) &flag, sizeof(TRUE)) == SOCKET_ERROR)
	{
	} 

	struct ip_mreq mcast;
	memset(&mcast, 0, sizeof(mcast));
	mcast.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
	mcast.imr_interface.s_addr = INADDR_ANY; // inet_addr(get_local_ip());

#if 0
	if(setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0)
	{
		//log_print("setsockopt IP_ADD_MEMBERSHIP error! %s\n",sys_os_get_socket_error());
		return -1;
	}
#else
	if(setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0)
	{
		if(setsockopt(fd, IPPROTO_IP, 5, (char*)&mcast, sizeof(mcast)) < 0)
		{
			//log_print("setsockopt IP_ADD_MEMBERSHIP error! %s\n",sys_os_get_socket_error());
			return -1;
		}
	}
#endif

	return fd;
}

char probe_req[] = 
	"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
	"xmlns:a=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\">"
	"<s:Header>"
	"<a:Action s:mustUnderstand=\"1\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</a:Action>"
	"<a:MessageID>uuid:160882d9-9f9f-4b6c-953e-802b85f62c22</a:MessageID>"
	"<a:ReplyTo>"
	"<a:Address>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</a:Address>"
	"</a:ReplyTo>"
	"<a:To s:mustUnderstand=\"1\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</a:To>"
	"</s:Header>"
	"<s:Body>"
	"<Probe xmlns=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\">"
	"<d:Types xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
	"xmlns:dp0=\"%s\">%s</d:Types>"
	"</Probe>"
	"</s:Body>"
	"</s:Envelope>";

int onvif_probe_req_tx(int fd)
{
	char send_buffer[1024 * 10];

	sprintf(send_buffer, probe_req, 
		"http://www.onvif.org/ver10/network/wsdl", 
		"dp0:NetworkVideoTransmitter");

	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("239.255.255.250");
	addr.sin_port = htons(3702);

	int len = strlen(send_buffer);
	int rlen = sendto(fd, send_buffer, len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (rlen != len)
	{
		//log_print("onvif_probe_req_tx::rlen = %d,slen = %d\r\n", rlen, len);
	}

	sprintf(send_buffer, probe_req, 
		"http://www.onvif.org/ver10/device/wsdl", 
		"dp0:Device");

	len = strlen(send_buffer);
	rlen = sendto(fd, send_buffer, len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (rlen != len)
	{
		//log_print("onvif_probe_req_tx::rlen = %d,slen = %d\r\n", rlen, len);
	}		

	return rlen;
}

int onvif_probe_res(XMLN * p_node, device_binfo * p_res)
{	
	XMLN * p_body = xml_node_soap_get(p_node, "s:Body");
	if (p_body)
	{
		XMLN * p_ProbeMatches = xml_node_soap_get(p_body, "d:ProbeMatches");
		if (p_ProbeMatches)
		{
			XMLN * p_ProbeMatch = xml_node_soap_get(p_ProbeMatches, "d:ProbeMatch");
			if (p_ProbeMatch)
			{
				XMLN * p_Types = xml_node_soap_get(p_ProbeMatch, "d:Types");
				if (p_Types && p_Types->data)
				{
					p_res->type = onvif_parse_type(p_Types->data);
				}

				XMLN * p_XAddrs = xml_node_soap_get(p_ProbeMatch, "d:XAddrs");
				if (p_XAddrs && p_XAddrs->data)
				{
					onvif_parse_xaddr(p_XAddrs->data, &p_res->xaddr);
				}

				return 0;
			}
		}
	}

	return -1;
}

void findDevice(onvif_device * pdevice, onvif_device ** pfound)
{
	onvif_device * p_dev = (onvif_device *) pps_lookup_start(m_dev_ul);
	while (p_dev)
	{
		if (p_dev->binfo.ip == pdevice->binfo.ip)
		{
			break;
		}

		p_dev = (onvif_device *) pps_lookup_next(m_dev_ul, p_dev);
	}

	pps_lookup_end(m_dev_ul);

	*pfound = p_dev;  
}

int onvif_probe_net_rx(int fd)
{
	char rbuf[65535];
	int ret;
	fd_set fdread;
	struct timeval tv = {1, 0};

	FD_ZERO(&fdread);
	FD_SET(fd, &fdread); 

	ret = select(fd+1, &fdread, NULL, NULL, &tv); 
	if (ret == 0) // Time expired 
	{ 
		return 0; 
	}
	else if (!FD_ISSET(fd, &fdread))
	{
		return -1;
	}

	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int rlen = recvfrom(fd, rbuf, sizeof(rbuf), 0, (struct sockaddr *)&addr, &addr_len);
	if (rlen <= 0)
	{
		//log_print("onvif_probe_net_rx::rlen = %d, fd = %d\r\n", rlen, fd);
		return -1;
	}

	unsigned int src_ip = addr.sin_addr.s_addr;
//	unsigned int src_port = addr.sin_port;

	XMLN * p_node = xxx_hxml_parse(rbuf, rlen);
	if (p_node == NULL)
	{
		//log_print("onvif_probe_net_rx::hxml parse err!!!\r\n", rlen, fd);
	}	
	else
	{
		device_binfo res;
		memset(&res, 0, sizeof(device_binfo));

		if (onvif_probe_res(p_node, &res) >= 0)
		{
			res.ip = src_ip;
			if (g_probe_cb)
			{
				g_probe_cb(&res, g_probe_cb_data);
			}
		}		
	}
	xml_node_del(p_node);

	return 0;
}

void * onvif_probe_thread(void * argv)
{
	//int count = 0;

	g_probe_fd = onvif_probe_init();
	if (g_probe_fd < 0)
	{
		g_probe_thread = 0;
		//log_print("onvif_probe_init fd failed\r\n");
		return NULL;
	}

	if (g_probe_interval < 10)
	{
		g_probe_interval = 30;
	}

	onvif_probe_req_tx(g_probe_fd);	
	usleep(500);
	while (g_probe_running)
	{
		onvif_probe_net_rx(g_probe_fd);
		usleep(500);
	}

	g_probe_thread = 0;

	return NULL;
}

void set_probe_cb(onvif_probe_cb cb, void * pdata)
{
	g_probe_cb = cb;
	g_probe_cb_data = pdata;
}


void send_probe_req()
{
	if (g_probe_fd > 0)
	{
		onvif_probe_req_tx(g_probe_fd);	
	}
}


int start_probe(int interval)
{
	g_probe_running = true;
	g_probe_interval = interval;

	g_probe_thread = sys_os_create_thread((void *)onvif_probe_thread, NULL);
	if (g_probe_thread)
	{
		return 0;
	}

	return -1;
}

void stop_probe()
{
	g_probe_running = false;

	if (g_probe_fd > 0)
	{
		closesocket(g_probe_fd);
		g_probe_fd = 0;
	}

	while (g_probe_thread)
	{
		usleep(100);
	}
}

BOOL onvif_StartRecvAlarm(onvif_device * p_dev)
{
	HTTPREQ http_req;
	onvif_init_req(&http_req, &p_dev->devCap.events.xaddr, p_dev);

	return http_onvif_trans(&http_req, 6000, eStartRecvAlarm);
}

BOOL onvif_GetAlarm(onvif_device * p_dev)
{
	HTTPREQ http_req;
	onvif_init_req(&http_req, &p_dev->devCap.message.xaddr, p_dev);

	return http_onvif_trans(&http_req, 6000, eRecvAlarm);
}

/***************************************************************************************/
HQUEUE * hqCreate(unsigned int unit_num,unsigned int unit_size,unsigned int queue_mode)
{
	unsigned int q_len = unit_num * unit_size + sizeof(HQUEUE);

	HQUEUE *phq = (HQUEUE *)XMALLOC(q_len);
	if(phq == NULL)
	{
		printf("hqCreate XMALLOC HQUEUE fail\n");
		return NULL;
	}
	phq->queue_mode = queue_mode;
	phq->unit_size = unit_size;
	phq->unit_num = unit_num;
	phq->front = 0;
	phq->rear = 0;
	phq->count_put_full = 0;
	phq->queue_buffer = sizeof(HQUEUE);

	if(queue_mode & HQ_NO_EVENT)
	{
	}
	else
	{
		phq->queue_nnulEvent = sys_os_create_sig();
		phq->queue_nfulEvent = sys_os_create_sig();
		phq->queue_putMutex = sys_os_create_mutex();
	}

	return phq;
}

void hqDelete(HQUEUE * phq)
{
	if(phq == NULL)return;

	if(phq->queue_mode & HQ_NO_EVENT)
	{
	}
	else
	{
		sys_os_destroy_sig_mutx(phq->queue_nnulEvent);
		sys_os_destroy_sig_mutx(phq->queue_nfulEvent);
		sys_os_destroy_sig_mutx(phq->queue_putMutex);
	}

	XFREE(phq);
}

/***************************************************************************************/
__inline void hqPutMutexEnter(HQUEUE * phq)
{
	if((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_mutex_enter(phq->queue_putMutex);
	}
}

__inline void hqPutMutexLeave(HQUEUE * phq)
{
	if((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_mutex_leave(phq->queue_putMutex);
	}
}

BOOL hqBufPut(HQUEUE * phq,char * buf)
{
	unsigned int real_rear,queue_count;
	unsigned int mem_addr;

	if(phq == NULL || buf == NULL)
	{
		//log_print("hqBufPut::phq=%p,buf=%p!!!",phq,buf);
		return FALSE;
	}

	hqPutMutexEnter(phq);

hqBufPut_start:

	queue_count = phq->rear - phq->front;

	if(queue_count == (phq->unit_num - 1))
	{
		if((phq->queue_mode & HQ_NO_EVENT) == 0)
		{
			if(phq->queue_mode & HQ_PUT_WAIT){
				sys_os_sig_wait(phq->queue_nfulEvent);
				goto hqBufPut_start;
			}
			else {
				phq->count_put_full++;
				hqPutMutexLeave(phq);
				//log_print("hqBufPut::queue_count=%d,full!!!",queue_count);
				return FALSE;
			}
		}
		else
		{
			hqPutMutexLeave(phq);
			return FALSE;
		}
	}

	real_rear = phq->rear % phq->unit_num;
	mem_addr = phq->queue_buffer + real_rear*phq->unit_size + (unsigned long)phq;
	memcpy((char *)mem_addr,buf,phq->unit_size);
	phq->rear++;

	if((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_sig_sign(phq->queue_nnulEvent);
	}

	hqPutMutexLeave(phq);

	return TRUE;
}

BOOL hqBufGet(HQUEUE * phq,char * buf)
{
	unsigned int real_front;

	if(phq == NULL || buf == NULL)
		return FALSE;

hqBufGet_start:

	if(phq->front == phq->rear)
	{
		if((phq->queue_mode & HQ_NO_EVENT) == 0)
		{
			if(phq->queue_mode & HQ_GET_WAIT){
				sys_os_sig_wait(phq->queue_nnulEvent);
				goto hqBufGet_start;
			}
			else {
				return FALSE;
			}
		}
		else
			return FALSE;
	}

	real_front = phq->front % phq->unit_num;
	memcpy(buf,((char *)phq) + phq->queue_buffer + real_front*phq->unit_size,phq->unit_size);
	phq->front++;

	if((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_sig_sign(phq->queue_nfulEvent);
	}

	return TRUE;
}

BOOL hqBufIsEmpty(HQUEUE * phq)
{
	if(phq == NULL)
		return TRUE;

	if(phq->front == phq->rear)
		return TRUE;

	return FALSE;
}

char * hqBufGetWait(HQUEUE * phq)
{
	unsigned int real_front;
	char * ptr = NULL;

	if(phq == NULL)
		return NULL;

hqBufGet_start:

	if(phq->front == phq->rear)
	{
		if((phq->queue_mode & HQ_NO_EVENT) == 0)
		{
			if(phq->queue_mode & HQ_GET_WAIT){
				sys_os_sig_wait(phq->queue_nnulEvent);
				goto hqBufGet_start;
			}
			else {
				return NULL;
			}
		}
		else
			return NULL;
	}

	real_front = phq->front % phq->unit_num;

	ptr = ((char *)phq) + phq->queue_buffer + real_front*phq->unit_size;
	return ptr;
}

// memcpy(buf,((char *)phq) + phq->queue_buffer + real_front*phq->unit_size,phq->unit_size);

void hqBufGetWaitPost(HQUEUE * phq)
{
	if(phq == NULL)
		return;

	phq->front++;

	if((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_sig_sign(phq->queue_nfulEvent);
	}
}

BOOL hqBufPeek(HQUEUE * phq,char * buf)
{
	unsigned int real_front;

	if(phq == NULL || buf == NULL)
		return FALSE;

hqBufPeek_start:

	if(phq->front == phq->rear)
	{
		if((phq->queue_mode & HQ_NO_EVENT) == 0)
		{
			if(phq->queue_mode & HQ_GET_WAIT){
				sys_os_sig_wait(phq->queue_nnulEvent);
				goto hqBufPeek_start;
			}
			else {
				return FALSE;
			}
		}
		else
			return FALSE;
	}

	real_front = phq->front % phq->unit_num;
	memcpy(buf,((char *)phq) + phq->queue_buffer + real_front*phq->unit_size,phq->unit_size);

	return TRUE;
}

/***************************************************************************************/
int dq_string_get(char *word_buf, unsigned int word_buf_len, char *input_buf, unsigned int *offset)
{
	unsigned int cur_offset = *offset;
	memset(word_buf,0,word_buf_len);
	if (input_buf[cur_offset] == '"')
	{
		cur_offset++;
		while(input_buf[cur_offset] != '"')cur_offset++;
		unsigned int dq_str_len = cur_offset - (*offset) -1;
		if (dq_str_len >= word_buf_len)
		{
			//log_print("asn_dq_string_get::string len %d > max len\r\n",dq_str_len);
			return -1;
		}

		memcpy(word_buf,input_buf+(*offset)+1,dq_str_len);

		*offset = ++cur_offset;
		return 0;
	}

	return -1;
}

BOOL is_char(char ch)
{
	if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
		(ch == '#') || (ch == '@') || (ch == '/'))
		return TRUE;
	return FALSE;
}

BOOL is_num(char ch)
{
	if(ch >= '0' && ch <= '9')
		return TRUE;
	return FALSE;
}

static char separator[]={' ','\t','\r','\n',',',':','{','}','(',')','\0','\'','"','?','<','>','=',';'};
				  
BOOL is_separator(char ch)
{
	unsigned int i;
	for(i=0; i<sizeof(separator); i++)
	{
		if(separator[i] == ch)
			return TRUE;
	}

	return FALSE;
}

BOOL GetLineText(char *buf,int cur_line_offset,int max_len,int * len,int * next_line_offset)
{
	char * ptr_start = buf+cur_line_offset;
	char * ptr_end = ptr_start;
	int	   line_len;
	BOOL   bHaveNextLine=TRUE;

	while( (*ptr_end != '\r') && (*ptr_end != '\n') &&
		(*ptr_end != '\0') && ((ptr_end - ptr_start) < max_len))
		ptr_end++;

	/*
		WWW-Authenticate: Digest realm="huawei.com",
		 nonce="4a04c0af2089e51f12788606b4bf8edd",stale=false,algorithm=MD5
	*/
	while(*(ptr_end-1) == ',')
	{
		while((*ptr_end == '\r') || (*ptr_end == '\n'))
		{
			*ptr_end = ' ';	
			ptr_end++;
		}

		while( (*ptr_end != '\r') && (*ptr_end != '\n') &&
			(*ptr_end != '\0') && ((ptr_end - ptr_start) < max_len))
			ptr_end++;
	}

	line_len = ptr_end - ptr_start;

	if((*ptr_end == '\r') && (*(ptr_end+1) == '\n'))
	{
		line_len += 2;
		if(line_len == max_len)
			bHaveNextLine = FALSE;
	}
	else if(*ptr_end == '\n')
	{
		line_len += 1;
		if(line_len == max_len)
			bHaveNextLine = FALSE;
	}
	else if((*ptr_end == '\0') || ((ptr_end - ptr_start) < max_len))
		bHaveNextLine = FALSE;
	else	
		bHaveNextLine = FALSE;

	*len = ptr_end - ptr_start;
	*next_line_offset = cur_line_offset + line_len;

	return bHaveNextLine;
}


BOOL GetSipLine(char *p_buf, int max_len, int * len, BOOL * bHaveNextLine)
{
	char * ptr_start = p_buf;
	char * ptr_end = ptr_start;
	int	   line_len;

	*bHaveNextLine = TRUE;

	while((*ptr_end != '\0') &&  (!((*ptr_end == '\r') && (*(ptr_end+1) == '\n'))) && ((ptr_end - ptr_start) < max_len))	ptr_end++;

	/*
		WWW-Authenticate: Digest realm="huawei.com",
		 nonce="4a04c0af2089e51f12788606b4bf8edd",stale=false,algorithm=MD5
	*/
	while(*(ptr_end-1) == ',')
	{
		while((*ptr_end == '\r') || (*ptr_end == '\n'))
		{
			*ptr_end = ' ';	
			ptr_end++;
		}

		while( (*ptr_end != '\r') && (*ptr_end != '\n') &&
			(*ptr_end != '\0') && ((ptr_end - ptr_start) < max_len))
			ptr_end++;
	}

	line_len = ptr_end - ptr_start;

	if((*ptr_end == '\r') && (*(ptr_end+1) == '\n'))
	{
		*ptr_end = '\0';
		*(ptr_end+1) = '\0';
		line_len += 2;
		if(line_len == max_len)
			*bHaveNextLine = FALSE;

		*len = line_len;
		return TRUE;
	}
	else	
		return FALSE;
}

BOOL GetLineWord(char *line,int cur_word_offset,int line_max_len,char *word_buf,int buf_len,int *next_word_offset,WORD_TYPE w_t)
{
	char *	ptr_start = line+cur_word_offset;
	char *	ptr_end = ptr_start;
	BOOL	bHaveNextWord = TRUE;

	word_buf[0] = '\0';	

	while(((*ptr_start == ' ') || (*ptr_start == '\t')) && 
		(cur_word_offset < line_max_len))
	{ 
		cur_word_offset++; 
		ptr_start++;
	}

	if(*ptr_start == '\0')
		return FALSE;	

	ptr_end = ptr_start;

	while((!is_separator(*ptr_end)) && ((ptr_end - ptr_start) < line_max_len))
		ptr_end++;

	int len = ptr_end - ptr_start;
	if(len >= buf_len)
	{
		word_buf[0] = '\0';
		return bHaveNextWord;
	}

	*next_word_offset = cur_word_offset + len;
	if((*next_word_offset >= line_max_len) || (line[*next_word_offset] == '\0'))
		bHaveNextWord = FALSE;

	switch(w_t)
	{
	case WORD_TYPE_NULL:
		break;

	case WORD_TYPE_STRING:
		{
			if(len == 0 && is_separator(*ptr_end))
			{
				(*next_word_offset)++;
				word_buf[0] = *ptr_end;
				word_buf[1] = '\0';
				return bHaveNextWord;
			}
		}
		break;

	case WORD_TYPE_NUM:
		{
			char * ptr;
			for(ptr=ptr_start; ptr<ptr_end; ptr++)
			{
				if(!is_num(*ptr))
				{
					word_buf[0] = '\0';
					return bHaveNextWord;
				}
			}
		}
		break;

	case WORD_TYPE_SEPARATOR:
		{
			if(is_separator(*ptr_end))
			{
				(*next_word_offset)++;
				word_buf[0] = *ptr_end;
				word_buf[1] = '\0';
				return bHaveNextWord;
			}
		}
		break;
	}

	memcpy(word_buf,ptr_start,len);
	word_buf[len] = '\0';

	return bHaveNextWord;
}

BOOL GetNameValuePair(char * text_buf,int text_len,const char * name,char *value,int value_len)
{
	char word_buf[256];
	int	 cur_offset=0,next_offset=0;
	char * value_start, * value_end;

	while(next_offset < text_len)
	{
		word_buf[0] = '\0';

		cur_offset = next_offset;

		GetLineWord(text_buf,cur_offset,text_len,word_buf,
			sizeof(word_buf),&next_offset,WORD_TYPE_STRING);
		if(strcmp(word_buf,name) == 0)
		{
			char * ptr = text_buf + next_offset;
			while((*ptr == ' ' || *ptr == '\t') && (next_offset <text_len))
					{ ptr++;next_offset++;}
			if((*ptr == ';') || (*ptr == ',') || (*ptr == '\0'))//空值的情况
			{
				value[0] = '\0';
				return TRUE;
			}

			if(*ptr != '=') return FALSE;

			ptr++;next_offset++;

			while((*ptr == ' ' || *ptr == '\t') && (next_offset <text_len))
					{ ptr++;next_offset++;}
			if(*ptr != '"')
			{
				value_start = ptr;
				while((*ptr != ';') && (*ptr != ',') && (next_offset <text_len))
					{ ptr++;next_offset++;}
				if((*ptr != ';') && (*ptr != ',') && (*ptr != '\0'))
					return FALSE;
				value_end = ptr;
			}
			else
			{
				ptr++;next_offset++;
				value_start = text_buf + next_offset;
				while((*ptr != '"') && (next_offset <text_len))
					{ ptr++;next_offset++;}
				if(*ptr != '"') return FALSE;
				value_end = ptr;
			}

			int len = value_end - value_start;
			if(len > value_len)
			{
				len = value_len - 1;
			}
			memcpy(value,value_start,len);
			value[len] = '\0';

			return TRUE;
		}
		else	
		{
			char * ptr = text_buf + next_offset;
			while((*ptr == ' ' || *ptr == '\t') && (next_offset <text_len))
					{ ptr++;next_offset++;}

			if((*ptr == ';') || (*ptr == ','))
			{
				next_offset++;
				continue;
			}

			if(*ptr != '=') return FALSE;

			ptr++;next_offset++;

			while((*ptr == ' ' || *ptr == '\t') && (next_offset <text_len))
					{ ptr++;next_offset++;}
			if(*ptr != '"')
			{
				while((*ptr != ';') && (*ptr != ',') && (next_offset <text_len))
					{ ptr++;next_offset++;}
				if((*ptr != ';') && (*ptr != ',') && (*ptr != '\0'))
					return FALSE;
				next_offset++;	
			}
			else
			{
				ptr++;next_offset++;
				while((*ptr != '"') && (next_offset <text_len))
					{ ptr++;next_offset++;}
				if(*ptr != '"') return FALSE;
				ptr++;next_offset++;
				while((*ptr == ' ' || *ptr == '\t') && (next_offset <text_len))
					{ ptr++;next_offset++;}
				if(*ptr != ',') return FALSE;
				next_offset++;	
			}
		}
	}

	return FALSE;
}

BOOL is_ip_address(const char * address)
{
	unsigned short byte_value;
	int i;

	int total_len = strlen(address);
	if(total_len > 15)
		return FALSE;

	int index = 0;

	for(i=0; i<4; i++)
	{
		if((address[index] < '0') || (address[index] > '9'))
			return FALSE;

		if(((address[index +1] < '0') || (address[index +1] > '9')) 
			&& (address[index +1] != '.'))
		{
			if((address[index +1] == '\0') && (i == 3))
				return TRUE;
			return FALSE;
		}
		if(address[index +1] == '.'){index+=2;continue;}

		if(((address[index +2] < '0') || (address[index +2] > '9'))	
			&& (address[index +2] != '.'))
		{
			if((address[index +2] == '\0') && (i == 3))
				return TRUE;
			return FALSE;
		}
		if(address[index +2] == '.'){index+=3;continue;}

		if(i < 3)	
		{
			if(address[index +3] != '.')
				return FALSE;
		}

		byte_value = (address[index] - '0') *100 + 
			(address[index +1] -'0') *10 + (address[index +2] - '0');

		if(byte_value > 255)
			return FALSE;

		if(i < 3)index += 4;
		else	 index += 3;
	}

	if(index != total_len)
		return FALSE;	

	return TRUE;
}

BOOL is_integer(char * p_str)
{
	int i;
	int len = strlen(p_str);
	for(i=0; i<len; i++)
		if(!is_num(p_str[i])) return FALSE;

	return TRUE;	
}

/***************************************************************************************/
int get_if_nums()
{
#if __WIN32_OS__

	char ipt_buf[512];
	MIB_IPADDRTABLE *ipt = (MIB_IPADDRTABLE *)ipt_buf;
	ULONG ipt_len = sizeof(ipt_buf);
	DWORD fr = GetIpAddrTable(ipt,&ipt_len,FALSE);
	if (fr != NO_ERROR)
		return 0;

	return ipt->dwNumEntries;

#elif __LINUX_OS__

	int socket_fd;
	struct ifconf conf;
	char buff[BUFSIZ];
	int num;

	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd <= 0)
	{
		return 0;
	}

	conf.ifc_len = BUFSIZ;
	conf.ifc_buf = buff;

	ioctl(socket_fd, SIOCGIFCONF, &conf);

	num = conf.ifc_len / sizeof(struct ifreq);

	closesocket(socket_fd);

	return num;

#endif

	return 0;
}

unsigned int get_if_ip(int index)
{
#if __WIN32_OS__

	char ipt_buf[512];
	MIB_IPADDRTABLE *ipt = (MIB_IPADDRTABLE *)ipt_buf;
	ULONG ipt_len = sizeof(ipt_buf);
	DWORD fr = GetIpAddrTable(ipt,&ipt_len,FALSE);
	if (fr != NO_ERROR)
		return 0;

	for (DWORD i=0; i<ipt->dwNumEntries; i++)
	{
		if (i == index)
			return ipt->table[i].dwAddr;
	}

#elif __LINUX_OS__

	int socket_fd;
	//struct sockaddr_in *sin;
	struct ifreq *ifr;
	struct ifconf conf;
	char buff[BUFSIZ];
	int num;

	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

	conf.ifc_len = BUFSIZ;
	conf.ifc_buf = buff;

	ioctl(socket_fd, SIOCGIFCONF, &conf);

	num = conf.ifc_len / sizeof(struct ifreq);
	ifr = conf.ifc_req;

	unsigned int ip_addr = 0;

	for (int i=0; i<num; i++)
	{
		if (i == index)
		{
			struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

			ioctl(socket_fd, SIOCGIFFLAGS, ifr);
			if ((ifr->ifr_flags & IFF_LOOPBACK) != 0)
			{
				ip_addr = 0;
			}
			else
			{
				ip_addr = sin->sin_addr.s_addr;
			}

			break;
		}

		ifr++;
	}

	closesocket(socket_fd);

	return ip_addr;

#endif

	return 0;
}

unsigned int get_route_if_ip(unsigned int dst_ip)
{
#if __WIN32_OS__

	DWORD dwIfIndex,fr;

	fr = GetBestInterface(dst_ip,&dwIfIndex);
	if (fr != NO_ERROR)
		return 0;

	char ipt_buf[512];
	MIB_IPADDRTABLE *ipt = (MIB_IPADDRTABLE *)ipt_buf;
	ULONG ipt_len = sizeof(ipt_buf);
	fr = GetIpAddrTable(ipt,&ipt_len,FALSE);
	if (fr != NO_ERROR)
		return 0;

	for (DWORD i=0; i<ipt->dwNumEntries; i++)
	{
		if (ipt->table[i].dwIndex == dwIfIndex)
			return ipt->table[i].dwAddr;
	}

#elif __VXWORKS_OS__

	char tmp_buf[24];
	char ifname[32];
	sprintf(ifname,"%s%d",hsip.ifname,0);
	STATUS ret = ifAddrGet(ifname,tmp_buf);
	if (ret == OK)
	{
		return inet_addr(tmp_buf);
	}

#elif __LINUX_OS__

	int socket_fd;
	//struct sockaddr_in *sin;
	struct ifreq *ifr;
	struct ifconf conf;
	char buff[BUFSIZ];
	int num;

	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

	conf.ifc_len = BUFSIZ;
	conf.ifc_buf = buff;

	ioctl(socket_fd, SIOCGIFCONF, &conf);

	num = conf.ifc_len / sizeof(struct ifreq);
	ifr = conf.ifc_req;

	unsigned int ip_addr = 0;

	for (int i=0; i<num; i++)
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

		ioctl(socket_fd, SIOCGIFFLAGS, ifr);
		if (((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
		{
			ip_addr = sin->sin_addr.s_addr;
			break;
		}

		ifr++;
	}

	closesocket(socket_fd);	
	return ip_addr;

#endif

	return 0;
}

unsigned int get_default_if_ip()
{
	return get_route_if_ip(0);
}

int get_default_if_mac(unsigned char * mac)
{
#if __WIN32_OS__

	IP_ADAPTER_INFO AdapterInfo[16];            // Allocate information for up to 16 NICs  
	DWORD dwBufLen = sizeof(AdapterInfo);       // Save the memory size of buffer  

	DWORD dwStatus = GetAdaptersInfo(           // Call GetAdapterInfo  
		AdapterInfo,                            // [out] buffer to receive data  
		&dwBufLen);                             // [in] size of receive data buffer  

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info  
	if (pAdapterInfo)
	{
		memcpy(mac, pAdapterInfo->Address, 6);	
		return 0;
	}  

#elif __LINUX_OS__

	int socket_fd;
	//struct sockaddr_in *sin;
	struct ifreq *ifr;
	struct ifreq ifreq;
	struct ifconf conf;
	char buff[BUFSIZ];
	int num;

	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

	conf.ifc_len = BUFSIZ;
	conf.ifc_buf = buff;

	ioctl(socket_fd, SIOCGIFCONF, &conf);

	num = conf.ifc_len / sizeof(struct ifreq);
	ifr = conf.ifc_req;

	for (int i=0; i<num; i++)
	{
		//struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

		if (ifr->ifr_addr.sa_family != AF_INET)
		{
			ifr++;
			continue;
		}

		ioctl(socket_fd, SIOCGIFFLAGS, ifr);
		if ((ifr->ifr_flags & IFF_LOOPBACK) != 0)
		{
			ifr++;
			continue;
		}

		strncpy(ifreq.ifr_name, ifr->ifr_name, sizeof(ifreq.ifr_name));
		if (ioctl(socket_fd, SIOCGIFHWADDR, &ifreq) < 0) 
		{
			ifr++;
			continue;
		}

		memcpy(mac, &ifreq.ifr_hwaddr.sa_data, 6);		

		close(socket_fd);
		return 0;
	}

	close(socket_fd);
#endif	

	return -1;
}

unsigned int get_address_by_name(char *host_name)
{
	unsigned int addr = 0;

	if (is_ip_address(host_name))
		addr = inet_addr(host_name);
	else
	{
		struct hostent * remoteHost = gethostbyname(host_name);
		if (remoteHost)
			addr = *(unsigned long *)(remoteHost->h_addr);
	}

	return addr;
}

char * lowercase(char *str) 
{
	unsigned int i;

	for (i = 0; i < strlen(str); ++i)
		str[i] = tolower(str[i]);

	return str;
}

char * uppercase(char *str)
{
	unsigned int i;

	for (i = 0; i < strlen(str); ++i)
		str[i] = toupper(str[i]);

	return str;
}

#define MIN(a, b)  ((a) < (b) ? (a) : (b))

int unicode(char **dst, char *src) 
{
	char *ret;
	int l, i;

	if (!src) 
	{
		*dst = NULL;
		return 0;
	}

	l = MIN(64, strlen(src));
	ret = (char *)XMALLOC(2*l);
	for (i = 0; i < l; ++i)
	{
		ret[2*i] = src[i];
		ret[2*i+1] = '\0';
	}

	*dst = ret;
	return 2*l;
}

static char hextab[17] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 0};
static int hexindex[128] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,
	-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

char * printmem(char *src, size_t len, int bitwidth) 
{
	char *tmp;
	unsigned int i;

	tmp = (char *)XMALLOC(2*len+1);
	for (i = 0; i < len; ++i) 
	{
		tmp[i*2] = hextab[((unsigned char)src[i] ^ (unsigned char)(7-bitwidth)) >> 4];
		tmp[i*2+1] = hextab[(src[i] ^ (unsigned char)(7-bitwidth)) & 0x0F];
	}

	return tmp;
}

char * scanmem(char *src, int bitwidth) 
{
	int h, l, i, bytes;
	char *tmp;

	if (strlen(src) % 2)
		return NULL;

	bytes = strlen(src)/2;
	tmp = (char *)XMALLOC(bytes+1);
	for (i = 0; i < bytes; ++i) 
	{
		h = hexindex[(int)src[i*2]];
		l = hexindex[(int)src[i*2+1]];
		if (h < 0 || l < 0) 
		{
			XFREE(tmp);
			return NULL;
		}
		tmp[i] = ((h << 4) + l) ^ (unsigned char)(7-bitwidth);
	}
	tmp[i] = 0;

	return tmp;
}


time_t get_time_by_string(char * p_time_str)
{
	struct tm st;
	memset(&st, 0, sizeof(struct tm));

	char * ptr_s = p_time_str;
	while(*ptr_s == ' ' || *ptr_s == '\t') ptr_s++;

	sscanf(ptr_s, "%04d-%02d-%02d %02d:%02d:%02d", &st.tm_year, &st.tm_mon, &st.tm_mday, &st.tm_hour, &st.tm_min, &st.tm_sec);

	st.tm_year -= 1900;
	st.tm_mon -= 1;

	time_t t = mktime(&st);
	return t;
}

int tcp_connect_timeout(unsigned int rip, int port, int timeout)
{
	int flag = 0;
	int cfd = socket(AF_INET, SOCK_STREAM, 0);
	if (cfd < 0)
		return -1;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = rip;
	addr.sin_port = htons((unsigned short)port);


	unsigned long ul = 1;
#if	__WIN32_OS__
	ioctlsocket(cfd, FIONBIO, &ul);
#elif __LINUX_OS__
	ioctl(cfd, FIONBIO, &ul);
#endif

	if (connect(cfd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
	{
		struct timeval tv;
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000) * 1000;

		fd_set set;
		FD_ZERO(&set);
		FD_SET(cfd, &set);

		if (select(cfd+1, NULL, &set, NULL, &tv) > 0)
		{
			int err=0, len=sizeof(int);
			getsockopt(cfd, SOL_SOCKET, SO_ERROR, (char *)&err, (socklen_t*) &len);
			if (err == 0)
				flag = 1;
		}
	}

	if (flag == 1)
	{
		ul = 0;
#if	__WIN32_OS__
		ioctlsocket(cfd, FIONBIO, &ul);
#elif __LINUX_OS__
		ioctl(cfd, FIONBIO, &ul);
#endif
		return cfd;
	}
	else
	{
		closesocket(cfd);
		return -1;
	}
}

const char *BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void _base64_encode_triple(unsigned char triple[3], char result[4])
{
    int tripleValue, i;

    tripleValue = triple[0];
    tripleValue *= 256;
    tripleValue += triple[1];
    tripleValue *= 256;
    tripleValue += triple[2];

    for (i=0; i<4; i++)
    {
		result[3-i] = BASE64_CHARS[tripleValue%64];
		tripleValue /= 64;
    }
}

/**
 * encode an array of bytes using Base64 (RFC 3548)
 *
 * @param source the source buffer
 * @param sourcelen the length of the source buffer
 * @param target the target buffer
 * @param targetlen the length of the target buffer
 * @return 1 on success, 0 otherwise
 */
int base64_encode(unsigned char *source, unsigned int sourcelen, char *target, unsigned int targetlen)
{
    /* check if the result will fit in the target buffer */
    if ((sourcelen+2)/3*4 > targetlen-1)
		return 0;

    /* encode all full triples */
    while (sourcelen >= 3)
    {
		_base64_encode_triple(source, target);
		sourcelen -= 3;
		source += 3;
		target += 4;
    }

    /* encode the last one or two characters */
    if (sourcelen > 0)
    {
		unsigned char temp[3];
		memset(temp, 0, sizeof(temp));
		memcpy(temp, source, sourcelen);
		_base64_encode_triple(temp, target);
		target[3] = '=';
		if (sourcelen == 1)
		    target[2] = '=';

		target += 4;
    }

    /* terminate the string */
    target[0] = 0;

    return 1;
}

/**
 * decode a base64 string and put the result in the same buffer as the source
 *
 * This function does not handle decoded data that contains the null byte
 * very well as the size of the decoded data is not returned.
 *
 * The result will be zero terminated.
 *
 * @deprecated use base64_decode instead
 *
 * @param str buffer for the source and the result
 */
void str_b64decode(char* str)
{
    size_t decoded_length;

    decoded_length = base64_decode(str, (unsigned char *)str, strlen(str));
    str[decoded_length] = '\0';
}

/**
 * decode base64 encoded data
 *
 * @param source the encoded data (zero terminated)
 * @param target pointer to the target buffer
 * @param targetlen length of the target buffer
 * @return length of converted data on success, -1 otherwise
 */
int base64_decode(const char *source, unsigned char *target, unsigned int targetlen) 
{
    const char *cur;
    unsigned char *dest, *max_dest;
    int d, dlast, phase;
    unsigned char c;
    static int table[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
    };

    d = dlast = phase = 0;
    dest = target;
    max_dest = dest+targetlen;

    for (cur = source; *cur != '\0' && dest<max_dest; ++cur) 
    {
        d = table[(int)*cur];
        if (d != -1) 
        {
            switch(phase) 
            {
			case 0:
			    ++phase;
			    break;
			case 1:
			    c = ((dlast << 2) | ((d & 0x30) >> 4));
			    *dest++ = c;
			    ++phase;
			    break;
			case 2:
			    c = (((dlast & 0xf) << 4) | ((d & 0x3c) >> 2));
			    *dest++ = c;
			    ++phase;
			    break;
			case 3:
			    c = (((dlast & 0x03 ) << 6) | d);
			    *dest++ = c;
			    phase = 0;
			    break;
		    }
		    
            dlast = d;
        }
    }

    /* we decoded the whole buffer */
    if (*cur == '\0') 
    {
		return dest-target;
    }

    /* we did not convert the whole data, buffer was to small */
    return -1;
}