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

#include "vcputilities.h"

#ifdef HAVE_DLOPEN

/* Utilities ------------------------------------------------------------- */
/*!
 * \internal
 * \brief Returns the text for the error that occurred.
 * \param e_ Error ID
 * \param r_ Struct containing the text and its length.
 */
void
Utilities::getErrorText(ErrorID e_, ResultErr& r_)
{
  if (std::unordered_multimap<ErrorID, std::string_view>::const_iterator it_ =
        mError.find(e_);
      it_ != mError.end()) {
    r_.msg = it_->second.data();
    r_.len = std::strlen(it_->second.data()) + 1;
  } else {
    r_.msg = mError.at(ErrorID::ERR_UNKNOWN).data();
    r_.len = std::strlen(r_.msg) + 1;
  }
}

/*!
 * \internal
 * \brief Utilities::double_equal
 * \param lhs_
 * \param rhs_
 * \return true|false
 * \note Based on:
 * https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
 */
bool
Utilities::double_equal(const double lhs_, const double rhs_) const
{
  static constexpr auto epsilon = std::numeric_limits<long double>::epsilon();
  // are (roughly) equal if the difference is less than epsilon
  return (fabs(lhs_ - rhs_) < epsilon);
}

/*!
 * \internal
 * \brief Checks if the requested operation is valid.
 * \param arg_ String that represents the operation.
 * \return Operator ID.
 * \note Valid operators: < <= > >= <>
 */
Utilities::Compare
Utilities::isValidOp(const std::string arg_)
{
  if (std::unordered_multimap<std::string_view, Compare>::const_iterator it_ =
        mOp.find(arg_);
      it_ != mOp.end()) {
    return std::move(it_->second);
  }
  return Compare::NOT_FOUND;
};

/* Times ------------------------------------------------------------------- */

constexpr double
Utilities::Hr2Min(const double d_)
{
  return (d_ * 60);
}

constexpr double
Utilities::Hr2Sec(const double d_)
{
  return (d_ * 3600);
}

constexpr double
Utilities::Hr2MSec(const double d_)
{
  return (d_ * 3600000);
}

constexpr double
Utilities::Min2Hr(const double d_)
{
  return (d_ / 60);
}

constexpr double
Utilities::Min2Sec(const double d_)
{
  return (d_ * 60);
}

constexpr double
Utilities::Min2MSec(const double d_)
{
  return (d_ * 60000);
}

constexpr double
Utilities::Sec2Hr(const double d_)
{
  return (d_ / 3600);
}

constexpr double
Utilities::Sec2Min(const double d_)
{
  return (d_ / 60);
}

constexpr double
Utilities::Sec2MSec(const double d_)
{
  return (d_ * 1000);
}

constexpr double
Utilities::MSec2Hr(const double d_)
{
  return (d_ / 3600000);
}

constexpr double
Utilities::MSec2Min(const double d_)
{
  return (d_ / 60000);
}

constexpr double
Utilities::MSec2Sec(const double d_)
{
  return (d_ / 1000);
}

/*!
 * \brief From time sexagesimal to centesimal.
 * \param d_ Time in format: hh.mmss
 * \return time centesimal
 */
constexpr double
Utilities::toBase10(const double d_)
{
  double ms60_ = std::abs(d_) - int(std::abs(d_));
  return int(d_) + (ms60_ / 0.60);
}

/*!
 * \brief From time centesimal to sexagesimal.
 * \param d_ Time in format: hh.mmss
 * \return time sexagesimal
 */

constexpr double
Utilities::toBase60(const double d_)
{
  double ms10_ = std::abs(d_) - int(std::abs(d_));
  return int(d_) + (ms10_ * 0.60);
}

/* -------------------------------------------------------------------------- */

using UTIL = Utilities;
using ErrID = UTIL::ErrorID;

