/****************************************************************************
* Name: Gdpisco.cpp GdpFrame class
* Purpose: display and process FITS images obtained with PISCO and PISCO2
*
* Derived from Gdisp1.cpp
*
* JLP
* Version 05/11/2022
****************************************************************************/
#include <stdlib.h>   // exit()
#include "time.h"

// JLP routines:
#include "gdp_frame.h"
#include "jlp_wx_ipanel.h"   // JLP_wxImagePanel class
#include "gdp_gdproc2.h"  // Gdp_wx_GDProc2 class

#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__) || defined(__WXMGL__) || defined(__WXX11__)
    #define USE_XPM
#endif

#ifdef USE_XPM
    #include "mondrian.xpm"
#endif

#include "wx/progdlg.h"


#if !wxUSE_TOGGLEBTN
    #define wxToggleButton wxCheckBox
    #define EVT_TOGGLEBUTTON EVT_CHECKBOX
#endif

// Definition of identifiers in "gdp_frame_id.h"
#include "gdp_frame_id.h"

BEGIN_EVENT_TABLE(GdpFrame, wxFrame)

// Menu items:
   EVT_MENU(ID_SAVE_TO_PST,    GdpFrame::OnSaveToPostscript)
   EVT_MENU(ID_LOAD_FITS,      GdpFrame::OnLoadFITSImage)
   EVT_MENU(ID_QUIT,           GdpFrame::OnQuit)

// Preprocessing:
   EVT_MENU (ID_OFFSET_CORR_RAW, GdpFrame::OnOffsetCorrection)
   EVT_MENU (ID_OFFSET_CORR_POSITIVE, GdpFrame::OnOffsetCorrection)
   EVT_MENU (ID_FFIELD_CORR_RAW, GdpFrame::OnFlatFieldCorrection)
   EVT_MENU (ID_FFIELD_CORR_ONE_SIGMA, GdpFrame::OnFlatFieldCorrection)
   EVT_MENU (ID_FFIELD_CORR_TWO_SIGMA, GdpFrame::OnFlatFieldCorrection)
   EVT_MENU (ID_FFIELD_CORR_THREE_SIGMA, GdpFrame::OnFlatFieldCorrection)

// Binaries
   EVT_MENU (ID_BINARIES_MODSUB,  GdpFrame::OnSpeckleModelSubtraction)
   EVT_MENU (ID_BINARIES_NOMODSUB,  GdpFrame::OnSpeckleNoModelSubtraction)

   EVT_MENU (ID_BINARIES_RESET,
             GdpFrame::OnBinariesStartMeasurement)
   EVT_MENU (ID_BINARIES_SIMPLE_MEAS,
             GdpFrame::OnBinariesStartMeasurement)
   EVT_MENU (ID_BINARIES_DOUBLE_MEAS,
             GdpFrame::OnBinariesStartMeasurement)
   EVT_MENU (ID_BINARIES_DMAG_MEAS,
             GdpFrame::OnBinariesStartMeasurement)
   EVT_MENU (ID_BINARIES_TWOSTARS_MEAS,
             GdpFrame::OnBinariesStartMeasurement)
// For later: 
#ifdef AUTOMT
   EVT_MENU (ID_BINARIES_AUTO_MEAS,
             GdpFrame::OnBinariesStartMeasurement)
#endif
   EVT_MENU (ID_BINARIES_SAVE_MEAS, GdpFrame::OnBinariesSaveMeasurements)

// Logbook (from menu):
   EVT_MENU(ID_LOGBOOK_SHOW, GdpFrame::OnViewLogbook)
   EVT_MENU(ID_LOGBOOK_HIDE, GdpFrame::OnViewLogbook)
   EVT_MENU (ID_CURSOR_LIST_START,  GdpFrame::OnListFromCursor_Start)
   EVT_MENU(ID_LOGBOOK_CLEAN, GdpFrame::OnCleanLogbook)
   EVT_MENU(ID_LOGBOOK_CLEAR, GdpFrame::OnClearLogbook)
   EVT_MENU(ID_LOGBOOK_SAVE, GdpFrame::OnSaveLogbook)

// 3D-utilities
   EVT_MENU(ID_NEXT_FRAME, GdpFrame::OnNextFrame)
   EVT_MENU(ID_PREVIOUS_FRAME, GdpFrame::OnPreviousFrame)
   EVT_MENU(ID_GOTO_ZERO_VIDEO, GdpFrame::OnGoToZeroVideo)
   EVT_MENU(ID_GOTO_FRAME, GdpFrame::OnGotoFrame)
   EVT_MENU(ID_DELAY_VIDEO, GdpFrame::OnSetVideoDelay)
   EVT_MENU(ID_PLAY_VIDEO, GdpFrame::OnPlayVideo)
   EVT_MENU(ID_STOP_VIDEO, GdpFrame::OnStopVideo)
   EVT_TIMER(ID_TIMER, GdpFrame::OnTimer)

