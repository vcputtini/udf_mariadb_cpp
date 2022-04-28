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
#include "geo_distance.h"

#ifdef HAVE_DLOPEN
using UTIL = Utilities;
using ErrID = UTIL::ErrorID;

#ifdef __cplusplus
extern "C"
{
#endif

  struct Temp
  {
    LocationTools::MetricalType mt;
  };

  my_bool geo_distance_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    UTIL util;
    UTIL::ResultErr r = {};

    initid->maybe_null = 1;

    if (args->arg_count == 5) {
      if ((args->arg_type[LAT_PTO_A] != REAL_RESULT) ||
          (args->arg_type[LON_PTO_A] != REAL_RESULT) ||
          (args->arg_type[LAT_PTO_B] != REAL_RESULT) ||
          (args->arg_type[LON_PTO_B] != REAL_RESULT)) {
        util.getErrorText(ErrID::ERR_LOCATION, r);
        std::sprintf(message, r.msg, "DOUBLE. Use: CAST(VALUE AS DOUBLE)");
        return MY_FALSE;
      }
      if ((*((double*)args->args[LAT_PTO_A]) < -90.0 ||
           *((double*)args->args[LAT_PTO_A]) > 90.0) ||
          (*((double*)args->args[LAT_PTO_B]) < -90.0 ||
           *((double*)args->args[LAT_PTO_B]) > 90.0)) {
        util.getErrorText(ErrID::ERR_LOCATION_LIMITS, r);
        std::sprintf(message, r.msg, "Latitude between -90.0 and 90.0");
        return MY_FALSE;
      }
      if ((*((double*)args->args[LON_PTO_A]) < -180.0 ||
           *((double*)args->args[LON_PTO_A]) > 180.0) ||
          (*((double*)args->args[LON_PTO_B]) < -180.0 ||
           *((double*)args->args[LON_PTO_B]) > 180.0)) {
        util.getErrorText(ErrID::ERR_LOCATION_LIMITS, r);
        std::sprintf(message, r.msg, "Longitude between -180.0 and 180.0");
        return MY_FALSE;
      }

      Temp* temp = new Temp;
      initid->ptr = (char*)temp;

      if (std::strcmp((char*)args->args[METRICTYPE], "km") == 0) {
        temp->mt = LocationTools::MetricalType::Km;
      } else if (std::strcmp((char*)args->args[METRICTYPE], "mi") == 0) {
        temp->mt = LocationTools::MetricalType::Mi;
      } else {
        util.getErrorText(ErrID::ERR_INVALID_ARG, r);
        std::sprintf(message, r.msg, 4, "Valid are: km | mi");
        return MY_FALSE;
      }
    } else {
      util.getErrorText(ErrID::ERR_INVALID_NUMBER_ARGS, r);
      std::sprintf(message, r.msg, "Start Lat/Lon & End Lat/Lon & Metric Type");
      return MY_FALSE;
    }

    return MY_TRUE;
  }

  void geo_distance_deinit(UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  double geo_distance(UDF_INIT* initid,
                      UDF_ARGS* args,
                      [[maybe_unused]] char* is_null,
                      [[maybe_unused]] char* error)
  {

    Temp* t = (Temp*)initid->ptr;
    return LocationTools::distance(*((double*)args->args[LAT_PTO_A]),
                                   *((double*)args->args[LON_PTO_A]),
                                   *((double*)args->args[LAT_PTO_B]),
                                   *((double*)args->args[LON_PTO_B]),
                                   t->mt);
  }

#ifdef __cplusplus
}
#endif

#endif // HAVE_DLOPEN