/* MariaDB UDA's implamentation -------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif

  /* AVG_IF() --------------------------------------------------------------- */
  /*!
   * \internal
   * \brief avg_if_init
   * \param initid
   * \param args
   * \param message
   * \return my_bool MY_TRUE|MY_FALSE
   */
  my_bool avg_if_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    UTIL util;
    UTIL::Buffer* buf_ = new UTIL::Buffer;
    buf_->value_ = 0.0;
    buf_->acc_ = 0L;

    initid->maybe_null = 0;
    // initid->decimals = 6; // using default
    initid->max_length = 20;
    initid->ptr = (char*)buf_;

    if (args->arg_count == 3) {
      UTIL::ResultErr r = {};
      if (args->arg_type[ARG_VALUE] == STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_ARG, r);
        std::sprintf(message, r.msg, 1);
        return MY_FALSE;
      }
      if (args->arg_type[ARG_DATA] == STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_ARG, r);
        std::sprintf(message, r.msg, 2);
        return MY_FALSE;
      }

      if (args->arg_type[ARG_VALUE] == INT_RESULT) {
        args->arg_type[ARG_DATA] = INT_RESULT;
      } else {
        if (args->arg_type[ARG_VALUE] == DECIMAL_RESULT) {
          args->arg_type[ARG_VALUE] = REAL_RESULT;
        }
        if (args->arg_type[ARG_DATA] == DECIMAL_RESULT) {
          args->arg_type[ARG_DATA] = REAL_RESULT;
        }
      }

      UTIL::Compare cmp_;
      const std::string s((char*)args->args[ARG_COND]);
      if ((cmp_ = util.isValidOp(s)) != UTIL::Compare::NOT_FOUND) {
        buf_->comp_ = cmp_;
      } else {
        UTIL::ResultErr r = {};
        util.getErrorText(ErrID::ERR_INVALID_OP, r);
        std::sprintf(message, r.msg, args->args[ARG_COND]);
        return MY_FALSE;
      }
    } else {
      UTIL::ResultErr r = {};
      util.getErrorText(ErrID::ERR_WRONG_NUM_ARGS, r);
      std::memcpy(message, r.msg, r.len);
      return MY_FALSE;
    }

    return MY_TRUE;
  }

  /*!
   * \internal
   * \brief avg_if_deinit
   * \param initid
   */
  void avg_if_deinit(UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  /*!
   * \intenal
   * \brief avg_if_clear
   * \param initid
   * \param args
   * \param is_null
   * \param error
   */
  void avg_if_clear(UDF_INIT* initid,
                    [[maybe_unused]] UDF_ARGS* args,
                    [[maybe_unused]] char* is_null,
                    [[maybe_unused]] char* error)
  {
    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;
    buf_->value_ = 0.0;
    buf_->acc_ = 0L;
  }

  /*!
   * \internal
   * \brief avg_if_reset
   * \param initid
   * \param args
   * \param is_null
   * \param error
   */
  void avg_if_reset(UDF_INIT* initid,
                    UDF_ARGS* args,
                    char* is_null,
                    char* error)
  {
    avg_if_clear(initid, args, is_null, error);
    avg_if_add(initid, args, is_null, error);
  }

  /*!
   * \internal
   * \brief avg_if_add
   * \param initid
   * \param args
   * \param is_null
   * \param error
   */
  void avg_if_add([[maybe_unused]] UDF_INIT* initid,
                  UDF_ARGS* args,
                  [[maybe_unused]] char* is_null,
                  char* error)
  {
    UTIL util;
    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;

    std::feclearexcept(FE_ALL_EXCEPT);

    switch (buf_->comp_) {
      case UTIL::Compare::NOT_FOUND:
      case UTIL::Compare::GT: {
        if (args->arg_type[ARG_VALUE] == INT_RESULT) {
          if (*((int64_t*)args->args[ARG_VALUE]) >
              *((int64_t*)args->args[ARG_DATA])) {
            buf_->acc_++;
            buf_->value_ += (double)*((int64_t*)args->args[ARG_VALUE]);
          }
        } else {
          if (std::isgreater(*((double*)args->args[ARG_VALUE]),
                             *((double*)args->args[ARG_DATA]))) {
            buf_->acc_++;
            buf_->value_ += *((double*)args->args[ARG_VALUE]);
          }
        }
        break;
      }
      case UTIL::Compare::GE: {
        if (args->arg_type[ARG_VALUE] == INT_RESULT) {
          if (*((int64_t*)args->args[ARG_VALUE]) >=
              *((int64_t*)args->args[ARG_DATA])) {
            buf_->acc_++;
            buf_->value_ += (double)*((int64_t*)args->args[ARG_VALUE]);
          }
        } else {
          if (std::isgreaterequal(*((double*)args->args[ARG_VALUE]),
                                  *((double*)args->args[ARG_DATA]))) {
            buf_->acc_++;
            buf_->value_ += *((double*)args->args[ARG_VALUE]);
          }
        }
        break;
      }
      case UTIL::Compare::LT: {
        if (args->arg_type[ARG_VALUE] == INT_RESULT) {
          if (*((int64_t*)args->args[ARG_VALUE]) <
              *((int64_t*)args->args[ARG_VALUE])) {
            buf_->acc_++;
            buf_->value_ += (double)*((int64_t*)args->args[ARG_VALUE]);
          }
        } else {
          if (std::isless(*((double*)args->args[ARG_VALUE]),
                          *((double*)args->args[ARG_DATA]))) {
            buf_->acc_++;
            buf_->value_ += *((double*)args->args[ARG_VALUE]);
          }
        }
        break;
      }
      case UTIL::Compare::LE: {
        if (args->arg_type[ARG_VALUE] == INT_RESULT) {
          if (*((int64_t*)args->args[ARG_VALUE]) <=
              *((int64_t*)args->args[ARG_DATA])) {
            buf_->acc_++;
            buf_->value_ += (double)*((int64_t*)args->args[ARG_VALUE]);
          }
        } else {
          if (std::islessequal(*((double*)args->args[ARG_VALUE]),
                               *((double*)args->args[ARG_DATA]))) {
            buf_->acc_++;
            buf_->value_ += *((double*)args->args[ARG_VALUE]);
          }
        }
        break;
      }
      case UTIL::Compare::EQ: {
        if (args->arg_type[ARG_VALUE] == INT_RESULT) {
          if (*((int64_t*)args->args[ARG_VALUE]) ==
              *((int64_t*)args->args[ARG_DATA])) {
            buf_->acc_++;
            buf_->value_ += (double)*((int64_t*)args->args[ARG_VALUE]);
          }
        } else {
          if (util.double_equal(*((double*)args->args[ARG_VALUE]),
                                *((double*)args->args[ARG_DATA]))) {
            buf_->acc_++;
            buf_->value_ += (double)*((double*)args->args[ARG_VALUE]);
          }
        }
        break;
      }
      case UTIL::Compare::NE: {
        if (args->arg_type[ARG_VALUE] == INT_RESULT) {
          if (*((int64_t*)args->args[ARG_VALUE]) !=
              *((int64_t*)args->args[ARG_DATA])) {
            buf_->acc_++;
            buf_->value_ += (double)*((int64_t*)args->args[ARG_VALUE]);
          }
        } else {
          if (!util.double_equal(*((double*)args->args[ARG_VALUE]),
                                 *((double*)args->args[ARG_DATA]))) {
            buf_->acc_++;
            buf_->value_ += *((double*)args->args[ARG_VALUE]);
          }
        }
      }
    } // switch

    if (std::fetestexcept(FE_OVERFLOW)) {
      *error = 1;
    }
  }

  /*!
   * \brief User interface
   * \param initid
   * \param args
   * \param is_null
   * \param error
   * \return double
   * \note How to register the function:
   * CREATE OR REPLACE AGGREGATE FUNCTION avg_if RETURNS REAL SONAME
   * 'libUTILlgorithm.so';
   */
  double avg_if(UDF_INIT* initid,
                [[maybe_unused]] UDF_ARGS* args,
                char* is_null,
                char* error)
  {
    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;

    int raised_;
    std::feclearexcept(FE_ALL_EXCEPT);
    double m_ = (buf_->value_ / buf_->acc_);
    raised_ = fetestexcept(FE_DIVBYZERO);
    if (raised_ & FE_DIVBYZERO) {
      *error = 1;
      *is_null = 1;
    }
    return m_;
  }

  /* SUM_IF() --------------------------------------------------------------- */
  /*!
   * \internal
   * \brief sum_if_init
   * \param initid
   * \param args
   * \param message
   * \return my_bool MY_TRUE|MY_FALSE
   */
  my_bool sum_if_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    UTIL util;
    UTIL::Buffer* buf_ = new UTIL::Buffer;
    buf_->value_ = 0.0;
    buf_->acc_ = 0L;

    initid->maybe_null = 1;
    // initid->decimals = 6; // using default
    initid->max_length = 20;
    initid->ptr = (char*)buf_;

    if (args->arg_count == 3) {
      UTIL::ResultErr r = {};
      if (args->arg_type[ARG_VALUE] == STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_ARG, r);
        std::sprintf(message, r.msg, 1);
        return MY_FALSE;
      }
      if (args->arg_type[ARG_DATA] == STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_ARG, r);
        std::sprintf(message, r.msg, 2);
        return MY_FALSE;
      }

      if (args->arg_type[ARG_VALUE] == INT_RESULT) {
        args->arg_type[ARG_DATA] = INT_RESULT;
      } else {
        if (args->arg_type[ARG_VALUE] == DECIMAL_RESULT)
          args->arg_type[ARG_VALUE] = REAL_RESULT;
        if (args->arg_type[ARG_DATA] == DECIMAL_RESULT)
          args->arg_type[ARG_DATA] = REAL_RESULT;
      }

      UTIL::Compare cmp_;
      const std::string s((char*)args->args[ARG_COND]);
      if ((cmp_ = util.isValidOp(s)) != UTIL::Compare::NOT_FOUND) {
        buf_->comp_ = cmp_;
      } else {
        UTIL::ResultErr r = {};
        util.getErrorText(ErrID::ERR_INVALID_OP, r);
        std::sprintf(message, r.msg, args->args[ARG_COND]);
        return MY_FALSE;
      }
    } else {
      UTIL::ResultErr r = {};
      util.getErrorText(ErrID::ERR_WRONG_NUM_ARGS, r);
      std::memcpy(message, r.msg, r.len);
      return MY_FALSE;
    }

    return MY_TRUE;
  }

  /*!
   * \internal
   * \brief sum_if_deinit
   * \param initid
   */
  void sum_if_deinit(UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  /*!
   * \intenal
   * \brief sum_if_clear
   * \param initid
   * \param args
   * \param is_null
   * \param error
   */
  void sum_if_clear(UDF_INIT* initid,
                    [[maybe_unused]] UDF_ARGS* args,
                    [[maybe_unused]] char* is_null,
                    [[maybe_unused]] char* error)
  {
    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;
    buf_->value_ = 0.0;
    buf_->acc_ = 0L;
  }

  /*!
   * \internal
   * \brief sum_if_reset
   * \param initid
   * \param args
   * \param is_null
   * \param error
   */
  void sum_if_reset(UDF_INIT* initid,
                    UDF_ARGS* args,
                    char* is_null,
                    char* error)
  {
    sum_if_clear(initid, args, is_null, error);
    sum_if_add(initid, args, is_null, error);
  }

  /*!
   * \internal
   * \brief sum_if_add
   * \param initid
   * \param args
   * \param is_null
   * \param error
   */
  void sum_if_add(UDF_INIT* initid,
                  UDF_ARGS* args,
                  [[maybe_unused]] char* is_null,
                  char* error)
  {
    UTIL util;
    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;

    std::feclearexcept(FE_ALL_EXCEPT);

    switch (buf_->comp_) {
      case UTIL::Compare::NOT_FOUND:
      case UTIL::Compare::GT: {
        if (isgreater(*((double*)args->args[ARG_VALUE]),
                      *((double*)args->args[ARG_DATA])))
          buf_->value_ += *((double*)args->args[ARG_VALUE]);
        break;
      }
      case UTIL::Compare::GE: {
        if (isgreaterequal(*((double*)args->args[ARG_VALUE]),
                           *((double*)args->args[ARG_DATA])))
          buf_->value_ += *((double*)args->args[ARG_VALUE]);
        break;
      }
      case UTIL::Compare::LT: {
        if (isless(*((double*)args->args[ARG_VALUE]),
                   *((double*)args->args[ARG_DATA])))
          buf_->value_ += *((double*)args->args[ARG_VALUE]);
        break;
      }
      case UTIL::Compare::LE: {
        if (islessequal(*((double*)args->args[ARG_VALUE]),
                        *((double*)args->args[ARG_DATA])))
          buf_->value_ += *((double*)args->args[ARG_VALUE]);
        break;
      }
      case UTIL::Compare::EQ: {
        if (args->arg_type[ARG_VALUE] == INT_RESULT) {
          if (*((int64_t*)args->args[ARG_VALUE]) ==
              *((int64_t*)args->args[ARG_DATA]))
            buf_->value_ += *((double*)args->args[ARG_VALUE]);
        } else {
          if (util.double_equal(*((double*)args->args[ARG_VALUE]),
                                *((double*)args->args[ARG_DATA])))
            buf_->value_ += *((double*)args->args[ARG_VALUE]);
        }
        break;
      }
      case UTIL::Compare::NE: {
        if (args->arg_type[ARG_VALUE] == INT_RESULT) {
          if (*((int64_t*)args->args[ARG_VALUE]) !=
              *((int64_t*)args->args[ARG_DATA]))
            buf_->value_ += *((double*)args->args[ARG_VALUE]);
        } else {
          if (!util.double_equal(*((double*)args->args[ARG_VALUE]),
                                 *((double*)args->args[ARG_DATA])))
            buf_->value_ += *((double*)args->args[ARG_VALUE]);
        }
      }
    } // switch

    if (std::fetestexcept(FE_OVERFLOW)) {
      *error = 1;
    }
  }

  /*!
   * \brief User interface
   * \param initid
   * \param args
   * \param is_null
   * \param error
   * \return double
   * \note How to register the function:
   * CREATE OR REPLACE AGGREGATE FUNCTION sum_if RETURNS REAL SONAME
   * 'libUTILlgorithm.so';
   */
  double sum_if(UDF_INIT* initid,
                [[maybe_unused]] UDF_ARGS* args,
                [[maybe_unused]] char* is_null,
                [[maybe_unused]] char* error)
  {
    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;
    return buf_->value_;
  }

  /* COUNT_IF() ------------------------------------------------------------- */
  /*!
   * \brief count_if_init
   * \param initid
   * \param args
   * \param message
   * \return my_bool MY_TRUE|MY_FALSE
   */
  my_bool count_if_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    UTIL util;
    UTIL::Buffer* buf_ = new UTIL::Buffer;
    buf_->value_ = 0.0;
    buf_->acc_ = 0L;

    initid->maybe_null = 1;
    initid->decimals = 0;
    initid->max_length = 20;
    initid->ptr = (char*)buf_;

    if (args->arg_count == 3) {
      UTIL::ResultErr r = {};
      if (args->arg_type[ARG_VALUE] == STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_ARG, r);
        std::sprintf(message, r.msg, 1);
        return MY_FALSE;
      }
      if (args->arg_type[ARG_DATA] == STRING_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_ARG, r);
        std::sprintf(message, r.msg, 2);
        return MY_FALSE;
      }

      if (args->arg_type[ARG_VALUE] == INT_RESULT) {
        args->arg_type[ARG_DATA] = INT_RESULT;
      } else {
        if (args->arg_type[ARG_VALUE] == DECIMAL_RESULT)
          args->arg_type[ARG_VALUE] = REAL_RESULT;
        if (args->arg_type[ARG_DATA] == DECIMAL_RESULT)
          args->arg_type[ARG_DATA] = REAL_RESULT;
      }

      UTIL::Compare cmp_;
      const std::string s((char*)args->args[ARG_COND]);
      if ((cmp_ = util.isValidOp(s)) != UTIL::Compare::NOT_FOUND) {
        buf_->comp_ = cmp_;
      } else {
        UTIL::ResultErr r = {};
        util.getErrorText(ErrID::ERR_INVALID_OP, r);
        std::sprintf(message, r.msg, args->args[ARG_COND]);
        return MY_FALSE;
      }
    } else {
      UTIL::ResultErr r = {};
      util.getErrorText(ErrID::ERR_WRONG_NUM_ARGS, r);
      std::memcpy(message, r.msg, r.len);
      return MY_FALSE;
    }

    return MY_TRUE;
  }

  /*!
   * \brief count_if_deinit
   * \param initid
   */
  void count_if_deinit(UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  /*!
   * \brief count_if_clear
   * \param initid
   * \param args
   * \param is_null
   * \param error
   */
  void count_if_clear(UDF_INIT* initid,
                      [[maybe_unused]] UDF_ARGS* args,
                      [[maybe_unused]] char* is_null,
                      [[maybe_unused]] char* error)
  {
    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;
    buf_->value_ = 0.0;
    buf_->acc_ = 0L;
  }

  /*!
   * \brief count_if_reset
   * \param initid
   * \param args
   * \param is_null
   * \param error
   */
  void count_if_reset(UDF_INIT* initid,
                      UDF_ARGS* args,
                      char* is_null,
                      char* error)
  {
    count_if_clear(initid, args, is_null, error);
    count_if_add(initid, args, is_null, error);
  }

  /*!
   * \brief count_if_add
   * \param initid
   * \param args
   * \param is_null
   * \param error
   */
  void count_if_add(UDF_INIT* initid,
                    UDF_ARGS* args,
                    [[maybe_unused]] char* is_null,
                    char* error)
  {
    UTIL util;
    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;

    std::feclearexcept(FE_ALL_EXCEPT);

    switch (buf_->comp_) {
      case UTIL::Compare::NOT_FOUND:
      case UTIL::Compare::GT: {
        if (isgreater(*((double*)args->args[ARG_VALUE]),
                      *((double*)args->args[ARG_DATA])))
          buf_->acc_++;
        break;
      }
      case UTIL::Compare::GE: {
        if (isgreaterequal(*((double*)args->args[ARG_VALUE]),
                           *((double*)args->args[ARG_DATA])))
          buf_->acc_++;
        break;
      }
      case UTIL::Compare::LT: {
        if (isless(*((double*)args->args[ARG_VALUE]),
                   *((double*)args->args[ARG_DATA])))
          buf_->acc_++;
        break;
      }
      case UTIL::Compare::LE: {
        if (islessequal(*((double*)args->args[ARG_VALUE]),
                        *((double*)args->args[ARG_DATA])))
          buf_->acc_++;
        break;
      }
      case UTIL::Compare::EQ: {
        if (args->arg_type[ARG_VALUE] == INT_RESULT) {
          if (*((int64_t*)args->args[ARG_VALUE]) ==
              *((int64_t*)args->args[ARG_DATA]))
            buf_->acc_++;
        } else {
          if (util.double_equal(*((double*)args->args[ARG_VALUE]),
                                *((double*)args->args[ARG_DATA])))
            buf_->acc_++;
        }
        break;
      }
      case UTIL::Compare::NE: {
        if (args->arg_type[ARG_VALUE] == INT_RESULT) {
          if (*((int64_t*)args->args[ARG_VALUE]) !=
              *((int64_t*)args->args[ARG_DATA]))
            buf_->acc_++;
        } else {
          if (!util.double_equal(*((double*)args->args[ARG_VALUE]),
                                 *((double*)args->args[ARG_DATA])))
            buf_->acc_++;
        }
      }
    } // switch

    if (std::fetestexcept(FE_OVERFLOW)) {
      *error = 1;
    }
  }

  /*!
   * \brief User interface
   * \param initid
   * \param args
   * \param is_null
   * \param error
   * \return int64_t (longlong)
   * \note How to register the function:
   * CREATE OR REPLACE AGGREGATE FUNCTION count_if RETURNS INTEGER SONAME
   * 'libUTILlgorithm.so';
   */
  int64_t count_if(UDF_INIT* initid,
                   [[maybe_unused]] UDF_ARGS* args,
                   [[maybe_unused]] char* is_null,
                   [[maybe_unused]] char* error)
  {
    UTIL::Buffer* buf_ = (UTIL::Buffer*)initid->ptr;
    return buf_->acc_;
  }

  /* TIME CONVERTIONS ----------------------------------------------------- */

  struct Temp
  {
    Utilities::ConvertionTypes flag;
  };

  my_bool time_convert_init(UDF_INIT* initid, UDF_ARGS* args, char* message)
  {
    initid->maybe_null = 1;
    initid->decimals = 4;

    UTIL util;

    Temp* temp_ = new Temp;
    initid->ptr = (char*)temp_;

    if (args->arg_count == 2) {
      UTIL::ResultErr r = {};
      if (args->arg_type[ARG_TIME_VALUE] != REAL_RESULT) {
        util.getErrorText(ErrID::ERR_INVALID_ARG, r);
        std::sprintf(message, r.msg, 1);
        return MY_FALSE;
      }
      std::string s_(args->args[ARG_CONV_TYPE], args->lengths[ARG_CONV_TYPE]);
      std::transform(s_.cbegin(), s_.cend(), s_.begin(), ::tolower);
      if (s_.compare("hour_to_min") == 0) {
        temp_->flag = Utilities::ConvertionTypes::HourToMin;
      } else if (s_.compare("hour_to_sec") == 0) {
        temp_->flag = Utilities::ConvertionTypes::HourToSec;
      } else if (s_.compare("hour_to_msec") == 0) {
        temp_->flag = Utilities::ConvertionTypes::HourToMSec;
      } else if (s_.compare("min_to_hour") == 0) {
        temp_->flag = Utilities::ConvertionTypes::MinToHour;
      } else if (s_.compare("min_to_sec") == 0) {
        temp_->flag = Utilities::ConvertionTypes::MinToSec;
      } else if (s_.compare("min_to_msec") == 0) {
        temp_->flag = Utilities::ConvertionTypes::MinToMSec;
      } else if (s_.compare("sec_to_hour") == 0) {
        temp_->flag = Utilities::ConvertionTypes::SecToHour;
      } else if (s_.compare("sec_to_min") == 0) {
        temp_->flag = Utilities::ConvertionTypes::SecToMin;
      } else if (s_.compare("sec_to_msec") == 0) {
        temp_->flag = Utilities::ConvertionTypes::SecToMSec;
      } else if (s_.compare("msec_to_hour") == 0) {
        temp_->flag = Utilities::ConvertionTypes::MSecToHour;
      } else if (s_.compare("msec_to_min") == 0) {
        temp_->flag = Utilities::ConvertionTypes::MSecToMin;
      } else if (s_.compare("msec_to_sec") == 0) {
        temp_->flag = Utilities::ConvertionTypes::MSecToSec;
      } else if (s_.compare("to_base10") == 0) {
        temp_->flag = Utilities::ConvertionTypes::toBase10;
      } else if (s_.compare("to_base60") == 0) {
        temp_->flag = Utilities::ConvertionTypes::toBase60;
      } else {
        return MY_FALSE;
      }
    }

    return MY_TRUE;
  }

  void time_convert_deinit(UDF_INIT* initid)
  {
    if (initid->ptr != NULL) {
      delete initid->ptr;
    }
  }

  double time_convert(UDF_INIT* initid,
                      UDF_ARGS* args,
                      [[maybe_unused]] char* is_null,
                      [[maybe_unused]] char* error)
  {
    const double d_ = *((double*)args->args[ARG_TIME_VALUE]);

    Temp* temp_ = (Temp*)initid->ptr;

    switch (temp_->flag) {
      case Utilities::ConvertionTypes::HourToMin: {
        return Utilities::Hr2Min(d_);
      }
      case Utilities::ConvertionTypes::HourToSec: {
        return Utilities::Hr2Sec(d_);
      }
      case Utilities::ConvertionTypes::HourToMSec: {
        return Utilities::Hr2MSec(d_);
      }
      case Utilities::ConvertionTypes::MinToHour: {
        return Utilities::Min2Hr(d_);
      }
      case Utilities::ConvertionTypes::MinToSec: {
        return Utilities::Min2Sec(d_);
      }
      case Utilities::ConvertionTypes::MinToMSec: {
        return Utilities::Min2MSec(d_);
      }
      case Utilities::ConvertionTypes::SecToHour: {
        return Utilities::Sec2Hr(d_);
      }
      case Utilities::ConvertionTypes::SecToMin: {
        return Utilities::Sec2Min(d_);
      }
      case Utilities::ConvertionTypes::SecToMSec: {
        return Utilities::Sec2MSec(d_);
      }
      case Utilities::ConvertionTypes::MSecToHour: {
        return Utilities::MSec2Hr(d_);
      }
      case Utilities::ConvertionTypes::MSecToMin: {
        return Utilities::MSec2Min(d_);
      }
      case Utilities::ConvertionTypes::MSecToSec: {
        return Utilities::MSec2Sec(d_);
      }

      case Utilities::ConvertionTypes::toBase10: {
        return Utilities::toBase10(d_);
      }
      case Utilities::ConvertionTypes::toBase60: {
        return Utilities::toBase60(d_);
      }
      default:
        return -1;
    }
  }

#ifdef __cplusplus
}
#endif

#endif // HAVE_DLOPEN
