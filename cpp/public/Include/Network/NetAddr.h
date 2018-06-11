//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//  Description:
//  $Id: NetAddr.h 17 2013-01-22 09:26:19Z   $
//

#ifndef __NET_NETADDR_H__
#define __NET_NETADDR_H__
#include <string>
#include "Defs.h"
#include "Base/IntTypes.h"

#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

using namespace Public::Base;

namespace Public{
namespace Network{

typedef  sockaddr 		SockAddr;
typedef  sockaddr_in 	SockAddrIPv4;
#ifdef SUPPORT_IPV6
typedef  struct sockaddr_in6	SockAddrIPv6;
#endif
class NETWORK_API NetAddr{
	struct NetAddrInternal;
public:
	NetAddr();
	NetAddr(const NetAddr& addr);
	NetAddr(const std::string& addr,int port);
	NetAddr(uint16_t port );
	NetAddr(const SockAddrIPv4& ipv4);
	NetAddr(const SockAddr& addr);

	~NetAddr();
	/// 获得实现类的SockAddr指针
	/// \param addr [out]  指针
	/// \param len  [in]  addr的长度
	/// \retval !=NULL 有效的地址
	/// \retval ==NULL 无效地址
	SockAddr * getAddr() const;

	SockAddrIPv4* getAddrV4() const;

	/// 获得实现类的SockAddr地址的长度
	/// \retval  >  0 地址长度
	/// \reval   == 0 地址无效
	uint32_t getAddrLen() const;

	/// 获得点分十进制地址字符串 或者 路径名称(UnixSocket时)
	/// \param  buf [out]    字符串地址
	/// \param  size [in]     buf字符串的长度
	/// \retval  != NULL 返回字符串的首地址
	/// \reval   == NULL 失败
	std::string getIP() const;

	
	/// 获得端口
	/// \reval > 0 有效端口号
	/// \reval = 0 无效端口号
	uint16_t getPort() const;
	
	
	/// 网络地址的类型
	enum NetAddrType{ 
		netaddr_unknown = 0,   //无效地址类型 
		netaddr_ipv4,      //IPv4地址类型
		netaddr_ipv6,      //IPv6地址类型
	};
		
	/// 获得网络地址的类型
	NetAddrType getType() const;

	/// 设置IP地址
	/// \param addr [in] 点分十进制IPv4字符串 或者 域名
	/// \param port [in]  端口号	
	/// \reval  >=0 成功
	/// \reval  < 0 失败
	int setAddr( const std::string& addr, uint16_t port);

	/// 设置IP地址
	/// \param [in] addr SockAddrIPv4
	/// \reval  >=0 成功
	/// \reval  < 0 失败	
	int setAddr( const SockAddrIPv4 & addr );

	/// 设置IP地址
	/// \param [in] addr SockAddrIPv4
	/// \reval  >=0 成功
	/// \reval  < 0 失败	
	int setAddr( const SockAddr & addr );

	/// 只设置端口
	/// \param port [in]  端口
	/// \reval  >=0 成功
	/// \reval  < 0 失败	
	int setPort( uint16_t port );
	
	// 当前的网络地址是否为有效的
	bool isValid() const;

#ifdef SUPPORT_IPV6
	NetAddr(const SockAddrIPv6& ipv6);
	SockAddrIPv6* getAddrV6() const;
	int setAddr( const SockAddrIPv6 & addr );
	NetAddr& operator=( NetAddrIPv6 const & other );
#endif	

	/// 赋值运算
	NetAddr& operator=( NetAddr const & other );

	/// 关系运算:等于
	bool operator==( NetAddr & other) const;
	/// 关系运算:等于
private:
	NetAddrInternal* internal;
};

} // namespace Net
} // namespace Public

#endif// __NET_NETFRAME_H__



