/***************************************************************************
 * Copyright (c) 2020-2022                                                 *
 *      Volnei Cervi Puttini.  All rights reserved.                        *
 *      vcputtini@gmail.com
 *                                                                         *
 * Redistribution and use in source and binary forms, with or without      *
 * modification, are permitted provided that the following conditions      *
 * are met:                                                                *
 * 1. Redistributions of source code must retain the above copyright       *
 *    notice, this list of conditions and the following disclaimer.        *
 * 2. Redistributions in binary form must reproduce the above copyright    *
 *    notice, this list of conditions and the following disclaimer in the  *
 *    documentation and/or other materials provided with the distribution. *
 * 4. Neither the name of the Author     nor the names of its contributors *
 *    may be used to endorse or promote products derived from this software*
 *    without specific prior written permission.                           *
 *                                                                         *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND *
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   *
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR      *
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS  *
 * BE LIABLEFOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR   *
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF    *
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS*
 * INTERRUPTION)                                                           *
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,     *
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING   *
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE      *
 * POSSIBILITY OFSUCH DAMAGE.                                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "locationtools.h"

/* Utilities ---------------------------------------------------------------- */
using UTIL = Utilities;
using ErrID = UTIL::ErrorID;

/*!
 * \internal
 * \brief Returns the text for the error that occurred.
 * \param e_ Error ID
 * \param r_ Struct containing the text and its length.
 */
void
Utilities::getErrorText(ErrorID e_, ResultErr& r_)
{
  if (std::unordered_map<ErrorID, std::string_view>::const_iterator it_ =
        mError.find(e_);
      it_ != mError.end()) {
    r_.msg = it_->second.data();
    r_.len = std::strlen(it_->second.data()) + 1;
  } else {
    r_.msg = mError.at(ErrorID::ERR_UNKNOWN).data();
    r_.len = std::strlen(r_.msg) + 1;
  }
}

template<typename TString, typename TSize>
TString
Utilities::toLower(TString s_, TSize sz_) const
{
  if (sz_ != s_.size()) {
    s_[sz_] = ::tolower(s_[sz_]);
    s_ = toLower(s_, ++sz_);
  }
  return s_;
}

/* -------------------------------------------------------------------------- */

/*!
 * \brief Calculate the azimuth from latitude and longitude.
 * PARAMS:
 * \param lat_{a|b} / lon_{a|b} Starting point: lat/lon and End point: lat/lon.
 * \return Azimuth in decimal degrees.
 * \note Format: DD (degree.decimal)
 * \note Adapted from orignal: <https://www.omnicalculator.com/other/azimuth>
 */
double
LocationTools::azimuth(const double lat_a,
                       const double lon_a,
                       const double lat_b,
                       const double lon_b)
{
  double phi1 = lat_a * RADIANS; // convert to radians
  double lambda1 = lon_a * RADIANS;
  double phi2 = lat_b * RADIANS;
  double lambda2 = lon_b * RADIANS;

  [[maybe_unused]] double d_phi = phi2 - phi1;
  double d_lambda = lambda2 - lambda1;

  double part1 = std::sin(d_lambda) * std::cos(phi2);
  double part2 = (std::cos(phi1) * std::sin(phi2)) -
                 (std::sin(phi1) * std::cos(phi2) * std::cos(d_lambda));
  double theta = std::atan2(part1, part2);

  double theta_deg = theta / M_PI * 180; // to degree
  if (theta_deg < 0) {
    theta_deg += 360;
  }

  return theta_deg;
}

/*!
 * \brief Returns 'x' Easting or 'y' Northing value of UTM coordinate AND
 *          zone.
 * \param
 *  IN: String json-formated
 *  OUT: xE | yN | zone
 *
 */
