//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: FuncTempl.h 3 2013-01-21 06:57:38Z  $
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


///////////////////////////////////默认返回值定义////////////////////////////////////////////
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

#ifndef GCCSUPORTC11

////////////////////////////////////全局定义//////////////////////////////////////////////////////
#define FUNCTION_INVOKER MACRO_JOIN(function_invoker, FUNCTION_NUMBER)
#define FUNCTION_VOID_INVOKER MACRO_JOIN(function_void_invoker, FUNCTION_NUMBER)
#define GET_FUNCTION_INVODER MACRO_JOIN(get_function_invoker, FUNCTION_NUMBER)
#define MEM_FUNCTION_INVOKER MACRO_JOIN(mem_function_invoker, FUNCTION_NUMBER)
#define MEM_FUNCTION_VOID_INVOKER MACRO_JOIN(mem_function_void_invoker, FUNCTION_NUMBER)
#define GET_MEM_FUNCTION_INVODER MACRO_JOIN(get_mem_function_invoker, FUNCTION_NUMBER)


////////////////////////////////////这一款为函数指针回调处理////////////////////////////////////////////

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

///////////////////////////////////////这一块为成员函数回调处理/////////////////////////////////////////

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
#else
///////////////////////////////////这一块为std::function的支持/////////////////////////////////////////////

#define STD_FUNCTION_INVOKER MACRO_JOIN(std_function_invoker, FUNCTION_NUMBER)
#define STD_FUNCTION_VOID_INVOKER MACRO_JOIN(std_function_void_invoker, FUNCTION_NUMBER)
#define GET_STD_FUNCTION_INVODER MACRO_JOIN(get_std_function_invoker, FUNCTION_NUMBER)


template<FUNCTION_CLASS_TYPES> struct STD_FUNCTION_INVOKER
{
	static typename Detail::function_return_type<R>::type invoke(std::function<FUNCTION_STDFUNCTIONPTR>& f FUNCTION_COMMA FUNCTION_TYPE_ARGS)
	{
		if (!f)
		{
			return FUNCTION_RETURN<R>::getDefaultValue();
		}

		return f(FUNCTION_ARGS);
	}
};
template<FUNCTION_CLASS_TYPES> struct STD_FUNCTION_VOID_INVOKER
{
	static typename Detail::function_return_type<R>::type invoke(std::function<FUNCTION_STDFUNCTIONPTR>& f FUNCTION_COMMA FUNCTION_TYPE_ARGS)
	{
		if(f)
			f(FUNCTION_ARGS);
	}
};

template<class RT> struct GET_STD_FUNCTION_INVODER
{
	template<FUNCTION_CLASS_TYPES> struct Invoker
	{
		typedef STD_FUNCTION_INVOKER<R FUNCTION_COMMA FUNCTION_TYPES> type;
	};
};

template<> struct GET_STD_FUNCTION_INVODER<void>
{
	template<FUNCTION_CLASS_TYPES> struct Invoker
	{
		typedef STD_FUNCTION_VOID_INVOKER<R FUNCTION_COMMA FUNCTION_TYPES> type;
	};
};

#endif

/////////////////////////////////////函数定义///////////////////////////////////////////

template <FUNCTION_CLASS_TYPES>
class FUNCTION_TEMPL
{
	typedef R(*PTR_FUNCTION)(FUNCTION_TYPES);

	class X {};
	typedef R(X::*MEM_FUNCTION)(FUNCTION_TYPES);

#ifndef GCCSUPORTC11
	enum FunctionType
	{
		typeEmpty,
		typeMember,
		typePointer,
	};
	
	///////////////////////////////这一块为了处理成员回调ptr*和sharedptr的处理////////////////////////////////////
	class MemObjectPtr
	{
	public:
		MemObjectPtr(){}
		MemObjectPtr(const MemObjectPtr& mptr) :ptr(mptr.ptr), sptr(mptr.sptr) {}
		MemObjectPtr(X* _ptr) :ptr(_ptr) {}
		MemObjectPtr(const weak_ptr<X>& wptr) :ptr(NULL), sptr(wptr.lock()) {}
		X*  getMemPtr() const { return (ptr != NULL ? ptr : sptr.get()); }
	private:
		X * ptr;
		shared_ptr<X> sptr;
	};
	
