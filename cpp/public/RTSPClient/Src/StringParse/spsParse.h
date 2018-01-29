#ifndef _SDP_H__
#define _SDP_H__

#include "math.h"
#include "stdio.h"
#include <string.h>

#define MAXIMUMVALUEOFcpb_cnt   32
#define MAXnum_ref_frames_in_pic_order_cnt_cycle  256

#define SE_HEADER 0
#define FREXT_HP        100      //!< YUV 4:2:0/8 "High"
#define FREXT_Hi10P     110      //!< YUV 4:2:0/10 "High 10"
#define FREXT_Hi422     122      //!< YUV 4:2:2/10 "High 4:2:2"
#define FREXT_Hi444     144      //!< YUV 4:4:4/12 "High 4:4:4"

/*****************************************************/

//供调用的接口 int DecodeSps(unsigned char* pSpsInfo, int nSize,int& width,int& height)

/*****************************************************/

typedef struct
{
	unsigned  cpb_cnt;                                          // ue(v)
	unsigned  bit_rate_scale;                                   // u(4)
	unsigned  cpb_size_scale;                                   // u(4)
	unsigned  bit_rate_value [MAXIMUMVALUEOFcpb_cnt];         // ue(v)
	unsigned  cpb_size_value[MAXIMUMVALUEOFcpb_cnt];          // ue(v)
	unsigned  vbr_cbr_flag[MAXIMUMVALUEOFcpb_cnt];            // u(1)
	unsigned  initial_cpb_removal_delay_length_minus1;          // u(5)
	unsigned  cpb_removal_delay_length_minus1;                  // u(5)
	unsigned  dpb_output_delay_length_minus1;                   // u(5)
	unsigned  time_offset_length;                               // u(5)
} hrd_parameters_t;

typedef struct
{
	int      aspect_ratio_info_present_flag;                   // u(1)
	unsigned  aspect_ratio_idc;                               // u(8)
	unsigned  sar_width;                                    // u(16)
	unsigned  sar_height;                                   // u(16)
	int      overscan_info_present_flag;                       // u(1)
	int      overscan_appropriate_flag;                      // u(1)
	int      video_signal_type_present_flag;                   // u(1)
	unsigned  video_format;                                   // u(3)
	int      video_full_range_flag;                          // u(1)
	int      colour_description_present_flag;                // u(1)
	unsigned  colour_primaries;                             // u(8)
	unsigned  transfer_characteristics;                     // u(8)
	unsigned  matrix_coefficients;                          // u(8)
	int      chroma_location_info_present_flag;                // u(1)
	unsigned   chroma_sample_loc_type_top_field;               // ue(v)
	unsigned   chroma_sample_loc_type_bottom_field;            // ue(v)
	int      timing_info_present_flag;                         // u(1)
	unsigned  num_units_in_tick;                              // u(32)
	unsigned  time_scale;                                     // u(32)
	int      fixed_frame_rate_flag;                          // u(1)
	int      nal_hrd_parameters_present_flag;                  // u(1)
	hrd_parameters_t nal_hrd_parameters;                      // hrd_paramters_t
	int      vcl_hrd_parameters_present_flag;                  // u(1)
	hrd_parameters_t vcl_hrd_parameters;                      // hrd_paramters_t
	// if ((nal_hrd_parameters_present_flag || (vcl_hrd_parameters_present_flag))
	int      low_delay_hrd_flag;                             // u(1)
	int      pic_struct_present_flag;                        // u(1)
	int      bitstream_restriction_flag;                       // u(1)
	int      motion_vectors_over_pic_boundaries_flag;        // u(1)
	unsigned  max_bytes_per_pic_denom;                        // ue(v)
	unsigned  max_bits_per_mb_denom;                          // ue(v)
	unsigned  log2_max_mv_length_vertical;                    // ue(v)
	unsigned  log2_max_mv_length_horizontal;                  // ue(v)
	unsigned  num_reorder_frames;                             // ue(v)
	unsigned  max_dec_frame_buffering;                        // ue(v)
} vui_seq_parameters_t;


