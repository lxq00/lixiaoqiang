#include "h265frame.h"

using namespace std;

static const uint8_t start_sequence[] = { 0x00, 0x00, 0x00, 0x01 };

H265Frame::H265Frame ()
{
  m_firstfrag = 0;
  m_lastfrag = 0;
  m_buflen = 0;
  m_buf = new unsigned char [2*1024*1024];
}

H265Frame::~H265Frame ()
{
	delete [] m_buf;
}

int H265Frame::handleHevcFrame(uint16_t seq,uint32_t timestamp,const uint8_t *buf, int len){
	int nal_type = 0;
	unsigned char outbuf[2048]={0};
	int outlen=0;
	nal_type = handleHevcRtpPackage(seq,timestamp,buf,len,outbuf,&outlen);
	if(nal_type < 0){
		return -1;
	}
	if(nal_type == 49){
		//first
		if(m_firstfrag){
			memcpy(m_buf,outbuf,outlen);
			m_buflen = outlen;
			return 0;
		}
		//end
		else if(m_lastfrag){
			memcpy(m_buf+m_buflen,outbuf,outlen);
			m_buflen += outlen;
			return 1;
		}
		//mid
		else{
			memcpy(m_buf+m_buflen,outbuf,outlen);
			m_buflen += outlen;
			return 0;
		}
	}
	memcpy(m_buf,outbuf,outlen);
	m_buflen = outlen;
	return 1;
}

/*
   handle one hevc rtp package
 */
int H265Frame::handleHevcRtpPackage(uint16_t seq,uint32_t timestamp,const uint8_t *buf, int len,uint8_t *outbuf,int *outlen){
	int ret = -1;
	int tid, lid, nal_type;
    int first_fragment, last_fragment, fu_type;
    uint8_t new_nal_header[2];

	//ori
    const uint8_t *rtp_pl = buf;

    /*
     * decode the HEVC payload header according to section 4 of draft version 6:
     *
     *    0                   1
     *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
     *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *   |F|   Type    |  LayerId  | TID |
     *   +-------------+-----------------+
     *
     *      Forbidden zero (F): 1 bit
     *      NAL unit type (Type): 6 bits
     *      NUH layer ID (LayerId): 6 bits
     *      NUH temporal ID plus 1 (TID): 3 bits
     */

    nal_type =  (buf[0] >> 1) & 0x3f;
    lid  = ((buf[0] << 5) & 0x20) | ((buf[1] >> 3) & 0x1f);
    tid  =   buf[1] & 0x07;

    /* sanity check for correct layer ID */
    if (lid) {
        /* future scalable or 3D video coding extensions */
        cout << "Multi-layer HEVC coding\n";
        return ret;
    }

    /* sanity check for correct temporal ID */
    if (!tid) {
        cout << "Illegal temporal ID in RTP/HEVC packet\n";
        return ret;
    }

    /* sanity check for correct NAL unit type */
    if (nal_type > 50) {
        cout << "Unsupported (HEVC) NAL type (" << nal_type << ")\n";
        return ret;
    }

	//process
    switch (nal_type) {
    /* video parameter set (VPS) */
    case 32:
    /* sequence parameter set (SPS) */
    case 33:
    /* picture parameter set (PPS) */
    case 34:
    /*  supplemental enhancement information (SEI) */
    case 39:
    /* single NAL unit packet */
    default:
		//cout << "nal_type:" << nal_type << "\n";
    	ret = nal_type;
    	/* the outlen */
		*outlen = sizeof(start_sequence) + len;
        /* A/V packet: copy start sequence */
        memcpy(outbuf, start_sequence, sizeof(start_sequence));
        /* A/V packet: copy NAL unit data */
        memcpy(outbuf+ sizeof(start_sequence), buf, len);
        break;
		
    /* aggregated packet (AP) - with two or more NAL units */
    case 48:
		//cout << "nal_type:" << nal_type << "\n";
        buf += 2;
        len -= 2;
		//handl aggregated p
        if(handleAggregatedPacket(buf, len,outbuf,outlen) == 0){
			ret = nal_type;
		}
        break;
    /* fragmentation unit (FU) */
    case 49:
        /* pass the HEVC payload header (two bytes) */
        buf += 2;
        len -= 2;

        /*
         *    decode the FU header
         *
         *     0 1 2 3 4 5 6 7
         *    +-+-+-+-+-+-+-+-+
         *    |S|E|  FuType   |
         *    +---------------+
         *
         *       Start fragment (S): 1 bit
         *       End fragment (E): 1 bit
         *       FuType: 6 bits
         */
        first_fragment = buf[0] & 0x80;
        last_fragment  = buf[0] & 0x40;
        fu_type        = buf[0] & 0x3f;

		m_firstfrag = first_fragment;
		m_lastfrag = last_fragment;

        /* pass the HEVC FU header (one byte) */
        buf += 1;
        len -= 1;

        //cout << "nal_type:" << nal_type << " FU type:" << fu_type << " first_frag:" << first_fragment << " last_frag:" << last_fragment << " with " << len <<" bytes\n";

        /* sanity check for size of input packet: 1 byte payload at least */
        if (len <= 0) {
			return -1;
        }

        if (first_fragment && last_fragment) {
			return -1;
        }

    	ret = nal_type;

		/*modify nal header*/
        new_nal_header[0] = (rtp_pl[0] & 0x81) | (fu_type << 1);
        new_nal_header[1] = rtp_pl[1];

        handleFragPackage(buf, len, first_fragment,new_nal_header, sizeof(new_nal_header), outbuf,outlen);

        break;
    /* PACI packet */
    case 50:
        /* Temporal scalability control information (TSCI) */
		cout << "****[hevc] unhandled hevc package : " << nal_type << "\n";
        cout << "PACI packets for RTP/HEVC\n";
        break;
    }

	return 	ret;
}

