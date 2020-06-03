/*************************************************************************
* \file gdp_gdproc2.cpp
* \class Gdp_wx_GDProc2 derived from JLP_wx_GDProc abstract class
* and used by JLP_iGDev_wxWID (image Graphic Device)
* \brief Image processing with data points entered interactively
*        Processing binary speckle images
* \author JLP
* \date 05/01/2015
*
* JLP
* Version 05/01/2015
**************************************************************************/
/* To define the Gdp_wx_GDProc2 class */
#include "gdp_gdproc2.h" // Gpd_wx_GDProc2 class

#include "jlp_wx_image1.h" // JLP_wxImage1 class

#include "gdp_frame.h"
#include "jlp_wx_patch_dlg.h"
#include "jlp_patch_set1.h"        // Cosmetic patches, POLY_CIRC_PATCH, ...
#include "jlp_wx_ipanel_speckle.h"   // astrom_barycenter(),...
#include "jlp_wx_ipanel_utils.h"    // gdp_sum_circle

/* Contains:
  int DoubleSpeckleMeasurement(int nx1, int ny1, double xc, double yc,
                               double radius);
  int BinaryTwoStarMeasurement(double *x_down, double *y_down,
                              double *x_up, double *y_up);
  int SingleSpeckleMeasurement(double xc, double yc, double radius,
                           double *b_maxi, double *g_maxi, int interactive);
  int AutomaticSpeckleBinaryMeasurement();
  int CosmeticPatch2(double *dble_image11, int nx11, int ny11,
                     double xc, double yc, double radius, int interactive);
  int CircularBackground2(double xc, double yc, double radius, int interactive)
*/