typedef struct
{
	int   Valid;                  // indicates the parameter set is valid

	unsigned  profile_idc;                                      // u(8)
	int   constrained_set0_flag;                            // u(1)
	int   constrained_set1_flag;                            // u(1)
	int   constrained_set2_flag;                            // u(1)
	int   constrained_set3_flag;                            // u(1)
	unsigned  level_idc;                                        // u(8)
	unsigned  seq_parameter_set_id;                             // ue(v)
	unsigned  chroma_format_idc;                                // ue(v)

	int  seq_scaling_matrix_present_flag;                   // u(1)
	int      seq_scaling_list_present_flag[8];                  // u(1)
	int      ScalingList4x4[6][16];                             // se(v)
	int      ScalingList8x8[2][64];                             // se(v)
	int  UseDefaultScalingMatrix4x4Flag[6];
	int  UseDefaultScalingMatrix8x8Flag[2];

	unsigned  bit_depth_luma_minus8;                            // ue(v)
	unsigned  bit_depth_chroma_minus8;                          // ue(v)

	unsigned  log2_max_frame_num_minus4;                        // ue(v)
	unsigned pic_order_cnt_type;
	// if( pic_order_cnt_type == 0 ) 
	unsigned log2_max_pic_order_cnt_lsb_minus4;                 // ue(v)
	// else if( pic_order_cnt_type == 1 )
	int delta_pic_order_always_zero_flag;               // u(1)
	int     offset_for_non_ref_pic;                         // se(v)
	int     offset_for_top_to_bottom_field;                 // se(v)
	unsigned  num_ref_frames_in_pic_order_cnt_cycle;          // ue(v)
	// for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
	int   offset_for_ref_frame[MAXnum_ref_frames_in_pic_order_cnt_cycle];   // se(v)
	unsigned  num_ref_frames;                                   // ue(v)
	int   gaps_in_frame_num_value_allowed_flag;             // u(1)
	unsigned  pic_width_in_mbs_minus1;                          // ue(v)
	unsigned  pic_height_in_map_units_minus1;                   // ue(v)
	int   frame_mbs_only_flag;                              // u(1)
	// if( !frame_mbs_only_flag ) 
	int   mb_adaptive_frame_field_flag;                   // u(1)
	int   direct_8x8_inference_flag;                        // u(1)
	int   frame_cropping_flag;                              // u(1)
	unsigned  frame_cropping_rect_left_offset;                // ue(v)
	unsigned  frame_cropping_rect_right_offset;               // ue(v)
	unsigned  frame_cropping_rect_top_offset;                 // ue(v)
	unsigned  frame_cropping_rect_bottom_offset;              // ue(v)
	int   vui_parameters_present_flag;                      // u(1)
	vui_seq_parameters_t vui_seq_parameters;                  // vui_seq_parameters_t
} seq_parameter_set_rbsp_t;


typedef struct
{
	// CABAC Decoding
	int           read_len;           //!< actual position in the codebuffer, CABAC only
	int           code_len;           //!< overall codebuffer length, CABAC only
	// UVLC Decoding
	int           frame_bitoffset;    //!< actual position in the codebuffer, bit-oriented, UVLC only
	int           bitstream_length;   //!< over codebuffer lnegth, byte oriented, UVLC only
	// ErrorConcealment
	unsigned char   *streamBuffer;      //!< actual codebuffer for read bytes
	int           ei_flag;            //!< error indication, 0: no error, else unspecified error
} Bitstream;


typedef struct
{
	unsigned int    Dlow, Drange;
	unsigned int    Dvalue;
	unsigned int    Dbuffer;
	int             Dbits_to_go;
	unsigned char   *Dcodestrm;
	int             *Dcodestrm_len;
} DecodingEnvironment;


typedef DecodingEnvironment *DecodingEnvironmentPtr;

