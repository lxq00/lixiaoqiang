#include "Base/ConsoleCommand.h"
#include "Base/Mutex.h"
#include "Base/Time.h"
#include "Base/String.h"
#include "Base/Base.h"
namespace Public {
namespace Base {
	
struct tagConsoleCommand_t
{
	tagConsoleCommand_t()
	{
	}

	std::string name;
	std::string desc;
	ConsoleCmdCallback cbfunc;
};

struct ConsoleCommand::tagConsoleInternal_t
{
	tagConsoleInternal_t()
	{
	}

	std::string title;
	list<tagConsoleCommand_t*> commands;
	Mutex mutex;
	bool standalone;
};


ConsoleCommand::ConsoleCommand() : Thread("ConsoleCommand"), consoleInternal(NULL), finished(false)
{
	consoleInternal = new tagConsoleInternal_t;
}

ConsoleCommand::~ConsoleCommand()
{
	terminateThread();

	consoleInternal->mutex.enter();
	while (0 < consoleInternal->commands.size())
	{
		delete consoleInternal->commands.front();
		consoleInternal->commands.pop_front();
	}
	consoleInternal->mutex.leave();

	if(NULL != consoleInternal)
	{
		delete consoleInternal;
		consoleInternal = NULL;
	}
}


void ConsoleCommand::setModuleTitle(const std::string& title)
{
	consoleInternal->title = title;
}

bool ConsoleCommand::getValid()
{
	return isThreadOver();
}

void ConsoleCommand::run(const std::string& title)
{
	consoleInternal->title = title;
	
	createThread();

	BaseSystem::waitSystemClose();
}

void ConsoleCommand::runing()
{
	addUserCmd("?", "Get command(s) list", NULL);
	addUserCmd("help", "Get help information", NULL);
	addUserCmd("exit", "Exit", NULL);

	Semaphore waitsem;

	while (looping())
	{
#ifndef DEBUG
		waitsem.pend(500);
#else
		printf("%s =>",consoleInternal->title.c_str());
		char command[1024] = {'\0'};
		
		if(NULL != fgets(command,1023,stdin))
		{
			parseCmd(command);
		}
#endif
	}
}


bool ConsoleCommand::addUserCmd(const std::string& name, const std::string& desc,const ConsoleCmdCallback& func)
{
	if("" != name && "" != desc && NULL != consoleInternal)
	{
		for (list<tagConsoleCommand_t*>::iterator it = consoleInternal->commands.begin(); it != consoleInternal->commands.end(); it++)
		{
			if (name == (*it)->name)
			{
				return false;
			}
		}

		tagConsoleCommand_t* cmd = new tagConsoleCommand_t;
		cmd->name = name;
		cmd->desc = desc;
		cmd->cbfunc = func;
		consoleInternal->mutex.enter();
 		consoleInternal->commands.push_back(cmd);
 		consoleInternal->commands.sort();
		consoleInternal->mutex.leave();
	}

	return false;
}

bool ConsoleCommand::parseCmd(const char* command)
{
	if(strcmp("", command))
	{
		bool ret = false;
		int argc = 0;
		char* argv[20] = {NULL};
		for (int i = 0; i != 20; i++)
		{
			argv[i] = new char[64];
			memset(argv[i], '\0', 64);
		}

		if(separatePara(command, argc, (char**)argv))
		{
			for (list<tagConsoleCommand_t*>::iterator it = consoleInternal->commands.begin(); it != consoleInternal->commands.end(); it++)
			{
				if (argv[0] == (*it)->name)
				{
					dispatchCmd(argc, argv, (*it)->cbfunc);
					ret = true;
					break;
				}
			}
		}

		for (int i = 0; i != 20; i++)
		{
			delete[] argv[i];
		}

		if(false == ret)
		{
			cout << command << " is not invaild command" << endl;
		}

		return ret;
	}

	return false;
}

void ConsoleCommand::threadProc()
{
	runing();
}

bool ConsoleCommand::dispatchCmd(const int argc, char** argv, ConsoleCmdCallback func)
{
	if(NULL != argv)
	{
		if(0 == strcmp("?", argv[0]) || 0 == strcmp("help", argv[0]))
		{
			char itemTitle[1024] = {'\0'};
			snprintf_x(itemTitle, 1024, "%-20s%-20s", "[Command Name]", "[Description]");
			cout <<itemTitle<< endl;

			for (list<tagConsoleCommand_t*>::iterator it = consoleInternal->commands.begin(); it != consoleInternal->commands.end(); it++)
			{
				char item[1024] = {'\0'};
				snprintf_x(item, 1024, "%-20s%-20s", (*it)->name.c_str(), (*it)->desc.c_str());
				cout << item << endl;
			}
			return true;
		}

		if(0 == strcmp("exit", argv[0]))
		{
			if(NULL != consoleInternal)
			{
				if(consoleInternal->standalone)
				{
					finished = true;
				}
				else
				{
					cancelThread();
				}
			}
			BaseSystem::consoleCommandClose(BaseSystem::CloseEvent_INPUT);
			
			return true;
		}

		func(argc, argv);
		return true;
	}

	return false;
}

bool ConsoleCommand::separatePara(const char* command, int& argc, char** argv)
{
	if(NULL != command && 0 == argc && NULL != argv)
	{
		int startPos = 0;
		size_t cmdSize = strlen(command);
		for (int i = 0; i != cmdSize; i++)
		{
			if(' ' == command[i])
			{
				memcpy(argv[argc++], command + startPos, i - startPos);
				startPos = i;
			}
		}
		memcpy(argv[argc], command + startPos, cmdSize - startPos);
		return true;
	}

	return false;
}


} // namespace Base
} // namespace Public
