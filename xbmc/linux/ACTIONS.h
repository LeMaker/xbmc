
#ifndef __ACTIONS_H__
#define __ACTIONS_H__


typedef unsigned char __u8;
typedef unsigned short __u16;

typedef long long  int64_t;


struct owlfb_sync_info {
	__u8 enabled;
	__u8 disp_id;
	__u16 reserved2;
};

#define OWL_IOW(num, dtype)	_IOW('O', num, dtype)
#define OWL_IOR(num, dtype)	_IOR('O', num, dtype)
#define OWL_IOWR(num, dtype)	_IOWR('O', num, dtype)
#define OWL_IO(num)		_IO('O', num)

#define OWLFB_WAITFORVSYNC	          OWL_IOW(57,long long)

#define OWLFB_VSYNC_EVENT_EN	      OWL_IOW(67, struct owlfb_sync_info)


typedef struct
{
	int fb_fd;
	unsigned int ui32VSyncPeriodns;
} ACTIONS_CONTEXT;

class CACTIONS
{
public:
  CACTIONS();
  ~CACTIONS();
  bool Initialize();
  bool Deinitialize();
  void WaitVsync();

private:
  int Actions_ContextCreate(void);
  int Actions_ContextDestroy(void);
  int Actions_ContextVSyncControl(void* hDisplayContext,
                                bool bEnabled);
  bool Actions_VSyncIoctlHandler(void* hDisplayContext,
  	                            int64_t i64TimeStamp);
private:
  bool       m_initialized;
  ACTIONS_CONTEXT* phDisplayContext;

};

//extern CATIONS g_CATIONS;
#endif /*__ACTIONS_H__*/

