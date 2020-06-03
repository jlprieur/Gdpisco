/*************************************************************************
* \file gdp_gdproc2.h 
* \class Gdp_wx_GDProc2 derived from JLP_wx_GDProc abstract class
* and used by JLP_GDev_wxWID (image Graphic Device) 
* \brief Image processing with data points entered interactively 
* \author JLP
* \date 05/08/2013
*
* InteractiveProcessingMode :
*-1=None
* 0=Interactive input of points
* 1=auto-correlation simple measurement (Speckle)
* 2=autocorrelation double measurement (speckle or DVA)
* 3=two-star measurement (long integration, e.g., Lucky Imaging)
* 4=automatic speckle binary measurement
*
* JLP
* Version 05/08/2013
**************************************************************************/
#ifndef __gdp_gdproc2_h                     /* sentry */
#define __gdp_gdproc2_h

#include <stdio.h>
// #include <math.h>
#include <ctype.h>

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#undef index  // To solve problems with index (conflicts with "string.h") ...
#include "wx/wx.h"
#endif

/* To define the JLP_wx_GDProc virtual class */ 
#include "jlp_wx_gdproc.h"

#include "jlp_gdev_wxwid.h" // JLP_GDev_wxWID class

/* To define "std" as the standard prefix (e.g. before printf, scanf, ...) */
using namespace std;

class GdpFrame; 
class JLP_wxImagePanel; 
class JLP_wxImage1;

// class Gdp_wx_GDProc2:
class Gdp_wx_GDProc2 : public JLP_wx_GDProc {

public:

// Constructor:
  Gdp_wx_GDProc2(GdpFrame *gdp_frame, JLP_wxImagePanel *jlp_wxpanel); 

/* Destructor:
* (Should be declared as virtual)
*/
  virtual ~Gdp_wx_GDProc2() {
  return;
  }

// Virtual routine of JLP_wx_GDProc: should be defined in this class!
// (defined in gdp_gdproc2.cpp)
 int DataProcessing(double *x_down, double *y_down, int n_down, 
                    double *x_up, double *y_up, int n_up, wxString label);

// Virtual routine of JLP_wx_GDProc: should be defined in this class!
// (defined in gdp_gdproc2.cpp)
 int SetNewProcessingMode(int processing_mode);

protected:

// In gdp_gdproc2_process.cpp:
/***********************************************************************
* InteractiveProcessingMode :
* 0=Interactive input of points
* 1=auto-correlation simple measurement (Speckle)
* 2=autocorrelation double measurement (speckle or DVA)
* 3=two-star measurement (long integration, e.g., Lucky Imaging)
* 4=automatic speckle binary measurement
***********************************************************************/

  int DoubleSpeckleMeasurement(double xc, double yc, double radius);
  int BinaryTwoStarMeasurement(double *user_x0, double *user_y0, int npts0);
  int SingleSpeckleMeasurement(double xc, double yc, double radius,
                               double *b_maxi, double *g_maxi, int interactive);
  int AutomaticSpeckleBinaryMeasurement();
  int CosmeticPatch2(double *dble_image11, int nx11, int ny11,
                     double xc, double yc, double radius, int interactive);
  int CircularBackground2(double xc, double yc, double radius, int interactive);
  // int AutomSpeckleMeasurement();

private:

 GdpFrame *m_GdpFrame;
 JLP_wxImagePanel *m_panel;
 JLP_wxImage1 *m_image1;
 JLP_GDev_wxWID *m_gdev_wxwid1;

// Parameters for cosmetic patches:
 double m_radius_fact, m_sigma_noise;
 int m_poly_order, m_polynomial_method;

}; 

#endif    /* __gdp_wx_gdproc2_h sentry */