typedef struct syntaxelement
{
	int           type;                  //!< type of syntax element for data part.
	int           value1;                //!< numerical value of syntax element
	int           value2;                //!< for blocked symbols, e.g. run/level
	int           len;                   //!< length of code
	int           inf;                   //!< info part of UVLC code
	unsigned int  bitpattern;            //!< UVLC bitpattern
	int           context;               //!< CABAC context
	int           k;                     //!< CABAC context for coeff_count,uv

#if TRACE
#define       TRACESTRING_SIZE 100           //!< size of trace string
	char          tracestring[TRACESTRING_SIZE]; //!< trace string
#endif

	//! for mapping of UVLC to syntaxElement
	void    (*mapping)(int len, int info, int *value1, int *value2);
	//! used for CABAC: refers to actual coding method of each individual syntax element type
	void  (*reading)(struct syntaxelement *, struct inp_par *, struct img_par *, DecodingEnvironmentPtr);

} SyntaxElement;


static void linfo_ue(int len, int info, int *value1, int *dummy)
{
	*value1 = (int)pow((double)2,(double)(len/2))+info-1; // *value1 = (int)(2<<(len>>1))+info-1;
}


static int GetVLCSymbol (unsigned char buffer[],int totbitoffset,int *info, int bytecount)
{   
	register int inf;
	long byteoffset;      // byte from start of buffer
	int bitoffset;      // bit from start of byte
	int ctr_bit=0;      // control bit for current bit posision
	int bitcounter=1;
	int len;
	int info_bit;

	byteoffset= totbitoffset/8;
	bitoffset= 7-(totbitoffset%8);
	ctr_bit = (buffer[byteoffset] & (0x01<<bitoffset));   // set up control bit

	len=1;
	while (ctr_bit==0)
	{                 // find leading 1 bit
		len++;
		bitoffset-=1;           
		bitcounter++;
		if (bitoffset<0)
		{                 // finish with current byte ?
			bitoffset=bitoffset+8;
			byteoffset++;
		}
		ctr_bit=buffer[byteoffset] & (0x01<<(bitoffset));
	}
	// make infoword
	inf=0;                          // shortest possible code is 1, then info is always 0
	for(info_bit=0;(info_bit<(len-1)); info_bit++)
	{
		bitcounter++;
		bitoffset-=1;
		if (bitoffset<0)
		{                 // finished with current byte ?
			bitoffset=bitoffset+8;
			byteoffset++;
		}
		if (byteoffset > bytecount)
		{
			return -1;
		}
		inf=(inf<<1);
		if(buffer[byteoffset] & (0x01<<(bitoffset)))
			inf |=1;
	}

	*info = inf;
	return bitcounter;           // return absolute offset in bit from start of frame
}


static int GetBits (unsigned char buffer[],int totbitoffset,int *info, int bytecount, 
					int numbits)
{

	register int inf;
	long byteoffset;      // byte from start of buffer
	int bitoffset;      // bit from start of byte

	int bitcounter=numbits;

	byteoffset= totbitoffset/8;
	bitoffset= 7-(totbitoffset%8);

	inf=0;
	while (numbits)
	{
		inf <<=1;
		inf |= (buffer[byteoffset] & (0x01<<bitoffset))>>bitoffset;
		numbits--;
		bitoffset--;
		if (bitoffset < 0)
		{
			byteoffset++;
			bitoffset += 8;
			if (byteoffset > bytecount)
			{
				return -1;
			}
		}
	}

	*info = inf;
	return bitcounter;           // return absolute offset in bit from start of frame
}  


static int readSyntaxElement_FLC(SyntaxElement *sym, Bitstream *currStream)
{
	int frame_bitoffset = currStream->frame_bitoffset;
	unsigned char *buf = currStream->streamBuffer;
	int BitstreamLengthInBytes = currStream->bitstream_length;

	if ((GetBits(buf, frame_bitoffset, &(sym->inf), BitstreamLengthInBytes, sym->len)) < 0)
		return -1;

	currStream->frame_bitoffset += sym->len; // move bitstream pointer
	sym->value1 = sym->inf;

	return 1;
}


