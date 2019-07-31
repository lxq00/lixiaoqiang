#pragma once
#include "rtp.h"
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

		while (allocktimes < udpstopport - udpstartport)
		{
			if (Host::checkPortIsNotUsed(udpport, Host::SocketType_UDP)
				&& Host::checkPortIsNotUsed(udpport + 1, Host::SocketType_UDP)
				&& Host::checkPortIsNotUsed(udpport + 2, Host::SocketType_UDP)
				&& Host::checkPortIsNotUsed(udpport + 3, Host::SocketType_UDP))
			{
				nowudpport = udpport + 4;

				return udpport;
			}

			udpport++;
			allocktimes++;
			if (udpport > udpstopport - 4) udpport = udpstartport;
		}

		return udpstartport;
	}
private:
	uint32_t					udpstartport;
	uint32_t					udpstopport;
	uint32_t					nowudpport;
};