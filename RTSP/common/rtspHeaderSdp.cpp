#include "rtspHeaderSdp.h"
#include "strDup.h"

bool parseSDPLine(char const* input, char const*& nextLine);
bool parseSDPLine_s(char const* sdpLine);
bool parseSDPLine_i(char const* sdpLine);
bool parseSDPLine_b(char const* sdpLine);
bool parseSDPLine_c(char const* sdpLine);
bool parseSDPAttribute_type(char const* sdpLine);
bool parseSDPAttribute_control(char const* sdpLine, std::string& pcontrol);
bool parseSDPAttribute_range(char const* sdpLine, std::string& pRange);
bool parseSDPAttribute_source_filter(char const* sdpLine);
bool parseSDPAttribute_rtpmap(char const* sdpLine, std::string& pCodecName, int &nPayLoad, int &nTimestampFrequency/*, int &nNumChannels*/);
bool parseSDPAttribute_framerate(char const* sdpLine, double &fVideoFPS);
bool parseSDPAttribute_fmtp(char const* sdpLine, std::string& pSpsBufer);
bool parseSDPAttribute_x_dimensions(char const* sdpLine, int& nWidth, int& nHeight);
bool parseSDPAttribute_rangetime(char const* sdpLine, uint32_t& starttime, uint32_t& stoptime);

//...........................................static.............................................

static char* lookupPayloadFormat(unsigned char rtpPayloadType, unsigned& rtpTimestampFrequency, unsigned& numChannels);
static unsigned guessRTPTimestampFrequency(char const* mediumName, char const* codecName);
static char* parseCLine(char const* sdpLine);
static bool parseRangeAttribute(char const* sdpLine, std::string& pRange);

bool ParseTrackID(char *sdp, MEDIA_INFO *sMediaInfo);


std::string rtsp_header_build_sdp(const MEDIA_INFO& info)
{
	std::string sdpstr;

	sdpstr += "v=0\r\n";
	sdpstr += "o=- " + Value(info.ssrc).readString() + " " + Value(info.ssrc).readString() + " IN IP4 0.0.0.0\r\n";
	sdpstr += "c=IN IP4 0.0.0.0\r\n";
	sdpstr += "t=" + Value(info.startRange).readString() + "-" + Value(info.stopRange).readString() + "\r\n";
	sdpstr += "a=control:*";

	if (info.bHasVideo)
	{
		sdpstr += "m=video 0 RTP/AVP "+Value(info.stStreamVideo.nPayLoad).readString()+ "\r\n";
		sdpstr += "a=control:"+info.stStreamVideo.szTrackID+"\r\n";
		sdpstr += "a=rtpmap:" + Value(info.stStreamVideo.nPayLoad).readString() + " " + info.stStreamVideo.szCodec + "/" + Value(info.stStreamVideo.nSampRate).readString() + "\r\n";
	}
	if (info.bHasAudio)
	{
		sdpstr += "m=video 0 RTP/AVP " + Value(info.stStreamAudio.nPayLoad).readString() + "\r\n";
		sdpstr += "a=control:" + info.stStreamAudio.szTrackID + "\r\n";
		sdpstr += "a=rtpmap:" + Value(info.stStreamAudio.nPayLoad).readString() + " " + info.stStreamAudio.szCodec + "/" + Value(info.stStreamAudio.nSampRate).readString() + "\r\n";
	}

	return sdpstr;
}

