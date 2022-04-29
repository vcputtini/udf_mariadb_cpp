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
#ifndef VCP_LIBLOCATION_UNITCONV_H
#define VCP_LIBLOCATION_UNITCONV_H

#include <mysql.h>

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

/*!
 * \note The USING_MARIADB flag, is defined in the CMakeLists.txt
 * file as follows: -DUSING_MARIADB=1
 */
#ifdef USING_MARIADB
#include <mariadb_ctype.h>
#else
#include <m_ctype.h> // maybe MySQL
#endif

#include "locationtools.h"
#include <boost/algorithm/string.hpp>
#include <cstdio>
#include <cstring>

#ifdef HAVE_DLOPEN

/*! \warning Don't change the values below, this will make the code fail. */
constexpr int ARG_VALUE = 0;
constexpr int ARG_DEC = 1;
constexpr int ARG_TYPE = 2;

constexpr int LAT_PTO_A = 0;
constexpr int LON_PTO_A = 1;
constexpr int UTM_ZONE = 2;

constexpr int UTM_X = 0;
constexpr int UTM_Y = 1;
constexpr int UTM_ZONE_B = 2;
constexpr int SOUTH_HEMI = 3;

#ifdef __cplusplus
extern "C"
{
#endif
  /*!
   * \internal
   * \brief unit_convert :Convenience function to convert between some
   * measurement units.
   * \param IN: (value to be converted, [dec-places,] convertion_type)
   * \param OUT: converted value.
   *
   * \example
   * unit_conv(100.3,"kmh_to_mih") Valid convertion types are:
   * kmh_to_mih|mih_to_kmh|mi_to_meters|meters_to_mi|
   *                             feet_to_meters|meters_to_feet
   */
  LIBLOCATION_EXPORT my_bool unit_convert_init(UDF_INIT* initid,
                                               UDF_ARGS* args,
                                               char* message);
  LIBLOCATION_EXPORT void unit_convert_deinit(UDF_INIT* initid);
  LIBLOCATION_EXPORT double unit_convert(UDF_INIT* initid,
                                         UDF_ARGS* args,
                                         char* is_null,
                                         char* error);

  /* VALIDATIONS ----------------------------------------------------------- */
  /*!
   * \warning  MariaDB doesn't provide a return of boolean values for UDF or
   * UDA.
   */
  LIBLOCATION_EXPORT my_bool check_options(UDF_ARGS* args,
                                           char* message,
                                           char* err_text);

  LIBLOCATION_EXPORT my_bool is_valid_lat_init(UDF_INIT* initid,
                                               UDF_ARGS* args,
                                               char* message);
  LIBLOCATION_EXPORT void is_valid_lat_deinit(UDF_INIT* initid);
  LIBLOCATION_EXPORT int64_t is_valid_lat(UDF_INIT* initid,
                                          UDF_ARGS* args,
                                          char* is_null,
                                          char* error);
  LIBLOCATION_EXPORT my_bool is_valid_lon_init(UDF_INIT* initid,
                                               UDF_ARGS* args,
                                               char* message);
  LIBLOCATION_EXPORT void is_valid_lon_deinit(UDF_INIT* initid);
  LIBLOCATION_EXPORT int64_t is_valid_lon(UDF_INIT* initid,
                                          UDF_ARGS* args,
                                          char* is_null,
                                          char* error);

  /* UTM ------------------------------------------------------------------- */
  LIBLOCATION_EXPORT my_bool coords_to_utm_init(UDF_INIT* initid,
                                                UDF_ARGS* args,
                                                char* message);
  LIBLOCATION_EXPORT void coords_to_utm_deinit(UDF_INIT* initid);
  LIBLOCATION_EXPORT char* coords_to_utm(UDF_INIT* initid,
                                         UDF_ARGS* args,
                                         char* result,
                                         unsigned long* length,
                                         char* is_null,
                                         char* /*error*/);

  LIBLOCATION_EXPORT my_bool utm_to_coords_init(UDF_INIT* initid,
                                                UDF_ARGS* args,
                                                char* message);
  LIBLOCATION_EXPORT void utm_to_coords_deinit(UDF_INIT* initid);
  LIBLOCATION_EXPORT char* utm_to_coords(UDF_INIT* initid,
                                         UDF_ARGS* args,
                                         char* result,
                                         unsigned long* length,
                                         char* is_null,
                                         char* /*error*/);

  LIBLOCATION_EXPORT my_bool utm_zone_init(UDF_INIT* initid,
                                           UDF_ARGS* args,
                                           char* message);
  LIBLOCATION_EXPORT void utm_zone_deinit(UDF_INIT* initid);
  LIBLOCATION_EXPORT int64_t utm_zone(UDF_INIT* initid,
                                      UDF_ARGS* args,
                                      char* is_null,
                                      char* error);

#ifdef __cplusplus
}
#endif

#endif // HAVE_DLOPEN

#endif // VCP_LIBLOCATION_UNITCONV_H
