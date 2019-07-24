#include <stdio.h>
#include <iostream>
#include <thread>  
#include "RabbitMq/RabbitmqClient.h"
#include "amqp.h"
#include "amqp_framing.h"
#include "amqp_tcp_socket.h"
#include <windows.h>

using namespace Public::Base;
using namespace Public::Rabbitmq;

struct RabbitmqClient::RabbitmqClientInternal:public Thread
{
	Mutex						mutex;
	amqp_socket_t               *m_pSock;
	amqp_connection_state_t     m_pConn;

	std::map<void*, SubscribeCallback> m_subcrbielist;


	string                      m_strQueueName;
	string                      m_strExchange;
	string                      m_strRoutekey;
	int							m_iChannel;
	ConnectStatusCallback		connectCallback;
	Timer						timer;
	bool						lastStatus;
	bool						curStatus;

	string						m_strvhost;
	string						m_strHostname;
	int							m_iPort;
	string						m_strUser;
	string						m_strPasswd;
	uint32_t					m_timeout;

	RabbitmqClientInternal(const ConnectStatusCallback &status) :Thread("RabbitmqClientInternal"),m_pSock(NULL), m_pConn(NULL), m_iChannel(1)
		, connectCallback(status)
		, timer("reconnect timer")
		, lastStatus(false)
		, curStatus(true)
	{
	}

	~RabbitmqClientInternal()
	{
		destroyThread();
		timer.stopAndWait();
	}

	void startReconnect()
	{
		!timer.isStarted() && timer.start(Timer::Proc(&RabbitmqClientInternal::TimerProc, this), 0, 1000, NULL);
	}

	void TimerProc(unsigned long param)
	{
		if (!curStatus)
		{
			close();
			connect();
		}
		if (lastStatus == curStatus)
		{
			return;
		}
		connectCallback(curStatus);

		lastStatus = curStatus;
	}

	bool connect()
	{
		Guard locker(mutex);
		m_pConn = amqp_new_connection();
		if (NULL == m_pConn)
		{
			logerror("amqp new connection failed\n");
			return false;
		}

		m_pSock = amqp_tcp_socket_new(m_pConn);
		if (NULL == m_pSock)
		{
			logerror("amqp tcp new socket failed\n");
			return false;
		}

		timeval tv = { 0 };
		tv.tv_usec = m_timeout * 1000;
		int status = amqp_socket_open_noblock(m_pSock, m_strHostname.c_str(), m_iPort, &tv);
		if (status < 0)
		{
			logdebug("amqp socket open failed\n");
			return false;
		}

		//if (0 != internal->ErrorMsg(amqp_login(internal->m_pConn, "vhost_zvan", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, strUser.c_str(), strPasswd.c_str()), "Logging in"))
		if (0 != ErrorMsg(amqp_login(m_pConn, m_strvhost.c_str(), 0, 131072, AMQP_DEFAULT_HEARTBEAT, AMQP_SASL_METHOD_PLAIN, m_strUser.c_str(), m_strPasswd.c_str()), "Logging in"))
		{

			return false;
		}
		curStatus = true;

		return true;
	}

	bool connect(const string &strvhost, const string &strHostname, int iPort, const string &strUser, const string &strPasswd, uint32_t timeout)
	{
		m_strvhost = strvhost;
		m_strHostname = strHostname;
		m_iPort = iPort;
		m_strUser = strUser;
		m_strPasswd = strPasswd;
		m_timeout = timeout;
		if (!connect())
		{
			startReconnect();
			return false;
		}
		startReconnect();
		return true;
	}

	void close()
	{
		Guard locker(mutex);
		if (NULL != m_pConn)
		{
			ErrorMsg(amqp_connection_close(m_pConn, AMQP_REPLY_SUCCESS), "Closing connection");

			if (amqp_destroy_connection(m_pConn) < 0)
			{
				logerror("amqp_destroy_connection() fail!");
			}

			m_pConn = NULL;
		}
	}