static int u_v (int LenInBits, char*tracestring, Bitstream *bitstream)
{
	SyntaxElement symbol = {0}, *sym = &symbol;

	sym->type = SE_HEADER;
	sym->mapping = linfo_ue;   // Mapping rule
	sym->len = LenInBits;
	readSyntaxElement_FLC(sym, bitstream);
	return sym->inf;
};


static int readSyntaxElement_VLC(SyntaxElement *sym, Bitstream *currStream)
{
	int frame_bitoffset = currStream->frame_bitoffset;
	unsigned char *buf = currStream->streamBuffer;
	int BitstreamLengthInBytes = currStream->bitstream_length;

	sym->len =  GetVLCSymbol (buf, frame_bitoffset, &(sym->inf), BitstreamLengthInBytes);
	if (sym->len == -1)
		return -1;
	currStream->frame_bitoffset += sym->len;
	sym->mapping(sym->len,sym->inf,&(sym->value1),&(sym->value2));

	return 1;
}


static int u_1 (char *tracestring, Bitstream *bitstream)
{
	return u_v (1, tracestring, bitstream);
}


static int ue_v (char *tracestring, Bitstream *bitstream)
{
	SyntaxElement symbol, *sym=&symbol;

	sym->type = SE_HEADER;
	sym->mapping = linfo_ue;   // Mapping rule
	readSyntaxElement_VLC (sym, bitstream);
	return sym->value1;
}


static void linfo_se(int len,  int info, int *value1, int *dummy)
{
	int n;
	n = (int)pow((double)2,(double)(len/2))+info-1;
	*value1 = (n+1)/2;
	if((n & 0x01)==0)                           // lsb is signed bit
		*value1 = -*value1;
}


static int se_v (char *tracestring, Bitstream *bitstream)
{
	SyntaxElement symbol, *sym=&symbol;

	sym->type = SE_HEADER;
	sym->mapping = linfo_se;   // Mapping rule: signed integer
	readSyntaxElement_VLC (sym, bitstream);
	return sym->value1;
}

static const unsigned char ZZ_SCAN[16] = {  0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15 };
static const unsigned char ZZ_SCAN8[64] =      {  0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63};

static void Scaling_List(int *scalingList, int sizeOfScalingList, int *UseDefaultScalingMatrix, Bitstream *s)
{
	int j, scanj;
	int delta_scale, lastScale, nextScale;

	lastScale      = 8;
	nextScale      = 8;

	for(j=0; j<sizeOfScalingList; j++)
	{
		scanj = (sizeOfScalingList==16) ? ZZ_SCAN[j] : ZZ_SCAN8[j];

		if(nextScale!=0)
		{
			delta_scale = se_v((char*)"   : delta_sl   ", s);
			nextScale = (lastScale + delta_scale + 256) % 256;
			*UseDefaultScalingMatrix = (scanj==0 && nextScale==0);
		}

		scalingList[scanj] = (nextScale==0) ? lastScale:nextScale;
		lastScale = scalingList[scanj];
	}
}


