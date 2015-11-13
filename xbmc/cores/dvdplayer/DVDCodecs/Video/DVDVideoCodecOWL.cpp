/*
 *      Copyright (C) 2010-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#if (defined HAVE_CONFIG_H) && (!defined TARGET_WINDOWS)
  #include "config.h"
#elif defined(TARGET_WINDOWS)
#include "system.h"
#endif

#if defined(HAS_OWL_PLAYER)
#include "DVDClock.h"

#include "DVDStreamInfo.h"
#include "utils/log.h"
#include "settings/Settings.h"
//* Modify by LeMaker -- begin
//#include "utils/fastmemcpy.h"
//* Modify by LeMaker -- end
#include "DVDVideoCodecOWL.h"

#define CLASSNAME "COWLCodec"
////////////////////////////////////////////////////////////////////////////////////////////

static int owl_codec_opened = 0;
CDVDVideoCodecOWL::CDVDVideoCodecOWL() : CDVDVideoCodec()
{
  m_omx_decoder = NULL;
  m_pFormatName = "omx-xxxx";
  m_isOpen = true;
  m_convert_bitstream = false;
  memset(&m_videobuffer, 0, sizeof(DVDVideoPicture));
  owl_codec_opened += 1;
}

CDVDVideoCodecOWL::~CDVDVideoCodecOWL()
{
  CLog::Log(LOGNOTICE, "%s::%s in",CLASSNAME, __func__);
  Dispose();
  CLog::Log(LOGNOTICE, "%s::%s out",CLASSNAME, __func__);
}

bool CDVDVideoCodecOWL::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options)
{
  // we always qualify even if DVDFactoryCodec does this too.
  if (!hints.software)
  {
    m_convert_bitstream = false;

    switch (hints.codec)
    {
      case AV_CODEC_ID_H264:
      {
        printf("\n ## hints.codec_tag == 0x%x \n ",hints.codec_tag);
        printf("\n ## hints.flags == %d \n ",hints.flags);
        if(hints.codec_tag == 0 && hints.flags == 0){
          return false;
        }

        m_pFormatName = "omx-h264";

	    // check for h264-avcC and convert to h264-annex-b
        if (hints.extradata && *(uint8_t*)hints.extradata == 1)
        {
          CLog::Log(LOGNOTICE, "%s::%s - avcC data extradata ", CLASSNAME, __func__);
          m_convert_bitstream = bitstream_convert_init(hints.extradata, hints.extrasize);
          if (!m_convert_bitstream)
          {
		    CLog::Log(LOGNOTICE,
                "%s::%s - avcC data extradata missing", CLASSNAME, __func__);
            return false;
          }		  
        }
      }
      break;
	  
      case AV_CODEC_ID_MPEG4:
        m_pFormatName = "omx-mpeg4";
      break;
      case AV_CODEC_ID_MPEG2VIDEO:
        m_pFormatName = "omx-mpeg2";
      break;

	  case AV_CODEC_ID_VC1:
	  case AV_CODEC_ID_WMV3:	 
        m_pFormatName = "omx-vc1";
      break;
	  
	  case AV_CODEC_ID_H263:
	  case AV_CODEC_ID_H263P:
	  case AV_CODEC_ID_H263I:
			m_pFormatName = "omx-h263";
      break;
		  
	  case AV_CODEC_ID_MJPEG:
			m_pFormatName = "omx-mjpeg";
	  break;
	  
	  case AV_CODEC_ID_FLV1:
		 m_pFormatName = "omx-flv1";
	  break;
		  
	  case AV_CODEC_ID_RV30:
	  case AV_CODEC_ID_RV40:
		m_pFormatName = "omx-rv";
	  break;
	  
	  case AV_CODEC_ID_MSMPEG4V2:
	  case AV_CODEC_ID_MSMPEG4V3:
		m_pFormatName = "omx-div3";
		return false;
	  break;
	  case AV_CODEC_ID_HEVC:
      {
		m_pFormatName = "omx-hevc";
	    return false;
	  }
	  break;
	  
	  case AV_CODEC_ID_VP6: 	
	  case AV_CODEC_ID_VP6A:
	  case AV_CODEC_ID_VP6F:
		m_pFormatName = "omx-vp6";
	  break;
	  
	  case AV_CODEC_ID_VP8:
		m_pFormatName = "omx-vp8";
	  break;
	  
      default:
	  	 CLog::Log(LOGERROR,
        "%s::%s - no support codec(%d)", CLASSNAME, __func__,hints.codec);
        return false;
      break;
    }

    m_omx_decoder = new COWLVideo;
    if (!m_omx_decoder->Open(hints))
    {
      CLog::Log(LOGERROR,
        "%s::%s - failed to open, codec(%d), profile(%d), level(%d)", 
        CLASSNAME, __func__, hints.codec, hints.profile, hints.level);
      return false;
    }

    // allocate a YV12 DVDVideoPicture buffer.
    // first make sure all properties are reset.
    memset(&m_videobuffer, 0, sizeof(DVDVideoPicture));

    m_videobuffer.dts = DVD_NOPTS_VALUE;
    m_videobuffer.pts = DVD_NOPTS_VALUE;
    m_videobuffer.format = RENDER_FMT_OWL;
    m_videobuffer.color_range  = 0;
    m_videobuffer.color_matrix = 4;
    m_videobuffer.iFlags  = DVP_FLAG_ALLOCATED;
    m_videobuffer.iWidth  = hints.width;
    m_videobuffer.iHeight = hints.height;
    m_videobuffer.iDisplayWidth  = hints.width;
    m_videobuffer.iDisplayHeight = hints.height;

	ConfigureOutputFormat(hints);

    return true;
  }

  return false;
}

void CDVDVideoCodecOWL::ConfigureOutputFormat(CDVDStreamInfo &hints)
{
  OMXOutputFormat* output;
  output = m_omx_decoder->getOutFormat();
  
  int width       = hints.width;//output->width;
  int height      = hints.height;//output->height;
  int stride      = output->stride;
  int slice_height= output->slice_height;
  int color_format= output->color_format;
  int crop_left   = output->crop_left;
  int crop_top    = output->crop_top;
  int crop_right  = output->crop_right;
  int crop_bottom = output->crop_bottom;


  CLog::Log(LOGNOTICE, "%s::width(%d), height(%d), stride(%d), slice-height(%d), color-format(%d)",
    CLASSNAME,width, height, stride, slice_height, color_format);
  CLog::Log(LOGNOTICE, "%s:: "
    "crop-left(%d), crop-top(%d), crop-right(%d), crop-bottom(%d)",CLASSNAME,
    crop_left, crop_top, crop_right, crop_bottom);

  {

    // No color-format? Initialize with the one we detected as valid earlier
    if (color_format == 0)
      color_format = OMX_COLOR_FormatYUV420SemiPlanar;
    if (stride <= width)
      stride = width;
    if (!crop_right)
      crop_right = width-1;
    if (!crop_bottom)
      crop_bottom = height-1;
    if (slice_height <= height)
    {
      slice_height = height;
    }

    // default picture format to none
    for (int i = 0; i < 4; i++)
      m_src_offset[i] = m_src_stride[i] = 0;

    // setup picture format and data offset vectors
    if(color_format == OMX_COLOR_FormatYUV420SemiPlanar)
    {
      CLog::Log(LOGDEBUG, "%s::%s OMX_COLOR_FormatYUV420SemiPlanar",CLASSNAME, __func__);

      // Y plane
      m_src_stride[0] = stride;
      m_src_offset[0] = crop_top * stride;
      m_src_offset[0]+= crop_left;

      // UV plane
      m_src_stride[1] = stride;
      //  skip over the Y plane
      m_src_offset[1] = slice_height * stride;
      m_src_offset[1]+= crop_top * stride;
      m_src_offset[1]+= crop_left;
      m_videobuffer.format  = RENDER_FMT_OWL;
    }
    else
    {
    
      CLog::Log(LOGERROR, "%s:: Fixme unknown color_format(%d)",CLASSNAME, color_format);
      return;
    }
  }

  if (width)
    m_videobuffer.iWidth  = width;
  if (height)
    m_videobuffer.iHeight = height;

  // picture display width/height include the cropping.
  m_videobuffer.iDisplayWidth  = crop_right  + 1 - crop_left;
  m_videobuffer.iDisplayHeight = crop_bottom + 1 - crop_top;

 CLog::Log(LOGNOTICE, "CDVDVideoCodecOWL:: hints.aspect ==%f, hints.forced_aspect =%d\n",
 	hints.aspect, hints.forced_aspect);
  if (hints.aspect > 1.0 && !hints.forced_aspect)
  {
    m_videobuffer.iDisplayWidth  = ((int)lrint(m_videobuffer.iHeight * hints.aspect)) & -3;
    if (m_videobuffer.iDisplayWidth > m_videobuffer.iWidth)
    {
      m_videobuffer.iDisplayWidth  = m_videobuffer.iWidth;
      m_videobuffer.iDisplayHeight = ((int)lrint(m_videobuffer.iWidth / hints.aspect)) & -3;
    }
  }

}


void CDVDVideoCodecOWL::Dispose()
{
  CLog::Log(LOGNOTICE, "%s::%s in",CLASSNAME, __func__);

  if (m_isOpen){
    owl_codec_opened -= 1;
    m_isOpen = false;
  }

  if (m_omx_decoder)
  {
    m_omx_decoder->Close();
    m_omx_decoder->Release();
    m_omx_decoder = NULL;
  }

  if (m_convert_bitstream)
  {
    if (m_sps_pps_context.sps_pps_data)
    {
      free(m_sps_pps_context.sps_pps_data);
      m_sps_pps_context.sps_pps_data = NULL;
    }
    m_convert_bitstream = false;
  }

  CLog::Log(LOGNOTICE, "%s::%s out",CLASSNAME, __func__);
}

void CDVDVideoCodecOWL::SetDropState(bool bDrop)
{
    m_omx_decoder->SetDropState(bDrop);
}

int CDVDVideoCodecOWL::Decode(uint8_t* pData, int iSize, double dts, double pts)
{
  if (pData)
  {
    int rtn;
    int demuxer_bytes = iSize;
    uint8_t *demuxer_content = pData;
    bool bitstream_convered  = false;
	
    if (m_convert_bitstream)
    {
      // convert demuxer packet from bitstream to bytestream (AnnexB)
      int bytestream_size = 0;
      uint8_t *bytestream_buff = NULL;

      bitstream_convert(demuxer_content, demuxer_bytes, &bytestream_buff, &bytestream_size);
      if (bytestream_buff && (bytestream_size > 0))
      {
        bitstream_convered = true;
        demuxer_bytes = bytestream_size;
        demuxer_content = bytestream_buff;
      }else{
        CLog::Log(LOGERROR,"%s::%s - bitstream_convert error", CLASSNAME, __func__);
		return m_omx_decoder->Decode(0, 0, 0, 0);
      }
    }

    rtn = m_omx_decoder->Decode(demuxer_content, demuxer_bytes, dts, pts);

    if (bitstream_convered)
      free(demuxer_content);

    return rtn;
  }
  
  return m_omx_decoder->Decode(0, 0, 0, 0);
}

void CDVDVideoCodecOWL::Reset(void)
{
  m_omx_decoder->Reset();
}

//0:open; 1:closing
int OWLCodecStatus(void)
{
  return (owl_codec_opened > 0)?0:1;
}

bool CDVDVideoCodecOWL::GetPicture(DVDVideoPicture* pDvdVideoPicture)
{
  int returnCode = 0;

  m_videobuffer.dts = DVD_NOPTS_VALUE;
  m_videobuffer.pts = DVD_NOPTS_VALUE;

  returnCode = m_omx_decoder->GetPicture(&m_videobuffer);

  if (returnCode & VC_PICTURE){
	  OWLVideoBufferHolder *bufferHolder = (OWLVideoBufferHolder*)(m_videobuffer.OWLBufferHolder);
	  OWLVideoBuffer *buffer = bufferHolder->m_OWLVideoBuffer;
  
	  char *src_ptr = (char*)buffer->omx_buffer->pBuffer;
	  int offset = buffer->omx_buffer->nOffset;
		  
	  src_ptr += offset;
	  
	  char *src_0   = src_ptr + m_src_offset[0];
	  bufferHolder->nLineStride[0] = m_src_stride[0];
	  bufferHolder->pData[0]= src_0;

	  char *src_1	 = src_ptr + m_src_offset[1];
	  bufferHolder->nLineStride[1] = m_src_stride[1];
	  bufferHolder->pData[1]= src_1;

	  bufferHolder->nWidth = m_videobuffer.iWidth;
	  bufferHolder->nHeight = m_videobuffer.iHeight;
  }
  *pDvdVideoPicture = m_videobuffer;
  return true;
}

bool CDVDVideoCodecOWL::ClearPicture(DVDVideoPicture* pDvdVideoPicture)
{
  if (pDvdVideoPicture->OWLBufferHolder){
    m_omx_decoder->ClearPicture(pDvdVideoPicture);
  }
  memset(pDvdVideoPicture, 0, sizeof(DVDVideoPicture));
  return true;
}

const char* CDVDVideoCodecOWL::GetName(void)
{
  return "OWL";
}


////////////////////////////////////////////////////////////////////////////////////////////
bool CDVDVideoCodecOWL::bitstream_convert_init(void *in_extradata, int in_extrasize)
{
  // based on h264_mp4toannexb_bsf.c (ffmpeg)
  // which is Copyright (c) 2007 Benoit Fouet <benoit.fouet@free.fr>
  // and Licensed GPL 2.1 or greater

  m_sps_pps_size = 0;
  m_sps_pps_context.sps_pps_data = NULL;
  
  // nothing to filter
  if (!in_extradata || in_extrasize < 6)
    return false;

  uint16_t unit_size;
  uint32_t total_size = 0;
  uint8_t *out = NULL, unit_nb, sps_done = 0;
  const uint8_t *extradata = (uint8_t*)in_extradata + 4;
  static const uint8_t nalu_header[4] = {0, 0, 0, 1};

  // retrieve length coded size
  m_sps_pps_context.length_size = (*extradata++ & 0x3) + 1;
  if (m_sps_pps_context.length_size == 3)
    return false;

  // retrieve sps and pps unit(s)
  unit_nb = *extradata++ & 0x1f;  // number of sps unit(s)
  if (!unit_nb)
  {
    unit_nb = *extradata++;       // number of pps unit(s)
    sps_done++;
  }
  while (unit_nb--)
  {
    unit_size = extradata[0] << 8 | extradata[1];
    total_size += unit_size + 4;
    if ( (extradata + 2 + unit_size) > ((uint8_t*)in_extradata + in_extrasize) )
    {
      free(out);
      return false;
    }
    uint8_t* new_out = (uint8_t*)realloc(out, total_size);
    if (new_out)
    {
      out = new_out;
    }
    else
    {
      CLog::Log(LOGERROR, "bitstream_convert_init failed - %s : could not realloc the buffer out",  __FUNCTION__);
      free(out);
      return false;
    }

    memcpy(out + total_size - unit_size - 4, nalu_header, 4);
    memcpy(out + total_size - unit_size, extradata + 2, unit_size);
    extradata += 2 + unit_size;

    if (!unit_nb && !sps_done++)
      unit_nb = *extradata++;     // number of pps unit(s)
  }

  m_sps_pps_context.sps_pps_data = out;
  m_sps_pps_context.size = total_size;
  m_sps_pps_context.first_idr = 1;

  return true;
}

bool CDVDVideoCodecOWL::bitstream_convert(uint8_t* pData, int iSize, uint8_t **poutbuf, int *poutbuf_size)
{
  // based on h264_mp4toannexb_bsf.c (ffmpeg)
  // which is Copyright (c) 2007 Benoit Fouet <benoit.fouet@free.fr>
  // and Licensed GPL 2.1 or greater

  uint8_t *buf = pData;
  uint32_t buf_size = iSize;
  uint8_t  unit_type;
  int32_t  nal_size;
  uint32_t cumul_size = 0;
  const uint8_t *buf_end = buf + buf_size;

  do
  {
    if (buf + m_sps_pps_context.length_size > buf_end)
      goto fail;

    if (m_sps_pps_context.length_size == 1)
      nal_size = buf[0];
    else if (m_sps_pps_context.length_size == 2)
      nal_size = buf[0] << 8 | buf[1];
    else
      nal_size = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];

    buf += m_sps_pps_context.length_size;
    unit_type = *buf & 0x1f;

    if (buf + nal_size > buf_end || nal_size < 0)
      goto fail;

    // prepend only to the first type 5 NAL unit of an IDR picture
    if (m_sps_pps_context.first_idr && unit_type == 5)
    {
      bitstream_alloc_and_copy(poutbuf, poutbuf_size,
        m_sps_pps_context.sps_pps_data, m_sps_pps_context.size, buf, nal_size);
      m_sps_pps_context.first_idr = 0;
    }
    else
    {
      bitstream_alloc_and_copy(poutbuf, poutbuf_size, NULL, 0, buf, nal_size);
      if (!m_sps_pps_context.first_idr && unit_type == 1)
          m_sps_pps_context.first_idr = 1;
    }

    buf += nal_size;
    cumul_size += nal_size + m_sps_pps_context.length_size;
  } while (cumul_size < buf_size);

  return true;

fail:
  free(*poutbuf);
  *poutbuf = NULL;
  *poutbuf_size = 0;
  return false;
}

void CDVDVideoCodecOWL::bitstream_alloc_and_copy(
  uint8_t **poutbuf,      int *poutbuf_size,
  const uint8_t *sps_pps, uint32_t sps_pps_size,
  const uint8_t *in,      uint32_t in_size)
{
  // based on h264_mp4toannexb_bsf.c (ffmpeg)
  // which is Copyright (c) 2007 Benoit Fouet <benoit.fouet@free.fr>
  // and Licensed GPL 2.1 or greater

  #define CHD_WB32(p, d) { \
    ((uint8_t*)(p))[3] = (d); \
    ((uint8_t*)(p))[2] = (d) >> 8; \
    ((uint8_t*)(p))[1] = (d) >> 16; \
    ((uint8_t*)(p))[0] = (d) >> 24; }

  uint32_t offset = *poutbuf_size;
  uint8_t nal_header_size = offset ? 3 : 4;

  *poutbuf_size += sps_pps_size + in_size + nal_header_size;
  *poutbuf = (uint8_t*)realloc(*poutbuf, *poutbuf_size);
  if (sps_pps)
    memcpy(*poutbuf + offset, sps_pps, sps_pps_size);

  memcpy(*poutbuf + sps_pps_size + nal_header_size + offset, in, in_size);
  if (!offset)
  {
    CHD_WB32(*poutbuf + sps_pps_size, 1);
  }
  else
  {
    (*poutbuf + offset + sps_pps_size)[0] = 0;
    (*poutbuf + offset + sps_pps_size)[1] = 0;
    (*poutbuf + offset + sps_pps_size)[2] = 1;
  }
}

#endif

