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

#include "convertions.h"

#ifdef HAVE_DLOPEN
using UTIL = Utilities;
using ErrID = UTIL::ErrorID;

#ifdef __cplusplus
extern "C"
{
#endif
  struct Temp
  {
    Utilities::ConvertionTypes flag;
  };

  my_bool unit_convert_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    initid->maybe_null = 1;

    UTIL util;
    Temp* temp_ = new Temp;
    initid->ptr = (char*)temp_;

    if (args->arg_count == 3) {
      UTIL::ResultErr r = {};
      if (args->arg_type[ARG_VALUE] != REAL_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 1, "DOUBLE. Use: CAST(VALUE AS DOUBLE)");
        return MY_FALSE;
      }
      if (args->arg_type[ARG_DEC] != INT_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 2, "INTEGER. Range: x >= 0 OR x <= 20");
        return MY_FALSE;
      } else {
        if (*((int*)args->args[ARG_DEC]) > 20) {
          initid->decimals = 20; // limit to 20 dec
        } else {
          initid->decimals = *((int*)args->args[ARG_DEC]);
        }
      }
      if (args->arg_type[ARG_TYPE] != STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 3, "STRING. e.g.: \"kmh_to_mih\"");
        return MY_FALSE;
      } else {
        if (std::strcmp((char*)args->args[ARG_TYPE], "kmh_to_mih") == 0) {
          temp_->flag = Utilities::ConvertionTypes::KmhToMih;
        } else if (std::strcmp((char*)args->args[ARG_TYPE], "mih_to_kmh") ==
                   0) {
          temp_->flag = Utilities::ConvertionTypes::MihToKmh;
        } else if (std::strcmp((char*)args->args[ARG_TYPE], "mi_to_meters") ==
                   0) {
          temp_->flag = Utilities::ConvertionTypes::MiToMeters;
        } else if (std::strcmp((char*)args->args[ARG_TYPE], "meters_to_mi") ==
                   0) {
          temp_->flag = Utilities::ConvertionTypes::MetersToMi;
        } else if (std::strcmp((char*)args->args[ARG_TYPE], "feet_to_meters") ==
                   0) {
          temp_->flag = Utilities::ConvertionTypes::FeetToMeters;
        } else if (std::strcmp((char*)args->args[ARG_TYPE], "meters_to_feet") ==
                   0) {
          temp_->flag = Utilities::ConvertionTypes::MetersToFeet;
        }
      }
    } else {
      UTIL::ResultErr r = {};
      util.getErrorText(ErrID::ERR_WRONG_NUM_ARGS, r);
      std::sprintf(message, r.msg, "(value, [dec], \"convertion_type\")");
      return MY_FALSE;
    }
    return MY_TRUE;
  }

  void unit_convert_deinit([[maybe_unused]] UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  double unit_convert([[maybe_unused]] UDF_INIT* initid,
                      UDF_ARGS* args,
                      [[maybe_unused]] char* is_null,
                      [[maybe_unused]] char* error)
  {
    double d_ = *((double*)args->args[ARG_VALUE]);

    Temp* temp_ = (Temp*)initid->ptr;

    switch (temp_->flag) {
      case Utilities::ConvertionTypes::KmhToMih: {
        return UnitConverter::Kmh2Mph(d_);
      }
      case Utilities::ConvertionTypes::MihToKmh: {
        return UnitConverter::Mph2Kmh(d_);
      }
      case Utilities::ConvertionTypes::MiToMeters: {
        return UnitConverter::Mi2Meters(d_);
      }
      case Utilities::ConvertionTypes::MetersToMi: {
        return UnitConverter::Meters2Mi(d_);
      }
      case Utilities::ConvertionTypes::FeetToMeters: {
        return UnitConverter::Feet2Meters(d_);
      }
      case Utilities::ConvertionTypes::MetersToFeet: {
        return UnitConverter::Meters2Feet(d_);
      }
      default: {
        return 0.0;
      }
    }
  }

  /* VALIDATIONS ----------------------------------------------------------- */

  my_bool check_options(UDF_ARGS* args, char* message, char* err_text)
  {
    UTIL util;

    if (args->arg_count == 1) {
      UTIL::ResultErr r = {};
      if (args->arg_type[0] != REAL_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 1, "DOUBLE. Use: CAST(VALUE AS DOUBLE)");
        return MY_FALSE;
      }

    } else {
      UTIL::ResultErr r = {};
      util.getErrorText(ErrID::ERR_WRONG_NUM_ARGS, r);
      std::sprintf(message, r.msg, err_text);
      return MY_FALSE;
    }
    return MY_TRUE;
  }

  my_bool is_valid_lat_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    initid->maybe_null = 0;

    char err_[] = "Inform Latitude in decimal degrees: [+/-]DD.MMSSSS";

    return check_options(args, message, err_);
  }

  void is_valid_lat_deinit([[maybe_unused]] UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  int64_t is_valid_lat([[maybe_unused]] UDF_INIT* initid,
                       UDF_ARGS* args,
                       [[maybe_unused]] char* is_null,
                       [[maybe_unused]] char* error)
  {
    if (*(double*)args->args[0] < -90.0 || *(double*)args->args[0] > 90.0) {
      return 0;
    }
    return 1;
  }

  /* ------------------------------------------------------------------------ */

  my_bool is_valid_lon_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    initid->maybe_null = 0;

    char err_[] = "Inform Longitude in decimal degrees: [+/-]DDD.MMSSSS";

    return check_options(args, message, err_);
  }

  void is_valid_lon_deinit([[maybe_unused]] UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  int64_t is_valid_lon([[maybe_unused]] UDF_INIT* initid,
                       UDF_ARGS* args,
                       [[maybe_unused]] char* is_null,
                       [[maybe_unused]] char* error)
  {
    if (*(double*)args->args[0] < -180.0 || *(double*)args->args[0] > 180.0) {
      return 0;
    }
    return 1;
  }

  /* UTM ------------------------------------------------------------------- */

  /* coords_to_utm --------------------------------------------------------- */
  my_bool coords_to_utm_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    // args: 0 = latitude; 1 = longitude; 2 = zone
    /*
     * UTM Zone: The UTM system results in the composition of 60 distinct time
     * zones that represent the Earth's surface. Each spindle has an amplitude
     * of 6º of longitude.
     */

    initid->maybe_null = 1;

    UTIL util;

    if (args->arg_count == 3) {
      UTIL::ResultErr r = {};
      if (args->arg_type[LAT_PTO_A] != REAL_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 1, "DOUBLE. Use: CAST(VALUE AS DOUBLE)");
        return MY_FALSE;
      }
      if (args->arg_type[LON_PTO_A] != REAL_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 2, "DOUBLE. Use: CAST(VALUE AS DOUBLE)");
        return MY_FALSE;
      }
      if (args->arg_type[UTM_ZONE] != INT_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 3, "INTEGER. Use: CAST(VALUE AS INTEGER)");
        return MY_FALSE;
      }
    } else {
      UTIL::ResultErr r = {};
      util.getErrorText(ErrID::ERR_LOCATION_NUMBER_ARGS, r);
      std::sprintf(message, r.msg, "(LAT, LON, ZONE).");
      return MY_FALSE;
    }

    return MY_TRUE;
  }

  void coords_to_utm_deinit([[maybe_unused]] UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  char* coords_to_utm([[maybe_unused]] UDF_INIT* initid,
                      UDF_ARGS* args,
                      char* result,
                      unsigned long* length,
                      [[maybe_unused]] char* is_null,
                      [[maybe_unused]] char* error)
  {
    std::string s = LocationTools::latlon2utm(
      *((double*)args->args[LAT_PTO_A]),
      *((double*)args->args[LON_PTO_A]),
      (*((int*)args->args[UTM_ZONE]) == 0)
        ? std::floor((*((double*)args->args[LON_PTO_A]) + 180.0) / 6) + 1
        : *((int*)args->args[UTM_ZONE]));

    std::memmove(result, s.c_str(), s.size());
    *length = s.size();
    return result;
  }

  /* utm_to_coords --------------------------------------------------------- */
  my_bool utm_to_coords_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    initid->maybe_null = 1;

    UTIL util;

    if (args->arg_count == 4) {
      UTIL::ResultErr r = {};
      if (args->arg_type[UTM_X] != REAL_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 1, "DOUBLE. Use: CAST(VALUE AS DOUBLE)");
        return MY_FALSE;
      }
      if (args->arg_type[UTM_Y] != REAL_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 2, "DOUBLE. Use: CAST(VALUE AS DOUBLE)");
        return MY_FALSE;
      }

      if (args->arg_type[UTM_ZONE_B] != INT_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 3, "INTEGER.");
        return MY_FALSE;
      }
      if (*((int*)args->args[UTM_ZONE_B]) < 1 ||
          *((int*)args->args[UTM_ZONE_B]) > 60) {
        util.getErrorText(ErrID::ERR_TEXT, r);
        std::sprintf(message, r.msg, "Arg #3: UTM zone must be informed.");
        return MY_FALSE;
      }

      if (args->arg_type[SOUTH_HEMI] != INT_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(
          message, r.msg, 4, "INTEGER/BOOLEAN. Use: 0 to False and 1 to True.");
        return MY_FALSE;
      }
      if (*((int*)args->args[SOUTH_HEMI]) < 0 ||
          *((int*)args->args[SOUTH_HEMI]) > 1) {
        util.getErrorText(ErrID::ERR_TEXT, r);
        std::sprintf(message, r.msg, "Arg #4: Use: 0 to False and 1 to True.");
        return MY_FALSE;
      }
    } else {
      UTIL::ResultErr r = {};
      util.getErrorText(ErrID::ERR_LOCATION_NUMBER_ARGS, r);
      std::sprintf(message, r.msg, "(UTM X, UTM Y, SOUTH_HEMISQUIT, ZONE)");
      return MY_FALSE;
    }

    return MY_TRUE;
  }

  void utm_to_coords_deinit([[maybe_unused]] UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  char* utm_to_coords([[maybe_unused]] UDF_INIT* initid,
                      UDF_ARGS* args,
                      char* result,
                      unsigned long* length,
                      [[maybe_unused]] char* is_null,
                      [[maybe_unused]] char* /*error*/)
  {

    std::string s = LocationTools::utm2latlon(*((double*)args->args[UTM_X]),
                                              *((double*)args->args[UTM_Y]),
                                              *((int*)args->args[UTM_ZONE_B]),
                                              *((int*)args->args[SOUTH_HEMI]));

    std::memcpy(result, s.c_str(), s.size());
    *length = s.size();
    return result;
  }

  /* Helper ---------------------------------------------------------------- */
  my_bool utm_zone_init([[maybe_unused]] UDF_INIT* initid,
                        [[maybe_unused]] UDF_ARGS* args,
                        [[maybe_unused]] char* message)
  {
    UTIL util;

    initid->maybe_null = 0;
    initid->decimals = 0;

    if (args->arg_count == 1) {
      UTIL::ResultErr r = {};

      if (args->arg_type[ARG_VALUE] != REAL_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 1, "DOUBLE. Use: CAST(VALUE AS DOUBLE)");
        return MY_FALSE;
      }
    } else {
      UTIL::ResultErr r = {};
      util.getErrorText(ErrID::ERR_LOCATION_NUMBER_ARGS, r);
      std::sprintf(message, r.msg, "Inform: Longitude in decimal");
      return MY_FALSE;
    }
    return MY_TRUE;
  }

  void utm_zone_deinit(UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  int64_t utm_zone([[maybe_unused]] UDF_INIT* initid,
                   UDF_ARGS* args,
                   [[maybe_unused]] char* is_null,
                   [[maybe_unused]] char* error)
  {
    int64_t zone = 0;
    zone = std::floor((*((double*)args->args[ARG_VALUE]) + 180.0) / 6) + 1;
    return zone;
  }

#ifdef __cplusplus
}
#endif

#endif // HAVE_DLOPEN
