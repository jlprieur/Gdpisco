#include <stdio.h>
#include "jlp_fitsio.h"

/*
int JLP_compute_epoch(char *date0, const double time0, double *epoch0);
int JLP_julian_day(double aa, int mm, int idd, double time, double *djul);
int JLP_besselian_epoch(double aa, int mm, int idd, double time,
                        double *b_date);
int JLP_jullian_epoch(double aa, int mm, int idd, double time, double *j_date);
*/
int main()
{
double aa, time0, b_date, j_date, j_date_0, djul;
int year0, month0, day0;

printf(" Enter current date as year,month,day,time :\n");
scanf("%d,%d,%d,%ll", &year0, &month0, &day0, &time0);

aa = (double) year0;

JLP_julian_day(aa, month0, day0, time0, &djul);

JLP_besselian_epoch(aa, month0, day0, time0, &b_date);

JLP_julian_epoch(aa, month0, day0, time0, &j_date);

JLP_besselian_to_julian_epoch(b_date, &j_date_0);

printf("Julian day: %.5f Besselian epoch: %.6f Julian epoch: %.6f (=%.6f ?)\n",
        djul, b_date, j_date, j_date_0);

return(0);
}

