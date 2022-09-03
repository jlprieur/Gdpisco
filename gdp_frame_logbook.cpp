/******************************************************************************
* Name:        gdp_frame_logbook.cpp (GpdFrame class)
* Purpose:     Logbook utilities 
* Author:      JLP 
* Version:     03/01/2015 
******************************************************************************/
#include "gdp_frame.h"
#include "gdp_frame_id.h"  // Menu identifiers

/*
int  SaveLogbook(wxString save_filename)
void ShowLogbook()
void HideLogbook()
void ClearLogbook()
void CleanLogbook()
int  WriteToLogbook(wxString str1, bool SaveToFile);
int  AddNewPointToLogbook(int xx, int yy)
int BinariesSaveMeasurements();
*/

/************************************************************************
* Save useful content of logbook to file 
* Input:
* save_filename: wxString whose value is set in gdp_frame_menu.cpp
************************************************************************/
int GdpFrame::SaveLogbook(wxString save_filename)
{
int status = -2;

 if(initialized == 1234) status = LogPanel->SaveLogbook(save_filename);

return(status);
}
/************************************************************************
* Showing logbook panel 
************************************************************************/
void GdpFrame::ShowLogbook()
{
 if(initialized != 1234) return;

 m_topsizer->Show(LogPanel);
 m_topsizer->Layout();
 menuLog->Check(ID_LOGBOOK_SHOW, true);
 menuLog->Check(ID_LOGBOOK_HIDE, false);
}
/************************************************************************
* Hiding logbook panel 
************************************************************************/
void GdpFrame::HideLogbook()
{
 if(initialized != 1234) return;

 m_topsizer->Hide(LogPanel);
 m_topsizer->Layout();
 menuLog->Check(ID_LOGBOOK_SHOW, false);
 menuLog->Check(ID_LOGBOOK_HIDE, true);
}
/*******************************************************************
* Clear the logbook: erase all its content
********************************************************************/
void GdpFrame::ClearLogbook()
{
wxString str1;

 if(initialized != 1234) return;

 LogPanel->Clear();

// Rewrite the filename at the beginning, since this is essential: 
 str1 = _T("%%\n%% Input file: ") + m_filename1 + _T("\n");
 WriteToLogbook(str1, true);

return;
}
/*******************************************************************
* Clean the logbook: only keep its useful content
********************************************************************/
void GdpFrame::CleanLogbook()
{
 if(initialized != 1234) return;
 LogPanel->Clean();
}
/************************************************************************
* Write to logbook 
*************************************************************************/
int  GdpFrame::WriteToLogbook(wxString str1, bool SaveToFile)
{
int status = -1;

 if(initialized == 1234) status = LogPanel->WriteToLogbook(str1, SaveToFile);

return(status);
}
/************************************************************************
* Add a new point to logbook 
*************************************************************************/
int GdpFrame::AddNewPointToLogbook(double xx, double yy, double value)
{
int status = -1;
wxString str1;

 if(initialized == 1234) { 
  str1.Printf("%.2f %.2f %.4g\n", xx, yy, value);
  status = LogPanel->WriteToLogbook(str1, true);
  }

return(status);
}
/**************************************************************************
* Binaries: process and saving measurements to Latex file 
*
* Look for information in the header of the input FITS file
* or in the "original file" if the input file is a "processed file"
**************************************************************************/
int GdpFrame::BinariesSaveMeasurements()
{
int status = -1;
wxString path, extension;
wxString original_fits_fname, processed_fits_fname;

 if(initialized != 1234) return(status);

// Look for the original FITS file name:
if(m_full_filename1.Contains("aw.fits")) {
printf("case 1\n");
  original_fits_fname = m_full_filename1.BeforeLast('a') + wxT("a.fits");
  } else if (m_full_filename1.Contains("aw.fit")) {
printf("case 2\n");
  original_fits_fname = m_full_filename1.BeforeLast('a') + wxT("a.fit");
  } else {
printf("case 3\n");
  original_fits_fname = m_full_filename1;
  }
// Look for information in the header of the original FITS file
/*
* INPUT:
*  original_fits_fname: FITS file obtained from the observations
*                        (full name with directory and extension)
*  processed_fits_fname: processed FITS file used for the measurements
*                        (name without directory and extension)
*/
wxFileName::SplitPath(m_full_filename1, &path, &processed_fits_fname,
                      &extension);

printf("BinariesSaveMeasurements/original_filename=%s\n processed_filename=%s\n", 
        static_cast<const char*>(original_fits_fname.c_str()),
        static_cast<const char*>(processed_fits_fname.c_str()));

 status = LogPanel->BinariesSaveMeasurements(original_fits_fname,
                                             processed_fits_fname);

 if(status == 0 && menuBinaries->FindItem(ID_BINARIES_SAVE_MEAS) != NULL) {
           menuBinaries->Enable( ID_BINARIES_SAVE_MEAS, false);
// Reset the number of measurements:
           nBinariesMeasurements = 0;
// Reset other measurements in logbook
           ClearLogbook();
           }


return(status);
}
