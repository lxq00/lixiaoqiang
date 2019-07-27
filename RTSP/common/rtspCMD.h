#ifndef __RTSPCMD_H__
#define __RTSPCMD_H__
#include "RTSPClient/RTSPClient.h"
using namespace Public::RTSPClient;

struct SDPInfo
{
	int		channel;
	int		ctrolChannel;

	int		clientPort;
	int		clientCtrolPort;

	int		serverPort;
	int		serverCtrolPort;

	SDPInfo(int startChannel);
	void parse(const char* sdkinfo);
	std::string build(RTP_RECV_Type type);
};

struct RTSPUrlInfo
{
public:
	std::string m_szUserName;
	std::string m_szPassWord;
	std::string m_szRtspUrl;
	std::string m_szServerIp;
	int			m_nServerPort;


	std::string					m_szSession;//Á÷µÄsession
	int							m_nCSeq;//ÃüÁîÐòÁÐºÅ
	std::string					m_userAgent;

private:
	bool						needAuth;
	bool						needBasic;

	std::string					m_szRealm;
	std::string					m_szNonce;
public:
	std::string buildAuthenString(const std::string& cmd);
	static bool checkAuthen(const std::string& cmd, const std::string& username, const std::string& password, const std::string& authenstr);
	static std::string buildWWWAuthen(const std::string& cmd, const std::string& username, const std::string& password);
public:

	RTSPUrlInfo();

	bool parse(const std::string& url);

	bool parseAuthenString(const std::string& pAuthorization);

	static std::string getAuthenAttr(const std::string& pAuthorization, const std::string& key);
};


typedef enum {
	RTSPCmd_OPTIONS,
	RTSPCmd_DESCRIBE,
	RTSPCmd_SETUP,
	RTSPCmd_PLAY,
	RTSPCmd_GET_PARAMETER,
	RTSPCmd_TEARDOWN,
}RTSPCmd;

class CMDBuilder
{
public:
	CMDBuilder(const std::string& _cmd, RTSPCmd _cmdtype):cmd(_cmd),cmdType(_cmdtype) {}
	virtual ~CMDBuilder() {}

	virtual std::string build(shared_ptr<RTSPUrlInfo>& url) = 0;
public:
	std::string cmd;
	uint32_t	m_nCSeq;
	RTSPCmd		cmdType;
};

class CMDBuilder_OPTIONS :public CMDBuilder
{
public:
	CMDBuilder_OPTIONS();
	~CMDBuilder_OPTIONS();

	std::string build(shared_ptr<RTSPUrlInfo>& url);
};
class CMDBuilder_DESCRIBE :public CMDBuilder
{
public:
	CMDBuilder_DESCRIBE();
	~CMDBuilder_DESCRIBE();

	std::string build(shared_ptr<RTSPUrlInfo>& url);
};

class CMDBuilder_SETUP :public CMDBuilder
{
public:
	CMDBuilder_SETUP(const std::string& trackurl, const std::string& port,bool video);
	~CMDBuilder_SETUP();

	std::string build(shared_ptr<RTSPUrlInfo>& url);
private:
	std::string			trackurl;
	std::string			transport;
public:
	bool				isVideo;
};

class CMDBuilder_PLAY :public CMDBuilder
{
public:
	CMDBuilder_PLAY(const std::string& type,const std::string& starttime,const std::string& stoptime);
	virtual~CMDBuilder_PLAY();

	std::string build(shared_ptr<RTSPUrlInfo>& url);

	std::string	 type;
	std::string	 startTime;
	std::string	 stopTime;
};

class CMDBuilder_GET_PARAMETER :public CMDBuilder
{
public:
	CMDBuilder_GET_PARAMETER();
	virtual ~CMDBuilder_GET_PARAMETER();

	std::string build(shared_ptr<RTSPUrlInfo>& url);
};

class CMDBuilder_TEARDOWN :public CMDBuilder
{
public:
	CMDBuilder_TEARDOWN();
	virtual ~CMDBuilder_TEARDOWN();

	std::string build(shared_ptr<RTSPUrlInfo>& url);
};



#endif //__RTSPCMD_H__
