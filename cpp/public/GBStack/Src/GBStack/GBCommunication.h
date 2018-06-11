#ifndef __GBCOMMUNICATION_H__
#define __GBCOMMUNICATION_H__

class GBCommunication
{
public:
	GBCommunication();
	~GBCommunication();
	
	EVENTId_Error udpListen(GBCommuID& commuId,int myPort,std::string myIp = "");
	EVENTId_Error tcpListen(GBCommuID& commuId,int myPort,std::string myIp = "");
	EVENTId_Error tcpConnect(GBCommuID& commuId,const std::string& destIp,int destPort,std::string myIp = "");
	
	EVENTId_Error getCommunicationInfos(GBStack::CommunicationInfo&info,GBCommuID commuId);
	EVENTId_Error getCommunicationSock(int &sock,GBCommuID commuId);
	
	EVENTId_Error deleteCommunication(GBCommuID commuId);
	
	
	static GBCommunication* instance();
private:
	
};

#endif //__GBCOMMUNICATION_H__
