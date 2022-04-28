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
#include "geo_sumdist.h"

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
    bool flag;
    double tmp_a = 0.0;
    double tmp_b = 0.0;
    double dist = 0.0;
  };

  my_bool geo_sumdist_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    UTIL util;
    UTIL::ResultErr r = {};

    Temp* temp = new Temp;
    temp->flag = true;
    temp->tmp_a = 0.0;
    temp->tmp_b = 0.0;
    temp->dist = 0.0;

    initid->ptr = (char*)temp;
    initid->maybe_null = 1;

    if (args->arg_count == 2 && (args->arg_type[ARG_LAT] == REAL_RESULT &&
                                 args->arg_type[ARG_LON] == REAL_RESULT)) {
      temp->mt = LocationTools::MetricalType::Km;
    } else if (args->arg_count == 3 &&
               (args->arg_type[ARG_LAT] == REAL_RESULT &&
                args->arg_type[ARG_LON] == REAL_RESULT &&
                args->arg_type[METRICTYPE] == STRING_RESULT)) {
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
      std::sprintf(message, r.msg, "Inform Latitude & Longitude");
      return MY_FALSE;
    }
    return MY_TRUE;
  }

  void geo_sumdist_deinit(UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  void geo_sumdist_clear(UDF_INIT* initid,
                         [[maybe_unused]] UDF_ARGS* args,
                         [[maybe_unused]] char* is_null,
                         [[maybe_unused]] char* error)
  {
    Temp* temp = (Temp*)initid->ptr;
    temp->flag = true;
    temp->tmp_a = 0.0;
    temp->tmp_b = 0.0;
    temp->dist = 0.0;
  }

  void geo_sumdist_reset(UDF_INIT* initid,
                         UDF_ARGS* args,
                         char* is_null,
                         char* error)
  {
    geo_sumdist_clear(initid, args, is_null, error);
    geo_sumdist_add(initid, args, is_null, error);
  }

  void geo_sumdist_add(UDF_INIT* initid,
                       UDF_ARGS* args,
                       [[maybe_unused]] char* is_null,
                       char* error)
  {
    std::feclearexcept(FE_ALL_EXCEPT);

    if (*((double*)args->args[ARG_LAT]) < -90.0 ||
        *((double*)args->args[ARG_LAT]) > 90.0) {
      *error = 1;
      return;
    }
    if (*((double*)args->args[ARG_LON]) < -180.0 ||
        *((double*)args->args[ARG_LON]) > 180.0) {
      *error = 1;
      return;
    }

    Temp* t = (Temp*)initid->ptr;
    if (t->flag) {
      t->tmp_a = *((double*)args->args[ARG_LAT]);
      t->tmp_b = *((double*)args->args[ARG_LON]);
      t->flag = false;
    } else {
      double d = LocationTools::distance(t->tmp_a,
                                         t->tmp_b,
                                         *((double*)args->args[ARG_LAT]),
                                         *((double*)args->args[ARG_LON]),
                                         t->mt);
      if (std::fetestexcept(FE_OVERFLOW)) {
        *error = 1;
      }
      t->dist += d;
      t->tmp_a = *((double*)args->args[ARG_LAT]);
      t->tmp_b = *((double*)args->args[ARG_LON]);
    }
  }

  double geo_sumdist(UDF_INIT* initid,
                     [[maybe_unused]] UDF_ARGS* args,
                     [[maybe_unused]] char* is_null,
                     [[maybe_unused]] char* error)
  {
    Temp* t = (Temp*)initid->ptr;

    return t->dist;
  }

#ifdef __cplusplus
}
#endif

#endif // HAVE_DLOPEN