	int ErrorMsg(amqp_rpc_reply_t x, char const *context)
	{
		switch (x.reply_type)
		{
		case AMQP_RESPONSE_NORMAL:
			return 0;

		case AMQP_RESPONSE_NONE:
			fprintf(stderr, "%s: missing RPC reply type!\n", context);
            logdebug("%s: missing RPC reply type!", context);
			break;

		case AMQP_RESPONSE_LIBRARY_EXCEPTION:
			fprintf(stderr, "%s: %s\n", context, amqp_error_string2(x.library_error));
            logdebug("%s: %s", context, amqp_error_string2(x.library_error));
			break;

		case AMQP_RESPONSE_SERVER_EXCEPTION:
			switch (x.reply.id)
			{
			case AMQP_CONNECTION_CLOSE_METHOD:
			{
				amqp_connection_close_t *m = (amqp_connection_close_t *)x.reply.decoded;
				fprintf(stderr, "%s: server connection error %uh, message: %.*s\n",
					context, m->reply_code, (int)m->reply_text.len, (char *)m->reply_text.bytes);
                logdebug("%s: server connection error %uh, message: %.*s",
                    context, m->reply_code, (int)m->reply_text.len, (char *)m->reply_text.bytes);

				break;
			}

			case AMQP_CHANNEL_CLOSE_METHOD:
			{
				amqp_channel_close_t *m = (amqp_channel_close_t *)x.reply.decoded;
				fprintf(stderr, "%s: server channel error %uh, message: %.*s\n",
					context, m->reply_code, (int)m->reply_text.len, (char *)m->reply_text.bytes);
                logdebug("%s: server channel error %uh, message: %.*s\n",
                    context, m->reply_code, (int)m->reply_text.len, (char *)m->reply_text.bytes);
				break;
			}
			default:
                logdebug("%s: unknown server error, method id 0x%08X\n", context, x.reply.id);
				break;
			}
			break;
		}

		if (x.library_error == -0x7 || x.library_error == -0x9 || x.library_error == -0xF || x.library_error == -0x11)
		{
			curStatus = false;
		}
		return -1;
	}
	void threadProc()
	{
		{
            amqp_channel_open(m_pConn, m_iChannel);

        	if (0 != ErrorMsg(amqp_get_rpc_reply( m_pConn), "open channel"))
        	{
        		amqp_channel_close( m_pConn,  m_iChannel, AMQP_REPLY_SUCCESS);
        		printf("threadProc--->amqp_channel_close return\n");
        		return ;
        	}

    
			//amqp_basic_qos(m_pConn, m_iChannel, 0, GetNum, 0);
			int ack = 1; // no_ack    是否需要确认消息后再从队列中删除消息
			amqp_bytes_t queuename = amqp_cstring_bytes(m_strQueueName.c_str());
			amqp_basic_consume(m_pConn, m_iChannel, queuename, amqp_empty_bytes, 0, ack, 0, amqp_empty_table);

			if (0 != ErrorMsg(amqp_get_rpc_reply(m_pConn), "Consuming"))
			{
				return;
			}
		}
		
		while ( looping() )
		{
			amqp_envelope_t envelope;

			amqp_maybe_release_buffers(m_pConn);

			struct timeval timeout;
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			amqp_rpc_reply_t res = amqp_consume_message(m_pConn, &envelope, &timeout, 0);

			if (AMQP_RESPONSE_NORMAL != res.reply_type)
			{
				continue;
			}

			string str((char *)envelope.message.body.bytes, envelope.message.body.len);

			std::map<void*, SubscribeCallback> subtmp;
			{
				Guard locker(mutex);
				subtmp = m_subcrbielist;
			}
			for (std::map<void*, SubscribeCallback>::iterator iter = subtmp.begin(); iter != subtmp.end(); iter++)
			{
				iter->second(str);
			}

			amqp_destroy_envelope(&envelope);
		}
	}
};

RabbitmqClient::RabbitmqClient(const ConnectStatusCallback &status)
{
	internal = new RabbitmqClientInternal(status);
}
 

RabbitmqClient::~RabbitmqClient() 
{
	internal->destroyThread();
	internal->close();

	SAFE_DELETE(internal);
}
 
bool RabbitmqClient::connect(const string &strvhost, const string &strHostname, int iPort, const string &strUser, const string &strPasswd, uint32_t timeout)
{
	return internal->connect(strvhost, strHostname, iPort, strUser, strPasswd, timeout);
}


