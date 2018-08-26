#ifndef __XMLObject_H__
#define __XMLObject_H__
#include "XML_Defs.h"
#include <string>
#include <list>
using namespace std;
namespace Public{
namespace XML{

class XML_API XMLObject
{
	struct XMLInternal;
public:
	class XML_API Value
	{
		friend class XMLObject;
		struct ValueInternal;
	public:
		Value();
		Value(const char* val);
		Value(const std::string& val);
		Value(int val);
		Value(unsigned int val);
		Value(long long val);
		Value(unsigned long long val);
		Value(float val);
		Value(char val);
		Value(double val);
		Value(const Value& val);
		~Value();

		std::string toString() const;
		int toInt32() const;
		unsigned int toUint32() const;
		unsigned long long toUint64() const;
		long long toInt64() const;
		float toFloat() const;
		char toChar() const;
		double toDouble() const;

		bool isEmpty() const;

		Value& operator = (const Value& val);
		bool operator == (const Value& val) const;
	private:
		ValueInternal* internal;
	};
	struct XML_API Attribute
	{
		Attribute();
		Attribute(const Attribute& attri);
		~Attribute();

		std::string		name;
		XMLObject::Value		value;

		bool isEmpty();
		bool operator == (const Attribute& val) const;
	};
	class XML_API Child
	{
		friend class XMLObject;
		struct ChildInternal;
	public:
		Child(const char* name = NULL);
		Child(const std::string& name);
		Child(const Child& child);
		~Child();

		void setName(const std::string& name);
		std::string getName();

		void setValue(const Value& value);
		Value getValue();

		Child& addChild(const Child& child);
		Child& getChild(const std::string& name,int index = 0);
		void removeChild(const std::string& name,int index = 0);

		int childCount() const;
		int attributeCount() const;

		void setAttribute(const std::string& key,const Value& val);
		Value& getAttribute(const std::string& key);
		void removeAttribute(const std::string& key);

		Child& firstChild();
		Child& nextChild();

		Attribute& firstAttribute();
		Attribute& nextAttribute();

		bool isEmpty() const;
		void clear();

		Child& operator = (const Child& child);
		bool operator == (const Child& child) const;
	private:
		ChildInternal* internal;
	};
public:
	enum Encoding
	{
		Encoding_Unknown,
		Encoding_UTF8,
		Encoding_GBK,
	};
public:
	XMLObject();
	~XMLObject();

	bool parseFile(const std::string& file);
	bool parseBuffer(const std::string& buf);

	Child& setRoot(const Child& root);
	Child& getRoot();

	std::string getRootName() const;
	void setRootName(const std::string& name);

	std::string toString(Encoding encode = Encoding_Unknown);
	bool saveAs(const std::string& file,Encoding encode = Encoding_Unknown);

	bool isEmpty() const;
	void clear();
private:
	XMLInternal* internal;
};


}
}

#endif //__XMLObject_H__
