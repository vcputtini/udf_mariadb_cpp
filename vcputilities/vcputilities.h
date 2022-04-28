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

#ifndef VCPUTILITIES_H
#define VCPUTILITIES_H

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) ||                  \
  defined(__WIN64__) || defined(WIN32) || defined(_WIN32) ||                   \
  defined(__WIN32__) || defined(__NT__)
#define Q_DECL_EXPORT __declspec(dllexport)
#define Q_DECL_IMPORT __declspec(dllimport)
#else
#define Q_DECL_EXPORT __attribute__((visibility("default")))
#define Q_DECL_IMPORT __attribute__((visibility("default")))
#endif

#if defined(VCPUTILITIES_LIBRARY)
#define VCPUTILITIES_EXPORT Q_DECL_EXPORT
#else
#define VCPUTILITIES_EXPORT Q_DECL_IMPORT
#endif

#include <mysql.h>
#ifdef USING_MARIADB
#include <mariadb_ctype.h> // Provides interface to UDA/UDF
#else
#include <m_ctype.h> // maybe MySQL
#endif

#include <algorithm>
#include <cfenv> // floating-point exceptions
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits> // std::numeric_limits<> ...
#include <math.h> // fabs etc...
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

#ifdef HAVE_DLOPEN

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

/*! \warning Don't change the values below, this will make the code fail. */
constexpr int ARG_VALUE = 0;
constexpr int ARG_COND = 1;
constexpr int ARG_DATA = 2;

constexpr int ARG_TIME_VALUE = 0;
constexpr int ARG_CONV_TYPE = 1;

/* Utilities --------------------------------------------------------------- */
/*!
 * \internal
 * \brief The Utilities struct
 */
struct VCPUTILITIES_EXPORT Utilities
{
  enum class ConvertionTypes
  {
    HourToMin,
    HourToSec,
    HourToMSec,

    MinToHour,
    MinToSec,
    MinToMSec,

    SecToHour,
    SecToMin,
    SecToMSec,

    MSecToHour,
    MSecToMin,
    MSecToSec,

    toBase10, // from time sexagesimal to centesimal
    toBase60, // from time centesimal to sexagesimal

    Unknown
  };

  enum class Compare
  {
    GT = 0x00, // greater than: >
    GE,        // greater than or equal to: >=
    LT,        // less than: <
    LE,        // less than or equal to: <=
    EQ,        // equal: =
    NE,        // not equal: <>
    NOT_FOUND
  };

  struct Buffer
  {
    Compare comp_ = {}; // don't clear
    double value_ = 0.0;
    int64_t acc_ = 0L;
  };

  const std::unordered_map<std::string_view, Compare> mOp = {
    { ">", Compare::GT },  { ">=", Compare::GE }, { "<", Compare::LT },
    { "<=", Compare::LE }, { "=", Compare::EQ },  { "<>", Compare::NE }
  };

  enum class ErrorID
  {
    ERR_INVALID_ARG = 0x00,
    ERR_WRONG_NUM_ARGS,
    ERR_INVALID_OP,
    ERR_FILE_INVALID_NUM_ARGS,
    ERR_UNKNOWN
  };

  const std::unordered_map<ErrorID, std::string_view> mError = {
    { ErrorID::ERR_INVALID_ARG,
      "Invalid Arg #%d: Type of argument must be numeric." },
    { ErrorID::ERR_WRONG_NUM_ARGS,
      "Wrong number of arguments: (fieldname,'operator',value|fieldname)" },
    { ErrorID::ERR_INVALID_OP,
      "Invalid operator [%s]. Valid are: > >= < <= = <>" },
    { ErrorID::ERR_FILE_INVALID_NUM_ARGS, "" },
    { ErrorID::ERR_UNKNOWN, "Unknown Error." }
  };

  struct ResultErr
  {
    const char* msg = {};
    size_t len = {};
  };

  void getErrorText(ErrorID e_, ResultErr& r_);
  inline bool double_equal(const double lhs_, const double rhs_) const;
  inline Compare isValidOp(const std::string arg_);

  static constexpr double Hr2Min(const double d_ = 0.0);
  static constexpr double Hr2Sec(const double d_ = 0.0);
  static constexpr double Hr2MSec(const double d_ = 0.0);