static int SPS_DEOCDE(unsigned char* pBuffer,int length,seq_parameter_set_rbsp_t* sps)
 {
        if(pBuffer == NULL) return -1;
        if(length <= 0)		return -2;
        if(sps == NULL)	return -3;
        
        Bitstream* s = new Bitstream;
        s->bitstream_length = length;
        s->code_len = 0;
        s->ei_flag = 0;
        s->frame_bitoffset = 0;
        s->read_len = 0;
        s->streamBuffer = pBuffer;
        
        sps->profile_idc = u_v(8,(char*)"SPS: profile_idc",s);

        sps->constrained_set0_flag                  = u_1  ((char*)"SPS: constrained_set0_flag", s);
        sps->constrained_set1_flag                  = u_1  ((char*)"SPS: constrained_set1_flag", s);
        sps->constrained_set2_flag                  = u_1  ((char*)"SPS: constrained_set2_flag", s);
        sps->constrained_set3_flag                  = u_1  ((char*)"SPS: constrained_set3_flag", s);
        /*int reserved_zero                         = */u_v  (4, (char*)"SPS: reserved_zero_4bits", s);
        sps->level_idc                              = u_v  (8, (char*)"SPS: level_idc", s);
        
        sps->seq_parameter_set_id                   = ue_v((char*)"SPS: seq_parameter_set_id", s);
        
        // Fidelity Range Extensions stuff
        sps->chroma_format_idc = 1;
        sps->bit_depth_luma_minus8   = 0;
        sps->bit_depth_chroma_minus8 = 0;
        
        
        if((sps->profile_idc==FREXT_HP   ) ||
            (sps->profile_idc==FREXT_Hi10P) ||
            (sps->profile_idc==FREXT_Hi422) ||
            (sps->profile_idc==FREXT_Hi444))
        {
            sps->chroma_format_idc                      = ue_v ((char*)"SPS: chroma_format_idc", s);
            
            // Residue Color Transform
            if(sps->chroma_format_idc == 3)
            {
                //img->residue_transform_flag = u_1  ("SPS: residue_transform_flag"            , s);
                /*int residue_transform_flag = */u_1  ((char*)"SPS: residue_transform_flag", s);
            }
            
            sps->bit_depth_luma_minus8                  = ue_v ((char*)"SPS: bit_depth_luma_minus8", s);
            sps->bit_depth_chroma_minus8                = ue_v ((char*)"SPS: bit_depth_chroma_minus8", s);
            
            /*int lossless_qpprime_flag                  =*/ u_1  ((char*)"SPS: lossless_qpprime_y_zero_flag", s);
            
            sps->seq_scaling_matrix_present_flag        = u_1  ((char*)"SPS: seq_scaling_matrix_present_flag", s);
            
            if(sps->seq_scaling_matrix_present_flag)
            {
                for(int i=0; i<8; i++)
                {
                    sps->seq_scaling_list_present_flag[i]   = u_1  ((char*)"SPS: seq_scaling_list_present_flag", s);
                    if(sps->seq_scaling_list_present_flag[i])
                    {
                        if(i<6)
                            Scaling_List(sps->ScalingList4x4[i], 16, &sps->UseDefaultScalingMatrix4x4Flag[i], s);
                        else
                            Scaling_List(sps->ScalingList8x8[i-6], 64, &sps->UseDefaultScalingMatrix8x8Flag[i-6], s);
                    }
                }
            }
        }
        
        
        sps->log2_max_frame_num_minus4              = ue_v ((char*)"SPS: log2_max_frame_num_minus4", s);
        sps->pic_order_cnt_type                     = ue_v ((char*)"SPS: pic_order_cnt_type", s);
        
        if (sps->pic_order_cnt_type == 0)
            sps->log2_max_pic_order_cnt_lsb_minus4 = ue_v ((char*)"SPS: log2_max_pic_order_cnt_lsb_minus4", s);
        else if (sps->pic_order_cnt_type == 1)
        {
  
        }
        
        sps->num_ref_frames                        = ue_v ((char*)"SPS: num_ref_frames", s);
        sps->gaps_in_frame_num_value_allowed_flag  = u_1  ((char*)"SPS: gaps_in_frame_num_value_allowed_flag", s);
        sps->pic_width_in_mbs_minus1               = ue_v ((char*)"SPS: pic_width_in_mbs_minus1", s);
        sps->pic_height_in_map_units_minus1        = ue_v ((char*)"SPS: pic_height_in_map_units_minus1", s);
        sps->frame_mbs_only_flag                   = u_1  ((char*)"SPS: frame_mbs_only_flag", s);
        if (!sps->frame_mbs_only_flag)
        {
            sps->mb_adaptive_frame_field_flag      = u_1  ((char*)"SPS: mb_adaptive_frame_field_flag", s);
        }
        sps->direct_8x8_inference_flag             = u_1  ((char*)"SPS: direct_8x8_inference_flag", s);
        sps->frame_cropping_flag                   = u_1  ((char*)"SPS: frame_cropping_flag", s);
        
        if (sps->frame_cropping_flag)
        {
            sps->frame_cropping_rect_left_offset   = ue_v ((char*)"SPS: frame_cropping_rect_left_offset", s);
            sps->frame_cropping_rect_right_offset  = ue_v ((char*)"SPS: frame_cropping_rect_right_offset", s);
            sps->frame_cropping_rect_top_offset    = ue_v ((char*)"SPS: frame_cropping_rect_top_offset", s);
            sps->frame_cropping_rect_bottom_offset = ue_v ((char*)"SPS: frame_cropping_rect_bottom_offset", s);
        }
        sps->vui_parameters_present_flag           = u_1  ((char*)"SPS: vui_parameters_present_flag", s);
		if (sps->vui_parameters_present_flag)
		{
			if ((sps->vui_seq_parameters.aspect_ratio_info_present_flag = u_1((char*)"SPS：aspect_ratio_info_present_flag", s) != 0))
			{
				sps->vui_seq_parameters.aspect_ratio_idc				= u_v(8, (char*)"SPS：aspect_ratio_idc", s);
				if (sps->vui_seq_parameters.aspect_ratio_idc == 255)
				{
					sps->vui_seq_parameters.sar_width						= u_v(16, (char*)"SPS：sar_width", s);
					sps->vui_seq_parameters.sar_height						= u_v(16, (char*)"SPS：sar_height", s);
				}
			}

			sps->vui_seq_parameters.overscan_info_present_flag		= u_1((char*)"SPS：overscan_info_present_flag", s);
			if (sps->vui_seq_parameters.overscan_info_present_flag)
			{
				sps->vui_seq_parameters.overscan_appropriate_flag		= u_1((char*)"SPS：overscan_appropriate_flag", s);
			}
			
			sps->vui_seq_parameters.video_signal_type_present_flag	= u_1((char*)"SPS：video_signal_type_present_flag", s);
			if (sps->vui_seq_parameters.video_signal_type_present_flag)
			{
				sps->vui_seq_parameters.video_format					= u_v(3, (char*)"SPS：video_format", s);
				sps->vui_seq_parameters.video_full_range_flag			= u_1((char*)"SPS：video_full_range_flag", s);
				sps->vui_seq_parameters.colour_description_present_flag	= u_1((char*)"SPS：colour_description_present_flag", s);
				if (sps->vui_seq_parameters.colour_description_present_flag)
				{
					sps->vui_seq_parameters.colour_primaries				= u_v(8, (char*)"SPS：colour_primaries", s);
					sps->vui_seq_parameters.transfer_characteristics		= u_v(8, (char*)"SPS：transfer_characteristics", s);
					sps->vui_seq_parameters.matrix_coefficients				= u_v(8, (char*)"SPS：matrix_coefficients", s);
				}
			}			
			sps->vui_seq_parameters.chroma_location_info_present_flag	= u_1((char*)"SPS：chroma_location_info_present_flag", s);
			if (sps->vui_seq_parameters.chroma_location_info_present_flag)
			{
				sps->vui_seq_parameters.chroma_sample_loc_type_top_field	= ue_v((char*)"SPS：chroma_sample_loc_type_top_field", s);
				sps->vui_seq_parameters.chroma_sample_loc_type_bottom_field	= ue_v((char*)"SPS：chroma_sample_loc_type_bottom_field", s);
			}
			sps->vui_seq_parameters.timing_info_present_flag		= u_1((char*)"SPS：timing_info_present_flag", s);
			if (sps->vui_seq_parameters.timing_info_present_flag)
			{
				sps->vui_seq_parameters.num_units_in_tick				= u_v(32, (char*)"SPS：num_units_in_tick", s);
				sps->vui_seq_parameters.time_scale						= u_v(32, (char*)"SPS：time_scale", s);
				sps->vui_seq_parameters.fixed_frame_rate_flag			= u_1((char*)"SPS：fixed_frame_rate_flag", s);
			}

			sps->vui_seq_parameters.nal_hrd_parameters_present_flag		= u_1((char*)"SPS：nal_hrd_parameters_present_flag", s);


		}
        delete s;
        return 0;
}

