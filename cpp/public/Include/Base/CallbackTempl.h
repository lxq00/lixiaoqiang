//
//  Copyright (c)1998-2012, Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: CallbackTempl.h 3 2013-01-21 06:57:38Z jiangwei $

/// \class CallbackX
/// \brief 信号类模块。
///
/// 一个信号被触发时，会调用所有连接到其上的TFunctionN对象，只支持
/// 返回值为void的FunctionN。CALLBACK_TEMPL是一个宏，根据参数个数会被替换成
/// CallbackN，用户通过 CallbackN<T1, T2, T3,..,TN>方式来使用，TN表示参数类型，
/// N表示函数参数个数，目前最大参数为6。

#define CALLBACK_TEMPL MACRO_JOIN(Callback,CALLBACK_NUMBER)
#define FUNCTION_TEMPL MACRO_JOIN(Function, CALLBACK_NUMBER)

#if (CALLBACK_NUMBER != 0)
template <CALLBACK_CLASS_TYPES>
#endif
class CALLBACK_TEMPL
{
	/// 回调节点信息
	enum NodeState
	{
		nodeStateEmpty,		///< 节点为空
		nodeStateNormal,	///< 节点以连接
	};

public:
	/// 节点挂载的位置
	enum NodePosition
	{
		any,				///< 任意地方
		back,				///< 挂载在末尾
		front				///< 挂载在头部
	};

	/// 错误码
	enum ErrorCode
	{
		errorNoFound = -1,		///< 没有找到制定的对象
		errorExsist = -2,		///< 对象已经存在
		errorFull = -3,			///< 已经到达能够连接的对象个数上限
		errorEmptyProc = -4,	///< 对象包含的函数指针为空，没有意义
	};

	/// 与信号模板参数类型匹配的函数指针对象类型
	typedef FUNCTION_TEMPL<void CALLBACK_TYPES_COMMA> Proc;

private:
	/// 点结构
	struct CallbackNode
	{
		Proc proc;
		NodeState state;
		bool running;
		uint32_t cost;
	};

	int numberMax;
	int number;
	CallbackNode* nodes;
	Mutex mutex;
	int	threadId;

public:
	/// 构造函数
	/// \param maxNodes 能够连接的最大函数指针对象的个数
	CALLBACK_TEMPL(int maxNodes) :
	  numberMax(maxNodes), number(0), threadId(-1)
	{
		nodes = new(std::nothrow) CallbackNode[maxNodes];
		for(int i = 0; i < numberMax; i++)
		{
			nodes[i].state = nodeStateEmpty;
			nodes[i].running = false;
		}
	}

	 /// 析构函数
	~CALLBACK_TEMPL()
	{
		delete []nodes;
	}

	/// 挂载函数指针对象
	/// \param proc 函数指针对象
	/// \param position 对象挂载位置
	/// \retval >=0 调用后已经挂载到信号的函数指针对象个数
	/// \retval <0 errorCode类型的错误码
	int attach(Proc proc, NodePosition position = any)
	{
		int i;

		if(proc.empty())
		{
			return errorEmptyProc;
		}

		if(isAttached(proc))
		{
			return errorExsist;
		}

		Guard guard(mutex);

		switch(position)
		{
		case any:
			for(i = 0; i < numberMax; i++)
			{
				if(nodes[i].state == nodeStateEmpty)
				{
					nodes[i].proc  = proc;
					nodes[i].state = nodeStateNormal;

					return ++number;
				}
			}
			break;
		case back:
			for(i = numberMax - 1; i >= 0; i--)
			{
				if(nodes[i].state == nodeStateEmpty)
				{
					for(int j = i; j < numberMax - 1; j++)
					{
						nodes[j] = nodes[j + 1];
					}
					nodes[numberMax - 1].proc  = proc;
					nodes[numberMax - 1].state = nodeStateNormal;

					return ++number;
				}
			}
			break;
		case front:
			for(i = 0; i < numberMax; i++)
			{
				if(nodes[i].state == nodeStateEmpty)
				{
					for(int j = i; j > 0; j--)
					{
						nodes[j] = nodes[j - 1];
					}
					nodes[0].proc  = proc;
					nodes[0].state = nodeStateNormal;

					return ++number;
				}
			}
			break;
		}

		return errorFull;
	}