// Automatic measurements
   EVT_MENU(ID_AUTO_MEASURE_HARTMANN, GdpFrame::OnAutoMeasureHartmann)

// Miscellaneous:
   EVT_MENU(ID_CONTEXT_HELP,   GdpFrame::OnContextHelp)
   EVT_MENU(ID_ABOUT,          GdpFrame::OnAbout)
   EVT_MENU(ID_HELP,           GdpFrame::OnHelp)

END_EVENT_TABLE()

//----------------------------------------------------------------------
// MyApp
//----------------------------------------------------------------------

class MyApp: public wxApp
{
public:
   bool OnInit();
};

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
// Transform coma into point for numbers:
setlocale(LC_NUMERIC, "C");

    // use standard command line handling:
    if ( !wxApp::OnInit() )
        return false;

    // parse the cmd line
    int xx = 1000, yy = 800;
    if ( argc == 3 )
    {
        wxSscanf(wxString(argv[1]), wxT("%d"), &xx);
        wxSscanf(wxString(argv[2]), wxT("%d"), &yy);
    }

#if wxUSE_HELP
    wxHelpProvider::Set( new wxSimpleHelpProvider );
#endif // wxUSE_HELP

// Create the main frame window
    GdpFrame *frame = new GdpFrame(_T("Gdpisco"), xx, yy);

// Give it an icon
// The wxICON() macros loads an icon from a resource under Windows
// and uses an #included XPM image under GTK+ and Motif
#ifdef USE_XPM
    frame->SetIcon( wxICON(mondrian) );
#endif

    frame->Show(true);

    return true;
}
/**********************************************************************
* GdpFrame constructor
*
* INPUT:
*   xx,yy : size of created window
*
***********************************************************************/
GdpFrame::GdpFrame(const wxChar *title, int xx, int yy)
       : wxFrame(NULL, wxID_ANY, title, wxPoint(-1, -1), wxSize(xx, yy))
{
wxString str1;
int should_fit_inside;

m_StatusBar = NULL;
menuVideo = NULL;
m_video_timer = NULL;
nBinariesMeasurements = 0;
m_full_filename1 = wxEmptyString;
// printf("m_f=>%s<\n", (const char *)m_full_filename1.mb_str());

// Status bar:
// Create a status bar with two fields at the bottom:
  m_StatusBar = CreateStatusBar(2);
// First field has a variable length, second has a fixed length:
  int widths[2];
  widths[0] = -1;
  widths[1] = 200;
  SetStatusWidths( 2, widths );

// Create topsizer to locate panels and log window
m_topsizer = new wxBoxSizer( wxVERTICAL );

// Create Logbook first:
  str1 = wxString("");
  LogPanel = new JLP_wxLogbook(this, str1, 100, 60);

  wxLog::SetActiveTarget( new wxLogTextCtrl(LogPanel) );



// Then create the panel with the scrolled bars:
should_fit_inside = 0;
m_panel = new JLP_wxImagePanel(this, LogPanel, m_StatusBar,
                               20, 20, xx-40, yy-40, should_fit_inside);

// Interactive processing:
m_gdproc2 = NULL;

// The initial size of m_scrolled1 is interpreted as the minimal size:
// 1 : make vertically stretchable
// wxEXPAND : make horizontally stretchable, and the item will be expanded
// to fill the space assigned to the item.
// Proportion set to 5, i.e., log window will be 1/6 of the image
if(m_panel != NULL)  m_topsizer->Add(m_panel, 5, wxEXPAND | wxALL);

// Create text ctrl with minimal size 100x60:
/*
  LogPanel = new wxTextCtrl( this, wxID_ANY, _T(""),
                         wxDefaultPosition, wxSize(100,60),
                         wxTE_MULTILINE | wxTE_READONLY );
*/
// Proportion set to 1, i.e., log window will be 1/6 of the image
  m_topsizer->Add( LogPanel, 1, wxEXPAND );

/*
// Min and Max size:
int iwidth, iheight;
  iwidth = xx;
  iheight = yy;
  my_init_size = wxSize(iwidth + 5, (iheight*6)/5 + 10);
  SetClientSize(my_init_size);
// To allow to see the menu and to enlarge the window after zooming:
  my_max_size = wxSize(max_size, max_size);
  SetMaxSize(my_max_size);
  SetMinSize(wxSize(min_size, min_size) + GetWindowBorderSize());
  SetSizer(m_topsizer);
*/

SetSizerAndFit(m_topsizer);

// Create a menu on top of the window:
  Gdp_SetupMenu();

// Timer for the video:
m_video_timer = new wxTimer(this, ID_TIMER);
// Set delay to obtain a rate of 10 images/second:
m_video_ms_delay = 100;

initialized = 1234;

// By default (JLP2022), hide it to have a large screen:
// Should be called only after initialized has been set to 1234 !
HideLogbook();

return;
}

