#include "db/keyobject.h"
#include "communication/communication.h"
#include "dump/storefactory.h"
#include "Medis/Medis.h"
#include "message/megssage.h"

using namespace Public::Medis;

struct Service::ServiceInternal
{
	ServiceInternal() {}
	~ServiceInternal() {}

	void userDisconnectCallback(void* user)
	{
		message->userOffline(user);
	}
	void commuRecvDatacallback(void* user, uint32_t dbindex, const shared_ptr<RedisValue>& value)
	{
		RedisValue retval;
		if (doServiceCommand(value, retval))
		{
			commandSendCallback(user, retval);
		}
		else if (message->inputCommand(CmdResultCallback(&ServiceInternal::commandSendCallback, this), user, dbindex, value))
		{

		}
		else
		{
			key->inputConnectionData(CmdResultCallback(&ServiceInternal::commandSendCallback, this), user, dbindex, value);
		}
	}
	void commandSendCallback(void* user, const RedisValue& value)
	{
		if (commu == NULL) return;
		shared_ptr<Connection> connection = commu->getConnection(user);
		if (connection == NULL) return;

		connection->sendResponse(value);
	}
private:
	bool doServiceCommand(const shared_ptr<RedisValue>& value, RedisValue& retval)
	{
		if (!value->isArray())
		{
			retval = RedisValue(false, "syntax error");
			return true;
		}
		const std::vector<RedisValue> & valuearray = value->getArray();
		std::string cmd = String::tolower(valuearray[0].toString());

		if (cmd == "ping")
		{
			retval = pingFunc(valuearray);
		}
		else if (cmd == "info")
		{
			retval = infoFunc(valuearray);
		}
		else if (cmd == "select")
		{
			retval = selectFunc(valuearray);
		}
		else
		{
			return false;
		}

		return true;
	}
	
