
#include "ACTIONS.h"
#include "utils/log.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>

#include <linux/ioctl.h>
#include <sys/ioctl.h>

#include <unistd.h>


CACTIONS::CACTIONS()
{
  Initialize();

}

CACTIONS::~CACTIONS()
{
  Deinitialize();
}

int CACTIONS::Actions_ContextCreate(void)
{
	int err = 0;
	
	phDisplayContext = (ACTIONS_CONTEXT*)malloc(sizeof(ACTIONS_CONTEXT));

	if (!phDisplayContext)
	{
		CLog::Log(LOGNOTICE,"Failed to allocate context");
		err = -1;
		goto err_ret;
	}

	//phDisplayContext->fb_fd = open("/dev/graphics/fb0", O_RDWR);

	phDisplayContext->fb_fd = open("/dev/fb0", O_RDWR);
	
	if (phDisplayContext->fb_fd < 0)
	{
		CLog::Log(LOGNOTICE,"Failed to open /dev/graphics/fb0 (%d)", errno);
		err = -1;
		goto err_close_disp;
	}

	/* FIXME: default vsync period */
	phDisplayContext->ui32VSyncPeriodns = 16666666;

err_ret:
	return err;
err_close_disp:
	close(phDisplayContext->fb_fd);
err_free_context:
	free(phDisplayContext);
	goto err_ret;
}

int CACTIONS:: Actions_ContextDestroy(void)
{
	if (!phDisplayContext)
	{
		return -1;
	}

	close(phDisplayContext->fb_fd);

	free(phDisplayContext);
	return 0;
}


int CACTIONS::Actions_ContextVSyncControl(void* hDisplayContext,
                                bool bEnabled)
{
	ACTIONS_CONTEXT *psCtx = (ACTIONS_CONTEXT*)hDisplayContext;
	struct owlfb_sync_info arg;
		
	arg.disp_id = 0;
	arg.enabled =  bEnabled ? 1 : 0;
		
	if (!psCtx)
	{
		return -1;
	}
	//HWC_DPF(ERROR, "HWC_OWL_ContextVSyncControl ~~~~ %d ", bEnabled);
	ioctl(psCtx->fb_fd, OWLFB_VSYNC_EVENT_EN, &arg);
	return 0;
}

bool CACTIONS::Actions_VSyncIoctlHandler(void* hDisplayContext,int64_t i64TimeStamp)
{
	int err;	
	ACTIONS_CONTEXT *psCtx = (ACTIONS_CONTEXT*)hDisplayContext;

	if (!psCtx)
	{
		return false;
	}	

	err = ioctl(psCtx->fb_fd, OWLFB_WAITFORVSYNC, &i64TimeStamp);
		
	if (err)
	{
		CLog::Log(LOGNOTICE,"Failed to send WAITFORVSYNC command (%d)", err);
		goto err_out;
	}
	return true;
err_out:
	return true;
}


bool CACTIONS:: Initialize(void)
{
	if (Actions_ContextCreate() == 0)
	{
		Actions_ContextVSyncControl((void*)phDisplayContext, true);
		return true;
	}else{
	    return false;
	}
}


bool CACTIONS:: Deinitialize(void)
{

	Actions_ContextVSyncControl((void*)phDisplayContext, false);
	Actions_ContextDestroy();
	return true;
}

void CACTIONS::WaitVsync(void){
	int poll_ret;
	int64_t i64LastVSyncUEventTimeNS = 0;
	poll_ret = Actions_VSyncIoctlHandler((void*)phDisplayContext,i64LastVSyncUEventTimeNS);	
}