/*
 * process one hevc frag package,add the startcode and nal header only when the package set start_bit 
 */
void H265Frame::handleFragPackage(const uint8_t *buf, int len,int start_bit,const uint8_t *nal_header,int nal_header_len,uint8_t *outbuf,int *outlen){
    int tot_len = len;
    int pos = 0;
    if (start_bit)
        tot_len += sizeof(start_sequence) + nal_header_len;
    if (start_bit) {
        memcpy(outbuf + pos, start_sequence, sizeof(start_sequence));
        pos += sizeof(start_sequence);
        memcpy(outbuf + pos, nal_header, nal_header_len);
        pos += nal_header_len;
    }
    memcpy(outbuf + pos, buf, len);
	*outlen = tot_len;
}

/* aggregated packet (AP) - with two or more NAL units 
 * we add start_sequence_code_header for every NAL unit
 */
int H265Frame::handleAggregatedPacket(const uint8_t *buf, int len,uint8_t *outbuf, int *outlen)
{
    int pass         = 0;
    int total_length = 0;
    uint8_t *dst     = NULL;

    // first we are going to figure out the total size
    for (pass = 0; pass < 2; pass++) {
        const uint8_t *src = buf;
        int src_len        = len;

        while (src_len > 2) {
            uint16_t nal_size = AV_RB16(src);

            // consume the length of the aggregate
            src     += 2;
            src_len -= 2;

            if (nal_size <= src_len) {
                if (pass == 0) {
                    // counting
                    total_length += sizeof(start_sequence) + nal_size;
                } else {
                    // copying
                    memcpy(dst, start_sequence, sizeof(start_sequence));
                    dst += sizeof(start_sequence);
                    memcpy(dst, src, nal_size);
                    dst += nal_size;
                }
            } else {
                cout << "[HEVC] Aggregated error,nal size exceeds length: " << nal_size << " " << src_len << "\n";
                return -1;
            }

            // eat what we handled
            src     += nal_size;
            src_len -= nal_size;
        }

        if (pass == 0) {
            dst = outbuf;
            *outlen = total_length;
        }
    }

    return 0;
}

