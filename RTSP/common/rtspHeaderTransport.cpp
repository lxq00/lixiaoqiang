#include "rtspHeaderTransport.h"

// RFC 2326 Real Time Streaming Protocol (RTSP)
// 12.39 Transport (p58)
//
// Transport = "Transport" ":" 1#transport-spec
// transport-spec = transport-protocol/profile[/lower-transport] *parameter
// transport-protocol = "RTP"
// profile = "AVP"
// lower-transport = "TCP" | "UDP"
// parameter = ( "unicast" | "multicast" )
//				| ";" "destination" [ "=" address ]
//				| ";" "interleaved" "=" channel [ "-" channel ]
//				| ";" "append"
//				| ";" "ttl" "=" ttl
//				| ";" "layers" "=" 1*DIGIT
//				| ";" "port" "=" port [ "-" port ]
//				| ";" "client_port" "=" port [ "-" port ]
//				| ";" "server_port" "=" port [ "-" port ]
//				| ";" "ssrc" "=" ssrc
//				| ";" "mode" = <"> 1\#mode <">
// ttl = 1*3(DIGIT)
// port = 1*5(DIGIT)
// ssrc = 8*8(HEX)
// channel = 1*3(DIGIT)
// address = host
// mode = <"> *Method <"> | Method
//
// Transport: RTP/AVP;unicast;client_port=4588-4589;server_port=6256-6257
// Transport: RTP/AVP;multicast;ttl=127;mode="PLAY",RTP/AVP;unicast;client_port=3456-3457;mode="PLAY"

// RTP Port define
// RFC 3550: 11. RTP over Network and Transport Protocols (p56)
// 1. For UDP and similar protocols, RTP should use an even destination port number and
//    the corresponding RTCP stream should use the next higher (odd) destination port number. 
// 2. For applications that take a single port number as a parameter and derive the RTP and RTCP port
//    pair from that number, if an odd number is supplied then the application should replace that
//    number with the next lower (even) number to use as the base of the port pair.


#define TRANSPORT_SPECIAL ",;\r\n"