bool RabbitmqClient::bind(const string &strQueueName, const string &strExchange, const string &strRoutekey, bool durable)
{
	internal->m_strQueueName = strQueueName;
	internal->m_strExchange = strExchange;
	internal->m_strRoutekey = strRoutekey;

	amqp_channel_open(internal->m_pConn, internal->m_iChannel);

	if (0 != internal->ErrorMsg(amqp_get_rpc_reply(internal->m_pConn), "open channel"))
	{
		return false;
	}

	//ExchangeDeclare
	amqp_bytes_t _exchange = amqp_cstring_bytes(internal->m_strExchange.c_str());
	amqp_bytes_t _type = amqp_cstring_bytes("direct");
	int _passive = 0;
	
	amqp_exchange_declare(internal->m_pConn, internal->m_iChannel, _exchange, _type, _passive, durable, 0, 0, amqp_empty_table);

	if (0 != internal->ErrorMsg(amqp_get_rpc_reply(internal->m_pConn), "exchange_declare"))
	{
		return false;
	}

	//QueueDeclare
	amqp_bytes_t _queue = amqp_cstring_bytes(internal->m_strQueueName.c_str());
	int32_t _exclusive = 0;
	int32_t _auto_delete = 0;

	amqp_queue_declare(internal->m_pConn, internal->m_iChannel, _queue, _passive, durable, _exclusive, _auto_delete, amqp_empty_table);

	if (0 != internal->ErrorMsg(amqp_get_rpc_reply(internal->m_pConn), "queue_declare"))
	{
		return false;
	}

	//QueueBind
	_queue = amqp_cstring_bytes(internal->m_strQueueName.c_str());
	_exchange = amqp_cstring_bytes(internal->m_strExchange.c_str());
	amqp_bytes_t _routkey = amqp_cstring_bytes(internal->m_strRoutekey.c_str());

	amqp_queue_bind(internal->m_pConn, internal->m_iChannel, _queue, _exchange, _routkey, amqp_empty_table);

	if (0 != internal->ErrorMsg(amqp_get_rpc_reply(internal->m_pConn), "queue_bind"))
	{
		return false;
	}
	amqp_channel_close(internal->m_pConn, internal->m_iChannel, AMQP_REPLY_SUCCESS);

	return true;
}



bool RabbitmqClient::disconnect() 
{
	internal->close();

	return false;
}
 
 
 
bool RabbitmqClient::publish( const string &strMessage ) 
{
	Guard locker(internal->mutex);
    if (NULL == internal->m_pConn)
    {
        return false;
    }

    amqp_channel_open(internal->m_pConn, internal->m_iChannel);
    
    if(0 != internal->ErrorMsg(amqp_get_rpc_reply(internal->m_pConn), "open channel"))
    {
        amqp_channel_close(internal->m_pConn, internal->m_iChannel, AMQP_REPLY_SUCCESS);
        return false;
    }
    
	amqp_bytes_t message_bytes;
    message_bytes.len = strMessage.length();
    message_bytes.bytes = (void *)(strMessage.c_str());
 
    amqp_bytes_t exchange = amqp_cstring_bytes(internal->m_strExchange.c_str());
    amqp_bytes_t routekey = amqp_cstring_bytes(internal->m_strRoutekey.c_str());
 
    if (0 != amqp_basic_publish(internal->m_pConn, internal->m_iChannel, exchange, routekey, 0, 0, NULL, message_bytes))
    {
        logwarn("publish amqp_basic_publish failed\n");
    }
    amqp_channel_close(internal->m_pConn, internal->m_iChannel, AMQP_REPLY_SUCCESS);
    return true;
}

bool RabbitmqClient::publish( const string &strRoutekey, const string &strMessage ) 
{
    Guard locker(internal->mutex);
    
    if (NULL == internal->m_pConn)
    {
        return false;
    }
    printf("RabbitmqClient::publish---->\n");
	amqp_channel_open(internal->m_pConn, internal->m_iChannel);

	if (0 != internal->ErrorMsg(amqp_get_rpc_reply(internal->m_pConn), "open channel"))
	{
		amqp_channel_close(internal->m_pConn, internal->m_iChannel, AMQP_REPLY_SUCCESS);
        printf("RabbitmqClient::publish---->false\n");
		return false;
	}

	amqp_bytes_t message_bytes;
    message_bytes.len = strMessage.length();
    message_bytes.bytes = (void *)(strMessage.c_str());
 
    amqp_bytes_t exchange = amqp_cstring_bytes(internal->m_strExchange.c_str());
    amqp_bytes_t routekey = amqp_cstring_bytes(strRoutekey.c_str());


 	amqp_basic_properties_t props = {0x0};
    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
    props.headers = {0x0};
    //props.content_type = amqp_cstring_bytes("text/plain");
    props.content_type = amqp_cstring_bytes("application/json");
    props.delivery_mode = 2;  
    

    //if (0 != amqp_basic_publish(internal->m_pConn, internal->m_iChannel, exchange, routekey, 0, 0, NULL, message_bytes))
    if (0 != amqp_basic_publish(internal->m_pConn, internal->m_iChannel, exchange, routekey, 0, 0, &props, message_bytes))
    {
        logwarn("publish amqp_basic_publish failed\n");
    }
	amqp_channel_close(internal->m_pConn, internal->m_iChannel, AMQP_REPLY_SUCCESS);
    printf("RabbitmqClient::publish---->true\n");
    return true;
}


bool RabbitmqClient::subscribe(void* flag, const SubscribeCallback& SubscribeCall)
{
	Guard locker(internal->mutex);
	internal->createThread();

	internal->m_subcrbielist[flag] = SubscribeCall;

	return true;
}
 
bool RabbitmqClient::unsubcribe(void* flag)
{
	Guard locker(internal->mutex);

	internal->m_subcrbielist.erase(flag);

	return true;
}

 