std::string
LocationTools::get_utmvalues(const std::string xy,
                             const std::string s,
                             double& val)
{
  if (s.empty()) {
    val = 0.0;
    return "Parameter 2: JSON is empty.";
  }

  std::string opt;
  if (xy == "x") {
    opt = "LatLonToUTMXY.xE";
  } else if (xy == "y") {
    opt = "LatLonToUTMXY.yN";
  } else if (xy == "z") {
    opt = "LatLonToUTMXY.zone";
  } else {
    return "Parameter 1: Must be one of these: 'x' or 'y' or 'z'.";
  }

  bptree::ptree root;
  std::string s_err = {};

  try {
    std::stringstream ss;
    ss << s; // we have to convert to stringstream
    read_json(ss, root);
  } catch (const bptree::ptree_error& e) {
    s_err = e.what();
  }

  std::string s_xyz = {};
  try {
    s_xyz = root.get<std::string>(opt);
  } catch (const bptree::ptree_error& e) {
    s_err = e.what();
  }

  if (s_err.empty()) {
    char* ptEnd;
    val = std::strtod(const_cast<char*>(s_xyz.c_str()), &ptEnd);
  } else {
    val = 0.0;
  }

  return s_err;
}

/*!
 * \brief Works as a middleware between the UTM::LatLonToUTMXY function
 * and the function in the MariaDB API. It returns the information from the
 * calculation in JSON format so that it can later be used via the
 * functions g_getutm_xe () and g_getutm_yn ()
 * \param lat
 * \param lon
 * \param zone
 * \return A JSON-formated string as shown:
 * \example
 * {   "LatLonToUTMXY":{
 *     "xE":"508115.914296",
 *      "yN":"9434849.847182",
 *      "zone":"33"}
 * }
 * \note REMEMBER THAT: Latitude == UTM coordinate y in meters and
 *            Longitude == UTM coordinate x in meters
 *
 * \note Reviewed in: 2022-03-23
 */
std::string
LocationTools::latlon2utm(const double lat, const double lon, const int zone)
{
  double xE = {};
  double yN = {};
  UTM utm;
  int z = utm.LatLonToUTMXY(lat, lon, zone, xE, yN);

  // Format to JSON
  std::stringstream ss;
  ss << "{\"LatLonToUTMXY\":"
     << "{\"xE\""
     << ":\"" << std::to_string(xE) << "\","
     << "\"yN\""
     << ":\"" << std::to_string(yN) << "\","
     << "\"zone\""
     << ":\"" << std::to_string(z) << "\""
     << "}}";

  return ss.str();
}

/*!
 * \brief Works as a middleware between the UTM::UTMXYToLatLon function
 * and the function in the MariaDB API. It returns the information from the
 * calculation in JSON format so that it can later be used via the
 * functions g_getutm_xe () and g_getutm_yn ()

 * \param x
 * \param y
 * \param zone
 * \param south_hemi
 * \return A JSON-formated string as shown:
 * {   "UTMXYToLatLon":{
 *     "lat":"any value in decimal",
 *      "lat":"any value in decimal",
 *      "zone":"33"}
 * }
 * \note REMEMBER THAT: Latitude == UTM coordinate y in meters and
 *       Longitude == UTM coordinate x in meters
 *
 * \note Reviewedin: 2022-03-23
 */
std::string
LocationTools::utm2latlon(const double x,
                          const double y,
                          const int zone,
                          const bool south_hemi)
{
  double d_lat = 0.0;
  double d_lon = 0.0;
  UTM utm;
  utm.UTMXYToLatLon(x, y, zone, south_hemi, d_lat, d_lon);

  std::setprecision(6);
  // Format to JSON
  std::stringstream ss;
  ss << "{\"UTMXYToLatLon\":"
     << "{\"lat\""
     << ":\"" << std::to_string(utm.RadToDeg(d_lat)) << "\","
     << "\"lon\""
     << ":\"" << std::to_string(utm.RadToDeg(d_lon)) << "\","
     << "\"south_hemi\""
     << ":\"" << std::to_string(south_hemi) << "\""
     << "}}";

  return ss.str();
}