	/// 卸载函数指针对象，根据对象中保存的函数指针来匹配。
	/// \param proc 函数指针对象
	/// \param wait 是否等待正在进行的回调结束。一般在使用者对象析构的时候需要等待，
	///             如果是在回调函数里卸载，则不能等待。等待要特别小心，防止死锁.
	/// \retval >=0 调用后已经挂载到信号的函数指针对象个数
	/// \retval <0 errorCode类型的错误码
	int detach(Proc proc, bool wait = false)
	{
		if (proc.empty())
		{
			return errorEmptyProc;
		}

		Guard guard(mutex);

		for (int i = 0; i < numberMax; i++)
		{
			if(nodes[i].proc ==  proc
				&& nodes[i].state == nodeStateNormal)
			{
				/// 回调线程和stop线程不是同一线程时，才需要等待，否则等待会引起自锁
				if(wait && nodes[i].running && Thread::getCurrentThreadID() != threadId)
				{
					while(nodes[i].running)
					{
						mutex.leave();
						Thread::sleep(1);
						infof("CALLBACK_TEMPL::Detach wait callback exit!\n");
						mutex.enter();
					}
				}

				nodes[i].state = nodeStateEmpty;
				return --number;
			}
		};

		return errorNoFound;
	}

	/// 卸载所有指针对象
	/// \param wait 是否等待正在进行的回调结束。一般在使用者对象析构的时候需要等待，
	///             如果是在回调函数里卸载，则不能等待。等待要特别小心，防止死锁.
	/// \retval >=0 调用后已经挂载到信号的函数指针对象个数
	/// \retval <0 errorCode类型的错误码
	int detachall(bool wait = false)
	{
		Guard guard(mutex);

		for (int i = 0; i < numberMax; i++)
		{
			if(nodes[i].state == nodeStateNormal)
			{
				/// 回调线程和stop线程不是同一线程时，才需要等待，否则等待会引起自锁
				if(wait && nodes[i].running && Thread::getCurrentThreadID() != threadId)
				{
					while(nodes[i].running)
					{
						mutex.leave();
						Thread::sleep(1);
						infof("CALLBACK_TEMPL::Detach wait callback exit!\n");
						mutex.enter();
					}
				}

				nodes[i].state = nodeStateEmpty;
				--number;
			}
		};
		return number;
	}

	
	/// 判断卸载函数指针对象是否挂载，根据对象中保存的函数指针来匹配。
	/// \param proc 函数指针对象
	bool isAttached(Proc proc)
	{
		Guard guard(mutex);

		if(proc.empty())
		{
			return false;
		}

		for(int i = 0; i < numberMax; i++)
		{
			if(nodes[i].proc ==  proc
				&& nodes[i].state == nodeStateNormal)
			{
				return true;
			}
		}

		return false;
	}

	/// 重载()运算符，可以以函数对象的形式来调用连接到信号的所有函数指针。
	inline void operator()(CALLBACK_TYPE_ARGS)
	{
		Guard guard(mutex);
		uint64_t us1 = 0, us2 = 0;

		threadId = Thread::getCurrentThreadID(); // 保存回调线程ID

		for(int i = 0; i < numberMax; i++) // call back functions one by one
		{
			if(nodes[i].state == nodeStateNormal)
			{
				Proc temp = nodes[i].proc;

				nodes[i].running = true;
				mutex.leave();

				// 函数执行与性能统计
	 			us1 = Time::getCurrentMicroSecond();
				temp(CALLBACK_ARGS);
				us2 = Time::getCurrentMicroSecond();
				nodes[i].cost = (us1 <= us2) ? uint32_t(us2 - us1) : 1;

				mutex.enter();
				nodes[i].running = false;
			}
		}
	}

	void stat()
	{
		int i;

		Guard guard(mutex);

		for(i = 0; i < numberMax; i++)
		{
			if(nodes[i].state == nodeStateNormal)
			{
				printf("\t%8d us, %p, %s\n",
				 nodes[i].cost,
				 nodes[i].proc.getObject(),
				 nodes[i].proc.getObjectType());
			}
		}
	}
};

#undef CALLBACK_TEMPL
#undef FUNCTION_TEMPL