/********************************************************************
* Setup the menu on top of main frame
********************************************************************/
void GdpFrame::Gdp_SetupMenu()
{
SetHelpText( _T("Program to plot images from FITS files") );

  menu_bar = new wxMenuBar;

// ***************** File menu **********************************
  menuFile = new wxMenu;

  menuFile->Append(ID_LOAD_FITS, _T("Open a FITS file"),
                    _T("FITS file may be 2D or 3D"));
  menuFile->AppendSeparator();
  menuFile->Append( ID_SAVE_TO_PST, _T("Save to postscript file"),
                    _T("Postscript file"));
  menuFile->AppendSeparator();

  menuFile->Append(ID_QUIT, _T("E&xit\tAlt-X"), _T("Quit program"));
  menu_bar->Append(menuFile, _T("&File"));

// ***************** Preprocessing menu ******************************
  menuPrep = new wxMenu;

// Offset submenu:
  menuPrepOffset = new wxMenu;
  menuPrepOffset->AppendRadioItem(ID_OFFSET_CORR_RAW,
                                  wxT("Raw offset correction"),
                                  wxT("No constraint on the result"));
  menuPrepOffset->AppendRadioItem(ID_OFFSET_CORR_POSITIVE,
                                  wxT("Tuned offset correction"),
                                  wxT("Positive constraint on the result"));
  menuPrep->Append(wxID_ANY, wxT("Offset correction"), menuPrepOffset,
                       wxT("To remove offset from input image"));

// Flat field submenu:
  menuPrepFField = new wxMenu;
  menuPrepFField->AppendRadioItem(ID_FFIELD_CORR_RAW,
                                  wxT("Raw flat field correction"),
                                  wxT("No constraint on the flat field"));
  menuPrepFField->AppendRadioItem(ID_FFIELD_CORR_ONE_SIGMA,
                                 wxT("One-sigma flat field correction"),
                                 wxT("One-sigma constraint on the flat field"));
  menuPrepFField->AppendRadioItem(ID_FFIELD_CORR_TWO_SIGMA,
                                 wxT("Two-sigma flat field correction"),
                                 wxT("Two-sigma constraint on the flat field"));
  menuPrepFField->AppendRadioItem(ID_FFIELD_CORR_THREE_SIGMA,
                               wxT("Three-sigma flat field correction"),
                               wxT("Three-sigma constraint on the flat field"));
  menuPrep->Append( wxID_ANY, _T("FlatField correction"), menuPrepFField,
                       wxT("Divide input image by flat field"));

  menu_bar->Append(menuPrep, _T("Preprocessing"));

// ***************** Binaries menu ******************************
  menuBinaries = new wxMenu;

  menuSubtractModel = new wxMenu;
  menuSubtractModel->Append( ID_BINARIES_MODSUB, _T("Model subtraction"),
                       wxT("Useful for close binaries"), wxITEM_RADIO);
  menuSubtractModel->Append( ID_BINARIES_NOMODSUB, _T("No subtraction"),
                       wxT("Back to original"), wxITEM_RADIO);
  menuBinaries->Append(wxID_ANY, wxT("Subtract Model"),
                       menuSubtractModel, _T("Useful to see details in the center"));
  menuBinaries->AppendSeparator();

  menuBinariesMeas = new wxMenu;
  menuBinariesMeas->Append(ID_BINARIES_RESET,
                         _T("Reset"),
                         wxT("Erase all previous measurements (if any)"));
  menuBinariesMeas->Append(ID_BINARIES_SIMPLE_MEAS, _T("Start autoc. simple measurements"),
                          wxT("One speckle measurement inside of a circle"), wxITEM_CHECK);
  menuBinariesMeas->Append(ID_BINARIES_DOUBLE_MEAS,
                         _T("Start autoc. double symmetric measurement mode"),
                         wxT("Two speckle measurements with one circle and its symmetric"),
                         wxITEM_CHECK);
  menuBinariesMeas->Append(ID_BINARIES_DMAG_MEAS,
                         _T("Start DVA double measurement mode (for Dmag)"),
                         wxT("Two speckle measurements with one circle and its symmetric (for DVA autocorrelations)"),
                         wxITEM_CHECK);
  menuBinariesMeas->Append(ID_BINARIES_TWOSTARS_MEAS,
                         _T("Start long exp. two stars (Lucky imaging) measurement mode"),
                         wxT("Measurements of Lucky imag. with 3 circles: two stars and background"),
                         wxITEM_CHECK);
// For later:
#ifdef AUTOMT
  menuBinariesMeas->Append(ID_BINARIES_AUTO_MEAS,
                         _T("Start automatic measurement mode"),
                         wxT("Automatic speckle measurements"),
                         wxITEM_CHECK);
#endif
  menuBinaries->Append(wxID_ANY, wxT("Measurement mode selection"),
                       menuBinariesMeas,
                       _T("Select measurement mode"));
  menuBinaries->AppendSeparator();
  menuBinaries->Append( ID_BINARIES_SAVE_MEAS, _T("Save measurements"),
                       wxT("To process and save the measurements"));
  menu_bar->Append(menuBinaries, _T("Binaries"));

menuBinaries->Check( ID_BINARIES_SIMPLE_MEAS, false);
menuBinaries->Check( ID_BINARIES_DOUBLE_MEAS, false);
menuBinaries->Check( ID_BINARIES_DMAG_MEAS, false);
menuBinaries->Check( ID_BINARIES_TWOSTARS_MEAS, false);
// For later:
#ifdef AUTOMT
menuBinaries->Check( ID_BINARIES_AUTO_MEAS, false);
#endif
menuBinaries->Enable( ID_BINARIES_SAVE_MEAS, false);

// ***************** Logbook menu ******************************
  menuLog = new wxMenu;
  menuLog->Append( ID_LOGBOOK_SHOW, _T("Show logbook"),
                       wxT("Display the logbook window"), wxITEM_RADIO);
  menuLog->Append( ID_LOGBOOK_HIDE, _T("Hide logbook"),
                       wxT("Hide the logbook window"), wxITEM_RADIO);
  menuLog->Append( ID_CURSOR_LIST_START, _T("List of points (start)"),
                       wxT("Individual points entered with the mouse"));
  menuLog->Append( ID_LOGBOOK_CLEAR, _T("Clear the logbook"),
                       wxT("Clear the logbook content"));
  menuLog->Append( ID_LOGBOOK_CLEAN, _T("Clean the logbook"),
                       wxT("Clean the logbook content"));
  menuLog->Append( ID_LOGBOOK_SAVE, _T("Save cleaned logbook"),
                       wxT("Save a selection from the logbook content"));
  menu_bar->Append(menuLog, _T("Logbook"));

// ***************** Video menu ******************************
  menuVideo = new wxMenu;
  menuVideo->Append( ID_NEXT_FRAME, _T("Next frame"),
                       wxT("Display next plane in 3D data cube"));
  menuVideo->Append( ID_PREVIOUS_FRAME, _T("Previous frame"),
                       wxT("Display previous plane in 3D data cube"));
  menuVideo->Append( ID_GOTO_ZERO_VIDEO, _T("Go to frame #1"),
                       wxT("Go to first image"));
  menuVideo->Append( ID_GOTO_FRAME, _T("Go to frame #"),
                       wxT("Go to a given plane in 3D data cube"));
  menuVideo->Append( ID_DELAY_VIDEO, _T("Set video delay"),
                       wxT("Define the delay between two frames"));
  menuVideo->Append( ID_PLAY_VIDEO, _T("Play"),
                       wxT("Start video playing"));
  menuVideo->Append( ID_STOP_VIDEO, _T("Stop"),
                       wxT("Stop video playing"));
  menu_bar->Append(menuVideo, _T("Video"));

// ***************** AutoMeasure menu ******************************
  menuAutoMeasure = new wxMenu;
  menuAutoMeasure->Append( ID_AUTO_MEASURE_BINARY,
                           _T("Auto-measure of binaries"),
                  wxT("Automatic measure of binaries (for autocorrelations)"));
  menuAutoMeasure->Append( ID_AUTO_MEASURE_HARTMANN,
                           _T("Auto-measure of wavefront"),
                  wxT("Automatic measure of phase wavefront (Shack-Hartmann)"));
  menu_bar->Append(menuAutoMeasure, _T("Automatic measurements"));

// ***************** Help menu ******************************
  menuHelp = new wxMenu;
  menuHelp->Append( ID_HELP, _T("Help"));
  menuHelp->Append(ID_CONTEXT_HELP, _T("&Context help...\tCtrl-H"),
                     _T("Get context help for a control"));
  menuHelp->Append( ID_ABOUT, _T("&About..."));
  menu_bar->Append(menuHelp, _T("Help"));

  SetMenuBar(menu_bar);

Gdp_ResetAllOptionsOfImageMenu();

return;
}
/******************************************************************
* Reset options to default (called for first initialisation
* and after loading a new FITS file
******************************************************************/
void GdpFrame::Gdp_ResetAllOptionsOfImageMenu()
{
  if(menuBinaries) {
    menuBinaries->Check(ID_BINARIES_NOMODSUB, true);
    EnableSaveBinariesMeasurements(false);
    menuBinaries->Check( ID_BINARIES_SIMPLE_MEAS, false);
    menuBinaries->Check( ID_BINARIES_DOUBLE_MEAS, false);
    menuBinaries->Check( ID_BINARIES_DMAG_MEAS, false);
    menuBinaries->Check( ID_BINARIES_TWOSTARS_MEAS, false);
#ifdef AUTOMT
    menuBinaries->Check( ID_BINARIES_AUTO_MEAS, false);
#endif
    menuBinaries->Enable( ID_BINARIES_SAVE_MEAS, false);
  }

return;
}


