//
//  Copyright (c)1998-2014, Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: FuncTempl.h 3 2013-01-21 06:57:38Z jiangwei $
//

#define FUNCTION_TEMPL MACRO_JOIN(Function,FUNCTION_NUMBER)

/// \class FUNCTION_TEMPL
/// \brief 函数指针类模块
///
/// 支持普通函数指针和成员函数指针的保存，比较，调用等操作。对于成员函数，
/// 要求类只能是普通类或者是单继承的类，不能是多继承或虚继承的类。FUNCTION_TEMPL是一个宏，
/// 根据参数个数会被替换成Function，用户通过FunctionN<R, T1, T2, T3,..,TN>方式来使用，
/// R表示返回值类型，TN表示参数类型， N表示函数参数个数，目前最大参数为6。示例如下：
/// \code
/// int g(int x)
/// {
/// 	return x * 2;
/// }
/// class G
/// {
/// public:
/// 	int g(int x)
/// 	{
/// 		return x * 3;
/// 	}
/// }gg;
/// void test()
/// {
/// 	Function1<int, int> f1(g);
/// 	Function1<int, int> f2(&G::g, &gg);
/// 	assert(f1(1) = 2);
/// 	assert(f2(1) = 3);
/// 	f1 = f2;
/// 	assert(f1 == f2);
/// }
/// \endcode


////////////////////////////////////////////////////////////////////////////////

// Comma if nonzero number of arguments
#if FUNCTION_NUMBER == 0
#  define FUNCTION_COMMA
#else
#  define FUNCTION_COMMA ,
#endif // FUNCTION_NUMBER > 0

#define FUNCTION_INVOKER MACRO_JOIN(function_invoker, FUNCTION_NUMBER)
#define FUNCTION_VOID_INVOKER MACRO_JOIN(function_void_invoker, FUNCTION_NUMBER)
#define GET_FUNCTION_INVODER MACRO_JOIN(get_function_invoker, FUNCTION_NUMBER)
#define MEM_FUNCTION_INVOKER MACRO_JOIN(mem_function_invoker, FUNCTION_NUMBER)
#define MEM_FUNCTION_VOID_INVOKER MACRO_JOIN(mem_function_void_invoker, FUNCTION_NUMBER)
#define GET_MEM_FUNCTION_INVODER MACRO_JOIN(get_mem_function_invoker, FUNCTION_NUMBER)
#define FUNCTION_RETURN MACRO_JOIN(function_return,FUNCTION_NUMBER)


template<class R>
struct FUNCTION_RETURN
{
	static R getDefaultValue()
	{
		R* ptr = new(std::nothrow) R();
		R tmp(*ptr);
		delete ptr;
		return tmp;
	}
};
////////////////////////////////////////////////////////////////////////////////

template<FUNCTION_CLASS_TYPES> struct FUNCTION_INVOKER
{
	template<class F>
	static typename Detail::function_return_type<R>::type invoke(F f FUNCTION_COMMA FUNCTION_TYPE_ARGS)
	{
		if(f == NULL)
		{
			return FUNCTION_RETURN<R>::getDefaultValue();
		}
		return f(FUNCTION_ARGS);
	}
};

template<FUNCTION_CLASS_TYPES> struct FUNCTION_VOID_INVOKER
{
	template<class F>
	static typename Detail::function_return_type<R>::type invoke(F f FUNCTION_COMMA FUNCTION_TYPE_ARGS)
	{
		if(f == NULL)
		{
			return;
		}		
		f(FUNCTION_ARGS);
	}
};

template<class RT> struct GET_FUNCTION_INVODER
{
	template<FUNCTION_CLASS_TYPES> struct Invoker
	{
		typedef FUNCTION_INVOKER<R FUNCTION_COMMA FUNCTION_TYPES> type;
	};
};

template<> struct GET_FUNCTION_INVODER<void>
{
	template<FUNCTION_CLASS_TYPES> struct Invoker
	{
		typedef FUNCTION_VOID_INVOKER<R FUNCTION_COMMA FUNCTION_TYPES> type;
	};
};

////////////////////////////////////////////////////////////////////////////////

template<FUNCTION_CLASS_TYPES> struct MEM_FUNCTION_INVOKER
{
	template<class O, class F>
		static typename Detail::function_return_type<R>::type invoke(O o, F f FUNCTION_COMMA FUNCTION_TYPE_ARGS)
	{
		if(o == NULL || f == NULL)
		{
			return FUNCTION_RETURN<R>::getDefaultValue();
		}
		
		return (o->*f)(FUNCTION_ARGS);
	}
};

