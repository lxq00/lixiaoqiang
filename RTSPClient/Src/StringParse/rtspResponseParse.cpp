#include "rtspResponseParse.h"

#ifndef WIN32
#define _strdup strdup
#endif

#define SEPERATOR				"\r\n"

#define SKIP_SPACE(a) {while (isspace(*(a)) && (*(a) != '\0'))(a)++;}
#define SET_DEC_FIELD(a) set_dec_field(&dec->a, line)

#define ADD_FIELD(fmt, value) \
	ret = sprintf(buffer + *at, (fmt), (value)); \
	*at += ret;

static void set_dec_field(char **field, const char *line)
{
	if (*field == 0)
	{
		*field = _strdup(line);
	}
	else
	{
		//DebugInfo("decoder fileld exist, old:%s new:%s\n", *field, line);
	}
}

#define CHECK_AND_FREE(a) if ((a) != 0) { free((void *)(a)); (a) = 0;}

#define RTSP_HEADER_FUNC(a) static void a (const char *line, RTSP_RESPONSE *dec)
RTSP_HEADER_FUNC(rtsp_header_allow_public)
{
	SET_DEC_FIELD(allow_public);
}
RTSP_HEADER_FUNC(rtsp_header_connection)
{
	if (strncmp(line, "close", strlen("close")) == 0)
	{
		dec->close_connection = 1;
	}
	else
	{
		dec->close_connection = 0;
	}
}
RTSP_HEADER_FUNC(rtsp_header_cookie)
{
	SET_DEC_FIELD(cookie);
}
RTSP_HEADER_FUNC(rtsp_header_content_base)
{
	SET_DEC_FIELD(content_base);
}
RTSP_HEADER_FUNC(rtsp_header_content_length)
{
	dec->content_length = atoi(line);
}
RTSP_HEADER_FUNC(rtsp_header_content_loc)
{
	SET_DEC_FIELD(content_location);
}
RTSP_HEADER_FUNC(rtsp_header_content_type)
{
	SET_DEC_FIELD(content_type);
}
RTSP_HEADER_FUNC(rtsp_header_cseq)
{
	dec->cseq = atoi(line);
}
RTSP_HEADER_FUNC(rtsp_header_location)
{
	SET_DEC_FIELD(location);
}
RTSP_HEADER_FUNC(rtsp_header_range)
{
	SET_DEC_FIELD(range);
}
RTSP_HEADER_FUNC(rtsp_header_retry_after)
{
	SET_DEC_FIELD(retry_after);
}
RTSP_HEADER_FUNC(rtsp_header_rtp)
{
	SET_DEC_FIELD(rtp_info);
}
RTSP_HEADER_FUNC(rtsp_header_session)
{
	SET_DEC_FIELD(session);
}
RTSP_HEADER_FUNC(rtsp_header_speed)
{
	SET_DEC_FIELD(speed);
}
RTSP_HEADER_FUNC(rtsp_header_transport)
{
	SET_DEC_FIELD(transport);
}
RTSP_HEADER_FUNC(rtsp_header_www)
{
	SET_DEC_FIELD(www_authenticate);
}
RTSP_HEADER_FUNC(rtsp_header_accept)
{
	SET_DEC_FIELD(accept);
}
RTSP_HEADER_FUNC(rtsp_header_accept_enc)
{
	SET_DEC_FIELD(accept_encoding);
}
RTSP_HEADER_FUNC(rtsp_header_accept_lang)
{
	SET_DEC_FIELD(accept_language);
}
RTSP_HEADER_FUNC(rtsp_header_auth)
{
	SET_DEC_FIELD(authorization);
}
RTSP_HEADER_FUNC(rtsp_header_bandwidth)
{
	SET_DEC_FIELD(bandwidth);
}
RTSP_HEADER_FUNC(rtsp_header_blocksize)
{
	SET_DEC_FIELD(blocksize);
}
RTSP_HEADER_FUNC(rtsp_header_cache_control)
{
	SET_DEC_FIELD(cache_control);
}
RTSP_HEADER_FUNC(rtsp_header_content_enc)
{
	SET_DEC_FIELD(content_encoding);
}
RTSP_HEADER_FUNC(rtsp_header_content_lang)
{
	SET_DEC_FIELD(content_language);
}
RTSP_HEADER_FUNC(rtsp_header_date)
{
	SET_DEC_FIELD(date);
}
RTSP_HEADER_FUNC(rtsp_header_expires)
{
	SET_DEC_FIELD(expires);
}
RTSP_HEADER_FUNC(rtsp_header_from)
{
	SET_DEC_FIELD(from);
}
RTSP_HEADER_FUNC(rtsp_header_ifmod)
{
	SET_DEC_FIELD(if_modified_since);
}
RTSP_HEADER_FUNC(rtsp_header_lastmod)
{
	SET_DEC_FIELD(last_modified);
}
RTSP_HEADER_FUNC(rtsp_header_proxyauth)
{
	SET_DEC_FIELD(proxy_authenticate);
}
RTSP_HEADER_FUNC(rtsp_header_proxyreq)
{
	SET_DEC_FIELD(proxy_require);
}
RTSP_HEADER_FUNC(rtsp_header_referer)
{
	SET_DEC_FIELD(referer);
}
RTSP_HEADER_FUNC(rtsp_header_scale)
{
	SET_DEC_FIELD(scale);
}
RTSP_HEADER_FUNC(rtsp_header_server)
{
	SET_DEC_FIELD(server);
}
RTSP_HEADER_FUNC(rtsp_header_unsup)
{
	SET_DEC_FIELD(unsupported);
}
RTSP_HEADER_FUNC(rtsp_header_uagent)
{
	SET_DEC_FIELD(user_agent);
}
RTSP_HEADER_FUNC(rtsp_header_via)
{
	SET_DEC_FIELD(via);
}

