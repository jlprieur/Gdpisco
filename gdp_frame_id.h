/****************************************************************************
* Name: gdp_frame_id.h  (GdpFrame class)
* Purpose: display and process FITS images obtained with PISCO and PISCO2
*
* list of ID used by GdpFrame class
* 
* JLP
* Version 22/07/2013
****************************************************************************/
#ifndef _gdp_frame_id_h_
#define _gdp_frame_id_h_

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

//---------------------------------------------------------------------
enum
{
// Pb in menu found in 2015
//  ID_QUIT               = wxID_EXIT,
// Solution:
  ID_QUIT               = 999,
// File:
  ID_SAVE_TO_PST = 1000,
  ID_LOAD_FITS,
  ID_SAVE_TO_FITS,
// Preprocessing:
  ID_OFFSET_CORR_RAW,
  ID_OFFSET_CORR_POSITIVE,
  ID_FFIELD_CORR_RAW,
  ID_FFIELD_CORR_ONE_SIGMA,
  ID_FFIELD_CORR_TWO_SIGMA,
  ID_FFIELD_CORR_THREE_SIGMA,
// Binaries processing:
  ID_BINARIES_MODSUB,
  ID_BINARIES_NOMODSUB,
  ID_BINARIES_RESET,
  ID_BINARIES_SIMPLE_MEAS,
  ID_BINARIES_DOUBLE_MEAS,
  ID_BINARIES_TWOSTARS_MEAS,
  ID_BINARIES_AUTO_MEAS,
  ID_BINARIES_SAVE_MEAS,
  ID_BINARIES_DMAG_MEAS,
// Logbook
  ID_LOGBOOK_SHOW,
  ID_LOGBOOK_HIDE,
  ID_CURSOR_LIST_START,
  ID_LOGBOOK_CLEAR,
  ID_LOGBOOK_CLEAN,
  ID_LOGBOOK_SAVE,
// Video/3D utilities
  ID_NEXT_FRAME,
  ID_PREVIOUS_FRAME,
  ID_GOTO_ZERO_VIDEO,
  ID_GOTO_FRAME,
  ID_DELAY_VIDEO,
  ID_PLAY_VIDEO,
  ID_STOP_VIDEO,

// Automatic measurements 
// Autocorrelation of binaries
  ID_AUTO_MEASURE_BINARY,
// Shack-Hartmann
  ID_AUTO_MEASURE_HARTMANN,

// panel menu
  ID_CONTEXT_HELP,

// Timer
  ID_TIMER,

// Help:
  ID_ABOUT             = wxID_ABOUT,
  ID_HELP              = wxID_HELP
};

#endif
