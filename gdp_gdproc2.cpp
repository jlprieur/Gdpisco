/*************************************************************************
* \file gdp_gdproc2.cpp
* \class Gdp_wx_GDProc2 derived from JLP_wx_GDProc abstract class
* and used by JLP_iGDev_wxWID (image Graphic Device)
* \brief Image processing with data points entered interactively
* \author JLP
* \date 03/06/2015
*
* JLP
* Version 03/06/2015
**************************************************************************/
/* To define the Gdp_wx_GDProc2 class */
#include "gdp_gdproc2.h"

#include "gdp_frame.h"
#include "jlp_wx_ipanel.h"   // JLP_wxImagePanel class
#include "jlp_wx_image1.h" // JLP_wxImage1 class
#include "jlp_process_images.h"  // CheckBoxLimits2(), StatisticsFromBox2, etc

#define DEBUG

/**************************************************************************
* Constructor
*
**************************************************************************/
Gdp_wx_GDProc2::Gdp_wx_GDProc2(GdpFrame *gdp_frame,
                                 JLP_wxImagePanel *jlp_wxpanel)
{
int pmode;

// Initialze private parameters:
m_panel = jlp_wxpanel;
m_GdpFrame = gdp_frame;

// JLP_Gdev_wxwid class :
if(m_panel != NULL) {
 m_gdev_wxwid1 = m_panel->Image_gdev();
 m_image1 = m_panel->wxImage1();
 } else {
 m_gdev_wxwid1 = NULL;
 m_image1 = NULL;
 }

// To avoid memory problems with long strings:
help_text1.Alloc(256);

// Dummy selection for initialisation only:
pmode = -1;
SetNewProcessingMode(pmode);

return;
}
/**************************************************************************
*
* InteractiveProcessingMode :
*-1=None
* 0=Interactive input of points
* 1=autocorrelation simple measurement (Speckle)
* 2=autocorrelation double measurement (speckle or DVA)
* 3=two-star measurement (long integration, e.g., Lucky Imaging)
* 4=automatic speckle measurement
*
* Input:
*  processing_mode: interactive processing mode
*
**************************************************************************/
int Gdp_wx_GDProc2::SetNewProcessingMode(int processing_mode)
{

// Save to private variable:
ProcessingMode1 = processing_mode;

/* InteractiveProcessingMode :
*-1=None
* 0=Interactive input of points
* 1=auto-correlation simple measurement (Speckle)
* 2=autocorrelation double measurement (speckle or DVA)
* 3=two-star measurement (long integration, e.g., Lucky Imaging)
* 4=automatic speckle measurement
*/

// Set limits box type: 0=none 1=line, 2=rectangle, 3=circle
// and number of points that are required:
switch(processing_mode) {
  case -1:
    help_text1 = wxT("");
    LimitsBoxType1 = 0;
    n_PointsRequired1 = 0;
    break;
  default:
  case 0:
    help_text1 = wxT("Points to logbook: click left button to select a point");
    LimitsBoxType1 = 0;
    n_PointsRequired1 = 1;
    break;
  case 1:
    help_text1 = wxT("Simple measurement: click and drag left button to select a peak");
    LimitsBoxType1 = 3;
    n_PointsRequired1 = 2;
    break;
  case 2:
    help_text1 = wxT("Double measurement: click and drag left button to select a peak");
    LimitsBoxType1 = 3;
    n_PointsRequired1 = 2;
    break;
  case 3:
    help_text1 = wxT("Two-stars: click and drag left button to select star1, star2 and sky");
    LimitsBoxType1 = 3;
    n_PointsRequired1 = 6;
    break;
  case 4:
    help_text1 = wxT("Automatic measurement: click left button to start automatice measurement");
    LimitsBoxType1 = 0;
    n_PointsRequired1 = 1;
    break;
}

return(0);
}
/******************************************************************
* Processing with box limits obtained by JLP_iGDev_wxWID:
*
* InteractiveProcessingMode :
*-1=None
* 0=Interactive input of points
* 1=auto-correlation simple measurement (Speckle)
* 2=autocorrelation double measurement (speckle or DVA)
* 3=two-star measurement (long integration, e.g., Lucky Imaging)
* 4=automatic speckle measurement
*
* Input:
*  x_down, y_down, n_down: (device coord) x,y and number of points
*                          entered by pressing the mouse down
*  x_up, y_up, n_up: (device coord) x,y and number of points entered
*                     by releasing the mouse
*  label: label to be plotted (not used here: only used by gdproc1)
********************************************************************/
int Gdp_wx_GDProc2::DataProcessing(double *x_down, double *y_down, int n_down,
                                   double *x_up, double *y_up, int n_up,
                                   wxString label)

{
double *array, user_x1[6], user_y1[6];
double wwx, wwy, xc, yc, radius, b_maxi, g_maxi;
int nx1, ny1, npts1, in_frame;
int i, status, interactive_mode;
wxString results;

m_image1->GetDoubleArray(&array, &nx1, &ny1);

if(ProcessingMode1 == 4)
  interactive_mode = 0;
else
  interactive_mode = 1;
 
if(interactive_mode == 1) {
npts1 = MINI(6, n_PointsRequired1);
if((n_down != npts1 / 2) || (n_up != npts1 / 2)) {
   fprintf(stderr,"Gdp_wx_GDProc2::DataProcessing:Error: n_down=%d n_up=%d (nrequired=%d)\n",
           n_down, n_up, n_PointsRequired1/2);
   return(-1);
   }

for(i = 0; i < (npts1 / 2); i++ ) {
 m_gdev_wxwid1->ConvDevToUser(x_down[i], y_down[i], &(user_x1[2 * i]),
                              &(user_y1[2 * i]), &in_frame);
 if(in_frame == 1) {
   m_gdev_wxwid1->ConvDevToUser(x_up[i], y_up[i], &(user_x1[2 * i + 1]),
                                &(user_y1[2 * i + 1]), &in_frame);
   }
 if(in_frame == 0) {
   fprintf(stderr,"Gdp_wx_GDProc2::DataProcessing:Error: point_up/_down nber %d out of frame !\n",
   i);
   return(-2);
  }
 } // EOF for loop

} // EOF interactive_mode

/* InteractiveProcessingMode :
*-1=None
* 0=Interactive input of points
* 1=autocorrelation simple measurement (Speckle)
* 2=autocorrelation double measurement (speckle or DVA)
* 3=two-star measurement (long integration, e.g., Lucky Imaging)
* 4=automatic speckle measurement
*/
switch(ProcessingMode1) {
 default:
 case 0:
   i = NINT(user_x1[0]) + nx1 * NINT(user_y1[0]);
   if(i < 0) i = 0;
   if(i > nx1 * ny1 - 1) i = nx1 * ny1 - 1;
   m_GdpFrame->LoadNewPointFromCursor(user_x1[0], user_y1[0], array[i]);
   break;
 case 1:
   status = CircleFromBox2(nx1, ny1, user_x1[0], user_y1[0],
                           user_x1[1], user_y1[1], &xc, &yc, &radius);
// Calling with interactive = 1:
   if(!status) SingleSpeckleMeasurement(xc, yc, radius, &b_maxi, &g_maxi, 1);
   break;
 case 2:
   status = CircleFromBox2(nx1, ny1, user_x1[0], user_y1[0], user_x1[1],
                           user_y1[1], &xc, &yc, &radius);
   if(status == 0) DoubleSpeckleMeasurement(xc, yc, radius);
   break;
 case 3:
   BinaryTwoStarMeasurement(user_x1, user_y1, npts1);
   break;
}

delete[] array;
return(0);
}