struct HEADER_TYPES
{
	const char *val;
	int val_length;
	void(*parse_routine)(const char *line, RTSP_RESPONSE *decode);
};



HEADER_TYPES header_types[] =
{
#define HEAD_TYPE(a, b) { a, sizeof(a) - 1, b }
	HEAD_TYPE("AlLow", rtsp_header_allow_public),
	HEAD_TYPE("Public", rtsp_header_allow_public),
	HEAD_TYPE("Connection", rtsp_header_connection),
	HEAD_TYPE("Content-Base", rtsp_header_content_base),
	HEAD_TYPE("Content-Length", rtsp_header_content_length),
	HEAD_TYPE("Content-Location", rtsp_header_content_loc),
	HEAD_TYPE("Content-Type", rtsp_header_content_type),
	HEAD_TYPE("CSeq", rtsp_header_cseq),
	HEAD_TYPE("Location", rtsp_header_location),
	HEAD_TYPE("Range", rtsp_header_range),
	HEAD_TYPE("Retry-After", rtsp_header_retry_after),
	HEAD_TYPE("RTP-client", rtsp_header_rtp),
	HEAD_TYPE("Session", rtsp_header_session),
	HEAD_TYPE("Set-Cookie", rtsp_header_cookie),
	HEAD_TYPE("Speed", rtsp_header_speed),
	HEAD_TYPE("Transport", rtsp_header_transport),
	HEAD_TYPE("WWW-Authenticate", rtsp_header_www),
	HEAD_TYPE("Accept", rtsp_header_accept),
	HEAD_TYPE("Accept-Encoding", rtsp_header_accept_enc),
	HEAD_TYPE("Accept-Language", rtsp_header_accept_lang),
	HEAD_TYPE("Authorization", rtsp_header_auth),
	HEAD_TYPE("Bandwidth", rtsp_header_bandwidth),
	HEAD_TYPE("Blocksize", rtsp_header_blocksize),
	HEAD_TYPE("Cache-Control", rtsp_header_cache_control),
	HEAD_TYPE("Content-Encoding", rtsp_header_content_enc),
	HEAD_TYPE("Content-Language", rtsp_header_content_lang),
	HEAD_TYPE("Date", rtsp_header_date),
	HEAD_TYPE("Expires", rtsp_header_expires),
	HEAD_TYPE("From", rtsp_header_from),
	HEAD_TYPE("If-Modified-Since", rtsp_header_ifmod),
	HEAD_TYPE("Last-Modified", rtsp_header_lastmod),
	HEAD_TYPE("Proxy-Authenticate", rtsp_header_proxyauth),
	HEAD_TYPE("Proxy-Require", rtsp_header_proxyreq),
	HEAD_TYPE("Referer", rtsp_header_referer),
	HEAD_TYPE("Scale", rtsp_header_scale),
	HEAD_TYPE("Server", rtsp_header_server),
	HEAD_TYPE("Unsupported", rtsp_header_unsup),
	HEAD_TYPE("User-Agent", rtsp_header_uagent),
	HEAD_TYPE("Via", rtsp_header_via),
	{ 0, 0, 0 },
};

const char* GetNextLine(const char *pParseBuf, int& nParseOffset);
void DecodeHeader(const char *line, void *response);


