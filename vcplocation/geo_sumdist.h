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
#ifndef VCP_LIBLOCATION_SUMDIST_H
#define VCP_LIBLOCATION_SUMDIST_H

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

#include <mysql.h>

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
#include <cfenv> // floating-point exceptions
#include <cstdio>
#include <cstring>

constexpr int ARG_LAT = 0;
constexpr int ARG_LON = 1;
constexpr int METRICTYPE = 2;

#ifdef HAVE_DLOPEN

/*!
 * \brief The sum of the distances between the points, resulting in the
 *          total walking distance.
 *
 * \param Starting point: lat/lon in decimal degrees.
 * \return DOUBLE(M,D)
 * \note It can be different from the distance calculated from the coordinates
 *       between the start point A and end point B directly.
 */
#ifdef __cplusplus
extern "C"
{
#endif
  LIBLOCATION_EXPORT my_bool geo_sumdist_init(UDF_INIT* initid,
                                              UDF_ARGS* args,
                                              char* message);
  LIBLOCATION_EXPORT void geo_sumdist_deinit(UDF_INIT* initid);
  LIBLOCATION_EXPORT void geo_sumdist_clear(UDF_INIT* initid,
                                            UDF_ARGS* args,
                                            char* is_null,
                                            char* error);
  LIBLOCATION_EXPORT void geo_sumdist_reset(UDF_INIT* initid,
                                            UDF_ARGS* args,
                                            char* is_null,
                                            char* error);
  LIBLOCATION_EXPORT void geo_sumdist_add(UDF_INIT* initid,
                                          UDF_ARGS* args,
                                          char* is_null,
                                          char* error);
  LIBLOCATION_EXPORT double geo_sumdist(UDF_INIT* initid,
                                        UDF_ARGS* args,
                                        char* is_null,
                                        char* error);
#ifdef __cplusplus
}
#endif

#endif // HAVE_DLOPEN

#endif // VCP_LIBLOCATION_SUMDIST_H