	RedisValue pingFunc(const std::vector<RedisValue>& val)
	{
		return RedisValue(true, "PONG");
	}
	RedisValue selectFunc(const std::vector<RedisValue>& val)
	{
		if (val.size() != 2) return RedisValue(false, "Param Error");

		int64_t index = val[1].toInt();
		if(index < 0 || index >= MAXDBINDEX) return RedisValue(false, "invalid index");

		return RedisValue(true,"OK");
	}
	RedisValue infoFunc(const std::vector<RedisValue>& val)
	{
		std::string ackstr = "# Server\r\n"\
			"redis_version:3.2.100\r\n"\
			"redis_git_sha1:00000000\r\n"\
			"redis_git_dirty:0\r\n"\
			"redis_build_id:dd26f1f93c5130ee\r\n"\
			"redis_mode:standalone\r\n"\
			"os:Windows\r\n"\
			"arch_bits:64\r\n"\
			"multiplexing_api:WinSock_IOCP\r\n"\
			"process_id:1984\r\n"\
			"run_id:a9969c04945de8d94103f9e0df8bc8ee9558058b\r\n"\
			"tcp_port:6379\r\n"\
			"uptime_in_seconds:442177\r\n"\
			"uptime_in_days:5\r\n"\
			"hz:10\r\n"\
			"lru_clock:1172229\r\n"\
			"executable:C:\\Program Files (x86)\\ZVan\\IVSIII\\Redis\\windows\\redis-server.exe\r\n"\
			"config_file:C:\\Program Files(x86)\\ZVan\\IVSIII\\Redis\\Redis.conf\r\n\r\n"\
			"# Clients\r\n"\
			"connected_clients:38\r\n"\
			"client_longest_output_list:0\r\n"\
			"client_biggest_input_buf:0\r\n"\
			"blocked_clients:0\r\n\r\n"\
			"# Memory\r\n"\
			"used_memory:5169752\r\n"\
			"used_memory_human:4.93M\r\n"\
			"used_memory_rss:5168496\r\n"\
			"used_memory_rss_human:4.93M\r\n"\
			"used_memory_peak:5815504\r\n"\
			"used_memory_peak_human:5.55M\r\n"\
			"total_system_memory:0\r\n"\
			"total_system_memory_human:0B\r\n"\
			"used_memory_lua:37888\r\n"\
			"used_memory_lua_human:37.00K\r\n"\
			"maxmemory:0\r\n"\
			"maxmemory_human:0B\r\n"\
			"maxmemory_policy:noeviction\r\n"\
			"mem_fragmentation_ratio:1.00\r\n"\
			"mem_allocator:jemalloc - 3.6.0\r\n\r\n"\
			"# Persistence\r\n"\
			"loading:0\r\n"\
			"rdb_changes_since_last_save:227\r\n"\
			"rdb_bgsave_in_progress:0\r\n"\
			"rdb_last_save_time:1561453272\r\n"\
			"rdb_last_bgsave_status:ok\r\n"\
			"rdb_last_bgsave_time_sec:0\r\n"\
			"rdb_current_bgsave_time_sec:-1\r\n"\
			"aof_enabled:0\r\n"\
			"aof_rewrite_in_progress:0\r\n"\
			"aof_rewrite_scheduled:0\r\n"\
			"aof_last_rewrite_time_sec:-1\r\n"\
			"aof_current_rewrite_time_sec:-1\r\n"\
			"aof_last_bgrewrite_status:ok\r\n"\
			"aof_last_write_status:ok\r\n\r\n"\
			"# Stats\r\n"\
			"total_connections_received:506755\r\n"\
			"total_commands_processed:8114060\r\n"\
			"instantaneous_ops_per_sec:14\r\n"\
			"total_net_input_bytes:1569669998\r\n"\
			"total_net_output_bytes:1133318699\r\n"\
			"instantaneous_input_kbps:2.41\r\n"\
			"instantaneous_output_kbps:1.45\r\n"\
			"rejected_connections:0\r\n"\
			"sync_full:0\r\n"\
			"sync_partial_ok:0\r\n"\
			"sync_partial_err:0\r\n"\
			"expired_keys:0\r\n"\
			"evicted_keys:0\r\n"\
			"keyspace_hits:2354426\r\n"\
			"keyspace_misses:88256\r\n"\
			"pubsub_channels:60\r\n"\
			"pubsub_patterns:0\r\n"\
			"latest_fork_usec:0\r\n"\
			"migrate_cached_sockets:0\r\n\r\n"\
			"# Replication\r\n"\
			"role:master\r\n"\
			"connected_slaves:0\r\n"\
			"master_repl_offset:0\r\n"\
			"repl_backlog_active:0\r\n"\
			"repl_backlog_size:1048576\r\n"\
			"repl_backlog_first_byte_offset:0\r\n"\
			"repl_backlog_histlen:0\r\n\r\n"\
			"# CPU\r\n"\
			"used_cpu_sys:128.12\r\n"\
			"used_cpu_user:124.05\r\n"\
			"used_cpu_sys_children:0.00\r\n"\
			"used_cpu_user_children:0.00\r\n\r\n"\
			"# Cluster\r\n"\
			"cluster_enabled:0\r\n\r\n"\
			"# Keyspace\r\n";
		//	"db0:keys=0,expires=0,avg_ttl=0\r\n\r\n";

		std::vector<uint32_t> keyinfo;
		key->info(keyinfo);

		for (uint32_t i = 0; i < keyinfo.size(); i++)
		{
			ackstr += "db" + Value(i).readString() + ":keys=" + Value(keyinfo[i]).readString() + ",expires=0,avg_ttl=0\r\n";
		}
		ackstr += "\r\n";

		return RedisValue(ackstr);
	}
public:
	shared_ptr<Communication> commu;
	shared_ptr<DataFactory>	  factory;
	shared_ptr<KeyObject>	  key;
	shared_ptr<Message>		  message;
};


Service::Service()
{
	internal = new ServiceInternal();
}
Service::~Service()
{
	stop();
	SAFE_DELETE(internal);
}
bool Service::start(const shared_ptr<IOWorker>& worker, uint32_t port)
{
	std::string savefilename = File::getExcutableFileFullPath() + "\\medis.db";
	
	internal->factory = make_shared<StoreFactory>(savefilename);
	internal->key = make_shared<KeyObject>(worker, internal->factory);

	{
		std::vector<shared_ptr<ValueHeader> > headerlist;
		std::vector<shared_ptr<ValueData> > datalist;

		StoreFactory* storetmp = (StoreFactory*)internal->factory.get();
		storetmp->loadDBInfo(headerlist, datalist);

		internal->key->initKeyObject(headerlist, datalist);
	}

	internal->commu = make_shared<Communication>(worker, RecvDataCallback(&ServiceInternal::commuRecvDatacallback, internal),
		ConnectionDisconnectCallback(&ServiceInternal::userDisconnectCallback,internal),port);
	internal->message = make_shared<Message>(worker);

	return true;
}
bool Service::stop()
{
	internal->commu = NULL;
	internal->key = NULL;
	internal->factory = NULL;

	return true;
}