template<FUNCTION_CLASS_TYPES> struct MEM_FUNCTION_VOID_INVOKER
{
	template<class O, class F>
		static typename Detail::function_return_type<R>::type invoke(O o, F f FUNCTION_COMMA FUNCTION_TYPE_ARGS)
	{
		if(o == NULL || f == NULL)
		{
			return;
		}
		(o->*f)(FUNCTION_ARGS);
	}
};

template<class RT> struct GET_MEM_FUNCTION_INVODER
{
	template<FUNCTION_CLASS_TYPES> struct Invoker
	{
		typedef MEM_FUNCTION_INVOKER<R FUNCTION_COMMA FUNCTION_TYPES> type;
	};
};

template<> struct GET_MEM_FUNCTION_INVODER<void>
{
	template<FUNCTION_CLASS_TYPES> struct Invoker
	{
		typedef MEM_FUNCTION_VOID_INVOKER<R FUNCTION_COMMA FUNCTION_TYPES> type;
	};
};

////////////////////////////////////////////////////////////////////////////////

template <FUNCTION_CLASS_TYPES>
class FUNCTION_TEMPL
{
	class X{};

	enum FunctionType
	{
		typeEmpty,
		typeMember,
		typePointer,
	};
	typedef R (X::*MEM_FUNCTION)(FUNCTION_TYPES);
	typedef R (*PTR_FUNCTION)(FUNCTION_TYPES);
	
	class MemObjectBasePtr
	{
	public:
		MemObjectBasePtr(X* _ptr):ptr(_ptr){}
		virtual ~MemObjectBasePtr(){}

		X*  getMemObject() const {return ptr;}
		virtual MemObjectBasePtr* clone() const
		{
			return new MemObjectBasePtr(ptr);
		}
	private:
		X*	ptr;
	};

	class MemObjectShardPtr:public MemObjectBasePtr
	{
	public:
		MemObjectShardPtr(const shared_ptr<X>& _pxx):MemObjectBasePtr(const_cast<X*>(_pxx.get())),pxx(_pxx){}
		virtual ~MemObjectShardPtr(){}

		virtual MemObjectBasePtr* clone() const
		{
			return new MemObjectShardPtr(pxx);
		}
	private:
		shared_ptr<X>	pxx;
	};

	struct MemFunctionObject
	{
	public:
		MemFunctionObject(X* _ptr):ptr(new MemObjectBasePtr(_ptr)){}
		MemFunctionObject(const shared_ptr<X>& _ptr):ptr(new MemObjectShardPtr(_ptr)){}
		MemFunctionObject(MemObjectBasePtr* _ptr):ptr(_ptr){}
		virtual ~MemFunctionObject(){delete ptr;}

		MemFunctionObject* clone() const {return new MemFunctionObject(ptr->clone());}

		bool isSame(const MemFunctionObject* obj)
		{
			if(ptr == NULL && obj->ptr == NULL)
			{
				return true;
			}
			if(ptr != NULL && obj->ptr != NULL)
			{
				return ptr->getMemObject() == obj->ptr->getMemObject();
			}

			return false;
		}

		X*  getMemObject() const {return ptr->getMemObject();}
	private:
		MemObjectBasePtr*	ptr;
	};

	union _function
	{
		struct
		{
			MEM_FUNCTION		proc;
			MemFunctionObject*	obj;
		}memFunction;
		PTR_FUNCTION ptrFunction;
	}function;

	FunctionType type;
	const char* objectType;
public:
	/// 缺省构造函数
	FUNCTION_TEMPL( ) :type(typeEmpty), objectType(0)
	{
	}

	/// 接受成员函数指针构造函数，将类的成员函数和类对象的指针绑定并保存。
	/// \param [in] f 类的成员函数
	/// \param [in] o 类对象的指针
	template<typename O>
	FUNCTION_TEMPL(R(O::*f)(FUNCTION_TYPES), const O * o):type(typeEmpty), objectType(0)
	{
		bind(f,o);
	}

	template<typename O>
	FUNCTION_TEMPL(R(O::*f)(FUNCTION_TYPES), const shared_ptr<O>& ptr):type(typeEmpty), objectType(0)
	{
		bind(f,ptr);
	}

