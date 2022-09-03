/******************************************************************************
* Name:        gdp_frame_menu (GdpFrame class)
* Purpose:     handling menu events of GdpFrame clas (see Gdpisco.cpp)
* Author:      JLP
* Version:     23/05/2016
******************************************************************************/
#include "gdp_frame.h"
#include "jlp_wx_ipanel.h"      // JLP_wxImagePanel class
#include "gdp_frame_id.h"       // Menu identifiers
#include "gdp_gdproc2.h"       // Gpd_wx_GDProc2 class
#include "jlp_fitsio.h"         // for JLP_RDFITS_2D
#include "jlp_numeric.h"        // JLP_QSORT_DOUBLE

/* Declared in gdp_frame.h

void OnSaveToPostscript(wxCommandEvent &WXUNUSED(event))
int LoadFITSImage(const bool change_filename, const int iframe)
void OnListFromCursor_Start(wxCommandEvent& WXUNUSED(event))
void OnViewLogbook(wxCommandEvent& event)
void OnSaveLogbook(wxCommandEvent& WXUNUSED(event));
void OnClearLogbook(wxCommandEvent& event);
void OnCleanLogbook(wxCommandEvent& event);
void OnBinariesStartMeasurement(wxCommandEvent& event)
void OnBinariesSaveMeasurements(wxCommandEvent& event);
void OnSpeckleModelSubtraction(wxCommandEvent& event);
void OnSpeckleNoModelSubtraction(wxCommandEvent& event);
void OnOffsetCorrection(wxCommandEvent& event);
void OnFlatFieldCorrection(wxCommandEvent& event);

*/