	class MemObjectBasePtr
	{
	public:
		MemObjectBasePtr(X* _ptr):ptr(_ptr){}
		virtual ~MemObjectBasePtr(){}

		virtual MemObjectPtr  getMemObject() const {return MemObjectPtr(ptr);}
		virtual shared_ptr<MemObjectBasePtr> clone() const
		{
			return shared_ptr<MemObjectBasePtr>(new MemObjectBasePtr(ptr));
		}
	private:
		X*	ptr;
	};

	class MemObjectShardPtr:public MemObjectBasePtr
	{
	public:
		MemObjectShardPtr(const weak_ptr<X>& _pxx):MemObjectBasePtr(NULL),pxx(_pxx){}
		virtual ~MemObjectShardPtr(){}
		virtual MemObjectPtr  getMemObject() const { return MemObjectPtr(pxx); }
		virtual shared_ptr<MemObjectBasePtr> clone() const
		{
			return shared_ptr<MemObjectBasePtr>(new MemObjectShardPtr(pxx));
		}
	private:
		weak_ptr<X>	pxx;
	};

	struct MemFunctionObject
	{
	public:
		MemFunctionObject(X* _ptr):ptr(new MemObjectBasePtr(_ptr)){}
		MemFunctionObject(const weak_ptr<X>& _ptr):ptr(new MemObjectShardPtr(_ptr)){}
		MemFunctionObject(const shared_ptr<MemObjectBasePtr>& _ptr):ptr(_ptr){}
		MemFunctionObject(const shared_ptr<MemFunctionObject>& obj):ptr(obj.get()->ptr){}
		virtual ~MemFunctionObject() { ptr = NULL; }

		shared_ptr<MemFunctionObject> clone() const {return shared_ptr<MemFunctionObject>(new MemFunctionObject(ptr->clone()));}

		bool isSame(const shared_ptr<MemFunctionObject>& obj)
		{
			if(ptr == NULL && obj->ptr == NULL)
			{
				return true;
			}
			if(ptr != NULL && obj->ptr != NULL)
			{
				MemObjectPtr ptr1 = ptr->getMemObject();
				MemObjectPtr ptr2 = obj->ptr->getMemObject();
				return ptr1.getMemPtr() == ptr2.getMemPtr();
			}

			return false;
		}

		MemObjectPtr getMemObject() const {return ptr->getMemObject();}
	private:
		shared_ptr<MemObjectBasePtr>	ptr;
	};

	////////////////////////////////下面为定义处理////////////////////////////////////////
	struct _function
	{
		struct
		{
			MEM_FUNCTION					proc;
			shared_ptr<MemFunctionObject>	obj;
		}memFunction;
		PTR_FUNCTION ptrFunction;
	}function;
	FunctionType type;
#else
	typedef std::function<FUNCTION_STDFUNCTIONPTR> STD_FUNCTION;
	STD_FUNCTION function;
#endif

public:
	/// 缺省构造函数
	FUNCTION_TEMPL( ) 
#ifndef GCCSUPORTC11
		:type(typeEmpty)
#endif
	{
	}

	/// 接受成员函数指针构造函数，将类的成员函数和类对象的指针绑定并保存。
	/// \param [in] f 类的成员函数
	/// \param [in] o 类对象的指针
	template<typename O>
	FUNCTION_TEMPL(R(O::*f)(FUNCTION_TYPES), const O * o)
#ifndef GCCSUPORTC11
		:type(typeEmpty)
#endif
	{
		bind(f,o);
	}

	template<typename O>
	FUNCTION_TEMPL(R(O::*f)(FUNCTION_TYPES), const weak_ptr<O>& ptr)
#ifndef GCCSUPORTC11	
		:type(typeEmpty)
#endif
	{
		bind(f,ptr);
	}