bool rtsp_header_parse_sdp(char const* sdpDescription, MEDIA_INFO* pMediaInfo)
{
	if (sdpDescription == NULL)
	{
		return false;
	}

	// Begin by processing all SDP lines until we see the first "m="
	char const* sdpLine = sdpDescription;
	char const* nextSDPLine;
	while (1)
	{
		if (!parseSDPLine(sdpLine, nextSDPLine))
		{
			return false;
		}
		// We should really check for:
		// "a=control:" attributes (to set the URL for aggregate control)
		// the correct SDP version (v=0)
		if (sdpLine[0] == 'm')
			break;

		sdpLine = nextSDPLine;

		if (sdpLine == NULL)
			break; // there are no m= lines at all

		// Check for various special SDP lines that we understand:
		if (parseSDPLine_s(sdpLine))
			continue;
		if (parseSDPLine_i(sdpLine))
			continue;
		if (parseSDPLine_c(sdpLine))
			continue;
		//if (parseSDPAttribute_control(sdpLine, pMediaInfo->szControl))
		//	continue;
		//if (parseSDPAttribute_range(sdpLine, pMediaInfo->szRange))
		//	continue;
		if(parseSDPAttribute_rangetime(sdpLine,pMediaInfo->startRange,pMediaInfo->stopRange))
			continue;
		if (parseSDPAttribute_type(sdpLine))
			continue;
		if (parseSDPAttribute_source_filter(sdpLine))
			continue;

	}

	//parse the follow info
	while (sdpLine != NULL) 
	{
		STREAM_INFO *pStreanInfo = NULL;
		//chaeck stream info
		if (0 == strncasecmp(sdpLine, "m=video", 7))
		{
			//video stream
			pStreanInfo = &(pMediaInfo->stStreamVideo);
			pMediaInfo->bHasVideo = true;
		}
		else if (0 == strncasecmp(sdpLine, "m=audio", 7))
		{
			//audio stream
			pStreanInfo = &(pMediaInfo->stStreamAudio);
			pMediaInfo->bHasAudio = true;
		}
		else if (0 == strncasecmp(sdpLine, "m=application", 13))
		{
			/*pStreanInfo = &(pMediaInfo->stStreamAudio);
			pMediaInfo->bHasAudio = true;*/
			//break;
		}

		unsigned short fClientPortNum;
		char* mediumName = strDupSize(sdpLine); // ensures we have enough space
		char const* protocolName = NULL;
		unsigned payloadFormat;

		if ((sscanf(sdpLine, "m=%s %hu RTP/AVP %u", mediumName, &fClientPortNum, &payloadFormat) == 3 ||
			sscanf(sdpLine, "m=%s %hu/%*u RTP/AVP %u",mediumName, &fClientPortNum, &payloadFormat) == 3)
			&& payloadFormat <= 127)
		{
			protocolName = "RTP";
		}
		else if ((sscanf(sdpLine, "m=%s %hu UDP %u", mediumName, &fClientPortNum, &payloadFormat) == 3 ||
			sscanf(sdpLine, "m=%s %hu udp %u", mediumName, &fClientPortNum, &payloadFormat) == 3 ||
			sscanf(sdpLine, "m=%s %hu RAW/RAW/UDP %u", mediumName, &fClientPortNum, &payloadFormat) == 3)
			&& payloadFormat <= 127)
		{
			// This is a RAW UDP source
			protocolName = "UDP";
		} 
		else
		{
			//error media info
			return false;
		}
		//»ñÈ¡PayLoad
		if(pStreanInfo)
			pStreanInfo->nPayLoad = payloadFormat;
		//copy info
		//pStreanInfo->szProtocol = protocolName;
		//pStreanInfo->szMediaName = mediumName;

		while (1)
		{
			sdpLine = nextSDPLine;
			if (sdpLine == NULL)
				break; // we've reached the end
			if (!parseSDPLine(sdpLine, nextSDPLine))
				return false;

			if (sdpLine[0] == 'm')
				break; // we've reached the next stream

			if (!pStreanInfo)
				continue;
			// Check for various special SDP lines that we understand:
			if (parseSDPLine_c(sdpLine))
				continue;
			if (parseSDPLine_b(sdpLine))
				continue;
			if (parseSDPAttribute_rtpmap(sdpLine, pStreanInfo->szCodec, pStreanInfo->nPayLoad, pStreanInfo->nSampRate/*, pStreanInfo->nChannles*/))
				continue;
			/*if (parseSDPAttribute_framerate(sdpLine, pStreanInfo->fFramRate))
				continue;
			if (parseSDPAttribute_x_dimensions(sdpLine, pMediaInfo->stStreamVideo.nWidth, pMediaInfo->stStreamVideo.nHight))
				continue;
			if (parseSDPAttribute_fmtp(sdpLine, pStreanInfo->szSpsPps))
				continue;*/
			if (parseSDPAttribute_control(sdpLine, pStreanInfo->szTrackID))
				continue;

			// 			if (parseSDPAttribute_source_filter(sdpLine))
			// 				continue;
			//if (parseSDPAttribute_x_dimensions(sdpLine))
			//	continue;

		}
	}

	return true;
}

