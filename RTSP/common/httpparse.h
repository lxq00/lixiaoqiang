#include "Base/Base.h"
using namespace Public::Base;

class HttpParse
{   
public:
    HttpParse(bool _isRequest)
    :isRequest(isRequest),statuscode(0),headerisfinish(false),content_len(0)
    {

    }
    ~HttpParse()
    {

    }

private:
    bool isRequest;

    std::string method;
    std::string url;
    std::string protocol;
    std::string version;

    int         statuscode;
    std::string statuserrmsg;

    std::map<std::string,std::string> header;
    bool   headerisfinish;
    int    content_len;
    std::string  body;
};