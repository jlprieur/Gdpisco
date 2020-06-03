/******************************************************************************
* Name:        gdp_frame_hartmann (GdpFrame class)
* Purpose:     Shack-Hartmann wavefront analysis for Gdpisco.cpp
* Author:      JLP
* Version:     25/05/2016
******************************************************************************/
#include "gdp_frame.h"
#include "jlp_wx_ipanel.h"      // JLP_wxImagePanel class
#include "jlp_fitsio.h"         // for JLP_RDFITS_2D
#include "jlp_numeric.h"        // JLP_QSORT_DOUBLE

static int MakePupilMask(double *pupil_image0, double *pupil_mask0,
                         int nx0, int ny0, double pupil_median_frac,
                         double *pupil_threshold);
static void SavePupilMaskToFITS(double *pupil_mask0, int nx0, int ny0,
                                char *comments0);
static int SaveListToAscii(double *peak_x, double *peak_y, int npeaks,
                           char *comments0);

/*************************************************************************
* Automatic measurement of phase wavefront (Shack-Hartmann sensor)
**************************************************************************/
void GdpFrame::OnAutoMeasureHartmann(wxCommandEvent& event)
{
wxString full_filename, filename, path, fname, extension;
char filename1[128], comments1[128], errmess1[128], comments0[256];
double *pupil_image0, *peak_x, *peak_y;
double *pupil_mask0, pupil_threshold, pupil_median_frac;
double x_half_width, y_half_width, sigma_threshold;
int npeaks, npeaks_maxi;
int nx0, ny0, nz0, iframe, status;

if((initialized != 1234) || (m_panel == NULL)) return;

// Enquire the name of the pupil image file and open it
  full_filename = wxFileSelector(_T("Select pupil image FITS file"), _T(""),
                      _T(""), _T("fits|fit|FIT"),
                      _T("FITS files (*.fits;*.fit;*.FIT)|*.fits;*.fit;*.FIT"));
  if( full_filename.empty() ) return;

// Removes the directory name (since the full path is generally too long...)
  wxFileName::SplitPath(full_filename, &path, &fname, &extension);
  filename = fname + _T(".") + extension;

// dble_image1: array with the data contained in FITS file
// nx0, ny0, nz0: size of data cube
// iframe: index of image plane to be loaded from 1 to nz (in case of 3D data)
iframe = 1;
strncpy(filename1, full_filename.mb_str(), 128);
status = JLP_RDFITS_2D_dble(&pupil_image0, &nx0, &ny0, &nz0, iframe, filename1,
                             comments1, errmess1);
if(status)return;

if((nx0 != nx1) || (ny0 != ny1)){
 fprintf(stderr, "OnAutoMeasureHartmann/Error pupil image size is not compatible with displayed image !\n");
 delete[] pupil_image0;
 return;
 }

// Allocate memory for the peak coordinates arrays
// DEBUG: set it to maximum value for safety...
npeaks_maxi = nx1 * ny1;

peak_x = new double[npeaks_maxi];
peak_y = new double[npeaks_maxi];

/* Look for the maxima:
* x_half_width, y_half_width : half size of the box around the pixel to
*                              be examined
* peak_x, peak_y : x,y coordinates of the peaks
* npeaks : number of peaks
* npeaks_max : size of peak_x and peak_y
*/

 x_half_width = 10;
 y_half_width = 10;
 sigma_threshold = 20.;
 pupil_median_frac = 0.4;

// Prompt for new parameters:
 LoadNewWavefrontParameters(&x_half_width, &y_half_width, &sigma_threshold,
                            &pupil_median_frac);

// Create pupil mask:
pupil_mask0 = new double[nx0 * ny0];
MakePupilMask(pupil_image0, pupil_mask0, nx0, ny0, pupil_median_frac,
              &pupil_threshold);
sprintf(comments0, "From %s with threshold=%f\n",
        (const char *)filename.mb_str(), pupil_threshold);

// Save pupil mask to FITS file:
SavePupilMaskToFITS(pupil_mask0, nx0, ny0, comments0);

// Look for maxima inside the displayed image
// and display the result of the search as a new image:
m_panel->AutoMeasureShackHartmann(pupil_mask0, nx0, ny0,
                                  x_half_width, y_half_width, sigma_threshold,
                                  peak_x, peak_y, &npeaks, npeaks_maxi);
printf("AutoMeasureShackHartmann/ npeaks=%d\n", npeaks);

// Save to file:
sprintf(comments0, "From %s and %s (xhw, yhw, pthr, pmed: %.1f %.1f %.1f %.2f)",
        (const char *)m_filename1.mb_str(), (const char *)filename.mb_str(),
        x_half_width, y_half_width, pupil_threshold, pupil_median_frac);
SaveListToAscii(peak_x, peak_y, npeaks, comments0);

delete[] peak_x;
delete[] peak_y;
delete[] pupil_image0;
delete[] pupil_mask0;
return;
}
/************************************************************************
* Output pupil mask to FITS file
************************************************************************/
static void SavePupilMaskToFITS(double *pupil_mask0, int nx0, int ny0,
                                char *comments0)
{
wxString filename;
char fits_filename[128], errmess0[128];

 wxFileDialog dialog(NULL, _T("Save pupil mask to FITS file"), wxEmptyString,
                     wxEmptyString, _T("Files (*.fits)|*.fits"),
                     wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
 if (dialog.ShowModal() != wxID_OK) return;

 filename = dialog.GetPath();
 strcpy(fits_filename, (const char *)filename.mb_str());
 JLP_WRFITS_2D_dble(pupil_mask0, nx0, ny0, nx0, fits_filename, comments0,
                   errmess0);

return;
}
/*************************************************************************
* Prompt for new parameters for wavefront analysis
*************************************************************************/
void GdpFrame::LoadNewWavefrontParameters(double *x_half_width,
                                          double *y_half_width,
                                          double *sigma_threshold,
                                          double *pupil_median_frac)
{
wxString s_values, s_question;
double ww1, ww2, ww3;
char buffer[64];

s_question.Printf(wxT("New values of half_width, sigma_threshold, pupil_median_frac"));

s_values.Printf(wxT("%.2f,%.2f,%.2f"), *x_half_width, *sigma_threshold,
                *pupil_median_frac);


// Prompt for a new value of iframe with a dialog box:
wxString result = wxGetTextFromUser(s_question,
_T("Load new wavefront parameters"), s_values, NULL);

if(!result.IsEmpty()){
  strncpy(buffer, (const char *)result.mb_str(), 64);
  if(sscanf(buffer, "%lf,%lf,%lf", &ww1, &ww2, &ww3) == 3) {
    *x_half_width = ww1;
    *y_half_width = ww1;
    *sigma_threshold = ww2;
    *pupil_median_frac = ww3;
    printf("OK: x_hwidth=%f y_hwidth=%f sigma=%f pupil_median_frac=%f\n",
           *x_half_width, *y_half_width, *sigma_threshold, *pupil_median_frac);
    } else {
    fprintf(stderr, "Load new wavefront parameters/Error : wrong syntax !\n");
    }
  }

return;
}
/*************************************************************************
**************************************************************************/
static int MakePupilMask(double *pupil_image0, double *pupil_mask0,
                         int nx0, int ny0, double pupil_median_frac,
                         double *pupil_threshold)
{
int i, j, imedian;
double *tmp_array, pupil_npts;
double pupil_surface;

// Look for pupil_threshold on the central line:
tmp_array = new double[nx0];

j = ny0 / 2;
for(i = 0; i < nx0; i++) tmp_array[i] = pupil_image0[i + j * nx0];

// Sort the array corresponding to bin #i in increasing order
// Defined in "jlp_sort.c" (prototypes in jlp_numeric.h):
// int JLP_QSORT_DBLE(double *array, int *nn);
JLP_QSORT_DBLE(tmp_array, &nx0);
// Median corresponds to the middle point tmp_array[ npixels / 2]
// I take something lower: (e.g. median_frac = 0.4) :
imedian = (int)(pupil_median_frac * (double)nx0);
*pupil_threshold = tmp_array[imedian];
delete[] tmp_array;

for(i = 0; i < nx0 * ny0; i++) pupil_mask0[i] = 0.;

pupil_npts = 0.;
 for(i = 0; i < nx0 * ny0; i++) {
// Check if pixel is inside the pupil:
// Should use >= instead of > to allow detection of binary pupil masks (0,1)
   if(pupil_image0[i] >= *pupil_threshold) {
     pupil_npts++;
     pupil_mask0[i] = 1.;
     } // if pupil_image0[i] >= *pupil_threshold
  }

// Surface of pupil:
pupil_surface =  pupil_npts/(double)(nx0 * ny0);
printf("Surface of pupil: %f of full surface (threshold=%f)\n",
       pupil_surface, *pupil_threshold);

return(0);
}
/****************************************************************************
*
****************************************************************************/
static int SaveListToAscii(double *peak_x, double *peak_y, int npeaks,
                           char *comments0)
{
wxString filename;
FILE *fp_out;
char list_fname[128];
int i;

 wxFileDialog dialog(NULL, _T("Save spot coordinates to ASCII file"),
                     wxEmptyString, wxEmptyString, _T("Files (*.txt)|*.txt"),
                     wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
 if (dialog.ShowModal() != wxID_OK) return(-1);

 filename = dialog.GetPath();
 strcpy(list_fname, (const char *)filename.mb_str());

if((fp_out = fopen(list_fname, "w")) == NULL) return(-1);

fprintf(fp_out, "%% %s\n", comments0);
fprintf(fp_out, "%% nspots=%d\n", npeaks);
for(i = 0; i < npeaks; i++) {
  fprintf(fp_out, "%.2f %.2f\n", peak_x[i], peak_y[i]);
  }

fclose(fp_out);

return(0);

}
