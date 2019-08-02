#pragma once
#include "RTSPProtocol.h"
#include "RTSP/RTSPStructs.h"

class RTPPortAlloc
{
public:
	RTPPortAlloc() :udpstartport(40000), udpstopport(41000), nowudpport(udpstartport) {}
	~RTPPortAlloc() {}

	void setUdpPortInfo(uint32_t start, uint32_t stop)
	{
		if (stop == start) stop = start + 1000;

		udpstartport = min(start, stop);
		udpstopport = max(start, stop);
		nowudpport = udpstartport;
	}

	uint32_t allockRTPPort()
	{
		uint32_t udpport = nowudpport;
		uint32_t allocktimes = 0;

		std::set<uint16_t> usedportmap;
		Host::getNowUsedPortMap(usedportmap, Host::SocketType_UDP);

		while (allocktimes < udpstopport - udpstartport)
		{
			if (usedportmap.find(udpport) == usedportmap.end() &&
				usedportmap.find(udpport + 1) == usedportmap.end() )
			{
				nowudpport = udpport + 2;

				return udpport;
			}

			udpport++;
			allocktimes++;
			if (udpport > udpstopport - 2) udpport = udpstartport;
		}

		return udpstartport;
	}
private:
	uint32_t					udpstartport;
	uint32_t					udpstopport;
	uint32_t					nowudpport;
};