#include <stdio.h>
//涉及到的函数

typedef struct GetBitContext 
{
    const unsigned char *buffer;
    int index;
    int size_in_bits;
} GetBitContext;

static  const unsigned char ff_log2_tab[256]=
{
	    0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};
static const unsigned char ff_golomb_vlc_len[512]=
{
	    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
		3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
		3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
		3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};
static const unsigned char ff_ue_golomb_vlc_code[512]=
{ 
	    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
		7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define bswap_32_h264(x) \
	(((x)  >> 24) | (((x) & 0x00ff0000) >>  8) | \
      (((x) & 0x0000ff00) <<  8) | ((x) << 24))
#define NEG_USR32(a,s) ((( unsigned int)(a))>>(32-(s)))


#ifdef __GNUC__
static inline unsigned int unaligned32_h264(const void *v) 
{
    struct Unaligned 
    {
		unsigned int i;
    } __attribute__((packed));
    return ((const struct Unaligned *) v)->i;
}
#else
static unsigned int unaligned32_h264(const void *v)
{
    return *(const unsigned int *) v;
}
#endif
static unsigned int unaligned32_be(const void *v)
{
  unsigned int x;
  x=unaligned32_h264(v);
  return bswap_32_h264(x);
}


static int av_log2_OWL(unsigned int v)
{
    int n;
    n = 0;
    if (v & 0xffff0000) 
    {
        v >>= 16;
        n += 16;
    }
    if (v & 0xff00) 
    {
        v >>= 8;
        n += 8;
    }
    n += ff_log2_tab[v];
    return n;
}
static int get_ue_golomb(GetBitContext *gb)
{
	unsigned int buf;
	int log;
	int re_index= gb->index;
	buf= unaligned32_be( ((unsigned char *)gb->buffer)+(re_index>>3) ) << (re_index&0x07); 
    if(buf >= (1<<27))
    {
        buf >>= 32 - 9;
		gb->index= re_index+ff_golomb_vlc_len[buf];
        return ff_ue_golomb_vlc_code[buf];
    }
    else
    {
		
        log= 2*av_log2_OWL(buf) - 31;     
       	gb->index= re_index+(32 - log);
        return ((buf>>log)-1);
		
    }
}


static void init_get_bits(GetBitContext *s, const unsigned char *buffer, int bit_size)
{
	s->buffer= buffer;
	s->size_in_bits= bit_size;
	s->index=0;
}

static int check_spspps(unsigned char *buf, int len)
{
	int i;
	unsigned int zerolen;
	unsigned int nal_unit_type;

	i = 0;
	zerolen = 0;
	nal_unit_type = 0;
	while(i + 3 < len)
	{		
		if (buf[i] == 0)
			zerolen++;
		else
		{
			if ((zerolen >= 2)&&(buf[i] == 0x01)) //find NAL
			{
				//nal_unit_type = buf[i + 1] & 0x1f;
				if ((nal_unit_type == 7)\
					||(nal_unit_type == 15))  //SUB sps
				{
					return nal_unit_type;
				}				
			//check spspps
			  nal_unit_type = buf[i + 1] & 0x1f;
			}
			zerolen = 0;
		}
		i++;
	}
	return 0;
}


typedef enum 
{
  P_SLICE = 0,
  B_SLICE,
  I_SLICE,
  SP_SLICE,
  SI_SLICE
} SliceType;

static int check_slice(unsigned char *buf, int len)
{
	int i;
	unsigned int zerolen;
	unsigned int nal_unit_type;
	GetBitContext gb = {0};
	int stype;
	
	i = 0;
	zerolen = 0;

	while(i + 3 < len)
	{		
		if (buf[i] == 0)
			zerolen++;
		else
		{
			if ((zerolen >= 2)&&(buf[i] == 0x01)) //find NAL
			{
			//check spspps
				nal_unit_type = buf[i + 1] & 0x1f;
				if (nal_unit_type == 5)
				{
					return nal_unit_type; 
				}
#if 1	
				if ((nal_unit_type == 1))
				{
					init_get_bits(&gb, &buf[i + 2], (len - (i + 2))*8);
					get_ue_golomb(&gb);	//first_mb
					stype = get_ue_golomb(&gb); //slice_type
					if (stype>4) 
						stype -=5;
//i slice		
					if (stype == I_SLICE)
					  return 5;
					else
					  return nal_unit_type;
				}	
#endif
			}
			zerolen = 0;
		}
		i++;
	}
	return 0;
}

int OWL_check_h264_IDR(unsigned char *buf, int len)
{
  int nal_type =0;
        
  nal_type = check_spspps(buf, len);
  if (!nal_type)
  {
    printf("\n not spspps\n");
	return -1; 
  }
  nal_type = check_slice(buf, len);
	
  if ((nal_type != 0x5))
  {
    printf("\n not IDR\n");
    return -1; 			
  }				
  return 0; 
}


#define I_TYPE 1
#define P_TYPE 2
#define B_TYPE 3

int OWL_check_mpeg2_IDR(unsigned char *buf, int len)
{
  int type;
  int size = len;
  int i;
  
  i = 0;
  while(i + 5 < len)
  {		
    if (buf[i] == 0x00 && buf[i+1] == 0x00 &&
        buf[i+2] == 0x01 && buf[i+3] == 0x00)
    {
      type = (buf[i+5] >>3)&0x7;
      if (type == I_TYPE) return 0;
    }
    i++;
  }
  return -1;
}