bool parseSDPLine(char const* inputLine, char const*& nextLine)
{
	// Begin by finding the start of the next line (if any):
	nextLine = NULL;
	for (char const* ptr = inputLine; *ptr != '\0'; ++ptr) {
		if (*ptr == '\r' || *ptr == '\n') {
			// We found the end of the line
			++ptr;
			while (*ptr == '\r' || *ptr == '\n') ++ptr;
			nextLine = ptr;
			if (nextLine[0] == '\0') nextLine = NULL; // special case for end
			break;
		}
	}

	// Then, check that this line is a SDP line of the form <char>=<etc>
	// (However, we also accept blank lines in the input.)
	if (inputLine[0] == '\r' || inputLine[0] == '\n') return true;
	if (strlen(inputLine) < 2 || inputLine[1] != '='
		|| inputLine[0] < 'a' || inputLine[0] > 'z') {
			//envir().setResultMsg("Invalid SDP line: ", inputLine);
			return false;
	}

	return true;
}

bool parseSDPLine_s(char const* sdpLine)
{
	// Check for "s=<session name>" line
	char* buffer = strDupSize(sdpLine);
	bool parseSuccess = false;

	if (sscanf(sdpLine, "s=%[^\r\n]", buffer) == 1) {
		//strcpy (m_sMediaInfo.sStreamAudio.szTrackID, buffer);
		parseSuccess = true;
	}
	delete[] buffer;

	return parseSuccess;
}

bool parseSDPLine_i(char const* sdpLine)
{ // Check for "i=<session description>" line
	char* buffer = strDupSize(sdpLine);
	bool parseSuccess = false;

	if (sscanf(sdpLine, "i=%[^\r\n]", buffer) == 1) {

		parseSuccess = true;
	}
	delete[] buffer;

	return parseSuccess;
}

unsigned fBandwidth;
bool parseSDPLine_b(char const* sdpLine)
{
	return sscanf(sdpLine, "b=AS:%u", &fBandwidth) == 1;
}

bool parseSDPLine_c(char const* sdpLine)
{
	char* connectionEndpointName = parseCLine(sdpLine);
	if (connectionEndpointName != NULL)
	{
		return true;
	}
	return false;
}

bool parseSDPAttribute_type(char const* sdpLine)
{
	bool parseSuccess = false;

	char* buffer = strDupSize(sdpLine);
	if (sscanf(sdpLine, "a=type: %[^ ]", buffer) == 1)
	{
		// 		delete[] fMediaSessionType;
		// 		fMediaSessionType = strDup(buffer);
		parseSuccess = true;
	}
	delete[] buffer;

	return parseSuccess;
}

bool parseSDPAttribute_framerate(char const* sdpLine, double &fVideoFPS)
{
	// Check for a "a=framerate: <fps>" or "a=x-framerate: <fps>" line:
	bool parseSuccess = false;

	float frate;
	int rate;
	if (sscanf(sdpLine, "a=framerate: %f", &frate) == 1 || sscanf(sdpLine, "a=framerate:%f", &frate) == 1)
	{
		parseSuccess = true;
		fVideoFPS = frate;
	}
	else if (sscanf(sdpLine, "a=x-framerate: %d", &rate) == 1)
	{
		parseSuccess = true;
		fVideoFPS = rate;
	}

	return parseSuccess;
}

bool parseSDPAttribute_control(char const* sdpLine, std::string& pcontrol)
{
	bool parseSuccess = false;
	char* controlPath = strDupSize(sdpLine); // ensures we have enough space
	if (sscanf(sdpLine, "a=control: %s", controlPath) == 1)
	{
		pcontrol = controlPath;

		parseSuccess = true;
	}
	delete[] controlPath;

	return parseSuccess;
}

bool parseSDPAttribute_range(char const* sdpLine, std::string& pRange)
{
	bool parseSuccess = false;

	if (parseRangeAttribute(sdpLine, pRange))
	{
		parseSuccess = true;
	}

	return parseSuccess;
}

bool parseSDPAttribute_rangetime(char const* sdpLine, uint32_t& starttime, uint32_t& stoptime)
{
	uint32_t start = 0, stop = 0;

	if (sscanf(sdpLine, "t=%d-", &start) == 1 || sscanf(sdpLine, "t=%d-%d", &start,&stop) == 2)
	{
		starttime = start;
		stoptime = stop;

		return true;
	}

	return false;
}

bool parseSDPAttribute_source_filter(char const* sdpLine)
{

	return true;
}

