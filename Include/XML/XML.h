#ifndef __XMLObject_H__
#define __XMLObject_H__
#include "XML_Defs.h"
#include "Base/Base.h"
namespace Public{
namespace XML{

using namespace Public::Base;

class XML_API XMLObject
{
	struct XMLInternal;
public:
	class Child;
	struct XML_API Attribute
	{
		Attribute();
		Attribute(const Attribute& attri);
		~Attribute();

		std::string		name;
		Value			value;

		bool isEmpty() const;
		bool operator == (const Attribute& val) const;
		operator bool() const;
	};
	class XML_API Child
	{
		friend class XMLObject;
		struct ChildInternal;
	public:
		Child(const std::string& name = "");
		Child(const Child& child);
		~Child();

		void name(const std::string& name);
		std::string name() const;

		void data(const Value& value);
		Value data() const;
		operator Value() const;

		Child& addChild(const XMLObject::Child& child);
		Child& getChild(const std::string& name,int index = 0);
		const Child& getChild(const std::string& name, int index = 0) const;
		void removeChild(const std::string& name,int index = 0);

		int childCount() const;
		int attributeCount() const;

		void attribute(const std::string& key,const Value& val);
		Value& attribute(const std::string& key);
		const Value& attribute(const std::string& key) const;
		void removeAttribute(const std::string& key);

		Child& firstChild(const std::string& childname="");
		const Child& firstChild(const std::string& childname = "")const;

		Child& nextChild();
		const Child& nextChild()const;

		Attribute& firstAttribute();
		const Attribute& firstAttribute()const;

		Attribute& nextAttribute();
		const Attribute& nextAttribute()const;

		bool isEmpty() const;
		void clear();

		Child& operator = (const Child& child);
		bool operator == (const Child& child) const;

		operator bool() const;
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
	const Child& getRoot()const;

	std::string getRootName() const;
	void setRootName(const std::string& name);

	std::string toString(Encoding encode = Encoding_Unknown)const;

	bool saveAs(const std::string& file, Encoding encode = Encoding_Unknown) const;

	bool isEmpty() const;
	void clear();
private:
	XMLInternal* internal;
};


}
}

#endif //__XMLObject_H__