  static constexpr double Min2Hr(const double d_ = 0.0);
  static constexpr double Min2Sec(const double d_ = 0.0);
  static constexpr double Min2MSec(const double d_ = 0.0);

  static constexpr double Sec2Hr(const double d_ = 0.0);
  static constexpr double Sec2Min(const double d_ = 0.0);
  static constexpr double Sec2MSec(const double d_ = 0.0);

  static constexpr double MSec2Hr(const double d_ = 0.0);
  static constexpr double MSec2Min(const double d_ = 0.0);
  static constexpr double MSec2Sec(const double d_ = 0.0);

  static constexpr double toBase10(const double d_ = 0.0);
  static constexpr double toBase60(const double d_ = 0.0);

}; // struct Utilities

/* MariaDB UDA's implamentation -------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif

  /* AVG_IF() --------------------------------------------------------------- */
  VCPUTILITIES_EXPORT my_bool avg_if_init(UDF_INIT* initid,
                                          UDF_ARGS* args,
                                          char* message);
  VCPUTILITIES_EXPORT void avg_if_deinit(UDF_INIT* initid);
  VCPUTILITIES_EXPORT void avg_if_clear(UDF_INIT* initid,
                                        UDF_ARGS* args,
                                        char* is_null,
                                        char* error);
  VCPUTILITIES_EXPORT void avg_if_reset(UDF_INIT* initid,
                                        UDF_ARGS* args,
                                        char* is_null,
                                        char* error);
  VCPUTILITIES_EXPORT void avg_if_add(UDF_INIT* initid,
                                      UDF_ARGS* args,
                                      char* is_null,
                                      char* error);
  VCPUTILITIES_EXPORT double avg_if(UDF_INIT* initid,
                                    UDF_ARGS* args,
                                    char* is_null,
                                    char* error);

  /* SUM_IF() --------------------------------------------------------------- */
  VCPUTILITIES_EXPORT my_bool sum_if_init(UDF_INIT* initid,
                                          UDF_ARGS* args,
                                          char* message);
  VCPUTILITIES_EXPORT void sum_if_deinit(UDF_INIT* initid);
  VCPUTILITIES_EXPORT void sum_if_clear(UDF_INIT* initid,
                                        UDF_ARGS* args,
                                        char* is_null,
                                        char* error);
  VCPUTILITIES_EXPORT void sum_if_reset(UDF_INIT* initid,
                                        UDF_ARGS* args,
                                        char* is_null,
                                        char* error);
  VCPUTILITIES_EXPORT void sum_if_add(UDF_INIT* initid,
                                      UDF_ARGS* args,
                                      char* is_null,
                                      char* error);
  VCPUTILITIES_EXPORT double sum_if(UDF_INIT* initid,
                                    UDF_ARGS* args,
                                    char* is_null,
                                    char* error);

  /* COUNT_IF() ------------------------------------------------------------ */
  VCPUTILITIES_EXPORT my_bool count_if_init(UDF_INIT* initid,
                                            UDF_ARGS* args,
                                            char* message);
  VCPUTILITIES_EXPORT void count_if_deinit(UDF_INIT* initid);
  VCPUTILITIES_EXPORT void count_if_clear(UDF_INIT* initid,
                                          UDF_ARGS* args,
                                          char* is_null,
                                          char* error);
  VCPUTILITIES_EXPORT void count_if_reset(UDF_INIT* initid,
                                          UDF_ARGS* args,
                                          char* is_null,
                                          char* error);
  VCPUTILITIES_EXPORT void count_if_add(UDF_INIT* initid,
                                        UDF_ARGS* args,
                                        char* is_null,
                                        char* error);
  VCPUTILITIES_EXPORT int64_t count_if(UDF_INIT* initid,
                                       UDF_ARGS* args,
                                       char* is_null,
                                       char* error);

  /* TIME CONVERTIONS ----------------------------------------------------- */
  VCPUTILITIES_EXPORT my_bool time_convert_init(UDF_INIT* initid,
                                                UDF_ARGS* args,
                                                char* message);
  VCPUTILITIES_EXPORT void time_convert_deinit(UDF_INIT* initid);
  VCPUTILITIES_EXPORT double time_convert(UDF_INIT* initid,
                                          UDF_ARGS* args,
                                          char* is_null,
                                          char* error);

#ifdef __cplusplus
}
#endif
#endif // HAVE_DLOPEN
#endif // VCPUTILITIES_H
