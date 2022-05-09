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

#include "squidlogparser_udf.h"

#ifdef HAVE_DLOPEN

/* Utilities ---------------------------------------------------------------- */
using UTIL = Utilities;
using ErrID = UTIL::ErrorID;

my_bool
Utilities::checkArgs([[maybe_unused]] UDF_INIT* initid,
                     UDF_ARGS* args,
                     char* message)
{
  UTIL util;

  if (args->arg_count >= 3) {
    UTIL::ResultErr r = {};
    if (args->arg_type[LOG_FORMAT] != STRING_RESULT) {
      util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
      std::sprintf(message, r.msg, 1, "String");
      return MY_FALSE;
    } else {
      std::string s_(args->args[LOG_FORMAT], args->lengths[LOG_FORMAT]);
      static constexpr std::string_view f_ =
        "squidcommoncombinedreferreruseragent";
      if (std::size_t found = f_.find(util.toLower(s_));
          found == std::string::npos) {
        util.getErrorText(ErrID::ERR_INVALID_ARG, r);
        std::sprintf(message,
                     r.msg,
                     1,
                     "Valid are: squid|common|combined|referrer|useragent");
        return MY_FALSE;
      }
    }
    if (args->arg_type[LOG_LINE] != STRING_RESULT) {
      util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
      std::sprintf(message, r.msg, 2, "Table field's name");
      return MY_FALSE;
    }
    if (args->arg_type[LOG_PART] != STRING_RESULT) {
      util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
      std::sprintf(message, r.msg, 3, "Field-id");
      return MY_FALSE;
    }

    if (args->arg_count == 4) {
      std::string log_part_(args->args[LOG_PART], args->lengths[LOG_PART]);
      std::transform(
        log_part_.begin(), log_part_.end(), log_part_.begin(), ::tolower);
      if (log_part_ == "url") {
        if (args->arg_type[URL_PART] != STRING_RESULT) {
          util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
          std::sprintf(message, r.msg, 4, "(STRING) URL-part");
          return MY_FALSE;
        }
      }
    }
  } else {
    UTIL::ResultErr r = {};
    util.getErrorText(ErrID::ERR_WRONG_NUM_ARGS, r);
    std::memmove(message, r.msg, r.len);
    return MY_FALSE;
  }

  return MY_TRUE;
}

/*!
 * \internal
 * \brief Returns the text for the error that occurred.
 * \param e_ Error ID
 * \param r_ Struct containing the text and its length.
 */
void
Utilities::getErrorText(ErrorID e_, ResultErr& r_)
{
  if (std::unordered_map<ErrorID, const std::string>::const_iterator it_ =
        mError.find(e_);
      it_ != mError.end()) {
    r_.msg = it_->second.c_str();
    r_.len = std::strlen(it_->second.c_str()) + 1;
  } else {
    r_.msg = mError.at(ErrorID::ERR_UNKNOWN).c_str();
    r_.len = std::strlen(r_.msg) + 1;
  }
}

