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

#ifndef VCPSQUIDLOGPARSER_H
#define VCPSQUIDLOGPARSER_H

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) ||                  \
  defined(__WIN64__) || defined(WIN32) || defined(_WIN32) ||                   \
  defined(__WIN32__) || defined(__NT__)
#define Q_DECL_EXPORT __declspec(dllexport)
#define Q_DECL_IMPORT __declspec(dllimport)
#else
#define Q_DECL_EXPORT __attribute__((visibility("default")))
#define Q_DECL_IMPORT __attribute__((visibility("default")))
#endif

#if defined(VCPSQUIDLOGPARSER_LIBRARY)
#define VCPSQUIDLOGPARSER_EXPORT Q_DECL_EXPORT
#else
#define VCPSQUIDLOGPARSER_EXPORT Q_DECL_IMPORT
#endif

#include <mysql.h>
#ifdef USING_MARIADB
#include <mariadb_ctype.h>
#else
#include <m_ctype.h> // maybe MySQL
#endif

#include <algorithm>
#include <cctype> // ::tolower
#include <cfenv>  // floating-point exceptions
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

#ifdef HAVE_DLOPEN

#include "squidlogparser.h"
using namespace squidlogparser;

using LogFormat = SquidLogParser::LogFormat;
using LogFields = SquidLogParser::Fields;
using SLPError = SquidLogParser::SLPError;

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
constexpr int LOG_FORMAT = 0;
constexpr int LOG_LINE = 1;
constexpr int LOG_PART = 2;
constexpr int URL_PART = 3;

constexpr int ARG_DATA_0 = 0;
constexpr int ARG_DATA_1 = 1;

/* Utilities ---------------------------------------------------------------- */
struct VCPSQUIDLOGPARSER_EXPORT Utilities
{
  struct Buffer
  {
    double value_ = 0.0;
    int64_t acc_ = 0L;
  };

  enum class ErrorID
  {
    ERR_INVALID_TYPE_ARG = 0x00,
    ERR_INVALID_ARG,
    ERR_WRONG_NUM_ARGS,
    ERR_WRONG_NUM_ARGS_1,
    ERR_UNKNOWN
  };

  const std::unordered_map<ErrorID, const std::string> mError = {
    { ErrorID::ERR_INVALID_TYPE_ARG,
      "Invalid Arg #%d: Type of argument must be: %s." },
    { ErrorID::ERR_INVALID_ARG, "Invalid Arg #%d: %s." },
    { ErrorID::ERR_WRONG_NUM_ARGS,
      "Wrong number of arguments: (\"LOG_FORMAT\", FIELD_NAME, \"FIELD-ID\")" },
    { ErrorID::ERR_WRONG_NUM_ARGS_1, "Number of valid arguments: [ 1 ]." },
    { ErrorID::ERR_UNKNOWN, "Unknown Error." }
  };

  const std::unordered_map<std::string_view, LogFields> mRWord = {
    { "timestamp", LogFields::Timestamp },
    { "source_ip_address", LogFields::CliSrcIpAddr },
    { "localtime", LogFields::LocalTime },
    { "username", LogFields::UserName },
    { "usernameident", LogFields::UserNameIdent },
    { "response_time", LogFields::ResponseTime },
    { "request_method", LogFields::ReqMethod },
    { "url", LogFields::ReqURL },
    { "request_proto_ver", LogFields::ReqProtoVersion },
    { "http_status", LogFields::HttpStatus },
    { "reqstatus_hierstatus", LogFields::ReqStatusHierStatus },
    { "total_size_reply", LogFields::TotalSizeReply },
    { "hier_status_server_ip", LogFields::HierStatusIpAddress },
    { "mimetype", LogFields::MimeContentType },
    { "originrcv_reqheader", LogFields::OrigRcvReqHeader },
    { "referrer", LogFields::Referrer },
    { "useragent", LogFields::UserAgent }
  };

  my_bool checkArgs(UDF_INIT* initid, UDF_ARGS* args, char* message);

  struct ResultErr
  {
    const char* msg = {};
    size_t len = {};
  };
  void getErrorText(ErrorID e_, ResultErr& r_);

  inline LogFields getFieldId(const std::string arg_ = std::string()) const;