/************************************************************************
* Output curve as a postscript file
************************************************************************/
void GdpFrame::OnSaveToPostscript(wxCommandEvent &WXUNUSED(event))
{
wxString filename;
char input_filename[128], pst_filename[128];


if((initialized != 1234) || (m_panel == NULL)) return;

 strcpy(input_filename, m_filename1.mb_str());

 wxFileDialog dialog(NULL, _T("Save to postscript file"), wxEmptyString,
                     wxEmptyString, _T("Files (*.ps)|*.ps"),
                     wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
 if (dialog.ShowModal() != wxID_OK) return;

 filename = dialog.GetPath();
 strcpy(pst_filename, filename.mb_str());

 m_panel->PstCopyOfDisplay(input_filename, pst_filename);

return;
}
/******************************************************************
* Load an image in a 2DFITS file or a single plane in 3D FITS cube
*
* INPUT:
* change_filename: true if a new filename is required
*                  false if the old filename is to be used (in case of 3D data)
* iframe: index of image plane to be loaded from 1 to nz (in case of 3D data)
*******************************************************************/
int GdpFrame::LoadFITSImage(const bool change_filename, const int iframe)
{
double *dble_image0;
int nx0, ny0, nz0;
wxString full_filename, str1, path, extension, fname;
char filename1[120], comments1[80], errmess1[200];
int status;

if((initialized != 1234) || (m_panel == NULL)) return(-1);

// According to "change_file" or not:
// Prompt the user for the FITS filename, if not set by the calling routine:
if(change_filename) {
  full_filename = wxFileSelector(_T("Select image FITS file"), _T(""), _T(""),
                      _T("fits|fit|FIT"),
                      _T("FITS files (*.fits;*.fit;*.FIT)|*.fits;*.fit;*.FIT"));

  if ( full_filename.empty() ) return(-1);
  m_full_filename1 = full_filename;
}

// dble_image1: array with the data contained in FITS file
// nx0, ny0, nz0: size of data cube
// iframe: index of image plane to be loaded from 1 to nz (in case of 3D data)


// Load a new image or image plane in FITS format
strncpy(filename1, m_full_filename1.mb_str(), 120);
status = JLP_RDFITS_2D_dble(&dble_image0, &nx0, &ny0, &nz0, iframe, filename1,
                             comments1, errmess1);
if (status) {
  fprintf(stderr, "Error loading image from %s (status=%d)\n %s\n",
          filename1, status, errmess1);
  wxLogError(_T("Couldn't load image from '") + m_full_filename1 + _T("'."));
  return(-2);
  }
/* TOBEDONE later: use (in gdp_utils.cpp)
 int gdp_LoadFITSImage(wxString &m_full_filename1, double **dble_image1,
                        int *nx1, int *ny1, int *nz1, int iframe)
*/


/***************************************************************/
if(!change_filename) {
  if(nx0 != nx1 || ny0 != ny1) {
    wxLogError(wxT("Error: incompatible size of new image plane"));
    return(-1);
    }
// Save current size for further comparison:
  } else {
    nx1 = nx0;
    ny1 = ny0;
  }

// Save current value of iframe and nz0 to private variables:
m_iframe = iframe;
m_nframes = nz0;

m_panel->wxIP_LoadImage(dble_image0, nx0, ny0);

// Reset image menu options:
Gdp_ResetAllOptionsOfImageMenu();

// Prepare interactive processing for binaries measurments:
if(change_filename) {
// New interactive processing setup:
  if(m_gdproc2 != NULL) delete m_gdproc2;
  m_gdproc2 = new Gdp_wx_GDProc2(this, m_panel);
  }

// Reset the number of measurements:
nBinariesMeasurements = 0;

// Removes the directory name (since the full path is generally too long...)
  wxFileName::SplitPath(m_full_filename1, &path, &fname, &extension);
  m_filename1 = fname + _T(".") + extension;

// Displays the filename on the status bar 
 str1 = _T("File ") + m_filename1 + _T(" successfuly loaded");
 SetStatusText(str1, 0);

 SetTitle(m_full_filename1);

//******************************************************************
// Logbook:
if(change_filename) {

// Clear logbook (to prevent problems with speckle measurements)
  LogPanel->Clear();

// Update logbook:
    str1 = _T("%%\n%% Input file: ") + m_filename1 + _T("\n");
    WriteToLogbook(str1, true);
} // if change_filename
//******************************************************************

// If 3D fits, write the frame number to the second field of the status bar:
if(m_nframes > 1)
  str1.Printf(wxT("Frame #%d/%d"), m_iframe, m_nframes);
// Otherwise display the number of measurements:
else
  str1.Printf(wxT("Nmeasures = %d"), nBinariesMeasurements);

SetStatusText(str1, 1);

// Enable logbook menu:
if(m_nframes > 1)
  menu_bar->EnableTop( menu_bar->FindMenu(_T("Video")), true);
else
  menu_bar->EnableTop( menu_bar->FindMenu(_T("Video")), false);

return(0);
}
/**************************************************************************
* Binaries measurement: astrometry with polynomial (or profile) fit
* of the background
*
  ID_BINARIES_SIMPLE_MEAS,
  ID_BINARIES_DOUBLE_MEAS,
  ID_BINARIES_TWOSTARS_MEAS,
  ID_BINARIES_AUTO_MEAS

* InteractiveProcessingMode :
*-1=None
* 0=Interactive input of points
* 1=auto-correlation simple measurement (Speckle)
* 2=autocorrelation double measurement (speckle or DVA)
* 3=two-star measurement (long integration, e.g., Lucky Imaging)
* 4=automatic speckle measurement
**************************************************************************/
void GdpFrame::OnBinariesStartMeasurement(wxCommandEvent& event)
{

if(initialized != 1234) return;

// Uncheck all options:
 if(menuBinaries->FindItem(ID_BINARIES_SIMPLE_MEAS) != NULL)
         menuBinaries->Check( ID_BINARIES_SIMPLE_MEAS, false);
 if(menuBinaries->FindItem(ID_BINARIES_DOUBLE_MEAS) != NULL)
         menuBinaries->Check( ID_BINARIES_DOUBLE_MEAS, false);
 if(menuBinaries->FindItem(ID_BINARIES_TWOSTARS_MEAS) != NULL)
         menuBinaries->Check( ID_BINARIES_TWOSTARS_MEAS, false);
 if(menuBinaries->FindItem(ID_BINARIES_AUTO_MEAS) != NULL)
         menuBinaries->Check( ID_BINARIES_AUTO_MEAS, false);
 if(menuBinaries->FindItem(ID_BINARIES_SAVE_MEAS) != NULL)
         menuBinaries->Enable( ID_BINARIES_SAVE_MEAS, false);

switch (event.GetId()) {
 case ID_BINARIES_RESET:
   BinariesStartMeasurement1(0);
   break;
 case ID_BINARIES_SIMPLE_MEAS:
   BinariesStartMeasurement1(1);
   if(menuBinaries->FindItem(ID_BINARIES_SIMPLE_MEAS) != NULL)
           menuBinaries->Check( ID_BINARIES_SIMPLE_MEAS, true);
   break;
 case ID_BINARIES_DOUBLE_MEAS:
   BinariesStartMeasurement1(2);
   if(menuBinaries->FindItem(ID_BINARIES_DOUBLE_MEAS) != NULL)
           menuBinaries->Check( ID_BINARIES_DOUBLE_MEAS, true);
   break;
 case ID_BINARIES_TWOSTARS_MEAS:
   BinariesStartMeasurement1(3);
   if(menuBinaries->FindItem(ID_BINARIES_TWOSTARS_MEAS) != NULL)
           menuBinaries->Check( ID_BINARIES_TWOSTARS_MEAS, true);
   break;
 case ID_BINARIES_AUTO_MEAS:
   BinariesStartMeasurement1(4);
   if(menuBinaries->FindItem(ID_BINARIES_AUTO_MEAS) != NULL)
           menuBinaries->Check( ID_BINARIES_AUTO_MEAS, true);
   break;
}
return;
}
/***********************************************************************
* Activate interactive processing for binary measurements
*
* Options
* 0=Reset
* 1=auto-correlation simple measurement (Speckle)
* 2=autocorrelation double measurement (speckle or DVA)
* 3=two-star measurement (long integration, e.g., Lucky Imaging)
* 4=automatic measurement
***********************************************************************/
int GdpFrame::BinariesStartMeasurement1(int ioption)
{
wxString str1;
int status = -1;
int processing_mode0;

if((initialized != 1234) || (m_panel == NULL)) return(-1);

// Reset nBinariesMeasurements
nBinariesMeasurements = 0;
// Reset other measurements in logbook
ClearLogbook();

JLP_GDev_wxWID *Image1_gdev;
/*
* InteractiveProcessingMode :
*-1=None
* 0=Interactive input of points
* 1=auto-correlation simple measurement (Speckle)
* 2=autocorrelation double measurement (speckle or DVA)
* 3=two-star measurement (long integration, e.g., Lucky Imaging)
* 4=automatic speckle measurement
*/
 switch(ioption) {
  default:
  case 0:
   processing_mode0 = -1;
   ClearLogbook();
   break;
  case 1:
  case 2:
  case 3:
  case 4:
   processing_mode0 = ioption;
   break;
 }

 if(m_gdproc2 != NULL) {
   Image1_gdev = m_panel->Image_gdev();
   if( Image1_gdev != NULL) {
// Cancel internal processing mode:
    status = Image1_gdev->SetExternalProcessingMode((JLP_wx_GDProc *)m_gdproc2);
// Set processing mode:
    m_gdproc2->SetNewProcessingMode(processing_mode0);
    }
 }

// Write current value of the number of measurements to the status bar:
// Need to refresh here since default is coordinate display
 str1.Printf(wxT("Nmeasures = %d"), nBinariesMeasurements);
 SetStatusText(str1, 1);

return(status);
}
/**************************************************************************
* Binaries: process and saving measurements to Latex file
**************************************************************************/
void GdpFrame::OnBinariesSaveMeasurements(wxCommandEvent& WXUNUSED(event))
{
int status;

if(initialized != 1234) return;

 status = BinariesSaveMeasurements();
 if(status == 0 && menuBinaries->FindItem(ID_BINARIES_SAVE_MEAS) != NULL) {
   menuBinaries->Enable( ID_BINARIES_SAVE_MEAS, false);
   if(menuBinaries->FindItem(ID_BINARIES_SIMPLE_MEAS) != NULL)
           menuBinaries->Check( ID_BINARIES_SIMPLE_MEAS, false);
   if(menuBinaries->FindItem(ID_BINARIES_DOUBLE_MEAS) != NULL)
           menuBinaries->Check( ID_BINARIES_DOUBLE_MEAS, false);
   if(menuBinaries->FindItem(ID_BINARIES_TWOSTARS_MEAS) != NULL)
           menuBinaries->Check( ID_BINARIES_TWOSTARS_MEAS, false);
   if(menuBinaries->FindItem(ID_BINARIES_AUTO_MEAS) != NULL)
           menuBinaries->Check( ID_BINARIES_AUTO_MEAS, false);
 }
return;
}
/************************************************************************
* List from cursor
* Series of points interactively selected by the user
* until he clicks on the "End" button
************************************************************************/
void GdpFrame::OnListFromCursor_Start(wxCommandEvent& WXUNUSED(event))
{

#if 0   // Oldies...
int i, nn;

if(initialized != 1234) return;

if(menu_bar != NULL) {

// Invalidate all other items of the menu,
// to prevent another task to be started:
  nn = menu_bar->GetMenuCount();
//  printf("Menu_count=%d\n", nn);
  for(i = 0; i < nn; i++) {
//    printf("Menu label(%d) = %s\n", i, menu_bar->GetMenuLabel(i).mb_str());
    menu_bar->EnableTop(i, false);
    }

// Enable logbook menu:
  menu_bar->EnableTop( menu_bar->FindMenu(_T("Logbook")), true);
  if(menuLog->FindItem(ID_CURSOR_LIST_START) != NULL)
        menuLog->Enable( ID_CURSOR_LIST_START, false);
  if(menuLog->FindItem(ID_CURSOR_LIST_END) != NULL)
        menuLog->Enable( ID_CURSOR_LIST_END, true);
  menu_bar->Refresh();
}
#endif

// Now only call next routine:
 ListFromCursor_Start();

return;
}
/**********************************************************************
* Handling ListFromCursor_Start in JLP_wxImagePanel class
*
* InteractiveProcessingMode :
*-1=None
* 0=Interactive input of points
* 1=auto-correlation simple measurement (Speckle)
* 2=autocorrelation double measurement (speckle or DVA)
* 3=two-star measurement (long integration, e.g., Lucky Imaging)
* 4=automatic speckle measurement
***********************************************************************/
int GdpFrame::ListFromCursor_Start()
{
int status = -1, processing_mode = 0;
JLP_GDev_wxWID *Image1_gdev;

if((initialized != 1234) || (m_panel == NULL)) return(-1);

 if(m_gdproc2 != NULL) {
  Image1_gdev = m_panel->Image_gdev();
  if( Image1_gdev != NULL) {
// Get values of limits_box_type and n_points_required:
  m_gdproc2->SetNewProcessingMode(processing_mode);

// Set external processing mode:
  status = Image1_gdev->SetExternalProcessingMode((JLP_wx_GDProc *)m_gdproc2);
  }
 }

return(status);
}
/************************************************************************
* Showing/hiding logbook panel
************************************************************************/
void GdpFrame::OnViewLogbook(wxCommandEvent& event)
{
if(initialized != 1234) return;

  switch (event.GetId())
  {
   case ID_LOGBOOK_SHOW:
     ShowLogbook();
     break;
   case ID_LOGBOOK_HIDE:
     HideLogbook();
     break;
   }
}
/************************************************************************
* Save useful content of logbook to file
************************************************************************/
void GdpFrame::OnSaveLogbook(wxCommandEvent& WXUNUSED(event))
{
wxString save_filename;

if(initialized != 1234) return;

// Select name for output logbook file:
wxFileDialog
saveFileDialog(this, wxT("Save logbook to file"), wxT(""), wxT(""),
               wxT("Logbook files (*.log;*.txt)|*.log;*.txt"),
               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

 if (saveFileDialog.ShowModal() == wxID_CANCEL) return;

save_filename = saveFileDialog.GetFilename();

SaveLogbook(save_filename);

return;
}
/*******************************************************************
* Clear the logbook: erase all its content
********************************************************************/
void GdpFrame::OnClearLogbook(wxCommandEvent& event)
{
if(initialized != 1234) return;
ClearLogbook();
return;
}
/*******************************************************************
* Clean the logbook: only keep its useful content
********************************************************************/
void GdpFrame::OnCleanLogbook(wxCommandEvent& event)
{
if(initialized != 1234) return;
CleanLogbook();
}
/*************************************************************************
* Speckle: subtraction of a model of the background
**************************************************************************/
void GdpFrame::OnSpeckleModelSubtraction(wxCommandEvent& WXUNUSED(event))
{
if((initialized != 1234) || (m_panel == NULL)) return;
 m_panel->SpeckleModelSubtraction();
return;
}
/*************************************************************************
* Speckle: back to original after a model subtraction
**************************************************************************/
void GdpFrame::OnSpeckleNoModelSubtraction(wxCommandEvent& WXUNUSED(event))
{
if((initialized != 1234) || (m_panel == NULL)) return;
 m_panel->RestoreOriginalImage();
 m_panel->Refresh();
return;
}
/*************************************************************************
* Offset correction
**************************************************************************/
void GdpFrame::OnOffsetCorrection(wxCommandEvent& event)
{
if((initialized != 1234) || (m_panel == NULL)) return;
switch (event.GetId()) {
 case ID_OFFSET_CORR_RAW:
   m_panel->OffsetCorrection(0);
   break;
 case ID_OFFSET_CORR_POSITIVE:
   m_panel->OffsetCorrection(1);
   break;
}
return;
}
/*************************************************************************
* FlatField correction
**************************************************************************/
void GdpFrame::OnFlatFieldCorrection(wxCommandEvent& event)
{
if((initialized != 1234) || (m_panel == NULL)) return;

switch (event.GetId()) {
 case ID_FFIELD_CORR_RAW:
   m_panel->FlatFieldCorrection(0);
   break;
 case ID_FFIELD_CORR_ONE_SIGMA:
   m_panel->FlatFieldCorrection(1);
   break;
 case ID_FFIELD_CORR_TWO_SIGMA:
   m_panel->FlatFieldCorrection(2);
   break;
 case ID_FFIELD_CORR_THREE_SIGMA:
   m_panel->FlatFieldCorrection(3);
   break;
}
return;
}
