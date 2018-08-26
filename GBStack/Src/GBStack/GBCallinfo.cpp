#include "GBCallinfo.h"
#include "Network/Network.h"
#include "GBRegister.h"
#include <sstream>
#include "GBEventPool.h"
namespace Public{
namespace GB28181{

enum GBCallStatus
{
	GBCallStatus_INIT,
	GBCallStatus_Req,
	GBCallStatus_Resp,
	GBCallStatus_Ack,
};

struct GBCallinfo::GBCallinfoInternal
{
	StreamSessionSDP	mysdp;
	int 				myEventId;
	SipDialog			callDialog;
	GBCallStatus		status;
	bool				isCallSender;
	uint64_t			prevUpdateTime;
	uint64_t			prevSendUpdateTime;

	weak_ptr<GBRegister> regist;
};

GBCallinfo::GBCallinfo(const weak_ptr<GBRegister>& reg,const shared_ptr<SIPSession>& session,const StreamSessionSDP& sdpinfo)
{
	internal = new GBCallinfoInternal;

	internal->regist = reg;
	internal->mysdp = sdpinfo;
	internal->isCallSender = session == NULL;
	internal->prevUpdateTime = internal->prevSendUpdateTime = Time::getCurrentMilliSecond();

	if(session == NULL)
	{
		SIPSession session("INVITE",internal->regist.lock()->getMyUrl(),
			/*internal->regist.lock()->getToUrl(),*/
			internal->regist.lock()->getToUrl(internal->mysdp.deviceId),
			internal->regist.lock()->getTransaction().get());
		std::string xml = SIPSession::buildSDPXml(internal->mysdp.deviceId,/*internal->regist.lock()->getMyUrl().getUserName(),*/internal->mysdp);

		std::string subject = SIPSession::buildSDPSubject(internal->regist.lock()->getMyUrl().getUserName(),internal->mysdp);

		session.sendRequest(xml,XMLINVITETYPE, subject);

		/*std::string subject;
		char xuliehao[32];
		for (int i=1; i<31; i++)
		xuliehao[i] = (char)((rand())%9+48);
		xuliehao[0] =  (internal->mysdp.type==StreamSessionType_Realplay)?'0':'1';
		xuliehao[20] = '\0';
		xuliehao[31] = '\0';
		std::ostringstream oss;
		oss << internal->mysdp.deviceId << ':' << xuliehao << ',' << internal->regist.lock()->getMyUrl().getUserName() << ':' << (xuliehao+21);

		session.sendRequest(xml,XMLINVITETYPE, oss.str());*/

		internal->callDialog = session.getDialog();
	}
	else
	{
		internal->callDialog = session->getDialog();
	}
	internal->status = GBCallStatus_Req;
}
GBCallinfo::~GBCallinfo()
{
	if(internal->status != GBCallStatus_INIT)
	{
		SIPSession session("BYE",*internal->callDialog.getSession().get());
		session.sendRequest();
	}
	SAFE_DELETE(internal);
}
void GBCallinfo::recvInviteResp(shared_ptr<SIPSession>& session)
{
	internal->status = GBCallStatus_Resp;

	if(session->getError().getErrorCode() == 200)
	{
		internal->callDialog = session->getDialog();
	}
}
void GBCallinfo::recvAck(shared_ptr<SIPSession>& session)
{
	internal->status = GBCallStatus_Ack;
}

void GBCallinfo::recvBye()
{
	internal->status = GBCallStatus_INIT;
}
void GBCallinfo::sendResp(GBEventID id, const SIPStackError& error,const StreamSDPInfo& sdp)
{
	StreamSessionSDP streamSdp = internal->mysdp;
	streamSdp.ipAddr = sdp.IpAddr == "" ? internal->regist.lock()->getMyUrl().getHost() : sdp.IpAddr;
	streamSdp.port = sdp.Port;
	streamSdp.MediaList = sdp.sdpList;

	std::string xml = SIPSession::buildSDPXml(internal->regist.lock()->getMyUrl().getUserName(),streamSdp);

	internal->callDialog.getSession()->sendFailure(error,error.getErrorCode() == SIPStackError_OK ? xml : "",XMLINVITETYPE);

	internal->status = GBCallStatus_Resp;
}

void GBCallinfo::sendACK(GBEventID id,const std::string sendip,int sendport)
{
	StreamSessionSDP sdp = internal->mysdp;
	sdp.ipAddr = sendip == "" ? internal->regist.lock()->getMyUrl().getHost() : sendip;
	sdp.port = sendport;

	SIPSession session("ACK",*internal->callDialog.getSession().get());
	std::string xml = SIPSession::buildSDPXml(internal->regist.lock()->getMyUrl().getUserName(),sdp);

	session.sendRequest(sendport == 0 ? "" : xml,XMLINVITETYPE);
	internal->status = GBCallStatus_Ack;
}
SipDialog GBCallinfo::getDialog()
{
	return internal->callDialog;
}
std::string GBCallinfo::getDeviceUrl()
{
	return internal->mysdp.deviceId;
}
void GBCallinfo::recvUpdate()
{
	internal->prevUpdateTime = Time::getCurrentMilliSecond();
}
void GBCallinfo::recvUpdateResp()
{
	internal->prevUpdateTime = Time::getCurrentMilliSecond();
}
bool GBCallinfo::poolCallAndCheckIsTimeout()
{
	return false;

#define CALLINFOTIMEOUT		60*1000
	if(internal->isCallSender && internal->status == GBCallStatus_Ack && Time::getCurrentMilliSecond() - internal->prevSendUpdateTime >= CALLINFOTIMEOUT/3)
	{
		SIPSession session("Update",*internal->callDialog.getSession().get());
		session.sendRequest();

		internal->prevSendUpdateTime = Time::getCurrentMilliSecond();
	}

	if(Time::getCurrentMilliSecond() - internal->prevUpdateTime >= CALLINFOTIMEOUT)
	{
		return true;
	}

	return false;
}
StreamSessionSDP GBCallinfo::buildSessionSDP(const std::string& devid,long long startTime,long long endTime,int type,int downSpeed,int streamType,const StreamSDPInfo& sdp)
{
	StreamSessionSDP ssdp;
	ssdp.deviceId = devid;
	ssdp.vodStartTime = (long)startTime;
	ssdp.VodEndTime = (long)endTime;
	ssdp.ipAddr = sdp.IpAddr;
	ssdp.port = sdp.Port;
	ssdp.streamType = streamType;
	ssdp.MediaList = sdp.sdpList;
	ssdp.SocketType = sdp.SocketType;
	ssdp.type = (StreamSessionType)type;
	ssdp.downSpeed = downSpeed;
	ssdp.ssrc = sdp.Ssrc;

	return ssdp;
}

StreamSDPInfo GBCallinfo::buildSDP(const StreamSessionSDP& ssdp)
{
	StreamSDPInfo sdp;
	sdp.IpAddr = ssdp.ipAddr;
	sdp.Port = ssdp.port;
	sdp.Ssrc = ssdp.ssrc;
	sdp.sdpList = ssdp.MediaList;
	sdp.SocketType = ssdp.SocketType;

	return sdp;
}

};
};