  template<typename TString = std::string, typename TSize = size_t>
  TString toLower(TString s_, TSize sz_ = 0) const;
};

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C"
{
#endif

  /* integers */
  VCPSQUIDLOGPARSER_EXPORT my_bool slp_int_init(UDF_INIT* initid,
                                                UDF_ARGS* args,
                                                char* message);
  VCPSQUIDLOGPARSER_EXPORT void slp_int_deinit(UDF_INIT* initid);
  VCPSQUIDLOGPARSER_EXPORT int64_t slp_int(UDF_INIT* initid,
                                           UDF_ARGS* args,
                                           char* is_null,
                                           char* error);

  VCPSQUIDLOGPARSER_EXPORT my_bool slp_toUnixTs_init(UDF_INIT* initid,
                                                     UDF_ARGS* args,
                                                     char* message);
  VCPSQUIDLOGPARSER_EXPORT void slp_toUnixTs_deinit(UDF_INIT* initid);
  VCPSQUIDLOGPARSER_EXPORT int64_t slp_toUnixTs(UDF_INIT* initid,
                                                UDF_ARGS* args,
                                                char* is_null,
                                                char* error);

  /* Strings */
  VCPSQUIDLOGPARSER_EXPORT my_bool slp_str_init(UDF_INIT* initid,
                                                UDF_ARGS* args,
                                                char* message);
  VCPSQUIDLOGPARSER_EXPORT void slp_str_deinit(UDF_INIT* initid);
  VCPSQUIDLOGPARSER_EXPORT char* slp_str(UDF_INIT* initid,
                                         UDF_ARGS* args,
                                         char* result,
                                         unsigned long* length,
                                         char* is_null,
                                         char* error);

  VCPSQUIDLOGPARSER_EXPORT my_bool slp_urldecode_init(UDF_INIT* initid,
                                                      UDF_ARGS* args,
                                                      char* message);
  VCPSQUIDLOGPARSER_EXPORT void slp_urldecode_deinit(UDF_INIT* initid);
  VCPSQUIDLOGPARSER_EXPORT char* slp_urldecode(UDF_INIT* initid,
                                               UDF_ARGS* args,
                                               char* result,
                                               unsigned long* length,
                                               char* is_null,
                                               char* error);

  VCPSQUIDLOGPARSER_EXPORT my_bool slp_urlparts_init(UDF_INIT* initid,
                                                     UDF_ARGS* args,
                                                     char* message);
  VCPSQUIDLOGPARSER_EXPORT void slp_urlparts_deinit(UDF_INIT* initid);
  VCPSQUIDLOGPARSER_EXPORT char* slp_urlparts(UDF_INIT* initid,
                                              UDF_ARGS* args,
                                              char* result,
                                              unsigned long* length,
                                              char* is_null,
                                              char* error);

  VCPSQUIDLOGPARSER_EXPORT my_bool slp_toSquidTs_init(UDF_INIT* initid,
                                                      UDF_ARGS* args,
                                                      char* message);
  VCPSQUIDLOGPARSER_EXPORT void slp_toSquidTs_deinit(UDF_INIT* initid);
  VCPSQUIDLOGPARSER_EXPORT char* slp_toSquidTs(UDF_INIT* initid,
                                               UDF_ARGS* args,
                                               char* result,
                                               unsigned long* length,
                                               char* is_null,
                                               char* error);

  /* Aggregations ----------------------------------------------------------- */
  VCPSQUIDLOGPARSER_EXPORT my_bool slp_sum_init(UDF_INIT* initid,
                                                UDF_ARGS* args,
                                                char* message);
  VCPSQUIDLOGPARSER_EXPORT void slp_sum_deinit(UDF_INIT* initid);
  VCPSQUIDLOGPARSER_EXPORT void slp_sum_clear(UDF_INIT* initid,
                                              UDF_ARGS* args,
                                              char* is_null,
                                              char* error);
  VCPSQUIDLOGPARSER_EXPORT void slp_sum_reset(UDF_INIT* initid,
                                              UDF_ARGS* args,
                                              char* is_null,
                                              char* error);
  VCPSQUIDLOGPARSER_EXPORT void slp_sum_add(UDF_INIT* initid,
                                            UDF_ARGS* args,
                                            char* is_null,
                                            char* error);
  VCPSQUIDLOGPARSER_EXPORT int64_t slp_sum(UDF_INIT* initid,
                                           UDF_ARGS* args,
                                           char* is_null,
                                           char* error);
  /* ------------------------------------------------------------------------ */
  VCPSQUIDLOGPARSER_EXPORT my_bool slp_countbyrm_init(UDF_INIT* initid,
                                                      UDF_ARGS* args,
                                                      char* message);
  VCPSQUIDLOGPARSER_EXPORT void slp_countbyrm_deinit(UDF_INIT* initid);
  VCPSQUIDLOGPARSER_EXPORT void slp_countbyrm_clear(UDF_INIT* initid,
                                                    UDF_ARGS* args,
                                                    char* is_null,
                                                    char* error);
  VCPSQUIDLOGPARSER_EXPORT void slp_countbyrm_reset(UDF_INIT* initid,
                                                    UDF_ARGS* args,
                                                    char* is_null,
                                                    char* error);
  VCPSQUIDLOGPARSER_EXPORT void slp_countbyrm_add(UDF_INIT* initid,
                                                  UDF_ARGS* args,
                                                  char* is_null,
                                                  char* error);
  VCPSQUIDLOGPARSER_EXPORT int64_t slp_countbyrm(UDF_INIT* initid,
                                                 UDF_ARGS* args,
                                                 char* is_null,
                                                 char* error);

  /* ------------------------------------------------------------------------ */
  VCPSQUIDLOGPARSER_EXPORT my_bool slp_countbyhttpcode_init(UDF_INIT* initid,
                                                            UDF_ARGS* args,
                                                            char* message);
  VCPSQUIDLOGPARSER_EXPORT void slp_countbyhttpcode_deinit(UDF_INIT* initid);
  VCPSQUIDLOGPARSER_EXPORT void slp_countbyhttpcode_clear(UDF_INIT* initid,
                                                          UDF_ARGS* args,
                                                          char* is_null,
                                                          char* error);
  VCPSQUIDLOGPARSER_EXPORT void slp_countbyhttpcode_reset(UDF_INIT* initid,
                                                          UDF_ARGS* args,
                                                          char* is_null,
                                                          char* error);
  VCPSQUIDLOGPARSER_EXPORT void slp_countbyhttpcode_add(UDF_INIT* initid,
                                                        UDF_ARGS* args,
                                                        char* is_null,
                                                        char* error);
  VCPSQUIDLOGPARSER_EXPORT int64_t slp_countbyrhttpcode(UDF_INIT* initid,
                                                        UDF_ARGS* args,
                                                        char* is_null,
                                                        char* error);

#ifdef __cplusplus
}
#endif

#endif // HAVE_DLOPEN

#endif // VCPSQUIDLOGPARSER_H
