/******************************************************************************
* Name:        GdpFrame class (same file for Gdpisco and FileSpeck2)
* Purpose:     video mode: displaying a plane of a 3D FITS file
*
* Author:      JLP 
* Version:     07/01/2015
******************************************************************************/
#include "gdp_frame.h"
#include "gdp_frame_id.h"   // ID_TIMER

/* Prototypes of routines contained here and defined in "gdp_frame.h":
void OnLoadFITSImage( wxCommandEvent &event);
void OnNextFrame(wxCommandEvent& event);
void OnPreviousFrame(wxCommandEvent& event);
void OnGoToZeroVideo(wxCommandEvent& event);
void OnGotoFrame(wxCommandEvent& event);

void OnSetVideoDelay(wxCommandEvent& WXUNUSED(event));
void OnPlayVideo(wxCommandEvent& WXUNUSED(event));
void OnTimer(wxTimerEvent &WXUNUSED(event));
void OnStopVideo(wxCommandEvent& event);
*/ 

/******************************************************************
* Load a new FITS file 
*******************************************************************/
void GdpFrame::OnLoadFITSImage( wxCommandEvent &event )
{

// Call the routine defined in Gdisplay1.cpp: 
// int GdpFrame::LoadFITSImage(const bool change_filename, const int iframe)

// change_filename=true  iframe=1
   LoadFITSImage(true, 1);

// Reset all options to default:
   JLP_ResetAllOptionsOfImageMenu();

return;
}
/*************************************************************************
* Display the next frame
*************************************************************************/
void GdpFrame::OnNextFrame(wxCommandEvent& event)
{
// change_filename=false  iframe=m_iframe;
if(m_iframe < m_nframes) {
  m_iframe++;
  LoadFITSImage(false, m_iframe);
  } 
return;
}
/*************************************************************************
* Display the previous frame
*************************************************************************/
void GdpFrame::OnPreviousFrame(wxCommandEvent& event)
{
// change_filename=false  iframe=m_iframe;
if(m_iframe > 1) {
  m_iframe--;
  LoadFITSImage(false, m_iframe);
  } 
return;
}
/*************************************************************************
* Display the first frame
*************************************************************************/
void GdpFrame::OnGoToZeroVideo(wxCommandEvent& WXUNUSED(event))
{
// change_filename=false  iframe=m_iframe;
 m_iframe = 1;
 LoadFITSImage(false, m_iframe);
return;
}
/*************************************************************************
* Goto a given frame
*************************************************************************/
void GdpFrame::OnGotoFrame(wxCommandEvent& WXUNUSED(event))
{
wxString s_values, s_question;
long iframe;

s_question.Printf(wxT("Enter frame number (from 1 to %d):"), m_nframes);

s_values.Printf(wxT("%d"), m_iframe);

// Prompt for a new value of iframe with a dialog box:
wxString result = wxGetTextFromUser(s_question, _T("Load new image plane"),
                                   s_values, NULL);
if(!result.IsEmpty()){
  if(result.ToLong(&iframe) == true) {
    if(iframe > 0 && iframe <= m_nframes) {
      m_iframe = iframe;
// change_filename=false  iframe=m_iframe;
      LoadFITSImage(false, m_iframe);
      } else {
      wxLogError(_T("Error/Bad value for iframe"));
      }
  }
}

return;
}
/***********************************************************************
* Select the delay between two frames for video display (for 3D data cubes)
***********************************************************************/
void GdpFrame::OnSetVideoDelay(wxCommandEvent& WXUNUSED(event))
{
wxString s_values, s_question;
long delay0;

s_question = wxT("Enter delay in millisec (range 0, 10000): ");

s_values.Printf(wxT("%d"), m_video_ms_delay);

// Prompt for a new value of iframe with a dialog box:
wxString result = wxGetTextFromUser(s_question, _T("Delay between two frames"),
                                   s_values, NULL);
if (!result.IsEmpty()){
  if(result.ToLong(&delay0) == true) {
    if(delay0 > 0 && delay0 <= 10000) {
      m_video_ms_delay = delay0;
      } else {
      wxLogError(_T("Error/Bad value for delay (ms)"));
      }
  }
}

return;
}
/***********************************************************************
* Start displaying continuously (for 3D data cubes)
***********************************************************************/
void GdpFrame::OnPlayVideo(wxCommandEvent& WXUNUSED(event))
{

if(m_video_timer->IsRunning()) {
  wxLogError(wxT("Error: video already running!"));
  return;
  }

// Start the timer:
m_video_timer->Start(m_video_ms_delay, wxTIMER_CONTINUOUS);

// To direct timer events to this class:
m_video_timer->SetOwner(this, ID_TIMER);

// Update menu buttons (true -> "stop=enabled" and "play=disabled") 
EnableStopVideo(true);

return;
}
/****************************************************************
* Handle timer events, when "video play" button has been pressed 
* (called every m_video_ms_delay milliseconds )
*****************************************************************/
void GdpFrame::OnTimer(wxTimerEvent &WXUNUSED(event))
{
int status = 0;

// change_filename=false  iframe=m_iframe;
if(m_iframe < m_nframes) {
// Display next image
  m_iframe++;
// change_filename=false  iframe=m_iframe;
  status = LoadFITSImage(false, m_iframe);
  if(status) {
    m_video_timer->Stop(); 
    fprintf(stderr, "OnTimer/error loading frame #%d\n", m_iframe);
// Update menu buttons (false -> "stop=disabled" and "play=enabled") 
    EnableStopVideo(false);
    }
  } else {
  m_video_timer->Stop(); 
// Update menu buttons (false -> "stop=disabled" and "play=enabled") 
  EnableStopVideo(false);
  } 

return;
}
/****************************************************************
* To stop video 
*****************************************************************/
void GdpFrame::OnStopVideo(wxCommandEvent& event)
{
 
if(!m_video_timer->IsRunning()) {
  wxLogError(wxT("Error: video already stopped!"));
  } else {
  m_video_timer->Stop();
// Update menu buttons (false -> "stop=disabled" and "play=enabled") 
  EnableStopVideo(false);
  }

return;
}