bool ParseResponse(const std::string& recvbuffer, RTSP_RESPONSE& lpResponse)
{
	char* pResponse = (char*)recvbuffer.c_str();
	uint32_t nBufLen = recvbuffer.length();

	int nParseLen = 0;
	const char* p = NULL;
	RTSP_RESPONSE *pTemp = &lpResponse;

	//Get return code first
	const char* pGetLine = GetNextLine(pResponse, nParseLen);
	if (NULL != pGetLine)
	{
		if(strncmp(pGetLine, RTSPHEADERFLAG, strlen(RTSPHEADERFLAG)) != 0)
		{
			char str[20];

			p = pGetLine;
			while ( *p != ' ')
			{
				p++;
			}
			memcpy(str, pGetLine, p - pGetLine);
			str[p-pGetLine] = '\0';
			pTemp->caption = _strdup(str);
		}
		else
		{
			p = pGetLine + strlen(RTSPHEADERFLAG);
			SKIP_SPACE(p);
			memcpy(pTemp->retcode, p, 3);
			pTemp->retcode[3] = '\0';
			p += 3;
			SKIP_SPACE(p);
			pTemp->retresp = _strdup(p);
			pTemp->caption = _strdup(RTSPHEADERFLAG);
		}

		do 
		{
			pGetLine = GetNextLine(pResponse, nParseLen);
			DecodeHeader(pGetLine, pTemp);

		} while ('\0' != *pGetLine);
	}

	if (pTemp->content_length > 0)
	{
		pTemp->body = (char*)malloc(pTemp->content_length + 1);
		pTemp->body[pTemp->content_length] = '\0';

		if ((int)nBufLen - nParseLen < pTemp->content_length)
		{
			//[TODO] data has not recv over!
			//memcpy(pTemp->body, pResponse + nParseLen, len);
		}
		else
		{
			memcpy(pTemp->body, pResponse + nParseLen, pTemp->content_length);
			nParseLen += pTemp->content_length;
		}
	}
	//check session string
	if (NULL != pTemp->session)
	{
		char* pdest = strstr(pTemp->session,";");
		if (NULL != pdest)
		{
			int nStrLen = strlen(pdest);
			memset(pdest, 0, nStrLen);
		}
	}

	return true;
}

const char* GetNextLine(const char *pParseBuf, int& nParseOffset)
{
	char *pOffset = (char*)strstr(pParseBuf + nParseOffset, SEPERATOR);
	if (NULL != pOffset)
	{
		const char *retval = pParseBuf + nParseOffset;
		nParseOffset = pOffset - pParseBuf;
		nParseOffset += strlen(SEPERATOR);
		*pOffset = '\0';
		//ignore parsed buffer
		pParseBuf += nParseOffset;
		return retval;
	}
	return NULL;
}

void DecodeHeader(const char *pLine, void *pResponse)
{
	int i;
	const char *after;

	i = 0;
	while (header_types[i].val != 0)
	{
		if(strncasecmp(pLine, header_types[i].val, header_types[i].val_length) == 0)
		{
			after = pLine + header_types[i].val_length;
			SKIP_SPACE(after);

			if (*after == ':')
			{
				after++;
				SKIP_SPACE(after);
				(header_types[i].parse_routine)(after, (RTSP_RESPONSE*)pResponse);
				return;
			}
		}
		i++;
	}
}




//int GetSession(char*)

RTSP_RESPONSE::RTSP_RESPONSE()
{
	memset(this, 0, sizeof(RTSP_RESPONSE));
}

RTSP_RESPONSE::~RTSP_RESPONSE()
{
	CHECK_AND_FREE(caption);
	CHECK_AND_FREE(retresp);
	CHECK_AND_FREE(body);
	CHECK_AND_FREE(accept);
	CHECK_AND_FREE(accept_encoding);
	CHECK_AND_FREE(accept_language);
	CHECK_AND_FREE(allow_public);
	CHECK_AND_FREE(authorization);
	CHECK_AND_FREE(bandwidth);
	CHECK_AND_FREE(blocksize);
	CHECK_AND_FREE(cache_control);
	CHECK_AND_FREE(content_base);
	CHECK_AND_FREE(content_encoding);
	CHECK_AND_FREE(content_language);
	CHECK_AND_FREE(content_location);
	CHECK_AND_FREE(content_type);
	CHECK_AND_FREE(date);
	CHECK_AND_FREE(cookie);
	CHECK_AND_FREE(expires);
	CHECK_AND_FREE(from);
	CHECK_AND_FREE(if_modified_since);
	CHECK_AND_FREE(last_modified);
	CHECK_AND_FREE(location);
	CHECK_AND_FREE(proxy_authenticate);
	CHECK_AND_FREE(proxy_require);
	CHECK_AND_FREE(range);
	CHECK_AND_FREE(referer);
	CHECK_AND_FREE(require);
	CHECK_AND_FREE(retry_after);
	CHECK_AND_FREE(rtp_info);
	CHECK_AND_FREE(scale);
	CHECK_AND_FREE(server);
	CHECK_AND_FREE(session);
	CHECK_AND_FREE(speed);
	CHECK_AND_FREE(transport);
	CHECK_AND_FREE(unsupported);
	CHECK_AND_FREE(user_agent);
	CHECK_AND_FREE(via);
	CHECK_AND_FREE(www_authenticate);
	content_length = 0;
	cseq = 0;
	close_connection = 0;
}