/***********************************************************************
* Measure the position of the two autocorrelation peaks:
*  start with the peak chosen by the user
*  and then measure the symmetric peak relative to the center of the frame
*
* INPUT:
*  xc, yc : coordinates of the circle center entered by the user (user coord.)
*  radius : radius of the circle entered by the user (user coord.)
***********************************************************************/
int Gdp_wx_GDProc2::DoubleSpeckleMeasurement(double xc,  double yc,
                                              double radius)
{
int nx1, ny1, x_center, y_center, status;
double b_maxi_1, g_maxi_1, b_maxi_2, g_maxi_2;
double b_delta_mag, g_delta_mag;
char b_dmag[40], g_dmag[40];
wxString str1;

nx1 = m_image1->Get_nx1();
ny1 = m_image1->Get_ny1();

x_center = nx1 / 2;
y_center = ny1 / 2;

// In case of error, return immediately:
if(radius <= 0) return(-1);

// First call with interactive = 1:
  status = SingleSpeckleMeasurement(xc, yc, radius, &b_maxi_1, &g_maxi_1, 1);
// If the user has cancelled it, return from here
  if(status) return(status);

  if(xc > x_center) xc = x_center - (xc - x_center);
    else xc = x_center + (x_center - xc);
  if(yc > y_center) yc = y_center - (yc - y_center);
    else yc = y_center + (y_center - yc);

// Second call with interactive = 0:
  SingleSpeckleMeasurement(xc, yc, radius, &b_maxi_2, &g_maxi_2, 0);

// Compute Delta mag:
// if DVA cross-correlations:
  b_delta_mag = -1;
  g_delta_mag = -1;
  b_dmag[0] = '\0';
  g_dmag[0] = '\0';
  if(b_maxi_1 > 0 && b_maxi_2 > 0) {
     b_delta_mag = 2.5 * log10(b_maxi_1 / b_maxi_2);
     if(b_delta_mag < 0) b_delta_mag *= -1.;
     sprintf(b_dmag,"Dm=%.2f (Bary.) ", b_delta_mag);
     }
  if(g_maxi_1 > 0 && g_maxi_2 > 0) {
     g_delta_mag = 2.5 * log10(g_maxi_1 / g_maxi_2);
     if(g_delta_mag < 0) g_delta_mag *= -1.;
     sprintf(g_dmag,"Dm=%.2f (Gauss.)", g_delta_mag);
     }
  if(b_delta_mag > 0 || g_delta_mag > 0) {
     str1 = wxT("%% ") + wxString(b_dmag, wxConvUTF8)
            + wxString(g_dmag, wxConvUTF8) + wxT("\n");
// Write measurement to logbook window, and also to file:
     m_GdpFrame->WriteToLogbook(str1, true);
   } // EOF b_delta_mag > 0

return(0);
}
/***********************************************************************
* Process the six points entered by the user
* SixPoints_function: two stars measurements
*
* INPUT:
*  user_x1[0], user_y1[0] : 1st point (down) entered by the user (user coord.)
*  user_x1[1], user_y1[1] : 2nd point (up) entered by the user (user coord.)
*  ...
*  user_x1[4], user_y1[4] : 5th point (down) entered by the user (user coord.)
*  user_x1[5], user_y1[5] : 6th point (up) entered by the user (user coord.)
***********************************************************************/
int Gdp_wx_GDProc2::BinaryTwoStarMeasurement(double *user_x0, double *user_y0,
                                             int npts0)
{
int nx1, ny1, status, ksky;
double xc[3], yc[3], radius[3], sum[3], npts[3], w1, mean_sky;
double xac[2], yac[2], maxi[2], flux[2];
double diam, v1, v2, rho, theta, theta180, Dmag;
double *dble_image1;
register int i, k;
wxString str1;

status = m_image1->GetDoubleArray(&dble_image1, &nx1, &ny1);
if(status != 0) return(-1);

// In case of error, return immediately:
 if(npts0 != 6) {
   fprintf(stderr, "BinaryTwoStarMeasurement/Error: npts=%d\n", npts0);
   return(-2);
   }
 if((user_x0[0] == user_x0[1] && user_y0[0] == user_y0[1])
   || (user_x0[2] == user_x0[3] && user_y0[2] == user_y0[3])
   || (user_x0[4] == user_x0[5] && user_y0[4] == user_y0[5])) {
   fprintf(stderr, "BinaryTwoStarMeasurement/Error: bad input points\n");
   return(-1);
   }

// SixPoints_function: two stars measurements
for(k = 0; k < 3; k++) {
   xc[k] = user_x0[2 * k];
   yc[k] = user_y0[2 * k];
   radius[k] = sqrt(SQUARE(user_x0[2 * k] - user_x0[2 * k + 1])
               + SQUARE(user_y0[2 * k] - user_y0[ 2 * k + 1]));
   gdp_sum_circle(dble_image1, nx1, ny1, xc[k], yc[k], radius[k], &(sum[k]),
                  &(npts[k]));

   if(npts[k] == 0.) npts[k] = 1.;

   str1.Printf(_T("%%%% k=%d xc=%.1f yc=%.1f rad=%.2f Sum=%.3g Mean=%3g\n"),
               k, xc[k], yc[k], radius[k], sum[k], sum[k]/npts[k]);
// Write to logbook window, and to logfile ("true"):
   m_GdpFrame->WriteToLogbook(str1, true);
 }

// Looking for the mean sky value:
w1 = sum[0] / npts[0];
ksky = 0;
 for(k = 1; k < 3; k++) {
   if(sum[k]/npts[k] < w1) {
     w1 = sum[k]/npts[k];
     ksky = k;
   }
 }
mean_sky = sum[ksky] / npts[ksky];

// Barycenter within the circle after removing the background:
i = 0;
 for(k = 0; k < 3; k++) {
   if(k != ksky) {
   diam = 2. * radius[k];
   astrom_barycenter(dble_image1, nx1, ny1, xc[k], yc[k], diam, mean_sky,
                     &(xac[i]), &(yac[i]), &(maxi[i]), &(flux[i]));
   i++;
   }
 }
// Computing rho and theta:
/* Polar conversion relative to the center of the frame (for autocorrelations)*/
 v1 = xac[1] - xac[0];
 v2 = yac[1] - yac[0];
 speckle_convert_to_polar(v1, v2, &rho, &theta, &theta180);
 if(maxi[0] == 0.) maxi[0] = 0.001;
 Dmag = -2.5 * log10(maxi[1] / maxi[0]);

 str1.Printf(_T("%%%% rho=%.2f theta=%.2f theta180=%.2f Dm=%.2f\n"),
             rho, theta, theta180, Dmag);
// Write to logbook window, and to logfile ("true"):
 m_GdpFrame->WriteToLogbook(str1, true);

// Increment the number of measurements:
 m_GdpFrame->Increment_nbinaries_meas();

// Enable saving if more than 1 measurement:
 m_GdpFrame->EnableSaveBinariesMeasurements(true);

delete[] dble_image1;
return(0);
}
/************************************************************************
* Binaries: astrometry within a circle after removing the background
*          fitted with a polynomial or a profile centered on the image
* (circle interactively selected by the user)
*
* INPUT:
* xc, yc: center (in pixels) of the center of the circle
* radius: radius of the circle around the patch to be measured
*
* OUTPUT:
* b_maxi: maximum in the circle (after subtraction of the sky background)
* g_maxi: maximum of the Gaussian (after subtraction of the sky background)
************************************************************************/
int Gdp_wx_GDProc2::SingleSpeckleMeasurement(double xc, double yc,
                                        double radius,
                                        double *b_maxi, double *g_maxi,
                                        int interactive)
{
int nx1, ny1;
int bad_fit, continue_anyhow, status, output_poly_order;
int astrom_only, centered_polar_coordinates, ifail;
double *dble_image1, *saved_data;
double mean_sky, sigma_sky, max_value, negative_percent;
double diam, xac, yac, flux, maxi;
char log_message[160], method[20];
register int i;
wxString str1;
bool reset_ITT_to_MinMax = false;

*b_maxi = -1;
*g_maxi = -1;

  m_image1->GetDoubleArray(&dble_image1, &nx1, &ny1);

  saved_data = new double[nx1 * ny1];
  for(i = 0; i < nx1 * ny1; i++) saved_data[i] = dble_image1[i];

// astrom_only and centered_polar_coordinates set according to BinariesMode
// Speckle autocorrelations:
   if(ProcessingMode1 == 1) {
    astrom_only = 1;
    centered_polar_coordinates = 1;
// DVA cross-correlations:
   } else if(ProcessingMode1 == 2) {
    astrom_only = 0;
    centered_polar_coordinates = 1;
// Lucky imaging:
   } else if(ProcessingMode1 == 3) {
    astrom_only = 0;
    centered_polar_coordinates = 0;
// Automatic measurements;  
   } else {
    astrom_only = 1;
    centered_polar_coordinates = 1;
   }

// Speckle: BinariesMode = 1
// DVA: BinariesMode = 2
// Lucky Imaging: BinariesMode = 3
if(ProcessingMode1 == 3) {
// Call CircularBackground2 (with polynomial order = 0):
 status = CircularBackground2(xc, yc, radius, interactive);
} else {
// Call Cosmetic Patch2 (with polynomial order = 3 by default
// and  m_polynomial_method = 0/1 (profile/polynomial)
 status = CosmeticPatch2(dble_image1, nx1, ny1, xc, yc, radius, interactive);
}

// If the user is not satisfied, return from here
 if(status) {
   delete[] saved_data;
   return(-1);
   }

// Compute the difference (saved image - new flattened image)
  delete[] dble_image1;
  m_image1->GetDoubleArray(&dble_image1, &nx1, &ny1);
  for(i = 0; i < nx1 * ny1; i++) dble_image1[i] = saved_data[i] - dble_image1[i];

/* Statistics on the edges: */
  speckle_patch_statistics(dble_image1, nx1, ny1, xc, yc, radius, &mean_sky,
                           &sigma_sky, &max_value, &negative_percent, &bad_fit);

 continue_anyhow = 1;

 str1.Printf(_T("Background: mean=%.2f, sigma=%.2f mean/max=%.2f\
 with %d%% of negative values"),
mean_sky, sigma_sky, mean_sky/max_value, (int)negative_percent);

// Write to logbook window, but not yet to file:
if(bad_fit == 0) {
     m_GdpFrame->WriteToLogbook(str1 + _T(" (good fit)\n"), false);
 } else {
     m_GdpFrame->WriteToLogbook(str1 + _T(" (bad fit)\n"), false);

  if(interactive) {
    str1 = wxT("WARNING: the background could not be estimated properly.\n\
I advise you to have another try with a different center and diameter.\n\n\
Continue anyhow?");
    if( wxMessageBox(str1, _T("Please confirm"),
                           wxICON_QUESTION | wxYES_NO) == wxYES ) {
// I continue with the bad data:
     continue_anyhow = 1;
    } else {
     continue_anyhow = 0;
    }
   } else {
    continue_anyhow = 1;  // if non interactive go ahead in any case
   }
   // EOF interactive
 } // EOF bad_fit

output_poly_order = (m_polynomial_method ? m_poly_order : -1);
/* bad_fit=1 means that background is of good quality,
* or that the user wants to proceed anyhow */
if(bad_fit == 0 || continue_anyhow) {

// Barycenter on the difference of the two images:
   diam = 2. * radius;
   status = astrom_barycenter(dble_image1, nx1, ny1, xc, yc, diam, mean_sky,
                                &xac, &yac, &maxi, &flux);
   if(status) {
     sprintf(log_message, "astrom_barycenter/Error: central patch is null!\
Empty circle or null sum\n");
     wxLogError(wxString(log_message, wxConvUTF8));
   } else {
     *b_maxi = maxi;
     strcpy(method,"Bary.");
     astrom_output_to_logfile(xac, yac, maxi, flux, xc, yc, diam,
                              output_poly_order, mean_sky, sigma_sky,
                              nx1, ny1, log_message, method,
                              astrom_only, centered_polar_coordinates);
     if(bad_fit)
       str1 = wxString(log_message, wxConvUTF8) + wxT(" (BAD FIT)\n");
     else
       str1 = wxString(log_message, wxConvUTF8) + wxT("\n");
// Write measurement to logbook window, and also to file:
    m_GdpFrame->WriteToLogbook(str1, true);

// Increment the number of measurements:
    m_GdpFrame->Increment_nbinaries_meas();
   }

/* Gaussian fit on the difference of the two images: */
// astrom_only = 1, centered_polar_coordinates=1
   diam = 2. * radius;
   status = astrom_gaussian_fit(dble_image1, nx1, ny1, xc, yc, diam, mean_sky,
                                &xac, &yac, &maxi, &flux, &ifail);
   if(status) {
     sprintf(log_message,"astrom_gaussian_fit/ifail = %d \n", ifail);
     wxLogError(wxString(log_message, wxConvUTF8));
   } else {
   *g_maxi = maxi;
   strcpy(method,"Gauss.");
   astrom_output_to_logfile(xac, yac, maxi, flux, xc, yc, diam,
                            output_poly_order, mean_sky, sigma_sky,
                            nx1, ny1, log_message, method,
                            astrom_only, centered_polar_coordinates);
     if(bad_fit)
       str1 = wxString(log_message, wxConvUTF8) + wxT(" (BAD FIT)\n");
     else
       str1 = wxString(log_message, wxConvUTF8) + wxT("\n");
// Write measurement to logbook window, and also to file:
    m_GdpFrame->WriteToLogbook(str1, true);

// Increment the number of measurements:
    m_GdpFrame->Increment_nbinaries_meas();
   }  // EOF status == 0

}  // EOF bad_fit == 0 || continue_anyhow

// Go back to previous version of the image:
for(i = 0; i < nx1 * ny1; i++) dble_image1[i] = saved_data[i];

// Re-display previous version of the image:
m_gdev_wxwid1->UpdateCImage1Values(dble_image1, nx1, ny1, reset_ITT_to_MinMax);

delete[] dble_image1;
delete[] saved_data;
return(0);
}
/***********************************************************************
* Automatic speckle binary measurement
* Measure the position of the two autocorrelation peaks:
*  start with the left button is pressed by the user
*  and then measure the symmetric peak relative to the center of the frame
*
***********************************************************************/
int Gdp_wx_GDProc2::AutomaticSpeckleBinaryMeasurement()
{
int nx11, ny11;
int bad_fit, continue_anyhow, status, output_poly_order;
int astrom_only, centered_polar_coordinates, ifail, n_for_patches;
double *dble_image11, *saved_data;
double mean_sky, sigma_sky, max_value, negative_percent;
double diam, xac, yac, flux, maxi;
char log_message[160], method[20];
int i, i_iter;
wxString str1;
bool reset_ITT_to_MinMax = false;
double xc, yc, radius, b_maxi, g_maxi;
double rho10, theta10, error_rho10, error_theta10;
int interactive;

printf("UUU OK: Start automatic measurement\n");
// astrom_only and centered_polar_coordinates set according to BinariesMode
// ProcessingMode1=4 automatic measurements;
  astrom_only = 1;
  centered_polar_coordinates = 1;
/// Get original data (before processing) 
  m_image1->GetDoubleArray(&dble_image11, &nx11, &ny11);
  saved_data = new double[nx11 * ny11];
  for(i = 0; i < nx11 * ny11; i++) saved_data[i] = dble_image11[i];

for(i_iter = 0; i_iter < 2; i_iter++) { 

// Process private array dble_image1 :
  n_for_patches = 40 + i_iter * 20;
// Circular profile filter (processing private array dble_image1):
  m_panel->BinaryMeasFromCircProfile(n_for_patches, &xc, &yc, &radius, 
                                     &rho10, &theta10,
                                     &error_rho10, &error_theta10);

/**************************************************************
* xc, yc: center (in pixels) of the center of the circle
* radius: radius of the circle around the patch to be measured
* b_maxi: maximum in the circle (after subtraction of the sky background)
* g_maxi: maximum of the Gaussian (after subtraction of the sky background)
************************************************************************/
b_maxi = -1;
g_maxi = -1;
interactive = 0;
// Speckle: BinariesMode = 1
// Call Cosmetic Patch2 (with polynomial order = 3 by default
// and  m_polynomial_method = 0/1 (profile/polynomial)
// Apply this to the original image:
 status = CosmeticPatch2(dble_image11, nx11, ny11, xc, yc, radius, interactive);
printf("UUU OK: From CosmeticPatch2 with xc=%.1f yc=%.1f radius= %.1f status=%d\n",
        xc, yc, radius, status);

// If the user is not satisfied, return from here
 if(status) {
   fprintf(stderr," Automatic Measurements/CosmeticPatch2 Error\n");
   delete[] saved_data;
   return(-1);
   }

// Compute the difference (saved image - new flattened image)
  delete[] dble_image11;
  m_image1->GetDoubleArray(&dble_image11, &nx11, &ny11);
  for(i = 0; i < nx11 * ny11; i++) dble_image11[i] = saved_data[i] - dble_image11[i];

/* Statistics on the edges: */
  speckle_patch_statistics(dble_image11, nx11, ny11, xc, yc, radius, &mean_sky,
                           &sigma_sky, &max_value, &negative_percent, &bad_fit);
printf("UUU OK: From speckle_patch_statistics \n bad_fit=%d\n",
        bad_fit);

 continue_anyhow = 1;

 str1.Printf(_T("Background: mean=%.2f, sigma=%.2f mean/max=%.2f\
 with %d%% of negative values"),
mean_sky, sigma_sky, mean_sky/max_value, (int)negative_percent);

// Write to logbook window, but not yet to file:
if(bad_fit == 0) {
     m_GdpFrame->WriteToLogbook(str1 + _T(" (good fit)\n"), false);
 } else {
     m_GdpFrame->WriteToLogbook(str1 + _T(" (bad fit)\n"), false);

// I continue with the bad data:
     continue_anyhow = 1;
 } // EOF bad_fit

output_poly_order = (m_polynomial_method ? m_poly_order : -1);
/* bad_fit=1 means that background is of good quality,
* or that the user wants to proceed anyhow */
if(bad_fit == 0 || continue_anyhow) {

// Barycenter on the difference of the two images:
   diam = 2. * radius;
   status = astrom_barycenter(dble_image11, nx11, ny11, xc, yc, diam, mean_sky,
                                &xac, &yac, &maxi, &flux);
printf("UUU OK: From astrom_barycenter status=%d\n", status);
   if(status) {
     sprintf(log_message, "astrom_barycenter/Error: central patch is null!\
Empty circle or null sum\n");
     wxLogError(wxString(log_message, wxConvUTF8));
   } else {
     b_maxi = maxi;
     strcpy(method,"Bary.");
     astrom_output_to_logfile(xac, yac, maxi, flux, xc, yc, diam,
                              output_poly_order, mean_sky, sigma_sky,
                              nx11, ny11, log_message, method,
                              astrom_only, centered_polar_coordinates);
printf("UUU OK: From astrom_output_to_logfile xac=%.2f yac=%.2f\n", xac, yac);
     if(bad_fit)
       str1 = wxString(log_message, wxConvUTF8) + wxT(" (BAD FIT)\n");
     else
       str1 = wxString(log_message, wxConvUTF8) + wxT("\n");
// Write measurement to logbook window, and also to file:
    m_GdpFrame->WriteToLogbook(str1, true);

// Increment the number of measurements:
    m_GdpFrame->Increment_nbinaries_meas();
   }

/* Gaussian fit on the difference of the two images: */
// astrom_only = 1, centered_polar_coordinates=1
   diam = 2. * radius;
   status = astrom_gaussian_fit(dble_image11, nx11, ny11, xc, yc, diam, mean_sky,
                                &xac, &yac, &maxi, &flux, &ifail);
printf("UUU OK: From astrom_gaussion_fit status=%d xac=%.2f yac=%.2f\n", 
        status, xac, yac);
   if(status) {
     sprintf(log_message,"astrom_gaussian_fit/ifail = %d \n", ifail);
     wxLogError(wxString(log_message, wxConvUTF8));
   } else {
   g_maxi = maxi;
   strcpy(method,"Gauss.");
   astrom_output_to_logfile(xac, yac, maxi, flux, xc, yc, diam,
                            output_poly_order, mean_sky, sigma_sky,
                            nx11, ny11, log_message, method,
                            astrom_only, centered_polar_coordinates);
     if(bad_fit)
       str1 = wxString(log_message, wxConvUTF8) + wxT(" (BAD FIT)\n");
     else
       str1 = wxString(log_message, wxConvUTF8) + wxT("\n");
// Write measurement to logbook window, and also to file:
    m_GdpFrame->WriteToLogbook(str1, true);

// Increment the number of measurements:
    m_GdpFrame->Increment_nbinaries_meas();
   }  // EOF status == 0

}  // EOF bad_fit == 0 || continue_anyhow

// Go back to previous version of the image:
for(i = 0; i < nx11 * ny11; i++) dble_image11[i] = saved_data[i];

// Re-display previous version of the image:
m_gdev_wxwid1->UpdateCImage1Values(dble_image11, nx11, ny11, reset_ITT_to_MinMax);
/// Get original data (before processing) 
  delete[] dble_image11;
  m_image1->GetDoubleArray(&dble_image11, &nx11, &ny11);
  for(i = 0; i < nx11 * ny11; i++) saved_data[i] = dble_image11[i];
} // EOF i_iter loop

delete[] dble_image11;
delete[] saved_data;
return(0);
}
/***********************************************************************
* Cosmetic patch
*
* INPUT:
*  xc, yc: user coordinates of the center of the patch
*  radius: radius of the circular patch
*  interactive: if interactive aknowledgement by the user
*               otherwise automatic background fit
*
* OUTPUT:
*   status = 1 if image was not modified
*          = 0 if image was modified
*          = -1 if problem in POLY_CIRC_PATCH
***********************************************************************/
int Gdp_wx_GDProc2::CosmeticPatch2(double *dble_image11, int nx11, int ny11,
                                   double xc, double yc, double radius,
                                  int interactive)
{
JLP_Patch_Dlg *PatchDlg;
wxString err_msg, str2;
double *saved_data1;
double *work_image1, *noise_array, sigma_sky;
int noise_dim, Patch_Dlg_answer, status;
char err_message[128];
register int i;
bool reset_ITT_to_MinMax = false;

// Set default values for background estimation:
// m_poly_order = 3 here by default
m_radius_fact = 1.8;
m_sigma_noise = 0.3;
m_poly_order = 3;
m_polynomial_method = 1;

PatchDlg = NULL;

// Store initial image to "saved_data"
saved_data1 = new double[nx11 * ny11];
work_image1 = new double[nx11 * ny11];
for(i = 0; i < nx11 * ny11; i++) saved_data1[i] = dble_image11[i];

// Calling POLY_CIRC_PATCH to prepare a new version of the image:
/*
int POLY_CIRC_PATCH(double *image1, int nx1, int ny1, int idim1,
                    double xp, double yp, double diam, double diam_factor,
                    double *noise_array, int noise_dim, double sigma_noise,
                    int poly_order, double *sigma_sky);
int PROFILE_CIRC_PATCH(double *image1, int nx1, int ny1, int idim1,
                       double xp, double yp, double diam, double diam_factor,
                       double *noise_array, int noise_dim, double sigma_noise,
                       double *sigma_sky);
int CREATE_NOISE_ARRAY(double **noise_array, int noise_dim);
int DELETE_NOISE_ARRAY(double *noise_array);
*/
noise_dim = 256;

CREATE_NOISE_ARRAY(&noise_array, noise_dim);

while(1) {

// Retrieve original image:
for(i = 0; i < nx11 * ny11; i++) work_image1[i] = saved_data1[i];

// Process it:
if(m_polynomial_method) {
  status = POLY_CIRC_PATCH(work_image1, nx11, ny11, nx11, xc, yc, 2.*radius,
                           m_radius_fact, noise_array, noise_dim,
                           m_sigma_noise, m_poly_order, &sigma_sky,
                           err_message);
  str2.Printf(wxT(" (status = %d\n"), status);
  if(status) err_msg = wxT("Error in Poly_circ_patch") +
                       wxString(err_message, wxConvUTF8) + str2;
} else {
  status = PROFILE_CIRC_PATCH(work_image1, nx11, ny11, nx11, xc, yc, 2.*radius,
                              m_radius_fact, noise_array, noise_dim,
                              m_sigma_noise, &sigma_sky, err_message);
  str2.Printf(wxT(" (status = %d\n"), status);
  if(status) err_msg = wxT("Error in Profile_circ_patch") +
                       wxString(err_message, wxConvUTF8) + str2;
}

// If there is a problem (that may occur after a few attempts,
// go back to initial image and exit from while loop:
if(status != 0) {
   for(i = 0; i < nx11 * ny11; i++) dble_image11[i] = saved_data1[i];
   m_gdev_wxwid1->UpdateCImage1Values(dble_image11, nx11, ny11, reset_ITT_to_MinMax);
// Display error message to logfile:
   wxLogError(err_msg);
   status = -2;
   break; }

// If OK, copy work_image1 to dble_image1 and display this version:
  for(i = 0; i < nx11 * ny11; i++) dble_image11[i] = work_image1[i];
   m_gdev_wxwid1->UpdateCImage1Values(dble_image11, nx11, ny11, reset_ITT_to_MinMax);

if(interactive) {  // BEGINNING OF INTERACTIVE CASE
// Try solving problem of wxwidget:
if(PatchDlg != NULL) delete PatchDlg;

// Check whether the user agrees:
PatchDlg = new JLP_Patch_Dlg(NULL, xc, yc, radius, m_radius_fact, m_poly_order,
                             m_sigma_noise, nx11, ny11, m_polynomial_method,
                             wxT("Cosmetic patch"));

// WARNING: There is a bug here coming from "wxwidgets"
// when using ShowModal, the system doesn't return
// The computer may even crash if there are too many of those processed hanging
// around and using CPU time !
 Patch_Dlg_answer = PatchDlg->ShowModal();

// If OK, exit from loop:
 if(Patch_Dlg_answer == 0) {
// status = 0 if image was modified
   status = 0;
   break;
// If no "New try":
 } else if(Patch_Dlg_answer != 2) {
   for(i = 0; i < nx11 * ny11; i++) dble_image11[i] = saved_data1[i];
   m_gdev_wxwid1->UpdateCImage1Values(dble_image11, nx11, ny11, reset_ITT_to_MinMax);
// status = 1 if image was not modified
   status = 1;
   break;
 } else {
// Patch_Dlg_answer = 1:
// New try:
// Since the user does not agree, go back to previous version of the image:
  for(i = 0; i < nx11 * ny11; i++) dble_image11[i] = saved_data1[i];

// Retrieve the patch parameters have another try with those parameters
   PatchDlg->RetrieveData(&xc, &yc, &radius, &m_radius_fact, &m_poly_order,
                          &m_sigma_noise, &m_polynomial_method);
   delete PatchDlg;
   PatchDlg = NULL;
 }
  // END OF INTERACTIVE CASE
} else {
// status = 0 if image was modified
status = 0;
break;   // NON-INTERACTIVE CASE
}

}  // EOF while(1)

if(PatchDlg != NULL) delete PatchDlg;

DELETE_NOISE_ARRAY(noise_array);

delete[] work_image1;
delete[] saved_data1;
// status = 1 if image was not modified
// status = 0 if image was modified
// status = -2 if problem in POLY_CIRC_PATCH
return(status);
}
/***********************************************************************
* Circular background estimation
*
* INPUT:
*  xc, yc: user coordinates of the center of the patch
*  radius: radius of the circular patch
*  interactive: if interactive aknowledgement by the user
*               otherwise automatic background fit
*
* OUTPUT:
*   status = 1 if image was not modified
*          = 0 if image was modified
***********************************************************************/
int Gdp_wx_GDProc2::CircularBackground2(double xc, double yc, double radius,
                                         int interactive)
{
JLP_Patch_Dlg *PatchDlg;
wxString err_msg, str2;
double *dble_image1, *saved_data1;
double *work_image1, *noise_array, sigma_sky;
int nx1, ny1, noise_dim, status;
int Patch_Dlg_answer;
char err_message[128];
register int i;
bool reset_ITT_to_MinMax = false;

PatchDlg = NULL;

// Set default values for background estimation:
// m_poly_order = 0 here by default
m_radius_fact = 1.8;
m_sigma_noise = 0.3;
m_poly_order = 0;
m_polynomial_method = 1;


// Store initial image to "saved_data"
m_image1->GetDoubleArray(&dble_image1, &nx1, &ny1);
saved_data1 = new double[nx1 * ny1];
work_image1 = new double[nx1 * ny1];

for(i = 0; i < nx1 * ny1; i++) saved_data1[i] = dble_image1[i];

noise_dim = 256;

CREATE_NOISE_ARRAY(&noise_array, noise_dim);

while(1) {

// Retrieve original image:
  for(i = 0; i < nx1 * ny1; i++) work_image1[i] = saved_data1[i];

// Process it (Polynomial fitting on the outer ring):
  status = POLY_CIRC_PATCH(work_image1, nx1, ny1, nx1, xc, yc, 2.*radius,
                           m_radius_fact, noise_array, noise_dim,
                           m_sigma_noise, m_poly_order, &sigma_sky,
                           err_message);
// If there is a problem (that may occur after a few attempts),
// go back to initial image and exit from while loop:
  if(status) {
   str2.Printf(wxT(" (status = %d\n"), status);
   err_msg = wxT("Error in Poly_circ_patch") +
                       wxString(err_message, wxConvUTF8) + str2;
   for(i = 0; i < nx1 * ny1; i++) dble_image1[i] = saved_data1[i];
   m_gdev_wxwid1->UpdateCImage1Values(dble_image1, nx1, ny1, reset_ITT_to_MinMax);
// Display error message to logfile:
   wxLogError(err_msg);
   status = -2;
   break; }

// If OK, copy work_image1 to dble_image1 and display this version:
  for(i = 0; i < nx1 * ny1; i++) dble_image1[i] = work_image1[i];
  m_gdev_wxwid1->UpdateCImage1Values(dble_image1, nx1, ny1, reset_ITT_to_MinMax);

// If interactive, check whether the user agrees:
if(interactive) {

if(PatchDlg != NULL) delete PatchDlg;

PatchDlg = new JLP_Patch_Dlg(NULL, xc, yc, radius, m_radius_fact,
                             m_poly_order, m_sigma_noise, nx1, ny1,
                             m_polynomial_method,
                             wxT("Photometric background"));

// WARNING: There is a bug here coming from "wxwidgets"
// when using ShowModal, the system doesn't return
// The computer may even crash if there are too many of those processed hanging
// around and using CPU time !
 Patch_Dlg_answer = PatchDlg->ShowModal();

// If OK, exit from loop:
 if(Patch_Dlg_answer == 0) {
// status = 0 if image was modified
   status = 0;
   break;
// If no "New try":
 } else if(Patch_Dlg_answer != 2) {
   for(i = 0; i < nx1 * ny1; i++) dble_image1[i] = saved_data1[i];
   m_gdev_wxwid1->UpdateCImage1Values(dble_image1, nx1, ny1, reset_ITT_to_MinMax);
// status = 1 if image was not modified
   status = 1;
   break;
 }

// New try:
// Since the user does not agree, go back to previous version of the image:
  for(i = 0; i < nx1 * ny1; i++) dble_image1[i] = saved_data1[i];

// Retrieve the patch parameters have another try with those parameters
   PatchDlg->RetrieveData(&xc, &yc, &radius, &m_radius_fact, &m_poly_order,
                          &m_sigma_noise, &m_polynomial_method);
   delete PatchDlg;
   PatchDlg = NULL;
  // END OF INTERACTIVE CASE
} else {
// status = 0 if image was modified
status = 0;
break;   // NON-INTERACTIVE CASE
}

// And have another try if needed
}  // EOF while(1)

if(PatchDlg != NULL) delete PatchDlg;
DELETE_NOISE_ARRAY(noise_array);

delete[] work_image1;
delete[] saved_data1;
// status = 1 if image was not modified
// status = 0 if image was modified
// status = -2 if problem in POLY_CIRC_PATCH
return(status);
}
