
#pragma once
#include "Base/Base.h"
using namespace Public::Base;

struct RTSP_RESPONSE
{
public:
	RTSP_RESPONSE();
	~RTSP_RESPONSE();
public:
	int content_length;
	int cseq;
	int close_connection;
	char retcode[4];
	char *caption;
	char *retresp;
	char *body;
	char *accept;
	char *accept_encoding;
	char *accept_language;
	char *allow_public;
	char *authorization;
	char *bandwidth;
	char *blocksize;
	char *cache_control;
	char *content_base;
	char *content_encoding;
	char *content_language;
	char *content_location;
	char *content_type;
	char *cookie;
	char *date;
	char *expires;
	char *from;
	char *if_modified_since;
	char *last_modified;
	char *location;
	char *proxy_authenticate;
	char *proxy_require;
	char *range;
	char *referer;
	char *require;
	char *retry_after;
	char *rtp_info;
	char *scale;
	char *server;
	char *session;
	char *speed;
	char *transport;
	char *unsupported;
	char *user_agent;
	char *via;
	char *www_authenticate;
};

#define RTSPHEADERFLAG "RTSP/1.0"
#define RTSPENDFLAG1 "\r\n\r\n"
#define RTSPENDFLAG2 "\n\n"

bool ParseResponse(const std::string& pResponse, RTSP_RESPONSE& lpResponse);