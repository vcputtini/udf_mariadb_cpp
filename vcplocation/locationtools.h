/***************************************************************************
 * Copyright (c) 2020-22                                                   *
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

#ifndef VCP_LOCATIONTOOLS_H
#define VCP_LOCATIONTOOLS_H

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) ||                  \
  defined(__WIN64__) || defined(WIN32) || defined(_WIN32) ||                   \
  defined(__WIN32__) || defined(__NT__)
#define Q_DECL_EXPORT __declspec(dllexport)
#define Q_DECL_IMPORT __declspec(dllimport)
#else
#define Q_DECL_EXPORT __attribute__((visibility("default")))
#define Q_DECL_IMPORT __attribute__((visibility("default")))
#endif

#if defined(LIBLOCATION_LIBRARY)
#define LIBLOCATION_EXPORT Q_DECL_EXPORT
#else
#define LIBLOCATION_EXPORT Q_DECL_IMPORT
#endif

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cmath>
#include <cstdlib> // strtod ...
#include <cstring>
#include <iomanip> // std::setprecision
#include <iostream>
#include <sstream> // stringstream
#include <string>
#include <unordered_map>

namespace bptree = boost::property_tree;

#include "UTM.h"

/*!
 * \internal
 * \brief Specific for use in UDF/UDA functions.
 * \note For all other cases use c++ true|false.
 */
#ifndef MY_TRUE
#define MY_TRUE 0
#endif

#ifndef MY_FALSE
#define MY_FALSE 1
#endif

/* Utilities ---------------------------------------------------------------- */
struct LIBLOCATION_EXPORT Utilities
{
  enum class ConvertionTypes
  {
    KmhToMih = 0x00,
    MihToKmh,
    MiToMeters,
    MetersToMi,
    FeetToMeters,
    MetersToFeet,
    Unknown = 0xff
  };

  enum class ErrorID
  {
    ERR_TEXT = 0x00,
    ERR_INVALID_TYPE_ARG,
    ERR_INVALID_ARG,
    ERR_INVALID_NUMBER_ARGS,
    ERR_WRONG_NUM_ARGS,
    ERR_LOCATION,
    ERR_LOCATION_LIMITS,
    ERR_LOCATION_NUMBER_ARGS,
    ERR_UNKNOWN = 0xff
  };

  const std::unordered_map<ErrorID, std::string_view> mError = {
    { ErrorID::ERR_TEXT, "%s" },
    { ErrorID::ERR_INVALID_TYPE_ARG,
      "Invalid Arg #%d: Type of argument must be: %s." },
    { ErrorID::ERR_INVALID_ARG, "Invalid Arg #%d: %s." },
    { ErrorID::ERR_INVALID_NUMBER_ARGS, "Invalid Number of Arguments: %s." },
    { ErrorID::ERR_WRONG_NUM_ARGS, "Wrong number of arguments: %s" },
    { ErrorID::ERR_LOCATION, "Lat/Lon must be a: %s." },
    { ErrorID::ERR_LOCATION_LIMITS, "Range Error: %s." },
    { ErrorID::ERR_LOCATION_NUMBER_ARGS, "Invalid number of arguments: %s." },
    { ErrorID::ERR_UNKNOWN, "Unknown Error." }
  };

  struct ResultErr
  {
    const char* msg = {};
    size_t len = {};
  };
  void getErrorText(ErrorID e_, ResultErr& r_);

  template<typename TString = std::string, typename TSize = size_t>
  TString toLower(TString s_, TSize sz_ = 0) const;
};

struct LIBLOCATION_EXPORT LocationTools
{

  static constexpr double RADIANS = 0.01745329252;
  static constexpr int EARTH_RADIUS_IN_KM = 6371; // WGS 84: 6371.0088 Km
  static constexpr int EARTH_RADIUS_IN_MILES = 3959;

  enum class MetricalType
  {
    Km = 0X00, // Kilometer
    Mi,        // Miles
    Meters
  };

  /*!
   * \note
   *
   * Latitude == UTM coordinate y in meters
   * Longitude == UTM coordinate x in meters
   *
   */
  static double distance(const double lat_a,
                         const double lon_a,
                         const double lat_b,
                         const double lon_b,
                         MetricalType mt = MetricalType::Km);

  static double azimuth(const double lat_a,
                        const double lon_a,
                        const double lat_b,
                        const double lon_b);

  // UTM
  static std::string latlon2utm(const double lat,
                                const double lon,
                                const int zone);
  static std::string utm2latlon(const double x,
                                const double y,
                                const int zone,
                                const bool south_hemi);

  static std::string get_utmvalues(const std::string xy,
                                   const std::string s,
                                   double& val);

  static double todec(const double coord);

  static bool isValidLat(const double coord);
  static bool isValidLon(const double coord);
};

struct LIBLOCATION_EXPORT UnitConverter
{
  static double Kmh2Mph(const double km = 0.0);
  static double Mph2Kmh(const double mi = 0.0);
  static double Mi2Meters(const double mi = 0.0);
  static double Meters2Mi(const double m = 0.0);
  static double Meters2Nm(const double m = 0.0);
  static double Feet2Meters(const double ft = 0.0);
  static double Meters2Feet(const double mt = 0.0);
};

#endif // VCP_LOCATIONTOOLS_H
