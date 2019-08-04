
#include "HTTPCommunication.h"
#include "HTTP/HTTPPublic.h"
#include "HTTP/HTTPServer.h"

namespace Public {
namespace HTTP {

HTTPServerSession::HTTPServerSession(const shared_ptr<HTTPCommunication>& commuSession,  HTTPCacheType type)
{
	shared_ptr<ReadContent> content = make_shared<ReadContent>(type);

	commuSession->recvContent = content;

	//HTTPServerRequest(const shared_ptr<HTTPHeader>& header,const shared_ptr<ReadContent>& content,const shared_ptr<Socket>& sock);
	request = make_shared<HTTPServerRequest>(commuSession->recvHeader, content, commuSession->socket);

	response = make_shared<HTTPServerResponse>(commuSession,type);
}
HTTPServerSession::~HTTPServerSession() {}

void HTTPServerSession::disconnected()
{
	request->discallback()(request, "socket disconnect");
}
	
}
}