LogFields
Utilities::getFieldId(const std::string arg_) const
{
  const std::string si_(toLower(arg_));
  if (const auto& it_ = mRWord.find(si_.data()); it_ != mRWord.end()) {
    return it_->second;
  } else {
    return LogFields::Unknown;
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

#ifdef __cplusplus
extern "C"
{
#endif

  my_bool slp_int_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    UTIL util;

    initid->maybe_null = 1;
    initid->decimals = 0;
    initid->max_length = 20;

    return util.checkArgs(initid, args, message);
  }

  void slp_int_deinit([[maybe_unused]] UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  /*!
   * \brief slparserint
   * \param is_null
   * \param error
   * \return int64_t
   *
   * \attention Returns the value LONG_MIN in case there's an error in parsing
   * the log line.
   */
  int64_t slp_int([[maybe_unused]] UDF_INIT* initid,
                  [[maybe_unused]] UDF_ARGS* args,
                  [[maybe_unused]] char* is_null,
                  [[maybe_unused]] char* error)
  {
    const std::string log_fmt_(args->args[LOG_FORMAT],
                               args->lengths[LOG_FORMAT]);
    const std::string raw_log_(args->args[LOG_LINE], args->lengths[LOG_LINE]);
    const std::string log_part_(args->args[LOG_PART], args->lengths[LOG_PART]);

    // MUST BE check if raw_log_ is empty
    if (raw_log_.empty()) {
      return LONG_MIN;
    }

    UTIL util;

    SquidLogParser p(util.toLower(log_fmt_));
    p.append(raw_log_);
    if (p.errorNum() != SLPError::SLP_SUCCESS) {
      return LONG_MIN;
    }

    return ((util.getFieldId(log_part_) == LogFields::CliSrcIpAddr) ||
            (util.getFieldId(log_part_) == LogFields::Timestamp))
             ? static_cast<int64_t>(p.getPartUInt(util.getFieldId(log_part_)))
             : p.getPartInt(util.getFieldId(log_part_));
  }

  my_bool slp_toUnixTs_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    UTIL util;

    initid->maybe_null = 1;
    initid->decimals = 0;
    initid->max_length = 20;

    if (args->arg_count == 1) {
      UTIL::ResultErr r = {};
      if (args->arg_type[ARG_DATA_0] != STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 1, "(STRING)");
        return MY_FALSE;
      }

    } else {
      UTIL::ResultErr r = {};
      util.getErrorText(ErrID::ERR_WRONG_NUM_ARGS, r);
      std::memmove(message, r.msg, r.len);
      return MY_FALSE;
    }

    return MY_TRUE;
  }

  void slp_toUnixTs_deinit(UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  int64_t slp_toUnixTs([[maybe_unused]] UDF_INIT* initid,
                       UDF_ARGS* args,
                       [[maybe_unused]] char* is_null,
                       [[maybe_unused]] char* error)
  {
    const std::string ts_(args->args[ARG_DATA_0], args->lengths[ARG_DATA_0]);

    SquidLogParser p;
    int64_t result_ = static_cast<int64_t>(p.unixTimestamp(ts_));

    return result_;
  }

  /* String ----------------------------------------------------------------- */
  my_bool slp_str_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    UTIL util;

    initid->maybe_null = 1;

    return util.checkArgs(initid, args, message);
  }

  void slp_str_deinit([[maybe_unused]] UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  /*!
   * \brief slparserstr
   *
   * \return char* The user-chosen part of the log line.
   *
   * \attention At this moment, if there's any error in the parsing of the log
   * line, it'll be returned in its original form.
   * This can happen, for example, if there're white spaces "%20" in the
   * formation of a URL after it has been decoded.
   */
  char* slp_str([[maybe_unused]] UDF_INIT* initid,
                UDF_ARGS* args,
                char* result,
                unsigned long* length,
                [[maybe_unused]] char* is_null,
                [[maybe_unused]] char* error)
  {
    const std::string log_fmt_(args->args[LOG_FORMAT],
                               args->lengths[LOG_FORMAT]);
    const std::string log_part_(args->args[LOG_PART], args->lengths[LOG_PART]);
    const std::string raw_log_(args->args[LOG_LINE], args->lengths[LOG_LINE]);

    // MUST BE check if raw_log_ is empty
    if (raw_log_.empty()) {
      return nullptr;
    }

    UTIL util;

    SquidLogParser* p = new SquidLogParser(util.toLower(log_fmt_));
    p->append(raw_log_);
    if (p->errorNum() != SLPError::SLP_SUCCESS) {
      delete p;
      result = new char[raw_log_.size()];
      std::strncpy(result, raw_log_.c_str(), raw_log_.size());
      *length = static_cast<unsigned long>(raw_log_.size());
      return result;
    }

    std::string str_;
    if (args->arg_count == 4) {
      const std::string url_part_(args->args[URL_PART],
                                  args->lengths[URL_PART]);

      if (std::string_view{ raw_log_ }.find("://") != std::string::npos) {
        str_ = p->getUrlParts(url_part_);
      } else {
        return nullptr;
      }
    } else {
      str_ = p->getPartStr(util.getFieldId(log_part_));
    }
    delete p;

    result = new char[str_.size()];
    std::strncpy(result, str_.c_str(), str_.size());
    *length = static_cast<unsigned long>(str_.size());

    return result;
  }

  /*!
   * \brief Show decoded URL.
   * \param initid
   * \param args
   * \param message
   * \return A URL decoded or NULL.
   */
  my_bool slp_urldecode_init([[maybe_unused]] UDF_INIT* initid,
                             UDF_ARGS* args,
                             char* message)
  {
    initid->maybe_null = 1;

    UTIL util;

    if (args->arg_count == 1) {
      UTIL::ResultErr r;
      if (args->arg_type[ARG_DATA_0] != STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 1, "A Valid URL format");
        return MY_FALSE;
      }
      return MY_TRUE;
    } else {
      Utilities::ResultErr r;
      util.getErrorText(ErrID::ERR_WRONG_NUM_ARGS_1, r);
      std::memmove(message, r.msg, r.len);
    }
    return MY_FALSE;
  }

  void slp_urldecode_deinit([[maybe_unused]] UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  char* slp_urldecode([[maybe_unused]] UDF_INIT* initid,
                      UDF_ARGS* args,
                      char* result,
                      unsigned long* length,
                      [[maybe_unused]] char* is_null,
                      [[maybe_unused]] char* error)
  {
    const std::string url_(args->args[ARG_DATA_0], args->lengths[ARG_DATA_0]);

    std::string tmp_;
    tmp_.resize(url_.size());

    if (std::string_view{ url_ }.find("://") != std::string::npos) {
      tmp_ = SquidLogParser::UrlDecode(std::move(url_));
    } else {
      return nullptr;
    }

    result = new char[tmp_.size()];
    std::strncpy(result, tmp_.c_str(), tmp_.size());
    *length = static_cast<unsigned long>(tmp_.size());

    return result;
  }

  my_bool slp_urlparts_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {

    initid->maybe_null = 1;

    UTIL util;

    if (args->arg_count == 2) {
      UTIL::ResultErr r;
      if (args->arg_type[ARG_DATA_0] != STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 1, "(STRING) URL");
        return MY_FALSE;
      }
      if (args->arg_type[ARG_DATA_1] != STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 2, "(STRING) URL-part");
        return MY_FALSE;
      } else {
        const std::string s_(args->args[ARG_DATA_1], args->lengths[ARG_DATA_1]);
        static constexpr std::string_view f_ =
          "schemedomainusernamepasswordpathqueryfragment";
        if (std::size_t found = f_.find(util.toLower(s_));
            found == std::string::npos) {
          util.getErrorText(ErrID::ERR_INVALID_ARG, r);
          std::sprintf(
            message,
            r.msg,
            2,
            "Valid are: scheme|domain|username|password|path|query|fragment");
          return MY_FALSE;
        }
      }
    } else {
      Utilities::ResultErr r;
      util.getErrorText(ErrID::ERR_WRONG_NUM_ARGS_1, r);
      std::memmove(message, r.msg, r.len);
      return MY_FALSE;
    }

    return MY_TRUE;
  }

  void slp_urlparts_deinit(UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  char* slp_urlparts([[maybe_unused]] UDF_INIT* initid,
                     UDF_ARGS* args,
                     char* result,
                     unsigned long* length,
                     [[maybe_unused]] char* is_null,
                     [[maybe_unused]] char* error)
  {
    const std::string url_(args->args[ARG_DATA_0], args->lengths[ARG_DATA_0]);
    const std::string part_(args->args[ARG_DATA_1], args->lengths[ARG_DATA_1]);

    UTIL util;
    std::string str_;

    SLPUrlParts url_part_(url_);

    if (util.toLower(part_) == "scheme") {
      str_ = url_part_.getScheme();
    } else if (util.toLower(part_) == "domain") {
      str_ = url_part_.getDomain();
    } else if (util.toLower(part_) == "username") {
      str_ = url_part_.getUsername();
    } else if (util.toLower(part_) == "password") {
      str_ = url_part_.getPassword();
    } else if (util.toLower(part_) == "path") {
      str_ = url_part_.getPath();
    } else if (util.toLower(part_) == "query") {
      str_ = url_part_.getQuery();
    } else if (util.toLower(part_) == "fragment") {
      str_ = url_part_.getFragment();
    } else {
      str_ = std::string();
    }

    result = new char[str_.size()];
    std::strncpy(result, str_.c_str(), str_.size());
    *length = static_cast<unsigned long>(str_.size());

    return result;
  }

  my_bool slp_toSquidTs_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    initid->maybe_null = 1;
    initid->decimals = 0;
    initid->max_length = 20;

    UTIL util;

    if (args->arg_count == 1) {
      UTIL::ResultErr r;
      if (args->arg_type[ARG_DATA_0] != INT_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 1, "(INTEGER))");
        return MY_FALSE;
      }
    } else {
      Utilities::ResultErr r;
      util.getErrorText(ErrID::ERR_WRONG_NUM_ARGS_1, r);
      std::memmove(message, r.msg, r.len);
      return MY_FALSE;
    }
    return MY_TRUE;
  }

  void slp_toSquidTs_deinit(UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  char* slp_toSquidTs([[maybe_unused]] UDF_INIT* initid,
                      UDF_ARGS* args,
                      char* result,
                      unsigned long* length,
                      [[maybe_unused]] char* is_null,
                      [[maybe_unused]] char* error)
  {
    const int64_t ts_ = (*(int64_t*)args->args[ARG_DATA_0]);
    SquidLogParser p;
    std::string str_ = p.unixToSquidDate(ts_);

    result = new char[str_.size()];
    std::strncpy(result, str_.c_str(), str_.size());
    *length = static_cast<unsigned long>(str_.size());

    return result;
  }

  /* Aggregations ---------------------------------------------------------- */

  my_bool slp_sum_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    int64_t* i_ptr_ = new int64_t;
    *i_ptr_ = 0;
    initid->ptr = (char*)i_ptr_;
    initid->maybe_null = 0;

    UTIL util;

    return util.checkArgs(initid, args, message);
  }

  void slp_sum_deinit(UDF_INIT* initid) { delete (int64_t*)initid->ptr; }

  void slp_sum_clear(UDF_INIT* initid,
                     [[maybe_unused]] UDF_ARGS* args,
                     [[maybe_unused]] char* is_null,
                     [[maybe_unused]] char* error)
  {
    *((int64_t*)initid->ptr) = 0;
  }

  void slp_sum_reset(UDF_INIT* initid,
                     UDF_ARGS* args,
                     char* is_null,
                     char* error)
  {
    slp_sum_clear(initid, args, is_null, error);
    slp_sum_add(initid, args, is_null, error);
  }

  void slp_sum_add(UDF_INIT* initid,
                   UDF_ARGS* args,
                   [[maybe_unused]] char* is_null,
                   char* error)
  {
    const std::string log_fmt_(args->args[LOG_FORMAT],
                               args->lengths[LOG_FORMAT]);
    const std::string raw_log_(args->args[LOG_LINE], args->lengths[LOG_LINE]);
    const std::string log_part_(args->args[LOG_PART], args->lengths[LOG_PART]);

    // MUST BE check if raw_log_ is empty
    if (raw_log_.empty()) {
      *error = 1;
      return;
    }

    UTIL util;

    SquidLogParser* p = new SquidLogParser(log_fmt_);
    p->append(raw_log_);
    if (p->errorNum() != SLPError::SLP_SUCCESS) {
      delete p;
      *error = 1;
      return;
    }

    int64_t sum_ = p->getPartInt(util.getFieldId(log_part_));
    delete p;
    if (*((int64_t*)initid->ptr) > 0 &&
        sum_ > LONG_MAX - *((int64_t*)initid->ptr)) {
      *error = 1; // overflow
    }

    *((int64_t*)initid->ptr) += sum_;
  }

  int64_t slp_sum(UDF_INIT* initid,
                  [[maybe_unused]] UDF_ARGS* args,
                  [[maybe_unused]] char* is_null,
                  [[maybe_unused]] char* error)
  {
    return *((int64_t*)initid->ptr);
  }

  /* ------------------------------------------------------------------------
   */

  my_bool slp_countbyrm_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    UTIL util;
    UTIL::Buffer* buf_ = new UTIL::Buffer;
    buf_->value_ = 0;
    buf_->acc_ = 0L;

    initid->maybe_null = 1;
    initid->decimals = 0;
    initid->max_length = 20;
    initid->ptr = (char*)buf_;

    if (args->arg_count >= 3) {
      UTIL::ResultErr r = {};
      if (args->arg_type[LOG_FORMAT] != STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 1, "String");
        return MY_FALSE;
      } else {
        std::string s_(args->args[LOG_FORMAT], args->lengths[LOG_FORMAT]);
        static constexpr std::string_view f_ = "squidcommoncombined";
        if (std::size_t found = f_.find(util.toLower(s_));
            found == std::string::npos) {
          util.getErrorText(ErrID::ERR_INVALID_ARG, r);
          std::sprintf(message, r.msg, 1, "Valid are: squid|common|combined");
          return MY_FALSE;
        }
      }
      if (args->arg_type[LOG_LINE] != STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 2, "Table field's name");
        return MY_FALSE;
      }
      if (args->arg_type[LOG_PART] != STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 3, "Field-id");
        return MY_FALSE;
      } else {
        std::string s_(args->args[LOG_PART], args->lengths[LOG_PART]);
        static constexpr std::string_view f_ =
          "getputpostconnectheaddeleteoptionspatchtraceothers";
        if (std::size_t found = f_.find(util.toLower(s_));
            found == std::string::npos) {
          util.getErrorText(ErrID::ERR_INVALID_ARG, r);
          std::sprintf(
            message,
            r.msg,
            1,
            "Use: "
            "GET|PUT|POST|CONNECT|HEAD|DELETE|OPTIONS|PATCH|TRACE|OTHERS");
          return MY_FALSE;
        }
      }
    } else {
      UTIL::ResultErr r = {};
      util.getErrorText(ErrID::ERR_WRONG_NUM_ARGS, r);
      std::memmove(message, r.msg, r.len);
      return MY_FALSE;
    }

    return MY_TRUE;
  }

  void slp_countbyrm_deinit(UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  void slp_countbyrm_clear(UDF_INIT* initid,
                           [[maybe_unused]] UDF_ARGS* args,
                           [[maybe_unused]] char* is_null,
                           [[maybe_unused]] char* error)
  {
    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;
    buf_->value_ = 0.0;
    buf_->acc_ = 0L;
  }

  void slp_countbyrm_reset(UDF_INIT* initid,
                           UDF_ARGS* args,
                           char* is_null,
                           char* error)
  {
    slp_countbyrm_clear(initid, args, is_null, error);
    slp_countbyrm_add(initid, args, is_null, error);
  }

  void slp_countbyrm_add(UDF_INIT* initid,
                         UDF_ARGS* args,
                         [[maybe_unused]] char* is_null,
                         char* error)
  {
    const std::string log_fmt_(args->args[LOG_FORMAT],
                               args->lengths[LOG_FORMAT]);
    const std::string raw_log_(args->args[LOG_LINE], args->lengths[LOG_LINE]);
    std::string log_part_(args->args[LOG_PART], args->lengths[LOG_PART]);
    std::transform(
      log_part_.cbegin(), log_part_.cend(), log_part_.begin(), ::toupper);

    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;

    // MUST BE check if raw_log_ is empty
    if (raw_log_.empty()) {
      *error = 1;
      return;
    }

    UTIL util;

    SquidLogParser p(util.toLower(log_fmt_));
    p.append(raw_log_);
    if (p.errorNum() != SLPError::SLP_SUCCESS) {
      *error = 1;
      return;
    }

    std::feclearexcept(FE_ALL_EXCEPT);

    if (p.getPartStr(SquidLogParser::Fields::ReqMethod) == log_part_) {
      ++buf_->acc_;
    }

    if (std::fetestexcept(FE_OVERFLOW)) {
      *error = 1;
    }
  }

  int64_t slp_countbyrm(UDF_INIT* initid,
                        [[maybe_unused]] UDF_ARGS* args,
                        [[maybe_unused]] char* is_null,
                        [[maybe_unused]] char* error)
  {
    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;
    return buf_->acc_;
  }

  /* ------------------------------------------------------------------------
   */

  my_bool slp_countbyhttpcode_init(UDF_INIT* initid,
                                   UDF_ARGS* args,
                                   char* message)
  {
    UTIL util;
    UTIL::Buffer* buf_ = new UTIL::Buffer;
    buf_->value_ = 0;
    buf_->acc_ = 0L;

    initid->maybe_null = 1;
    initid->decimals = 0;
    initid->max_length = 20;
    initid->ptr = (char*)buf_;

    if (args->arg_count >= 3) {
      UTIL::ResultErr r = {};
      if (args->arg_type[LOG_FORMAT] != STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 1, "String");
        return MY_FALSE;
      } else {
        std::string s_(args->args[LOG_FORMAT], args->lengths[LOG_FORMAT]);
        static constexpr std::string_view f_ = "squidcommoncombined";
        if (std::size_t found = f_.find(util.toLower(s_));
            found == std::string::npos) {
          util.getErrorText(ErrID::ERR_INVALID_ARG, r);
          std::sprintf(message, r.msg, 1, "Valid are: squid|common|combined");
          return MY_FALSE;
        }
      }
      if (args->arg_type[LOG_LINE] != STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 2, "Table field's name");
        return MY_FALSE;
      }
      if (args->arg_type[LOG_PART] != INT_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_TYPE_ARG, r);
        std::sprintf(message, r.msg, 3, "INTEGER. HTTP Status Code");
        return MY_FALSE;
      }
    } else {
      UTIL::ResultErr r = {};
      util.getErrorText(ErrID::ERR_WRONG_NUM_ARGS, r);
      std::memmove(message, r.msg, r.len);
      return MY_FALSE;
    }

    return MY_TRUE;
  }

  void slp_countbyhttpcode_deinit(UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  void slp_countbyhttpcode_clear(UDF_INIT* initid,
                                 [[maybe_unused]] UDF_ARGS* args,
                                 [[maybe_unused]] char* is_null,
                                 [[maybe_unused]] char* error)
  {
    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;
    buf_->value_ = 0.0;
    buf_->acc_ = 0L;
  }

  void slp_countbyhttpcode_reset(UDF_INIT* initid,
                                 UDF_ARGS* args,
                                 char* is_null,
                                 char* error)
  {
    slp_countbyhttpcode_clear(initid, args, is_null, error);
    slp_countbyhttpcode_add(initid, args, is_null, error);
  }

  void slp_countbyhttpcode_add(UDF_INIT* initid,
                               UDF_ARGS* args,
                               [[maybe_unused]] char* is_null,
                               char* error)
  {
    const std::string log_fmt_(args->args[LOG_FORMAT],
                               args->lengths[LOG_FORMAT]);
    const std::string raw_log_(args->args[LOG_LINE], args->lengths[LOG_LINE]);
    const short log_part_ = *((short*)args->args[LOG_PART]);

    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;

    // MUST BE check if raw_log_ is empty
    if (raw_log_.empty()) {
      *error = 1;
      return;
    }

    UTIL util;

    SquidLogParser p(util.toLower(log_fmt_));
    p.append(raw_log_);
    if (p.errorNum() != SLPError::SLP_SUCCESS) {
      *error = 1;
      return;
    }

    std::feclearexcept(FE_ALL_EXCEPT);

    short code_;
    if (util.toLower(log_fmt_) == "common" ||
        util.toLower(log_fmt_) == "combined") {
      code_ = std::move(p.getPartInt(SquidLogParser::Fields::HttpStatus));
    } else {
      code_ = std::move(std::stoi(p.strRight(
        p.getPartStr(SquidLogParser::Fields::ReqStatusHierStatus), '/')));
    }

    if (code_ == log_part_) {
      ++buf_->acc_;
    }

    if (std::fetestexcept(FE_OVERFLOW)) {
      *error = 1;
    }
  }

  int64_t slp_countbyhttpcode(UDF_INIT* initid,
                              [[maybe_unused]] UDF_ARGS* args,
                              [[maybe_unused]] char* is_null,
                              [[maybe_unused]] char* error)
  {
    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;
    return buf_->acc_;
  }

#ifdef __cplusplus
}
#endif

#endif // HAVE_DLOPEN
