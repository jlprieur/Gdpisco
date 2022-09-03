****************************************************************
With old formula:
*besselian_epoch = 2000. + (djul - 2451544.53)/365.242189;

pisco/Gdpisco$ runs jlp_test_julian_and_bessel
 Enter current date as year,month,day,time :
2018,7,11,6.6
Julian day: 2458310.50000 Besselian epoch: 2018.524612 Julian epoch: 2018.522930 (Besselian epoch converted to Julian with WDS formula: =2018.522938 )

****************************************************************
With new formula:
*besselian_epoch = 1900. + (djul - 2415020.31352)/365.242198781;

 Enter current date as year,month,day,time :
2018,7,11,6.6
ZZZ: besselian_epoch=2018.52461245
ZZZ: new besselian_epoch=2018.52460265
Julian day: 2458310.50000 Besselian epoch: 2018.524603 Julian epoch: 2018.522930 (Besselian epoch converted to Julian with WDS formula: 2018.522929 )
