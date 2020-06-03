/*************************************************************************
* \file jlp_wx_overlay.cpp
* \class JLP_wxOverlay 
* \brief Ruberband effect used for selecting an area in the displayed frame 
* \author JLP
* \date 03/06/2015
*
* JLP
* Version 05/02/2015
**************************************************************************/
#include <stdio.h>
#include <ctype.h>

#include "jlp_wx_overlay.h" // JLP_wxOverlay class

/* To define "std" as the standard prefix (e.g. before printf, scanf, ...) */
using namespace std;

/************************************************************************
* Constructor:
************************************************************************/
JLP_wxOverlay::JLP_wxOverlay(JLP_wxImagePanel *jlp_wxpanel, wxDC *dc)
{

// Save input parameters to private variables
 m_panel = jlp_wxpanel;
 m_dc = dc;

// Create OverlayDC
 m_overlay = new wxOverlay();
 m_dc_overlay = new wxDCOverlay(*m_overlay, dc);
 initialized = 12345;
}

/************************************************************************ 
* Destructor:
************************************************************************/
JLP_wxOverlay::~JLP_wxOverlay() {
  return;
  }

/************************************************************************ 
* handle paint events 
* void OnPaint( wxPaintEvent &event );
************************************************************************/
/*
JLP_wxOverlay::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
// Construct a Device Context on which graphics and text
// can be drawn.
// wxPaintDC is a constructor and pass a pointer to the window
// on which you wish to paint:
wxPaintDC dc( this );

render(dc);

return;
}
*/

