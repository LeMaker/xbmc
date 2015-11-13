#pragma once
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

#if defined(HAS_OWL_PLAYER)

#include <OMX_Component.h>
#include <DVDResource.h>
#include "threads/Thread.h"
#include "OpenMax.h"

class COWLVideo;

enum BufferStatus {
	OWNED_BY_US,
	OWNED_BY_COMPONENT,
	OWNED_BY_CLIENT,
};


// an omx video frame
struct OWLVideoBuffer : public IDVDResourceCounted<OWLVideoBuffer> {
  OWLVideoBuffer();
  virtual ~OWLVideoBuffer();

  OMX_BUFFERHEADERTYPE *omx_buffer;
  int src_offset[4];
  int src_stride[4];
  int width;
  int height;
  int index;
  bool done;
  BufferStatus mStatus;
  void PassBackToRenderer();
  void ReleaseTexture();
  void SetOWLVideo(COWLVideo *OWLVideo);

private:
  COWLVideo *m_OWLVideo;
};

typedef struct {
	int width;
	int height;
	int stride;
	int slice_height;
	int color_format;
	int crop_left;
	int crop_top;
	int crop_right;
	int crop_bottom;
}OMXOutputFormat;

enum {
	kPortIndexInput  = 0,
	kPortIndexOutput = 1
};

enum PortStatus {
	ENABLED,
	DISABLING,
	DISABLED,
	ENABLING,
	SHUTTING_DOWN,
};

struct CodecSpecificData {
	size_t mSize;
	uint8_t mData[1];
};

struct OMXMessage{
	enum {
        EVENT,
        EMPTY_BUFFER_DONE,
        FILL_BUFFER_DONE,
    } type;

	union{
	    struct{
	        OMX_HANDLETYPE hComponent;
	        OMX_PTR pAppData;
			OMX_EVENTTYPE eEvent;
			OMX_U32 nData1;
			OMX_U32 nData2;
			OMX_PTR pEventData;
		}event_data;
		
        struct{
	  		OMX_HANDLETYPE hComponent;
  			OMX_PTR pAppData;
  			OMX_BUFFERHEADERTYPE* pBuffer;
        }buffer_data;
	}u;
};

typedef int (*OWLCALLBACK)(void *callbackpriv);


class OWLVideoBufferHolder : public IDVDResourceCounted<OWLVideoBufferHolder> {
public:
  OWLVideoBufferHolder(OWLVideoBuffer *OWLVideoBuffer);
  virtual ~OWLVideoBufferHolder();

  OWLVideoBuffer *m_OWLVideoBuffer;
  int     nLineStride[4]; 
  char*   pData[4];

  int nWidth;
  int nHeight;

  OWLCALLBACK   callback;
  void       *callbackpriv;
};

class COWLVideo : public COpenMax, public IDVDResourceCounted<COWLVideo>, public CThread
{
public:
  COWLVideo();
  virtual ~COWLVideo();

  // Required overrides
  bool Open(CDVDStreamInfo &hints);
  void Close(void);
  int  Decode(uint8_t *pData, int iSize, double dts, double pts);
  void Reset(void);
  int GetPicture(DVDVideoPicture *pDvdVideoPicture);
  bool ClearPicture(DVDVideoPicture *pDvdVideoPicture);
  void ReleaseBuffer(OWLVideoBuffer *buffer);
  void SetDropState(bool bDrop);
  OMXOutputFormat* getOutFormat(void){return &mOutputFormat;}
protected:

  virtual void OnStartup();
  virtual void OnExit();
  virtual void Process(); 

  int EnqueueDemuxPacket(omx_demux_packet demux_packet);

  void QueryCodec(void);
  OMX_ERRORTYPE PrimeFillBuffers(void);
  OMX_ERRORTYPE AllocOMXInputBuffers(void);
  OMX_ERRORTYPE FreeOMXInputBuffers(bool wait);
  OMX_ERRORTYPE AllocOMXOutputBuffers(void);
  OMX_ERRORTYPE FreeOMXOutputBuffers(bool wait);

  // TODO Those should move into the base class. After start actions can be executed by callbacks.
  OMX_ERRORTYPE StartDecoder(void);
  OMX_ERRORTYPE StopDecoder(void);

  void ReleaseDemuxQueue();