static bool IsSpsData(unsigned char* pData)
{
	if(NULL == pData)
	{
		return false;
	}

	if(0x27 == pData[3] || 0x67 == pData[3] || 0x27 == pData[4] || 0x67 == pData[4])
	{
		return true;
	}
	else
	{
		return false;
	}
}


static int DecodeSps(unsigned char* pSpsInfo, int nSize,int& width,int& height, int& frameRate)
{
	if(IsSpsData(pSpsInfo))
	{
		//deocde sps and check if valid
		seq_parameter_set_rbsp_t sps;
		memset(&sps, 0, sizeof(seq_parameter_set_rbsp_t));

		//int i = sizeof(seq_parameter_set_rbsp_t);

		if(SPS_DEOCDE(pSpsInfo+5, nSize-5, &sps) < 0)	
		{
			return -1;
		}
		if( (sps.pic_width_in_mbs_minus1 <= 0) || (sps.pic_height_in_map_units_minus1 <=0) || (sps.num_ref_frames <= 0))
		{
			return -1;
		}

		//get image width/height from sps
		width	= (sps.pic_width_in_mbs_minus1 + 1) * 16;
		height	= (sps.pic_height_in_map_units_minus1 + 1) * 16;
		if (sps.vui_parameters_present_flag == 1)
		{
			if (sps.vui_seq_parameters.num_units_in_tick != 0)
			{
				frameRate = sps.vui_seq_parameters.time_scale / sps.vui_seq_parameters.num_units_in_tick / 2;
			}
		}
		else
		{
			frameRate = 25;
		}
	}
	else
	{
		return -1;
	}
	return 0;
}