bool parseSDPAttribute_rtpmap(char const* sdpLine, std::string& pCodecName, int &nPayLoad, int &nTimestampFrequency/*, int &nNumChannels*/)
{
	// Check for a "a=rtpmap:<fmt> <codec>/<freq>" line:
	// (Also check without the "/<freq>"; RealNetworks omits this)
	// Also check for a trailing "/<numChannels>".
	bool parseSuccess = false;

	unsigned rtpmapPayloadFormat;
	char* codecName = strDupSize(sdpLine); // ensures we have enough space
	unsigned rtpTimestampFrequency = 0;
	unsigned numChannels = 1;
	if (sscanf(sdpLine, "a=rtpmap: %u %[^/]/%u/%u", &rtpmapPayloadFormat, codecName, &rtpTimestampFrequency, &numChannels) == 4 ||
		sscanf(sdpLine, "a=rtpmap: %u %[^/]/%u", &rtpmapPayloadFormat, codecName, &rtpTimestampFrequency) == 3 ||
		sscanf(sdpLine, "a=rtpmap: %u %s", &rtpmapPayloadFormat, codecName) == 2)
	{
		pCodecName = codecName;
		nTimestampFrequency = rtpTimestampFrequency;
		nPayLoad = rtpmapPayloadFormat;
	//	nNumChannels = numChannels;

		parseSuccess = true;
		if (rtpmapPayloadFormat == 96)
		{
			{
				for (char* p = codecName; *p != '\0'; ++p)
					*p = toupper(*p);
			}
		}
	}
	delete[] codecName;

	return parseSuccess;
}

//.................................................. static ................................................

static char* lookupPayloadFormat(unsigned char rtpPayloadType, unsigned& rtpTimestampFrequency, unsigned& numChannels)
{
	return NULL;
}
static unsigned guessRTPTimestampFrequency(char const* mediumName, char const* codecName)
{
	return 0;
}



static char* parseCLine(char const* sdpLine)
{
	char* resultStr = NULL;
	char* buffer = strDupSize(sdpLine); // ensures we have enough space
	if (sscanf(sdpLine, "c=IN IP4 %[^/\r\n]", buffer) == 1)
	{
		// Later, handle the optional /<ttl> and /<numAddresses> #####
		resultStr = strDup(buffer);
	}
	delete[] buffer;

	return resultStr;
}

static bool parseRangeAttribute(char const* sdpLine, std::string& strRange)
{
	char rangebuffer[1024] = { 0 };
	if (sscanf(sdpLine, "a=range:npt=%[^/\r\n]", rangebuffer) != 1) return false;

	strRange = rangebuffer;

	return true;
}

//.........................................................................................................

bool parseSDPAttribute_x_dimensions(char const* sdpLine, int& nWidth, int& nHeight)
{
	// Check for a "a=x-dimensions:<width>,<height>" line:
	bool parseSuccess = false;

	int width, height;
	if (sscanf(sdpLine, "a=x-dimensions:%d,%d", &width, &height) == 2)
	{
		parseSuccess = true;
		nWidth = (unsigned short)width;
		nHeight = (unsigned short)height;
	}

	return parseSuccess;
}