/*!
 * \brief Check if latitude is in valid range: [-90,90].
 * \parma cood Latitude in sexagesimal.
 * \return true|false
 * \note Use, Lat/Lon [+/-] in decimal degrees.
 */
bool
LocationTools::isValidLat(const double coord)
{
  return ((int(coord) < -90) || (int(coord) > 90) ? false : true);
}

/*!
 * \brief Check if longitude is in valid range: [-180,180]
 * \param cood Longitude in sexagesimal.
 * \return true|false
 * \note Use, Lat/Lon [+/-] in decimal degrees.
 */
bool
LocationTools::isValidLon(const double coord)
{
  return ((int(coord) < -180) || (int(coord) > 180) ? false : true);
}

/*!
 * \brief sexagesimal coordinate in decimal
 * \param coord Coordenate in sexagesimal.
 * \return Coordenate in decimal.
 * \note Use, GPS ISO 6709: LAT [-]ddmmss.ssss and
 *       LON [-]dddmmss.ssss in sexagesimal.
 */
double
LocationTools::todec(const double coord)
{
  double intdeg = {};
  double fracmin = std::abs(std::modf(coord / 10000, &intdeg)); // minute
  double intmin = {};
  double fracsec = std::abs(std::modf((fracmin)*100, &intmin));
  double min = intmin + (fracsec * 100) / 60;
  double d = std::abs(intdeg) + (min / 60);

  return (intdeg < 0 ? d * (-1) : d);
}

/*!
 * \note Calculates the straight-line distance between two geographic
 * points,taking into account the curvature of the earth.
 *
 * \param lat_a & lon_a = Latitude and Longitude of A Point in decimal
 * \parma lat_b & lon_b = latitude and Longitude of B Point in decimal
 * \return Distance in Km or Mi.
 */
double
LocationTools::distance(const double lat_a,
                        const double lon_a,
                        const double lat_b,
                        const double lon_b,
                        MetricalType mt)
{
  switch (mt) {
    case MetricalType::Km: {
      return EARTH_RADIUS_IN_KM *
             std::acos(std::cos(M_PI * (90 - lat_b) / 180) *
                         std::cos((90 - lat_a) * M_PI / 180) +
                       std::sin((90 - lat_b) * M_PI / 180) *
                         std::sin((90 - lat_a) * M_PI / 180) *
                         std::cos((lon_a - lon_b) * M_PI / 180));
    }
    case MetricalType::Mi: {
      return EARTH_RADIUS_IN_MILES *
             std::acos(std::cos(M_PI * (90 - lat_b) / 180) *
                         std::cos((90 - lat_a) * M_PI / 180) +
                       std::sin((90 - lat_b) * M_PI / 180) *
                         std::sin((90 - lat_a) * M_PI / 180) *
                         std::cos((lon_a - lon_b) * M_PI / 180));
    }
    default: {
      return 0.0;
    }
  }
}
/*!
 * \brief KMH to MPH convertion.
 */
double
UnitConverter::Kmh2Mph(const double km)
{
  return (km * 0.621371);
}

/*!
 * \brief MPH to KMH convertion.
 */
double
UnitConverter::Mph2Kmh(const double mi)
{
  return (mi * 1.60934);
}

/*!
 * \brief land MI to Meters convertion.
 */
double
UnitConverter::Mi2Meters(const double mi)
{
  return (mi * 1609.34);
}

/*!
 * \brief Meters to land miles convertion.
 */
double
UnitConverter::Meters2Mi(const double m)
{
  /* 1 meter == 6.2E-04 miles */
  return (m * 0.00062137119);
}

/*!
 * \brief Meters to Nautical Miles convertion.
 */
double
UnitConverter::Meters2Nm(const double m)
{
  return (m * 0.000547045);
}

/*!
 * \brief Feet to meters convertion.
 */
double
UnitConverter::Feet2Meters(const double ft)
{
  return (ft * 0.3048);
}

/*!
 * \brief Meters to feet convertion.
 */
double
UnitConverter::Meters2Feet(const double mt)
{
  return (mt * 3.2808);
}