	/// 接受普通函数指针构造函数，保存普通函数指针。
	/// \param [in] f 函数指针
	FUNCTION_TEMPL(PTR_FUNCTION f):type(typeEmpty), objectType(0)
	{
		if(f != NULL)
		{
			type = typePointer;
			function.ptrFunction = f;
		}
	}
	FUNCTION_TEMPL(const FUNCTION_TEMPL& f):type(typeEmpty), objectType(0)
	{
		swap(f);		
	}
	~FUNCTION_TEMPL()
	{
		realse();
	}
	/// 拷贝构造函数
	/// \param [in] f 源函数指针对象
	FUNCTION_TEMPL& operator=(const FUNCTION_TEMPL& f)
	{
		swap(f);

		return *this;
	}
	/// 将类的成员函数和类对象的指针绑定并保存。其他类型的函数指针可以=运算符和隐式转换直接完成。
	template<typename O>
	void bind(R(O::*f)(FUNCTION_TYPES), const O * o)
	{
		realse();

		if(f == NULL || o == NULL)
		{
			return;
		}

		type = typeMember;
		function.memFunction.proc = horrible_cast<MEM_FUNCTION>(f); //what's safty, hei hei
		function.memFunction.obj = new MemFunctionObject(horrible_cast<X*>(o));
		objectType = typeid(O).name();
	}

	template<typename O>
	void bind(R(O::*f)(FUNCTION_TYPES), const shared_ptr<O>& ptr)
	{
		realse();

		if(f == NULL || ptr == NULL)
		{
			return;
		}

		type = typeMember;
		function.memFunction.proc = horrible_cast<MEM_FUNCTION>(f); //what's safty, hei hei
		function.memFunction.obj = new MemFunctionObject(ptr);
		objectType = typeid(O).name();
	}

	/// 判断函数指针是否为空
	bool empty() const
	{
		return (type == typeEmpty || (type == typePointer && function.ptrFunction == NULL));
	}

	/// 比较两个函数指针是否相等
	bool operator ==(const FUNCTION_TEMPL& f) const
	{
		if(type != f.type)
		{
			return false;
		}
		if(type == typeMember)
		{
			return (function.memFunction.proc == f.function.memFunction.proc
				&& function.memFunction.obj->isSame(f.function.memFunction.obj));
		}
		else if(type == typePointer)
		{
			return (function.ptrFunction == f.function.ptrFunction);
		}
		else
		{
			return true;
		}
	}
	/// 比较两个函数指针是否相等
	bool operator !=(const FUNCTION_TEMPL& f) const
	{
		if(type != f.type)
		{
			return true;
		}
		if(type == typeMember)
		{
			return (function.memFunction.proc != f.function.memFunction.proc
				|| !function.memFunction.obj->isSame(f.function.memFunction.obj));
		}
		else if(type == typePointer)
		{
			return (function.ptrFunction != f.function.ptrFunction);
		}
		else
		{
			return false;
		}
	}
	/// 重载()运算符，可以以函数对象的形式来调用保存的函数指针。
	inline typename Detail::function_return_type<R>::type operator()(FUNCTION_TYPE_ARGS)
	{
		if(type == typeMember)
		{
			typedef typename GET_MEM_FUNCTION_INVODER<R>::template Invoker<R FUNCTION_COMMA FUNCTION_TYPES>::type Invoker;
			return Invoker::invoke(function.memFunction.obj->getMemObject(), function.memFunction.proc FUNCTION_COMMA FUNCTION_ARGS);
		}
		else
		{
			typedef typename GET_FUNCTION_INVODER<R>::template Invoker<R FUNCTION_COMMA FUNCTION_TYPES>::type Invoker;
			return Invoker::invoke((type == typeEmpty) ? NULL : function.ptrFunction FUNCTION_COMMA FUNCTION_ARGS);
		}
	}
	
	void * getObject() const
	{
		if(type != typeMember || function.memFunction.obj == NULL)
		{
			return NULL;
		}
		return function.memFunction.obj->getMemObject();
	}
	
	const char* getObjectType() const
	{
		return objectType;
	}

	void realse()
	{
		if(type == typeMember && function.memFunction.obj != NULL)
		{
			delete function.memFunction.obj;
		}
		type = typeEmpty;
		objectType = 0;
	}
	void swap(const FUNCTION_TEMPL& f)
	{
		if (&f == this)
			return;

		realse();

		type = f.type;
		objectType = f.objectType;
		if(type == typePointer)
		{
			function.ptrFunction = f.function.ptrFunction;
		}
		else if(type == typeMember)
		{
			function.memFunction.proc = f.function.memFunction.proc;
			function.memFunction.obj = f.function.memFunction.obj->clone(); 
		}
	}
};

#undef FUNCTION_TEMPL
#undef FUNCTION_COMMA
#undef FUNCTION_INVOKER
#undef FUNCTION_VOID_INVOKER
#undef GET_FUNCTION_INVODER
#undef MEM_FUNCTION_INVOKER
#undef MEM_FUNCTION_VOID_INVOKER
#undef GET_MEM_FUNCTION_INVODER