	/// 接受普通函数指针构造函数，保存普通函数指针。
	/// \param [in] f 函数指针
	FUNCTION_TEMPL(PTR_FUNCTION f)
#ifndef GCCSUPORTC11	
		:type(typeEmpty)
#endif
	{
		bind(f);
	}
	FUNCTION_TEMPL(const FUNCTION_TEMPL& f)
#ifndef GCCSUPORTC11	
		:type(typeEmpty)
#endif
	{
		swap(f);		
	}

#ifdef GCCSUPORTC11
	FUNCTION_TEMPL(const std::function<FUNCTION_STDFUNCTIONPTR>& f)	
	{
		bind(f);
	}
#endif

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
#ifndef GCCSUPORTC11	
		type = typeMember;
		function.memFunction.proc = horrible_cast<MEM_FUNCTION>(f); //what's safty, hei hei
		function.memFunction.obj = shared_ptr<MemFunctionObject>(new MemFunctionObject(horrible_cast<X*>(o)));
#else
		function = std::bind(horrible_cast<MEM_FUNCTION>(f), horrible_cast<X*>(o) FUNCTION_COMMA FUNCTION_STDPLACEHOLDERS);
#endif
	}

	void bind(PTR_FUNCTION f)
	{
		realse();

		if (f == NULL)
		{
			return;
		}

#ifndef GCCSUPORTC11	
		type = typePointer;
		function.ptrFunction = f;
#else
		function = f;
#endif
	}
	template<typename O>
	void bind(R(O::*f)(FUNCTION_TYPES), const weak_ptr<O>& ptr)
	{
		realse();
		shared_ptr<O> ptrtmp = ptr.lock();

		if(f == NULL || ptrtmp == NULL)
		{
			return;
		}
#ifndef GCCSUPORTC11	
		type = typeMember;
		function.memFunction.proc = horrible_cast<MEM_FUNCTION>(f); //what's safty, hei hei
		function.memFunction.obj = shared_ptr<MemFunctionObject>(new MemFunctionObject(ptr));
#else
		function = std::bind(f, ptrtmp FUNCTION_COMMA FUNCTION_STDPLACEHOLDERS);
#endif
	}
#ifdef GCCSUPORTC11
	void bind(const std::function<FUNCTION_STDFUNCTIONPTR>& f) 
	{
		realse();

		function = f;
	}
	operator std::function<FUNCTION_STDFUNCTIONPTR>()
	{
		return function;
	}
	std::function<FUNCTION_STDFUNCTIONPTR>& operator*()
	{
		return function;
	}
#endif
	operator bool() const
	{
#ifndef GCCSUPORTC11
		return (type != typeEmpty
			|| (type != typePointer && function.ptrFunction != NULL));
#else
		return (bool)function;
#endif
	}

	bool operator!() const
	{
		return !(bool(*this));
	}

	/// 重载()运算符，可以以函数对象的形式来调用保存的函数指针。
	inline typename Detail::function_return_type<R>::type operator()(FUNCTION_TYPE_ARGS)
	{
#ifndef GCCSUPORTC11
		if(type == typeMember)
		{
			MemObjectPtr ptr = function.memFunction.obj->getMemObject();
			typedef typename GET_MEM_FUNCTION_INVODER<R>::template Invoker<R FUNCTION_COMMA FUNCTION_TYPES>::type Invoker;
			return Invoker::invoke(ptr.getMemPtr(), function.memFunction.proc FUNCTION_COMMA FUNCTION_ARGS);
		}
		else if(type == typePointer)
		{
			typedef typename GET_FUNCTION_INVODER<R>::template Invoker<R FUNCTION_COMMA FUNCTION_TYPES>::type Invoker;
			return Invoker::invoke((type == typeEmpty) ? NULL : function.ptrFunction FUNCTION_COMMA FUNCTION_ARGS);
		}
#else
		typedef typename GET_STD_FUNCTION_INVODER<R>::template Invoker<R FUNCTION_COMMA FUNCTION_TYPES>::type Invoker;
		return Invoker::invoke(function FUNCTION_COMMA FUNCTION_ARGS);
#endif
	}
	
	void realse()
	{
#ifndef GCCSUPORTC11
		function.ptrFunction = NULL;
		function.memFunction.proc = NULL;
		function.memFunction.obj = NULL;
		type = typeEmpty;
#else
		function = NULL;
#endif
		
	}
	void swap(const FUNCTION_TEMPL& f)
	{
		if (&f == this)
			return;
#ifndef GCCSUPORTC11
		realse();

		type = f.type;
		if(type == typePointer)
		{
			function.ptrFunction = f.function.ptrFunction;
		}
		else if(type == typeMember)
		{
			function.memFunction.proc = f.function.memFunction.proc;
			function.memFunction.obj = f.function.memFunction.obj->clone(); 
		}
#else
		function = f.function;
#endif
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