void GdpFrame::OnQuit (wxCommandEvent& event)
{
printf("OnQuit: calling Close\n");
    Close(true);
// event.Skip();
}

/*****************************************************************
* Help
*****************************************************************/
void GdpFrame::OnHelp( wxCommandEvent& WXUNUSED(event) )
{
 (void)wxMessageBox(_T("Sorry: \"Help\" is not implemented yet\n")
                    _T("Current version: Nov 5th 2022"),
                    _T("Gdpisco"),
                     wxICON_INFORMATION | wxOK );
}
/*****************************************************************
* About
*****************************************************************/
void GdpFrame::OnAbout( wxCommandEvent& WXUNUSED(event) )
{
 (void)wxMessageBox( _T("Gdpisco\n")
                     _T("Jean-Louis Prieur (c) 2022\n")
                     _T("Created with wxWidgets"), _T("Gdpisco"),
                     wxICON_INFORMATION | wxOK );
}
/*****************************************************************
* Context help
*****************************************************************/
void GdpFrame::OnContextHelp(wxCommandEvent& WXUNUSED(event))
{
    // starts a local event loop
    wxContextHelp chelp(this);
}
/************************************************************************
* Display text in status bar
*************************************************************************/
void GdpFrame::SetText_to_StatusBar(wxString str1, const int icol)
{
// Update the first field (since 2nd argument is 0 here) of the status bar:
  if(m_StatusBar != NULL) m_StatusBar->SetStatusText(str1, icol);
}
/************************************************************************
* Add a new point to logbook
*************************************************************************/
int GdpFrame::LoadNewPointFromCursor(double xx, double yy, double value)
{
int status;
wxString str1;

 str1.Printf("%.2f %.2f %.4g\n", xx, yy, value);

 status = LogPanel->WriteToLogbook(str1, true);

return(status);
}
/******************************************************************
* Enable processing and saving binary measurements
* or disable it if needed
*******************************************************************/
void GdpFrame::EnableSaveBinariesMeasurements(bool flag)
{
if(menuBinaries != NULL) {
 if(menuBinaries->FindItem(ID_BINARIES_SAVE_MEAS) != NULL)
          menuBinaries->Enable( ID_BINARIES_SAVE_MEAS, flag);
}
return;
}
/******************************************************************
* Reset options to default (called for first initialisation
* and after loading a new FITS file
******************************************************************/
void GdpFrame::JLP_ResetAllOptionsOfImageMenu()
{
  if(menuBinaries) menuBinaries->Check(ID_BINARIES_NOMODSUB, true);

  if(menuBinaries) menuBinaries->Check(ID_BINARIES_NOMODSUB, true);

  EnableSaveBinariesMeasurements(false);

return;
}
/******************************************************************
* Update menu Stop and Play buttons
*  if true: ("stop=enabled" and "play=disabled")
*  if false: ("stop=disabled" and "play=enabled")
*
*******************************************************************/
void GdpFrame::EnableStopVideo(bool flag)
{
if(menuVideo == NULL) return;

 if(menuVideo->FindItem(ID_STOP_VIDEO) != NULL)
      menuVideo->Enable(ID_STOP_VIDEO, flag);

 if(menuVideo->FindItem(ID_PLAY_VIDEO) != NULL)
       menuVideo->Enable(ID_PLAY_VIDEO, !flag);

return;
}