  OMX_ERRORTYPE SetComponentRole(OMX_VIDEO_CODINGTYPE codecType);
  OMX_ERRORTYPE setVideoPortFormatType(
		  OMX_U32 portIndex,
		  OMX_VIDEO_CODINGTYPE compressionFormat,
		  OMX_COLOR_FORMATTYPE colorFormat); 
  OMX_ERRORTYPE setVideoOutputFormat(OMX_VIDEO_CODINGTYPE codecType);
  OMX_ERRORTYPE initOutputFormat(void);
  OMX_ERRORTYPE configureCodec(OMX_VIDEO_CODINGTYPE codecType);
  void setState(OMX_CLIENT_STATE newState);
  void onStateChange(OMX_STATETYPE newState);
  void onCmdComplete(OMX_PTR pAppData,OMX_COMMANDTYPE cmd, OMX_U32 data);
  void onPortSettingsChanged(OMX_U32 portIndex);
  bool flushPortAsync(OMX_U32 portIndex);
  OMX_ERRORTYPE disablePortAsync(OMX_U32 portIndex);

  OMX_ERRORTYPE enablePortAsync(OMX_U32 portIndex);
  bool isIntermediateState(OMX_CLIENT_STATE state);

  OMX_ERRORTYPE stopOmxComponent_l(void);
  int fillinPrivateData(void);
  void addCodecSpecificData(const void *data, size_t size);
  void clearCodecSpecificData(void);
  void drainInputBuffers(void);

  void fillOutputBuffers(void);

  OMX_CLIENT_STATE mState;
  OMXOutputFormat mOutputFormat;
  PortStatus mPortStatus[2];
  bool mOutputPortSettingsChangedPending;
  CDVDStreamInfo  m_hints;
  bool mInitialBufferSubmit;

  OMX_VIDEO_CODINGTYPE mCodecType;

  std::vector<CodecSpecificData*>mCodecSpecificData;
  size_t mCodecSpecificDataIndex;
  
  OMX_ERRORTYPE OnEventHandler(
	OMX_HANDLETYPE hComponent,
	OMX_PTR pAppData,
	OMX_EVENTTYPE eEvent,
	OMX_U32 nData1,
	OMX_U32 nData2,
	OMX_PTR pEventData);

  
  OMX_ERRORTYPE OnEmptyBufferDone(
    OMX_HANDLETYPE hComponent, 
    OMX_PTR pAppData, 
    OMX_BUFFERHEADERTYPE* pBuffer);

  OMX_ERRORTYPE OnFillBufferDone(
    OMX_HANDLETYPE hComponent, 
    OMX_PTR pAppData, 
    OMX_BUFFERHEADERTYPE* pBufferHeader);


  // OpenMax decoder callback routines.
  virtual OMX_ERRORTYPE DecoderEventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
    OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData);
  virtual OMX_ERRORTYPE DecoderEmptyBufferDone(
    OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer);
  virtual OMX_ERRORTYPE DecoderFillBufferDone(
    OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBufferHeader);

  bool              m_drop_state;
  int               m_decoded_width;
  int               m_decoded_height;

  std::queue<double> m_dts_queue;
  std::queue<omx_demux_packet> m_demux_queue;

  // Synchronization
  pthread_mutex_t   m_omx_queue_mutex;
  pthread_cond_t    m_omx_queue_available;

  pthread_mutex_t	m_event_queue_mutex;
  pthread_cond_t    m_event_queue_available;
  
  // OpenMax input buffers (demuxer packets)
  std::queue<OMX_BUFFERHEADERTYPE*> m_omx_input_avaliable;
  std::vector<OMX_BUFFERHEADERTYPE*> m_omx_input_buffers;
  bool              m_omx_input_eos;
  int               m_omx_input_port;
  CEvent            m_input_consumed_event;

  // OpenMax output buffers (video frames)
  std::queue<OWLVideoBuffer*> m_omx_output_busy;
  std::queue<OWLVideoBuffer*> m_omx_output_ready;
  std::vector<OWLVideoBuffer*> m_omx_output_buffers;

  std::queue<OMXMessage*> m_omx_message;
  
  bool              m_omx_output_eos;
  int               m_omx_output_port;

  bool              m_portChanging;

  volatile bool     m_videoplayback_done;
  bool mSeeking;
  bool m_isIFrameStart;
  bool m_noCheckIFrame;
};

#endif

