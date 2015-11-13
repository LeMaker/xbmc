/*
 *      Copyright (C) 2005-2014 Team XBMC
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

#include "system.h"

#include "video/videosync/VideoSyncActions.h"
#include "guilib/GraphicContext.h"
#include "windowing/WindowingFactory.h"
#include "utils/TimeUtils.h"
#include "utils/log.h"
#include "linux/ACTIONS.h"
#include "threads/Thread.h"

bool CVideoSyncActions::Setup(PUPDATECLOCK func)
{
  UpdateClock = func;
  m_abort = false;
  g_Windowing.Register(this);
  CLog::Log(LOGDEBUG, "CVideoReferenceClock: setting up RPi");
  g_CACTIONS = new CACTIONS();
  
  return true;
}

void CVideoSyncActions::Run(volatile bool& stop)
{
  /* This shouldn't be very busy and timing is important so increase priority */
  CThread::GetCurrentThread()->SetPriority(CThread::GetCurrentThread()->GetPriority()+1);

  while (!stop && !m_abort)
  {
    g_CACTIONS->WaitVsync();
    uint64_t now = CurrentHostCounter();
    UpdateClock(1, now);
  }
}

void CVideoSyncActions::Cleanup()
{
  CLog::Log(LOGDEBUG, "CVideoReferenceClock: cleaning up RPi");
  delete g_CACTIONS;
  g_Windowing.Unregister(this);
}

float CVideoSyncActions::GetFps()
{
  m_fps = g_graphicsContext.GetFPS();
  CLog::Log(LOGDEBUG, "CVideoReferenceClock: fps: %.2f", m_fps);
  return m_fps;
}

void CVideoSyncActions::OnResetDevice()
{
  m_abort = true;
}