std::string rtsp_header_build_transport(const TRANSPORT_INFO& transport)
{
	std::string transportstr;
	if (transport.transport == TRANSPORT_INFO::TRANSPORT_NONE)
	{

	}
	else if (transport.transport == TRANSPORT_INFO::TRANSPORT_RTP_TCP)
	{
		transportstr = "RTP/AVP/TCP;unicast";
		transportstr += ";interleaved="+Value(transport.rtp.t.dataChannel).readString() + "-" + Value(transport.rtp.t.contorlChannel).readString();
	}
	else if (transport.transport == TRANSPORT_INFO::TRANSPORT_RTP_UDP)
	{
		transportstr = "RTP/AVP;unicast";
		transportstr += ";client_port="+Value(transport.rtp.u.client_port1).readString()+"-"+ Value(transport.rtp.u.client_port2).readString();
		if (transport.rtp.u.server_port1 != 0)
		{
			transportstr += ";server_port=" + Value(transport.rtp.u.server_port1).readString() + "-" + Value(transport.rtp.u.server_port2).readString();
		}
	}
	if (transport.ssrc != 0)
	{
		transportstr += ";ssrc=" + Value(transport.ssrc).readString();
	}

	return transportstr;
}

 int rtsp_header_parse_transport(const char* field, TRANSPORT_INFO* t)
{
	const char* p1;
	const char* p = field;
	size_t n;

	memset(t, 0, sizeof(*t));
//	t->multicast = TRANSPORT_INFO::MULTICAST_UNICAST; // default unicast
	t->transport = TRANSPORT_INFO::TRANSPORT_RTP_UDP;

	while (p && *p)
	{
		p1 = strpbrk(p, TRANSPORT_SPECIAL);
		n = p1 ? (size_t)(p1 - p) : strlen(p); // ptrdiff_t -> size_t

		switch (*p)
		{
		case 'r':
		case 'R':
			if (11 == n && 0 == strncasecmp("RTP/AVP/UDP", p, 11))
			{
				t->transport = TRANSPORT_INFO::TRANSPORT_RTP_UDP;
			}
			else if (11 == n && 0 == strncasecmp("RTP/AVP/TCP", p, 11))
			{
				t->transport = TRANSPORT_INFO::TRANSPORT_RTP_TCP;
			}
			else if (11 == n && 0 == strncasecmp("RAW/RAW/UDP", p, 11))
			{
				t->transport = TRANSPORT_INFO::TRANSPORT_RAW;
			}
			else if (7 == n && 0 == strncasecmp("RTP/AVP", p, 7))
			{
				t->transport = TRANSPORT_INFO::TRANSPORT_RTP_UDP;
			}
			break;

		case 'u':
		case 'U':
			if (7 == n && 0 == strncasecmp("unicast", p, 7))
			{
			//	t->multicast = TRANSPORT_INFO::MULTICAST_UNICAST;
			}
			break;

		case 'm':
		case 'M':
			if (9 == n && 0 == strncasecmp("multicast", p, 9))
			{
			//	t->multicast = TRANSPORT_INFO::MULTICAST_MULTICAST;
			}
			else if (n > 5 && 0 == strncasecmp("mode=", p, 5))
			{
				/*if ((11 == n && 0 == strcasecmp("\"PLAY\"", p + 5)) || (9 == n && 0 == strcasecmp("PLAY", p + 5)))
					t->mode = TRANSPORT_INFO::MODE_PLAY;
				else if ((13 == n && 0 == strcasecmp("\"RECORD\"", p + 5)) || (11 == n && 0 == strcasecmp("RECORD", p + 5)))
					t->mode = TRANSPORT_INFO::MODE_RECORD;*/
			}
			break;

		case 'd':
		case 'D':
			if (n >= 12 && 0 == strncasecmp("destination=", p, 12))
			{
				//t->destination = std::string(p + 12, n - 12);
			}
			break;

		case 's':
		case 'S':
			if (n >= 7 && 0 == strncasecmp("source=", p, 7))
			{
				//t->source = std::string(p + 7, n - 7);
			}
			else if (13 == n && 0 == strncasecmp("ssrc=", p, 5))
			{
				// unicast only
		//		assert(0 == t->multicast);
				t->ssrc = (int)strtol(p + 5, NULL, 16);
			}
			else if (2 == sscanf(p, "server_port=%hu-%hu", &t->rtp.u.server_port1, &t->rtp.u.server_port2))
			{
		//		assert(0 == t->multicast);
			}
			else if (1 == sscanf(p, "server_port=%hu", &t->rtp.u.server_port1))
			{
		//		assert(0 == t->multicast);
				t->rtp.u.server_port1 = t->rtp.u.server_port1 / 2 * 2; // RFC 3550 (p56)
				t->rtp.u.server_port2 = t->rtp.u.server_port1 + 1;
			}
			break;

		case 'a':
			if (6 == n && 0 == strcasecmp("append", p))
			{
				//t->append = 1;
			}
			break;

		case 'p':
	//		if (2 == sscanf(p, "port=%hu-%hu", &t->rtp.m.port1, &t->rtp.m.port2))
	//		{
	////			assert(1 == t->multicast);
	//		}
	//		else if (1 == sscanf(p, "port=%hu", &t->rtp.m.port1))
	//		{
	////			assert(1 == t->multicast);
	//			t->rtp.m.port1 = t->rtp.m.port1 / 2 * 2; // RFC 3550 (p56)
	//			t->rtp.m.port2 = t->rtp.m.port1 + 1;
	//		}
			break;

		case 'c':
			if (2 == sscanf(p, "client_port=%hu-%hu", &t->rtp.u.client_port1, &t->rtp.u.client_port2))
			{
				//assert(0 == t->multicast);
			}
			else if (1 == sscanf(p, "client_port=%hu", &t->rtp.u.client_port1))
			{
				//assert(0 == t->multicast);
				t->rtp.u.client_port1 = t->rtp.u.client_port1 / 2 * 2; // RFC 3550 (p56)
				t->rtp.u.client_port2 = t->rtp.u.client_port1 + 1;
			}
			break;

		case 'i':
			if (2 == sscanf(p, "interleaved=%d-%d", &t->rtp.t.dataChannel, &t->rtp.t.contorlChannel))
			{
			}
			else if (1 == sscanf(p, "interleaved=%d", &t->rtp.t.dataChannel))
			{
				t->rtp.t.contorlChannel = t->rtp.t.dataChannel + 1;
			}
			break;

		case 't':
			//if (1 == sscanf(p, "ttl=%d", &t->rtp.m.ttl))
			//{
			//	//assert(1 == t->multicast);
			//}
			break;

		case 'l':
			/*if (1 == sscanf(p, "layers=%d", &t->layer))
			{
			}*/
			break;
		}

		if (NULL == p1 || '\r' == *p1 || '\n' == *p1 || '\0' == *p1 || ',' == *p1)
			break;
		p = p1 + 1;
	}

	return 0;
}

