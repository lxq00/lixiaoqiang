
#include "HTTPCommunication.h"
#include "HTTP/HTTPPublic.h"
#include "HTTP/HTTPServer.h"

namespace Public {
namespace HTTP {

HTTPServerSession::HTTPServerSession(const shared_ptr<HTTPCommunication>& commuSession, HTTPCacheType type)
{
	commuSession->recvContent = make_shared<ReadContent>(type);

	request = make_shared<HTTPServerRequest>(commuSession->recvHeader, commuSession->recvContent, commuSession->socket);

	response = make_shared<HTTPServerResponse>(commuSession,type);
}
HTTPServerSession::~HTTPServerSession() {}

void HTTPServerSession::disconnected()
{
	request->discallback()(request, "socket disconnect");
}
	
}
}

