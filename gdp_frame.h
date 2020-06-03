/****************************************************************************
* Name: gdp_frame.h GdpFrame class
* Purpose: display and process FITS images obtained with PISCO and PISCO2
*
* JLP
* Version 23/01/2015
****************************************************************************/
#ifndef _gdp_frame__
#define _gdp_frame__
#include <stdlib.h> // exit()

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/tglbtn.h"
#include "wx/bookctrl.h"
#include "wx/imaglist.h"
#include "wx/filename.h"   //wxFileName class
#include "wx/cshelp.h"

#if wxUSE_TOOLTIPS
    #include "wx/tooltip.h"
#endif

#include "jlp_gdev_wxwid.h"  // JLP_GDev_wxWID class
#include "jlp_wxlogbook.h"    // JLP_wxLogbook class
#include "jlp_wx_ipanel.h"    // JLP_wxImagePanel class

// Simple declaration to avoid "boomerang effect"
class Gdp_wx_GDProc2;

//----------------------------------------------------------------------
// class definitions
//----------------------------------------------------------------------

class GdpFrame: public wxFrame
{
public:

// In "Gdpisco.cpp":
    GdpFrame(const wxChar *title, int x, int y);
// Not exiting...
//    ~GdpFrame() {printf("DDestructor called\n"); Destroy(); return;};
    ~GdpFrame() {printf("exit function called\n"); exit(-1);};

    void Gdp_SetupMenu();
    void Gdp_ResetAllOptionsOfImageMenu();
    void OnLoadCurveFromFile(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);
    void EnableSaveBinariesMeasurements(bool flag);
    void JLP_ResetAllOptionsOfImageMenu();
    void EnableStopVideo(bool flag);
    void SetText_to_StatusBar(wxString text, const int icol);
    int LoadNewPointFromCursor(double xx, double yy, double value);

#if wxUSE_TOOLTIPS
    void OnToggleTooltips(wxCommandEvent& event);
#endif // wxUSE_TOOLTIPS

    void OnContextHelp(wxCommandEvent& event);

// In "gdp_frame_video.cpp":
    void OnLoadFITSImage( wxCommandEvent &event);
    void OnNextFrame(wxCommandEvent& event);
    void OnPreviousFrame(wxCommandEvent& event);
    void OnGoToZeroVideo(wxCommandEvent& event);
    void OnGotoFrame(wxCommandEvent& event);
    void OnSetVideoDelay(wxCommandEvent& event);
    void OnPlayVideo(wxCommandEvent& event);
    void OnStopVideo(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);

// in "gdp_frame_logbook.cpp":
    int SaveLogbook(wxString save_filename);
    void ShowLogbook();
    void HideLogbook();
    void ClearLogbook();
    void CleanLogbook();
    int WriteToLogbook(wxString str1, bool SaveToFile);
    int AddNewPointToLogbook(double xx, double yy, double value);
    int BinariesSaveMeasurements();

// In "gdp_frame_menu.cpp":
    void OnSaveToPostscript(wxCommandEvent& event);
    int  LoadFITSImage(const bool change_filename, const int iframe);
    void OnListFromCursor_Start(wxCommandEvent& WXUNUSED(event));
    int ListFromCursor_Start();
    void OnViewLogbook(wxCommandEvent& event);
    void OnSaveLogbook(wxCommandEvent& event);
    void OnClearLogbook(wxCommandEvent& event);
    void OnCleanLogbook(wxCommandEvent& event);
    void OnBinariesStartMeasurement(wxCommandEvent& event);
    int BinariesStartMeasurement1(int processing_mode);
    void OnBinariesSaveMeasurements(wxCommandEvent& event);
    void OnSpeckleModelSubtraction(wxCommandEvent& event);
    void OnSpeckleNoModelSubtraction(wxCommandEvent& event);
    void OnOffsetCorrection(wxCommandEvent& event);
    void OnFlatFieldCorrection(wxCommandEvent& event);
    void OnAutoMeasureBinary(wxCommandEvent& event);
    void OnAutoMeasureHartmann(wxCommandEvent& event);
    void LoadNewWavefrontParameters(double *x_half_width, double *y_half_width,
                                    double *sigma_threshold,
                                    double *pupil_median_frac);

  void Increment_nbinaries_meas(){
    wxString str1;
    nBinariesMeasurements++;
    str1.Printf(wxT("Nmeasures = %d"), nBinariesMeasurements);
    SetStatusText(str1, 1);
// Enable processing if more than 4 measurements:
    if(nBinariesMeasurements >= 4) EnableSaveBinariesMeasurements(true);
    return;
    }

private:
  JLP_wxImagePanel *m_panel;
  wxStatusBar *m_StatusBar;
  int nx1, ny1, initialized;

// Binaries processing:
  Gdp_wx_GDProc2 *m_gdproc2;

// Number of binaries measurements:
  int nBinariesMeasurements;

// Menus:
  wxMenuBar *menu_bar;
  wxMenu *menuFile, *menuProcess, *menuBinaries, *menuHelp;
  wxMenu *menuPrep, *menuPrepOffset, *menuPrepFField, *menuHartmann;
  wxMenu *menuVideo, *menuBinariesMode, *menuSubtractModel;
  wxMenu *menuBinariesMeas, *menuAutoMeasure;
  wxBoxSizer  *m_topsizer;
  wxString    m_filename1, m_full_filename1;
  int m_iframe, m_nframes;

  wxTimer *m_video_timer;
  int m_video_ms_delay;

// Logbook:
  wxMenu     *menuLog;
  wxString    m_Logbook;
  JLP_wxLogbook *LogPanel;

  DECLARE_EVENT_TABLE()
};

#endif
