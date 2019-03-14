#include "redisclientimpl.h"
namespace redisclient {

RedisClientImpl::RedisClientImpl(const shared_ptr<IOWorker>& _worker):worker(_worker)
{}

RedisClientImpl::~RedisClientImpl()
{
    close();
}

void RedisClientImpl::close()
{
	weak_ptr<Socket> socktmp = sock;
	sock = NULL;

	while (socktmp.lock() != NULL)
	{
		Thread::sleep(10);
	}
}

bool RedisClientImpl::isConnected() const
{
	shared_ptr<Socket> tmp = sock;
	if (tmp == NULL) return false;

	return tmp->getStatus() == NetStatus_connected;
}

void RedisClientImpl::asyncConnect(const NetAddr& addr, const ConnectCallback& callback, const DisconnectCallback& discallback)
{
	Guard locker(mutex);
	sublist.clear();

	connectCallback = callback;
	disconnectCallback = discallback;

	sock = TCPClient::create(worker);
	sock->async_connect(addr, Socket::ConnectedCallback(&RedisClientImpl::socketConnectCallback, this));
}

void RedisClientImpl::socketConnectCallback(const weak_ptr<Socket>&)
{
	connectCallback();

	shared_ptr<Socket> socktmp = sock;

	if (socktmp != NULL)
	{
		socktmp->setDisconnectCallback(Socket::DisconnectedCallback(&RedisClientImpl::socketDisconnectCallback, this));
		socktmp->async_recv(Socket::ReceivedCallback(&RedisClientImpl::socketRecvCallback, this));
	}	

	//连接成功要启动数据的发送
	{
		Guard locker(mutex);
		_socketSendCmd();
	}
}
void RedisClientImpl::socketDisconnectCallback(const weak_ptr<Socket>& sock, const std::string&)
{
	disconnectCallback();
}
void RedisClientImpl::doAsyncCommand(const std::string& cmdstr, const CmdCallback&  handler)
{
	Guard locker(mutex);

	shared_ptr<CmdInfo> cmd = make_shared<CmdInfo>();
	cmd->cmd = std::move(cmdstr);
	cmd->handler = handler;
	
	int nowcmdsize = cmdSendList.size();

	cmdSendList.push_back(cmd);
	cmdWaitRecvList.push_back(cmd);

	//还没命令，处理命令
	if (nowcmdsize == 0)
	{
		_socketSendCmd();
	}
}

void RedisClientImpl::_socketSendCmd()
{
	shared_ptr<Socket> socktmp = sock;

	if (cmdSendList.size() == 0 || socktmp == NULL) return;

	shared_ptr<CmdInfo> cmd = cmdSendList.front();

	socktmp->async_send(cmd->cmd.c_str(), cmd->cmd.length(), Socket::SendedCallback(&RedisClientImpl::socketSendCallback, this));
}
void RedisClientImpl::socketSendCallback(const weak_ptr<Socket>&s, const char* tmp, int len)
{
	Guard locker(mutex);
	if (cmdSendList.size() == 0) return;

	cmdSendList.pop_front();

	_socketSendCmd();
}
void RedisClientImpl::socketRecvCallback(const weak_ptr<Socket>&s, const char* tmp, int len)
{
	std::string tmpstr = std::string(tmp, len);
	if (!input(tmp, len))
	{
		assert(0);
		return;
	}

	while (true)
	{
		shared_ptr<RedisValue> val = result();
		if (val == NULL) break;

		doProcessMessage(val);
	}

	shared_ptr<Socket> socktmp = sock;

	if (socktmp != NULL)
	{
		socktmp->async_recv(Socket::ReceivedCallback(&RedisClientImpl::socketRecvCallback, this));
	}
}
void RedisClientImpl::doProcessMessage(const shared_ptr<RedisValue>& v)
{
	if (v->isArray())
	{
		std::vector<RedisValue> result = v->toArray();
		int resultSize = result.size();
		if(resultSize >= 3)
		{
			const RedisValue &command = result[0];
			const RedisValue &queueName = result[(resultSize == 3) ? 1 : 2];
			const RedisValue &value = result[(resultSize == 3) ? 2 : 3];
			const RedisValue &pattern = (resultSize == 4) ? result[1] : queueName;

			std::string cmd = command.toString();

			if (cmd == "message" || cmd == "pmessage")
			{
				std::map<void*, shared_ptr<SubcribeInfo> > subitemlist;
				{
					std::string patternstr = pattern.toString();
					Guard locker(mutex);
					std::map<std::string, std::map<void*, shared_ptr<SubcribeInfo> > >::iterator iter = sublist.find(patternstr);
					if (iter != sublist.end())
					{
						subitemlist = iter->second;
					}
				}
				for (std::map<void*, shared_ptr<SubcribeInfo> >::iterator iter = subitemlist.begin(); iter != subitemlist.end(); iter++)
				{
					CmdCallback	handler = iter->second->handler;
					handler(value);
				}

				return;
			}
			else if (cmd == "subscribe" || cmd == "unsubscribe" || cmd == "psubscribe" || cmd == "punsubscribe")
			{
				shared_ptr<CmdInfo> cmd;
				{
					Guard locker(mutex);
					if (cmdWaitRecvList.size() > 0)
					{
						cmd = cmdWaitRecvList.front();
						cmdWaitRecvList.pop_front();
					}
				}
				if (cmd != NULL)
				{
					cmd->handler(value);
				}

				return;
			}
		}
	}
	
	{
		shared_ptr<CmdInfo> cmd;
		{
			Guard locker(mutex);
			if (cmdWaitRecvList.size() > 0)
			{
				cmd = cmdWaitRecvList.front();
				cmdWaitRecvList.pop_front();
			}
		}
		if (cmd != NULL)
		{
			cmd->handler(*v.get());
		}
	}
}
void* RedisClientImpl::subscribe(const std::string &command, const std::string &channel, const SubcribeHandler&  subhandler, const CmdCallback& callback)
{
	void* subid = NULL;
	{
		Guard locker(mutex);
		std::map<std::string, std::map<void*, shared_ptr<SubcribeInfo> > >::iterator iter = sublist.find(channel);
		if (iter == sublist.end())
		{
			sublist[channel] = std::map<void*, shared_ptr<SubcribeInfo> >();
			iter = sublist.find(channel);
		}
		shared_ptr<SubcribeInfo> info = make_shared<SubcribeInfo>();
		info->handler = subhandler;
		subid = info.get();
		iter->second[subid] = info;
	}

	std::deque<Value> items{ command, channel };
	doAsyncCommand(RedisBuilder::makeCommand(items), callback);

	return subid;
}

void RedisClientImpl::unsubscribe(const std::string &command, const std::string &channel, void* subid)
{
	Guard locker(mutex);
	std::map<std::string, std::map<void*, shared_ptr<SubcribeInfo> > >::iterator iter = sublist.find(channel);
	if (iter != sublist.end())
	{
		iter->second.erase(subid);
	}

	std::deque<Value> items{ command, channel };
	doAsyncCommand(RedisBuilder::makeCommand(items), CmdCallback());
}

}
