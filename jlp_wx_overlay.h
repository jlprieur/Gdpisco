/*************************************************************************
* \file jlp_wx_overlay.h 
* \class JLP_wxOverlay 
* \brief Ruberband effect used for selecting an area in the displayed frame 
* \author JLP
* \date 03/06/2015
*
* JLP
* Version 03/06/2015
**************************************************************************/
#ifndef __jlp_wx_overlay_h                     /* sentry */
#define __jlp_wx_overlay_h

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#undef index  // To solve problems with index (conflicts with "string.h") ...
#include "wx/wx.h"
#include "wx/dc.h"
#include "wx/overlay.h"
#endif

#include "jlp_wx_ipanel.h" // JLP_wxImagePanel class

class JLP_wxImagePanel; 
class JLP_wxImage1;

// class JLP_wxOverlay 
class JLP_wxOverlay 
{

public:

// Constructor:
  JLP_wxOverlay(JLP_wxImagePanel *jlp_wxpanel, wxDC *dc); 

/* Destructor:
*/
  ~JLP_wxOverlay();

protected:

private:

 int initialized;
 JLP_wxImagePanel *m_panel;
 wxDC *m_dc;
 wxOverlay *m_overlay;
 wxDCOverlay *m_dc_overlay;

}; 

#endif    /* __jlp_wx_overlay_h sentry */