static int DecodeSpsPpsInfo(char* pSpsPpsBufer, int& nWidth, int& nHetght,char* m_szSpsBuffer, int& m_nSpsLen,char* m_szPpsBuffer, int& m_nPpsLen, int&	m_nFrameRate)
{
	//decode sps info;
	char szSpsBufer[1024] = { 0 };
	char szPpsBufer[1024] = { 0 };

	char* pFindPps = strstr(pSpsPpsBufer, ",");
	if (NULL == pFindPps)
	{
		//pps not found
		return -1;
	}
	else
	{
		//防止SPS PPS字符串出错导致拷贝崩溃. 例如Public IPC 的SPS PPS 只有一个"," .
		int nPpsLen = pFindPps - pSpsPpsBufer;
		if (nPpsLen > 0)
		{
			memcpy(szSpsBufer, pSpsPpsBufer, nPpsLen);
			pFindPps++;
			if (NULL != pFindPps)
			{
				strcpy((char*)szPpsBufer, pFindPps);
			}
		}
	}
	//sps set star code
	memset(m_szSpsBuffer, 0x00, 1);
	memset(m_szSpsBuffer + 1, 0x00, 1);
	memset(m_szSpsBuffer + 2, 0x00, 1);
	memset(m_szSpsBuffer + 3, 0x01, 1);

	//pps set star code
	memset(m_szPpsBuffer, 0x00, 1);
	memset(m_szPpsBuffer + 1, 0x00, 1);
	memset(m_szPpsBuffer + 2, 0x00, 1);
	memset(m_szPpsBuffer + 3, 0x01, 1);

	//sps Base64 decode
	char baseDecBuffer[1024] = { 0 };
	unsigned int nRetLen = Base64::decode(baseDecBuffer, szSpsBufer);

	if (nRetLen <= 0)
	{
		return -1;
	}
	memcpy(m_szSpsBuffer + 4, baseDecBuffer, nRetLen);
	m_nSpsLen += nRetLen;

	//Decode sps get video size
	int nRet = DecodeSps((unsigned char*)m_szSpsBuffer, m_nSpsLen, nWidth, nHetght, m_nFrameRate);
	if (0 != nRet)
	{
		m_nFrameRate = 25;
		return -1;
	}

	//sps Base64 decode
	nRetLen = Base64::decode(baseDecBuffer, szPpsBufer);
	if (nRetLen <= 0)
	{
		return -1;
	}
	memcpy(m_szPpsBuffer + 4, baseDecBuffer, nRetLen);
	m_nPpsLen += nRetLen;

	return 0;
}


#endif