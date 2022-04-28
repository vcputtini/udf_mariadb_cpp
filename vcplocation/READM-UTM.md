# UTM <-> WGS84 conversion in C++

[From original: https://alephnull.net/software/gis/UTM_WGS84_C_plus_plus.shtml](https://alephnull.net/software/gis/UTM_WGS84_C_plus_plus.shtml)

## About

This is a simple port of the code on the Geographic/UTM Coordinate Converter
page (offsite link) from Javascript to C++. Using this you can easily convert
between UTM and WGS84 (latitude and longitude). Accuracy seems to be around 50cm
(I suspect rounding errors are limiting precision).
This code is provided as-is and has been minimally tested; enjoy but use at your
 own risk! **The license for UTM.cpp and UTM.h is the same as the original
 Javascript: "The C++ source code in UTM.cpp and UTM.h may be copied and
 reused without restriction."**
Though nominally written in C++ it should compile fine using a C99 compiler
(just rename UTC.cpp to UTC.c).

## Functions
Two main functions are provided:<br>
*int LatLonToUTMXY(FLOAT lat, FLOAT lon, int zone, FLOAT& x, FLOAT& y)*<br>
Convert a latitude/longitude pair (WGS84) to UTM zone/easting/northing.
A zone can be specified; if none is given (zone == 0) then the correct one will
be computed.
Returns the zone used and places the easting and northing (in meters)
into x and y.

*void UTMXYToLatLon (FLOAT x, FLOAT y, int zone, bool southhemi, FLOAT& lat, FLOAT& lon)*<br>
Convert UTM position (zone/easting/northing) to a latitude/longitude pair (WGS84).
If the position is in the northern hemisphere then southhemi should be set to false
If the position is in the southern hemisphere then southhemi should be set to true
No return value; the computed latitude and longitude are passed in lat and lon.
For both of these functions, FLOAT is either double (for 64-bit floating point)
or float (for 32-bit floating point). Since the return values are passed by
reference the type of these variables must be the same in your code and in
the library. See below for details.