bool parseSDPAttribute_fmtp(char const* sdpLine, std::string& pSpsBufer)
{
	// Check for a "a=fmtp:" line:
	// TEMP: We check only for a handful of expected parameter names #####
	// Later: (i) check that payload format number matches; #####
	//        (ii) look for other parameters also (generalize?) #####
	do {
		if (strncmp(sdpLine, "a=fmtp:", 7) != 0) break; sdpLine += 7;
		while (isdigit(*sdpLine)) ++sdpLine;

		// The remaining "sdpLine" should be a sequence of
		//     <name>=<value>;
		// parameter assignments.  Look at each of these.
		// First, convert the line to lower-case, to ease comparison:
		char* const lineCopy = strDup(sdpLine); char* line = lineCopy;
		{
			for (char* c = line; *c != '\0'; ++c) *c = tolower(*c);
		}
		while (*line != '\0' && *line != '\r' && *line != '\n')
		{
			unsigned u;
			char* valueStr = strDupSize(line);
			if (sscanf(line, " auxiliarydatasizelength = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " constantduration = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " constantsize; = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " crc = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " ctsdeltalength = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " de-interleavebuffersize = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " dtsdeltalength = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " indexdeltalength = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " indexlength = %u", &u) == 1) 
			{
			}
			else if (sscanf(line, " interleaving = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " maxdisplacement = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " objecttype = %u", &u) == 1) 
			{
			}
			else if (sscanf(line, " octet-align = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " profile-level-id = %x", &u) == 1)
			{
				// Note that the "profile-level-id" parameter is assumed to be hexadecimal
			}
			else if (sscanf(line, " robust-sorting = %u", &u) == 1)
			{
			} else if (sscanf(line, " sizelength = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " streamstateindication = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " streamtype = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " cpresent = %u", &u) == 1)
			{
			}
			else if (sscanf(line, " randomaccessindication = %u", &u) == 1)
			{
			}
			else if (sscanf(sdpLine, " config = %[^; \t\r\n]", valueStr) == 1 ||
				sscanf(sdpLine, " configuration = %[^; \t\r\n]", valueStr) == 1)
			{
				// Note: We used "sdpLine" here, because the value may be case-sensitive (if it's Base-64).
			}
			else if (sscanf(line, " mode = %[^; \t\r\n]", valueStr) == 1)
			{
			}
			else if (sscanf(sdpLine, " sprop-parameter-sets = %[^; \t\r\n]", valueStr) == 1)
			{
				// Note: We used "sdpLine" here, because the value is case-sensitive.
				pSpsBufer = valueStr;
			}
			else if (sscanf(line, " emphasis = %[^; \t\r\n]", valueStr) == 1)
			{
			}
			else if (sscanf(sdpLine, " channel-order = %[^; \t\r\n]", valueStr) == 1)
			{
				// Note: We used "sdpLine" here, because the value is case-sensitive.
			}
			else
			{
				// Some of the above parameters are Boolean.  Check whether the parameter
				// names appear alone, without a "= 1" at the end:
				if (sscanf(line, " %[^; \t\r\n]", valueStr) == 1)
				{
					if (strcmp(valueStr, "octet-align") == 0)
					{
					} else if (strcmp(valueStr, "cpresent") == 0)
					{
					} else if (strcmp(valueStr, "crc") == 0)
					{
					} else if (strcmp(valueStr, "robust-sorting") == 0)
					{
					} else if (strcmp(valueStr, "randomaccessindication") == 0)
					{
					}
				}
			}
			delete[] valueStr;

			// Move to the next parameter assignment string:
			while (*line != '\0' && *line != '\r' && *line != '\n'
				&& *line != ';') ++line;
			while (*line == ';') ++line;

			// Do the same with sdpLine; needed for finding case sensitive values:
			while (*sdpLine != '\0' && *sdpLine != '\r' && *sdpLine != '\n'
				&& *sdpLine != ';') ++sdpLine;
			while (*sdpLine == ';') ++sdpLine;
		}
		delete[] lineCopy;
		return true;
	} while (0);

	return false;
}

bool ParseTrackID(char *sdp, MEDIA_INFO *sMediaInfo)
{
	string strInfo(sdp);
	int nVedioPose = strInfo.find("m=video");

	if ((int)std::string::npos == nVedioPose)
		return false;
	int beginPos_tarck_video = strInfo.find("a=control:", nVedioPose + 1);
	if ((int)std::string::npos == beginPos_tarck_video)
		return false;
	int endPos_tarck_video = strInfo.find('\r', beginPos_tarck_video + 1);
	if ((int)std::string::npos == endPos_tarck_video) 
		endPos_tarck_video = strInfo.find('\n', beginPos_tarck_video + 1);
	if ((int)std::string::npos == endPos_tarck_video) 
		return false;
	// /trackVedio
	sMediaInfo->stStreamVideo.szTrackID = std::string(sdp + beginPos_tarck_video + strlen("a=control:"), endPos_tarck_video - beginPos_tarck_video - strlen("a=control:"));

	int nAudioPose =  strInfo.find("m=audio");
	if ((int)std::string::npos == nAudioPose)
	{
		
	}
	else
	{

		int beginPos_tarck_audio = strInfo.find("a=control:", nAudioPose + 1);
		if ((int)std::string::npos == beginPos_tarck_audio)
			return false;
		int endPos_tarck_audio = strInfo.find('\r', beginPos_tarck_audio + 1);
		if ((int)std::string::npos == endPos_tarck_audio) 
			endPos_tarck_audio = strInfo.find('\n', beginPos_tarck_audio + 1);
		if ((int)std::string::npos == endPos_tarck_audio) 
			return false;
		sMediaInfo->stStreamAudio.szTrackID = std::string(sdp + beginPos_tarck_audio + strlen("a=control:"), endPos_tarck_audio - beginPos_tarck_audio - strlen("a=control:"));
	}

	return true;
}
