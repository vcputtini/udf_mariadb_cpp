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

#include "squidlogparser.h"

namespace squidlogparser {

/* Utilities ---------------------------------------------------------------- */
IPv4Addr::IPv4Addr()
  : str_({})
  , num_(0UL){};

IPv4Addr::IPv4Addr(const std::string addr_)
  : str_(addr_){};

IPv4Addr::IPv4Addr(const char* addr_)
{
  str_ = std::string(addr_, ::strlen(addr_));
};

/*!
 * \brief IPv4Addr::getAddr
 * \return
 */
std::string
IPv4Addr::getAddr() const
{
  return str_;
};

/*!
 * \brief IPv4Addr::getInt
 * \return
 */
uint32_t
IPv4Addr::getInt() const
{
  return num_;
};

IPv4Addr&
IPv4Addr::operator=(const IPv4Addr& rhs_)
{
  if (this == &rhs_) {
    return *this;
  }
  if (isValid(rhs_.str_)) {
    str_ = rhs_.str_;
    num_ = IPv4Addr::iptol(rhs_.str_);
  } else {
    str_ = "Invalid Address";
    num_ = 0UL;
  }
  return *this;
}

bool
IPv4Addr::operator>(const IPv4Addr& rhs_) const
{
  return IPv4Addr::iptol(str_) > IPv4Addr::iptol(rhs_.str_);
}

bool
IPv4Addr::operator<(const IPv4Addr& rhs_) const
{
  return IPv4Addr::iptol(str_) < IPv4Addr::iptol(rhs_.str_);
}

bool
IPv4Addr::operator==(const IPv4Addr& rhs_) const
{
  return IPv4Addr::iptol(str_) == IPv4Addr::iptol(rhs_.str_);
}

bool
IPv4Addr::operator!=(const IPv4Addr& rhs_) const
{
  return IPv4Addr::iptol(str_) != IPv4Addr::iptol(rhs_.str_);
}

/**
 * \internal
 * \brief IPv4Addr::splitP Private member to internal use only.
 * \param arr_ std::array<std::string,4> with tokens
 * \param src String to be splited
 *
 */
void
IPv4Addr::splitP(std::array<std::string, 4>& arr_, const std::string src_)
{
  if (src_.size() > 15) {
    return;
  }
  std::stringstream ss_(src_);
  std::string tok_ = {};
  int8_t i = 0;
  while (std::getline(ss_, tok_, '.')) {
    arr_[i] = std::move(tok_);
    ++i;
  }
}

/**
 * \internal
 * \brief IPv4Addr::isValid
 * \param addr
 * \return true|false
 * \note e.g.: isValid("192.168.1.100");
 */
bool
IPv4Addr::isValid(const std::string addr_)
{
  struct sockaddr_in saddr_in_;
  return inet_pton(AF_INET, addr_.c_str(), &saddr_in_);
}

/**
 * \internal
 * \brief IPv4Addr::iptol Converts an IPv4 address to its decimal equivalent
 * \param addr
 * \return long int Decimal represetation of IpV4 address
 * \note long int il = iptol("192.168.1.110"); (3232235886)
 */
uint32_t
IPv4Addr::iptol(const std::string addr)
{
  if (isValid(addr)) {
    std::array<std::string, 4> a_;
    IPv4Addr::splitP(a_, addr);
    return (std::stol(a_[0]) << 24) | (std::stol(a_[1]) << 16) |
           (std::stol(a_[2]) << 8) | std::stol(a_[3]);
  }
  return 0UL;
}

/**
 * \internal
 * \brief IPv4Addr::ltoip Converts a decimal IPv4 address to its formatted
 * equivalent.
 * \param addr Decimal representation of IpV4 address
 * \return string IpV4 address
 * \note std::string ltoip(3232235886); ("192.168.1.110")
 */
std::string
IPv4Addr::ltoip(uint32_t addr)
{
  std::stringstream ss;
  ss << (addr >> 24) << "." << ((addr >> 16) & 0xFF) << "."
     << ((addr >> 8) & 0xFF) << "." << (addr & 0xFF);
  return ss.str();
}

/* --------------------------------------------------------------------------
 */

/* Visitor ------------------------------------------------------------------
 */
/*!
 * \internal
 * \brief After deducing the type of data stored in the var_t structure, it
 * returns a corresponding integer value.
 * \param t std::variant() structure with data stored for type deduction.
 * \return int Variable type.
 */
Visitor::TypeVar
Visitor::varType(var_t t_) const
{
  TypeVar typevar_ = {};
  std::visit(
    overloadedP{
      [&typevar_]([[maybe_unused]] int arg) { typevar_ = TypeVar::TInt; },
      [&typevar_]([[maybe_unused]] long arg) { typevar_ = TypeVar::TLong; },
      [&typevar_]([[maybe_unused]] uint32_t arg) { typevar_ = TypeVar::TUint; },
      [&typevar_]([[maybe_unused]] const std::string& arg) {
        typevar_ = TypeVar::TString;
      } },
    t_);
  return typevar_;
}

/* SquidLogParser -----------------------------------------------------------
 */
/*!
 * \brief SquidLogParser::SquidLogParser
 * \param log_fmt_
 */
SquidLogParser::SquidLogParser(LogFormat log_fmt_)
  : re_id_fmt_squid_(cp_id_fmt_squid_)
  , re_id_fmt_common_(cp_id_fmt_common_)
  , re_id_fmt_combined_(cp_id_fmt_combined_)
  , re_id_fmt_referrer_(cp_id_fmt_referrer_)
  , re_id_fmt_useragent_(cp_id_fmt_useragent_)
{
  logFmt_ = log_fmt_;
};

/*!
 * \brief Constructor that takes as an argument a string that represents the
 * name of the format to be used. Convert this string to the corresponding value
 * of the LogFormat enum for internal use.
 *
 * \param log_fmt_ A valid format name: squid, common, combined, referrer or
 * useragent.
 */
SquidLogParser::SquidLogParser(const std::string_view&& log_fmt_)
  : re_id_fmt_squid_(cp_id_fmt_squid_)
  , re_id_fmt_common_(cp_id_fmt_common_)
  , re_id_fmt_combined_(cp_id_fmt_combined_)
  , re_id_fmt_referrer_(cp_id_fmt_referrer_)
  , re_id_fmt_useragent_(cp_id_fmt_useragent_)
{
  // Ugly... but efficient at the moment.
  if (log_fmt_ == "squid") {
    logFmt_ = LogFormat::Squid;
  } else if (log_fmt_ == "common") {
    logFmt_ = LogFormat::Common;
  } else if (log_fmt_ == "combined") {
    logFmt_ = LogFormat::Combined;
  } else if (log_fmt_ == "referrer") {
    logFmt_ = LogFormat::Referrer;
  } else if (log_fmt_ == "useragent") {
    logFmt_ = LogFormat::UserAgent;
  } else {
    logFmt_ = LogFormat::Unknown;
  }
}

/*!
 * \brief UrlDecode
 * \param raw_ Raw URL
 * \return string URL decoded.
 *
 * \note This function is the result of Internet research and adaptations that
 * we consider convenient
 */
const std::string
SquidLogParser::UrlDecode(const std::string raw_)
{

  if (size_t p_ = raw_.find("%"); p_ == std::string::npos) {
    return raw_;
  }

  std::string tmp_(raw_, raw_.size()); // tries to decrease dynamic allocation.

  for (auto it_ = raw_.cbegin(); it_ != raw_.cend(); ++it_) {
    switch (*it_) {
      case '%': {
        ++it_;
        std::string byte_;
        byte_.push_back(std::move(*it_));
        ++it_;
        byte_.push_back(std::move(*it_));
        tmp_.push_back(
          std::move(static_cast<char>(std::stol(byte_, nullptr, 16))));
        break;
      }
      case '+': {
        tmp_.push_back(std::move(' '));
        break;
      }
      default: {
        tmp_.push_back(std::move(*it_));
      }
    }
  }
  return tmp_;
}

/*!
 * \brief Appends the log entries to the system.
 * \param raw_log_
 */
SquidLogParser&
SquidLogParser::append(const std::string& raw_log_)
{
  try {
    std::string tmp_(std::move(raw_log_), raw_log_.size());
    removeExtraWhiteSpaces(raw_log_, tmp_);
    rawLog_.resize(tmp_.size());
    rawLog_ = tmp_;
    switch (logFmt_) {
      case LogFormat::Squid: {
        if (parserSquid() == SLPError::SLP_SUCCESS) {
          mEntry.insert({ DataKey(ds_squid_.timeStamp, ds_squid_.cliSrcIpAddr),
                          ds_squid_ });
        }
        break;
      }
      case LogFormat::Common: {
        if (parserCommon() == SLPError::SLP_SUCCESS) {
          mEntry.insert({ DataKey(unixTimestamp(ds_squid_.localTime),
                                  ds_squid_.cliSrcIpAddr),
                          ds_squid_ });
        }
        break;
      }
      case LogFormat::Combined: {
        if (parserCombined() == SLPError::SLP_SUCCESS) {
          mEntry.insert({ DataKey(unixTimestamp(ds_squid_.localTime),
                                  ds_squid_.cliSrcIpAddr),
                          ds_squid_ });
        }
        break;
      }
      case LogFormat::Referrer: {
        if (parserReferrer() == SLPError::SLP_SUCCESS) {
          mEntry.insert({ DataKey(ds_squid_.timeStamp, ds_squid_.cliSrcIpAddr),
                          ds_squid_ });
        }
        break;
      }
      case LogFormat::UserAgent: {
        if (parserUserAgent() == SLPError::SLP_SUCCESS) {
          mEntry.insert({ DataKey(unixTimestamp(ds_squid_.localTime),
                                  ds_squid_.cliSrcIpAddr),
                          ds_squid_ });
        }
      }
      default: {
        ;
      }
    }
  } catch (const std::exception& e) {
    std::cout << "\n"
              << __FUNCTION__ << ": [" << __LINE__ << "] "
              << __FILE__ ": An exception occurred: " << e.what() << "\n\n";
  };

  return *this;
}

/*!
 * \brief Returns the number of log entries read.
 * \return size_t  Is an unsigned integral type.
 */
size_t
SquidLogParser::size() const
{
  return mEntry.size();
}

/*!
 * \brief SquidLogParser::clear
 */
void
SquidLogParser::clear()
{
  mEntry.clear();
}

/*!
 * \brief SquidLogParser::getPartInt
 * \param f_
 * \return int
 */
int
SquidLogParser::getPartInt(Fields f_) const
{
  return intFields(f_, ds_squid_);
}

/*!
 * \brief SquidLogParser::getPartUInt
 * \param f_
 * \return uint32_t
 */
uint32_t
SquidLogParser::getPartUInt(Fields f_) const
{
  return uint32Fields(f_, ds_squid_);
}

/*!
 * \brief SquidLogParser::getPartStr
 * \param f_
 * \return std::string
 */
std::string
SquidLogParser::getPartStr(Fields f_) const
{
  return strFields(f_, ds_squid_);
}

/*!
 * \brief SquidLogParser::getUrlParts
 * \return
 */
std::string
SquidLogParser::getUrlParts(const std::string part_) const
{
  SLPUrlParts up_(strFields(Fields::ReqURL, ds_squid_));

  if (part_ == "scheme") {
    return up_.getScheme();
  } else if (part_ == "domain") {
    return up_.getDomain();
  } else if (part_ == "username") {
    return up_.getUsername();
  } else if (part_ == "passwo rd") {
    return up_.getPassword();
  } else if (part_ == "path") {
    return up_.getPath();
  } else if (part_ == "query") {
    return up_.getQuery();
  } else if (part_ == "fragment") {
    return up_.getFragment();
  } else {
    return std::string();
  }
}

/*!
 * \brief addrToNumeric is a convenience function to be used
 * in simple conversions or together with the the DataKey() object.
 * \param addr_ IPv4 address in dot-decimal notation.
 * \return uint32_t Decimal representation.
 * \note DataKey usage:
 *
 */
uint32_t
SquidLogParser::addrToNumeric(const std::string&& addr_) const
{
  return (!addr_.empty() ? IPv4Addr::iptol(addr_) : 0UL);
}

/*!
 * \brief numericToAddr, convenience function to be used in conversions.
 * \param ip_ Decimal IPv4 representation.
 * \return string IPv4 address in dot-decimal notation.
 */
std::string
SquidLogParser::numericToAddr(const uint32_t&& ip_) const
{
  return (ip_ != 0UL ? IPv4Addr::ltoip(ip_) : std::string());
}

/*!
 * \brief unixTimestamp, convenience function that convert the human-readable
 * format date to a Unix timestamp.
 * \param d_ string date 'n' time
 * \return uint32_t Epoch
 * \note Date format accepted:dd/Mmm/yyyy:hh:mm:ss [TZ]
 * \note TZ is ignored
 */
uint32_t
SquidLogParser::unixTimestamp(const std::string d_) const
{
  if (!d_.empty()) {
    boost::regex re_(cp_fmt_squid_date);
    boost::match_results<std::string::const_iterator> match;
    boost::regex_match(d_, match, re_);
    if (!match.empty()) {
      auto [dd_, mm_, yy_, hh_, mn_, ss_] = std::tuple(std::stoi(match[1]),
                                                       monthToNumber(match[2]),
                                                       std::stoi(match[3]),
                                                       std::stoi(match[4]),
                                                       std::stoi(match[5]),
                                                       std::stoi(match[6]));

      if ((dd_ >= 1 && dd_ <= 31) && (mm_ >= 1 && mm_ <= 12) && (yy_ >= 1970) &&
          (hh_ >= 0 && hh_ <= 23) && (mn_ >= 0 && mn_ <= 59) &&
          (ss_ >= 0 && ss_ <= 59)) {
        std::tm tm_ = mkTime(d_);
        return std::mktime(&tm_);
      }
    }
  }
  return 0;
}

/*!
 * \brief unixToSquidDate, convenience function that convert the numeric
 * Unix timestamp to a human-readable format date.
 * \param uts_ Unix Timestamp.
 * \return std::string Squid-format date in human-readable.
 */
std::string
SquidLogParser::unixToSquidDate(std::time_t uts_) const
{
  char buf_[27];
  struct std::tm tm_ = { *std::localtime(&uts_) };
  ::strftime(buf_, sizeof(buf_), "%d/%b/%Y:%H:%M:%S %z", &tm_);
  return std::string(buf_);
}

/* protected----------------------------------------------------------------
 */
/*!
 * \internal
 * \brief Simple recursive uppercase to lowercase characters conversion.
 * \param s_ Text with (maybe) uppercase  characteres.
 * \return Text in lowercase.
 */
template<typename TString, typename TSize>
TString
SquidLogParser::toLower(TString s_, TSize sz_)
{
  if (sz_ != s_.size()) {
    s_[sz_] = ::tolower(s_[sz_]);
    s_ = toLower(s_, ++sz_);
  }
  return s_;
}

/*!
 * \brief Returns the right part of string until the end. From position+1 of
 * the informed separator.
 *
 * \param src_
 * \param sep_
 * \return std::string
 *
 * \example std::string s_ = strRight("NONE/200"s,'/'); // s_ = "200";
 *
 */
std::string
SquidLogParser::strRight(const std::string src_, const char sep_) const
{
  if (size_t f = std::string_view{ src_ }.find(sep_); f != std::string::npos) {
    return src_.substr(f + 1, src_.length());
  }
  return std::string();
}
/*!
 * \internal
 * \brief Returns true or false if month name is corret
 * \param s Month name abbreviation
 * \return true|false
 * \code{.cc}
 * bool b = isMonth("Jan");
 * \endcode
 */
bool
SquidLogParser::isMonth(const std::string&& s_)
{
  auto begin_ = std::cbegin(nmonths_);
  auto end_ = std::cend(nmonths_);

  const auto f_ = std::find_if(begin_, end_, [&s_](const char* cptr_) {
    if (std::strcmp(s_.c_str(), cptr_) == 0) {
      return true;
    }
    return false;
  });

  return (f_ != end_);
}

/*!
 * \internal
 * \brief Converts the month's name abbreviation to number.
 * \param s_
 * \return int
 */
int
SquidLogParser::monthToNumber(const std::string&& s_) const
{
  auto begin_ = std::cbegin(nmonths_);
  auto end_ = std::cend(nmonths_);

  const auto f_ = std::find_if(begin_, end_, [&s_](const char* cptr_) {
    if (std::strcmp(s_.c_str(), cptr_) == 0) {
      return true;
    }
    return false;
  });

  if (f_ != end_) {
    return std::distance(begin_, f_) + 1;
  }
  return 0;
}

/*!
 * \brief Converts the month's number to abbreviation name.
 * \param m_ A number between 1 and 12
 * \return Abbrev of month's name
 */
std::string
SquidLogParser::numberToMonth(const int m_) const
{
  if ((m_ >= 1) && (m_ <= 12)) {
    return std::string(nmonths_[m_ - 1]);
  }
  return std::string();
}

/*!
 * \internal
 * \brief Parses date and time strings and save the values inside std::tm.
 * \param d_ Date (dd/Mmm/yyyy:hh:mm:ss -0000)
 * \return std::tm
 */
std::tm
SquidLogParser::mkTime(const std::string d_) const
{
  struct std::tm tm_tmp = {};
  tm_tmp.tm_year = std::move(std::stoi(d_.substr(7, 4))) - 1900;
  tm_tmp.tm_mon = std::move(monthToNumber(d_.substr(3, 3))) - 1;
  tm_tmp.tm_mday = std::move(std::stoi(d_.substr(0, 2)));
  tm_tmp.tm_hour = std::move(std::stoi(d_.substr(12, 2)));
  tm_tmp.tm_min = std::move(std::stoi(d_.substr(15, 2)));
  tm_tmp.tm_sec = std::move(std::stoi(d_.substr(18, 2)));

  return tm_tmp;
}

/*!
 * \internal
 * \brief SquidLogParser::setError
 * \param e_ Error code
 */
void
SquidLogParser::setError(SLPError e_)
{
  slpError_ = e_;
}

std::string
SquidLogParser::getErrorRE(boost::regex_error& e_) const
{
  switch (e_.code()) {
    case boost::regex_constants::error_collate: {
      const_cast<SquidLogParser*>(this)->setError(
        SLPError::SLP_ERR_REGEX_COLLATE);
      break;
    }
    case boost::regex_constants::error_ctype: {
      const_cast<SquidLogParser*>(this)->setError(
        SLPError::SLP_ERR_REGEX_CTYPE);
      break;
    }
    case boost::regex_constants::error_escape: {
      const_cast<SquidLogParser*>(this)->setError(
        SLPError::SLP_ERR_REGEX_ESCAPE);
      break;
    }
    case boost::regex_constants::error_backref: {
      const_cast<SquidLogParser*>(this)->setError(
        SLPError::SLP_ERR_REGEX_BACKREF);
      break;
    }
    case boost::regex_constants::error_brack: {
      const_cast<SquidLogParser*>(this)->setError(
        SLPError::SLP_ERR_REGEX_BRACK);
      break;
    }
    case boost::regex_constants::error_paren: {
      const_cast<SquidLogParser*>(this)->setError(
        SLPError::SLP_ERR_REGEX_PAREN);
      break;
    }
    case boost::regex_constants::error_brace: {
      const_cast<SquidLogParser*>(this)->setError(
        SLPError::SLP_ERR_REGEX_BRACE);
      break;
    }
    case boost::regex_constants::error_badbrace: {
      const_cast<SquidLogParser*>(this)->setError(
        SLPError::SLP_ERR_REGEX_BADBRACE);
      break;
    }
    case boost::regex_constants::error_range: {
      const_cast<SquidLogParser*>(this)->setError(
        SLPError::SLP_ERR_REGEX_RANGE);
      break;
    }
    case boost::regex_constants::error_space: {
      const_cast<SquidLogParser*>(this)->setError(
        SLPError::SLP_ERR_REGEX_SPACE);
      break;
    }
    case boost::regex_constants::error_badrepeat: {
      const_cast<SquidLogParser*>(this)->setError(
        SLPError::SLP_ERR_REGEX_BADREPEAT);
      break;
    }
    case boost::regex_constants::error_complexity: {
      const_cast<SquidLogParser*>(this)->setError(
        SLPError::SLP_ERR_REGEX_COMPLEXITY);
      break;
    }
    case boost::regex_constants::error_stack: {
      const_cast<SquidLogParser*>(this)->setError(
        SLPError::SLP_ERR_REGEX_STACK);
      break;
    }
    default: {
      const_cast<SquidLogParser*>(this)->setError(SLPError::SLP_ERR_UNKNOWN);
    }
  }
  return getErrorText();
}

/*!
 * \internal
 * \brief Returns the value of integer fields
 * \param f_ Field Id
 * \param d_ Data
 * \return int
 */
constexpr int
SquidLogParser::intFields(Fields f_, const DataSet_Squid& d_) const
{

  switch (f_) {
    case Fields::ResponseTime: {
      return d_.responseTime;
    }
    case Fields::TotalSizeReply: {
      return d_.totalSizeReply;
    }
    case Fields::HttpStatus: {
      return d_.httpStatus;
    }
    default: {
      return -1;
    }
  }
}

/*!
 * \internal
 * \brief Returns the value of uint32_t fields
 * \param f_ Field Id
 * \param d_ Data
 * \return uint32_t
 */
constexpr uint32_t
SquidLogParser::uint32Fields(Fields f_, const DataSet_Squid& d_) const
{
  switch (f_) {
    case Fields::Timestamp: {
      return d_.timeStamp;
    }
    case Fields::CliSrcIpAddr: {
      return d_.cliSrcIpAddr;
    }
    default: {
      return 0;
    }
  }
}

/*!
 * \internal
 * \brief Returns the value of string fields
 * \param f_ Field Id
 * \param d_ Data
 * \return std::string
 */
std::string
SquidLogParser::strFields(Fields f_, const DataSet_Squid& d_) const
{
  switch (f_) {
    case Fields::Timestamp: {
      return unixToSquidDate(d_.timeStamp);
    }
    case Fields::CliSrcIpAddr: {
      return IPv4Addr::ltoip(d_.cliSrcIpAddr);
    }
    case Fields::LocalTime: {
      return d_.localTime;
    }
    case Fields::UserName: {
      return d_.userName;
    }
    case Fields::UserNameIdent: {
      return d_.userNameIdent;
    }
    case Fields::ReqMethod: {
      return d_.reqMethod;
    }
    case Fields::ReqURL: {
      return d_.reqURL;
    }
    case Fields::ReqProtoVersion: {
      return d_.reqProtoVersion;
    }
    case Fields::ReqStatusHierStatus: {
      return d_.reqStatusHierStatus;
    }
    case Fields::HierStatusIpAddress: {
      return d_.hierStatusIpAddress;
    }
    case Fields::MimeContentType: {
      return d_.mimeTypeContent;
    }
    case Fields::OrigRcvReqHeader: {
      return d_.origRcvReqHeader;
    }
    case Fields::Referrer: {
      return d_.referrer;
    }
    case Fields::UserAgent: {
      return d_.userAgent;
    }
    default: {
      return invalidText.data();
    }
  }
}

/*!
 * \internal
 * \brief This template function implements the logical AND and OR operations
 * for functions like "between A AND B" or "between A OR B".
 *
 * \param data_ The data to be compared with its limits.
 * \param min_ Lower value.
 * \param max_ Highest value.
 * \param cmp_ Type of logical operation: BTWAND | BTWOR
 *
 *
 */
template<typename TVarD, typename TMin, typename TMax, typename TCompare>
bool
SquidLogParser::decision(TVarD&& data_,
                         TMin&& min_,
                         TMax&& max_,
                         TCompare&& cmp_) const
{
  switch (cmp_) {
    case Compare::BTWAND: {
      return ((data_ >= min_) && (data_ <= max_));
    }
    case Compare::BTWOR: {
      return ((data_ >= min_) || (data_ <= max_));
    }
    default: {
      return false;
    }
  }
};

/*!
 * \internal
 * \brief Overloaded: This template function implements the logical
 * operations:
 * == < > <= >= !=
 * \param lhs_ Argument 1
 * \param rhs_ Argument 2
 * \param cmp_ Type of logical operation. EQ, LT, GT, LE, GE, NE
 */
template<typename TVarS, typename TVarD, typename TCompare>
bool
SquidLogParser::decision(TVarS&& lhs_, TVarD&& rhs_, TCompare&& cmp_) const
{
  switch (cmp_) {
    case Compare::EQ: {
      return lhs_ == rhs_;
    }
    case Compare::LT: {
      return lhs_ < rhs_;
    }
    case Compare::GT: {
      return lhs_ > rhs_;
    }
    case Compare::LE: {
      return lhs_ <= rhs_;
    }
    case Compare::GE: {
      return lhs_ >= rhs_;
    }
    case Compare::NE: {
      return lhs_ != rhs_;
    }
    default: {
      return false;
    }
  }
};

/*!
 * \brief Return a error code.
 * \return
 */
SquidLogParser::SLPError
SquidLogParser::errorNum() const noexcept
{
  return slpError_;
}

/*!
 * \brief Return the description of the error.
 * \return std::string
 */
std::string
SquidLogParser::getErrorText() const
{
  if (const auto& it_(mError.find(slpError_)); it_ != mError.end()) {
    return it_->second.data();
  }
  return mError.at(SLPError::SLP_ERR_UNKNOWN).data();
}

/* private------------------------------------------------------------------
 */

/*!
 * \internal
 *
 * \note The constants below are defined in CMakeLists.txt
 *
 * DEBUG_PARSER_SQUID
 * DEBUG_PARSER_COMMON
 * DEBUG_PARSER_COMBINED
 * DEBUG_PARSER_REFERRER
 * DEBUG_PARSER_USERAGENT
 *
 */

/*!
 * \internal
 * \brief SquidLogParser::parserSquid
 * \return
 *
 */
SquidLogData::SLPError
SquidLogParser::parserSquid()
{
#ifdef DEBUG_PARSER_SQUID
  std::cout << "raw : " << rawLog_ << "\n";
#endif

  try {
    boost::match_results<std::string::const_iterator> match;
    boost::regex_match(rawLog_, match, re_id_fmt_squid_);
    if (match.empty()) {
      setError(SLPError::SLP_ERR_PARSER_FAILED);
      return SLPError::SLP_ERR_PARSER_FAILED;
    }

    ds_squid_ = {};
    ds_squid_.timeStamp = std::move(std::stod(match[1]));
    ds_squid_.responseTime = std::move(std::stoi(match[2]));
    ds_squid_.cliSrcIpAddr = std::move(IPv4Addr::iptol(match[3]));
    ds_squid_.reqStatusHierStatus = std::move(match[4]);
    ds_squid_.totalSizeReply = std::move(std::stoi(match[5]));
    ds_squid_.reqMethod = std::move(match[6]);
    ds_squid_.reqURL = std::move(match[7]);
    ds_squid_.userName = std::move(match[8]);
    ds_squid_.hierStatusIpAddress = std::move(match[9]);
    ds_squid_.mimeTypeContent = std::move(match[10]);

#ifdef DEBUG_PARSER_SQUID
    std::cout << "ds_squid :\n";
    std::cout << ds_squid_.timeStamp << "\n"
              << ds_squid_.responseTime << "\n"
              << ds_squid_.cliSrcIpAddr << "\n"
              << ds_squid_.reqStatusHierStatus << "\n"
              << ds_squid_.totalSizeReply << "\n"
              << ds_squid_.reqMethod << "\n"
              << ds_squid_.reqURL << "\n"
              << ds_squid_.userName << "\n"
              << ds_squid_.hierStatusIpAddress << "\n"
              << ds_squid_.mimeTypeContent << "\n";
    for (size_t i = 1; i < match.size(); i++) {
      std::cout << "> " << i << " -- " << match[i] << "\n";
    }
#endif
  } catch (boost::regex_error& e_) {
    std::cout << "Parser PFLogentry regex error = " << e_.what() << "\n";
    setError(SLPError::SLP_ERR_PARSER_FAILED);
    return SLPError::SLP_ERR_PARSER_FAILED;
  } catch (const std::exception& e) {
    std::cout << "[" << __LINE__ << "] "
              << __FILE__ ": An exception occurred: " << e.what() << "\n";
  };

  setError(SLPError::SLP_SUCCESS);
  return SLPError::SLP_SUCCESS;
}

/*!
 * \internal
 * \brief SquidLogParser::parserCommon
 * \return
 */
SquidLogData::SLPError
SquidLogParser::parserCommon()
{

#ifdef DEBUG_PARSER_COMMON
  std::cout << "raw : " << rawLog_ << "\n";
#endif

  try {
    boost::match_results<std::string::const_iterator> match;
    boost::regex_match(rawLog_, match, re_id_fmt_common_);
    if (match.empty()) {
      setError(SLPError::SLP_ERR_PARSER_FAILED);
      return SLPError::SLP_ERR_PARSER_FAILED;
    }

    ds_squid_ = {};
    ds_squid_.cliSrcIpAddr = std::move(IPv4Addr::iptol(match[1]));
    ds_squid_.userNameIdent = std::move(match[2]);
    ds_squid_.userName = std::move(match[3]);
    ds_squid_.localTime = std::move(match[4]);
    ds_squid_.reqMethod = std::move(match[5]);
    ds_squid_.reqURL = std::move(match[6]);
    ds_squid_.reqProtoVersion = std::move(match[7]);
    ds_squid_.httpStatus = std::move(std::stoi(match[8]));
    ds_squid_.totalSizeReply = std::move(std::stoi(match[9]));
    ds_squid_.reqStatusHierStatus = std::move(match[10]);

#ifdef DEBUG_PARSER_COMMON
    std::cout << "ds_common :\n";
    std::cout << ds_squid_.cliSrcIpAddr << "\n"
              << ds_squid_.userNameIdent << "\n"
              << ds_squid_.userName << "\n"
              << ds_squid_.localTime << "\n"
              << ds_squid_.reqMethod << "\n"
              << ds_squid_.reqURL << "\n"
              << ds_squid_.reqProtoVersion << "\n"
              << ds_squid_.httpStatus << "\n"
              << ds_squid_.totalSizeReply << "\n"
              << ds_squid_.reqStatusHierStatus << "\n";

    for (size_t i = 1; i < match.size(); i++) {
      std::cout << "> " << i << " -- " << match[i] << "\n";
    }
#endif
  } catch (boost::regex_error& e_) {
    std::cout << "Parser PFLogentry regex error = " << e_.what() << "\n";
    setError(SLPError::SLP_ERR_PARSER_FAILED);
    return SLPError::SLP_ERR_PARSER_FAILED;
  } catch (const std::exception& e) {
    std::cout << "[" << __LINE__ << "] "
              << __FILE__ ": An exception occurred: " << e.what() << "\n";
  };

  setError(SLPError::SLP_SUCCESS);
  return SLPError::SLP_SUCCESS;
}

/*!
 * \internal
 * \brief SquidLogParser::parserCombined
 * \return
 */
SquidLogData::SLPError
SquidLogParser::parserCombined()
{
#ifdef DEBUG_PARSER_COMBINED
  std::cout << "raw : " << rawLog_ << "\n";
#endif

  try {
    boost::match_results<std::string::const_iterator> match;
    boost::regex_match(rawLog_, match, re_id_fmt_combined_);
    if (match.empty()) {
      setError(SLPError::SLP_ERR_PARSER_FAILED);
      return SLPError::SLP_ERR_PARSER_FAILED;
    }

    ds_squid_ = {};
    ds_squid_.cliSrcIpAddr = std::move(IPv4Addr::iptol(match[1]));
    ds_squid_.userNameIdent = std::move(match[2]);
    ds_squid_.userName = std::move(match[3]);
    ds_squid_.localTime = std::move(match[4]);
    ds_squid_.reqMethod = std::move(match[5]);
    ds_squid_.reqURL = std::move(match[6]);
    ds_squid_.reqProtoVersion = std::move(match[7]);
    ds_squid_.httpStatus = std::move(std::stoi(match[8]));
    ds_squid_.totalSizeReply = std::move(std::stoi(match[9]));
    ds_squid_.referrer = std::move(match[10]);
    ds_squid_.userAgent = std::move(match[11]);
    ds_squid_.reqStatusHierStatus = std::move(match[12]);

#ifdef DEBUG_PARSER_COMBINED
    std::cerr << "ds_combined :\n";
    /*
    std::cerr << ds_squid_.cliSrcIpAddr << "\n"
              << ds_squid_.userNameIdent << "\n"
              << ds_squid_.userName << "\n"
              << ds_squid_.localTime << "\n"
              << ds_squid_.reqMethod << "\n"
              << ds_squid_.reqURL << "\n"
              << ds_squid_.reqProtoVersion << "\n"
              << ds_squid_.httpStatus << "\n"
              << ds_squid_.totalSizeReply << "\n"
              << ds_squid_.referrer << "\n"
              << ds_squid_.userAgent << "\n"
              << ds_squid_.reqStatusHierStatus << "\n";
    */
    for (size_t i = 1; i < match.size(); i++) {
      std::cerr << "> " << i << " -- " << match[i] << "\n";
    }
#endif
  } catch (boost::regex_error& e_) {
    std::cout << "Parser PFLogentry regex error = " << e_.what() << "\n";
    setError(SLPError::SLP_ERR_PARSER_FAILED);
    return SLPError::SLP_ERR_PARSER_FAILED;
  } catch (const std::exception& e) {
    std::cout << "[" << __LINE__ << "] "
              << __FILE__ ": An exception occurred: " << e.what() << "\n";
  };

  setError(SLPError::SLP_SUCCESS);
  return SLPError::SLP_SUCCESS;
}

/*!
 * \internal
 * \brief SquidLogParser::parserReferrer
 * \return
 */
SquidLogData::SLPError
SquidLogParser::parserReferrer()
{
#ifdef DEBUG_PARSER_REFERRER
  std::cout << "raw : " << rawLog_ << "\n";
#endif

  try {
    boost::match_results<std::string::const_iterator> match;
    boost::regex_match(rawLog_, match, re_id_fmt_referrer_);
    if (match.empty()) {
      setError(SLPError::SLP_ERR_PARSER_FAILED);
      return SLPError::SLP_ERR_PARSER_FAILED;
    }

    ds_squid_ = {};
    ds_squid_.timeStamp = std::move(std::stoul(match[1]));
    ds_squid_.cliSrcIpAddr = std::move(IPv4Addr::iptol(match[2]));
    ds_squid_.referrer = std::move(match[3]);
    ds_squid_.reqURL = std::move(match[4]);

#ifdef DEBUG_PARSER_REFERRER
    std::cout << "ds_referrer :\n";
    std::cout << ds_squid_.timeStamp << "\n"
              << ds_squid_.cliSrcIpAddr << "\n"
              << ds_squid_.referrer << "\n"
              << ds_squid_.reqURL << "\n";

    for (size_t i = 1; i <= match.size(); i++) {
      std::cout << "> " << i << " -- " << match[i] << "\n";
    }
#endif
  } catch (boost::regex_error& e_) {
    std::cout << "Parser PFLogentry regex error = " << e_.what() << "\n";
    setError(SLPError::SLP_ERR_PARSER_FAILED);
    return SLPError::SLP_ERR_PARSER_FAILED;
  } catch (const std::exception& e) {
    std::cout << "[" << __LINE__ << "] "
              << __FILE__ ": An exception occurred: " << e.what() << "\n";
  };

  setError(SLPError::SLP_SUCCESS);
  return SLPError::SLP_SUCCESS;
}

/*!
 * \internal
 * \brief SquidLogParser::parserUserAgent
 * \return
 */
SquidLogData::SLPError
SquidLogParser::parserUserAgent()
{

#ifdef DEBUG_PARSER_USERAGENT
  std::cout << "raw : " << rawLog_ << "\n";
#endif

  try {
    boost::match_results<std::string::const_iterator> match;
    boost::regex_match(rawLog_, match, re_id_fmt_useragent_);
    if (match.empty()) {
      setError(SLPError::SLP_ERR_PARSER_FAILED);
      return SLPError::SLP_ERR_PARSER_FAILED;
    }

    ds_squid_ = {};
    ds_squid_.cliSrcIpAddr = std::move(IPv4Addr::iptol(match[1]));
    ds_squid_.localTime = std::move(match[2]);
    ds_squid_.userAgent = std::move(match[3]);

#ifdef DEBUG_PARSER_USERAGENT
    std::cout << "ds_useragent :\n";
    std::cout << ds_squid_.cliSrcIpAddr << "\n"
              << ds_squid_.localTime << " "
              << unixTimestamp(ds_squid_.localTime) << "\n"
              << ds_squid_.userAgent << "\n";

    for (size_t i = 1; i <= match.size(); i++) {
      std::cout << "> " << i << " -- " << match[i] << "\n";
    }
#endif
  } catch (boost::regex_error& e_) {
    std::cout << "Parser PFLogentry regex error = " << e_.what() << "\n";
    setError(SLPError::SLP_ERR_PARSER_FAILED);
    return SLPError::SLP_ERR_PARSER_FAILED;
  } catch (const std::exception& e) {
    std::cout << "[" << __LINE__ << "] "
              << __FILE__ ": An exception occurred: " << e.what() << "\n";
  };

  setError(SLPError::SLP_SUCCESS);
  return SLPError::SLP_SUCCESS;
}

/*!
 * \internal
 * \brief  Normalize a string removing the extra white spaces between words.
 * \param input_
 * \param output_
 * \note Adapted from:
 * https://stackoverflow.com/questions/35301432/remove-extra-white-spaces-in-c/35302029
 */
void
SquidLogParser::removeExtraWhiteSpaces(const std::string& input_,
                                       std::string& output_)
{
  output_.clear(); // unless you want to add at the end of existing string...
  std::unique_copy(
    input_.cbegin(),
    input_.cend(),
    std::back_insert_iterator<std::string>(output_),
    [](char a_, char b_) { return ::isspace(a_) && ::isspace(b_); });
}

/* SLPUrlParts-------------------------------------------------------------- */
/*!
 * \brief Parses URLs (http[s]) the log line and returns the parts inside the
 * UrlAnatomy_t structure.
 *
 * \param rawUrl_
 */
SLPUrlParts::SLPUrlParts(const std::string rawUrl_)
  : raw_url_(std::move(rawUrl_))
{
  parseUrl();
}

/*!
 * \brief SLPUrlParts::getProtocol
 * \return string
 */
std::string
SLPUrlParts::getScheme() const
{
  return url_t.scheme_;
}

/*!
 * \brief SLPUrlParts::getDomain
 * \return string
 */
std::string
SLPUrlParts::getDomain() const
{
  return url_t.domain_;
}

/*!
 * \brief SLPUrlParts::getUsername
 * \return
 */
std::string
SLPUrlParts::getUsername() const
{
  return url_t.username_;
}

/*!
 * \brief SLPUrlParts::getPassword
 * \return
 */
std::string
SLPUrlParts::getPassword() const
{
  return url_t.password_;
}

/*!
 * \brief SLPUrlParts::getPath
 * \return string
 */
std::string
SLPUrlParts::getPath() const
{
  return url_t.path_;
}

/*!
 * \brief SLPUrlParts::getQuery
 * \return string
 */
std::string
SLPUrlParts::getQuery() const
{
  return url_t.query_;
}

/*!
 * \brief SLPUrlParts::getFragment
 * \return string
 */
std::string
SLPUrlParts::getFragment() const
{
  return url_t.fragment_;
}

/*!
 * \private
 * \brief Separates the URL into its parts.
 *
 * \verbatin
 * RFC 2396
 * 3. URI Syntactic Components
 *
 *  The URI syntax is dependent upon the scheme.  In general, absolute
 *  URI are written as follows:
 *
 *     <scheme>:<scheme-specific-part>
 *
 *  An absolute URI contains the name of the scheme being used (<scheme>)
 *  followed by a colon (":") and then a string (the <scheme-specific-
 *  part>) whose interpretation depends on the scheme.
 *
 *  The URI syntax does not require that the scheme-specific-part have
 *  any general structure or set of semantics which is common among all
 *  URI.  However, a subset of URI do share a common syntax for
 *  representing hierarchical relationships within the namespace.  This
 *  "generic URI" syntax consists of a sequence of four main components:
 *
 *      <scheme>://<authority><path>?<query>
 *
 *  each of which, except <scheme>, may be absent from a particular URI.
 *  For example, some URI schemes do not allow an <authority> component,
 *  and others do not use a <query> component.
 *
 *     absoluteURI   = scheme ":" ( hier_part | opaque_part )
 *
 *  URI that are hierarchical in nature use the slash "/" character for
 *  separating hierarchical components.  For some file systems, a "/"
 *  character (used to denote the hierarchical structure of a URI) is the
 *  delimiter used to construct a file name hierarchy, and thus the URI
 *  path will look similar to a file pathname.  This does NOT imply that
 *  the resource is a file or that the URI maps to an actual filesystem
 *  (...)
 * \endverbatim
 */
void
SLPUrlParts::parseUrl()
{
  if (!raw_url_.empty()) {
    std::string work_url_ = raw_url_;

    size_t p_frag_ = std::string_view{ work_url_ }.find("#");
    if (p_frag_ != std::string::npos) {
      url_t.fragment_ =
        std::string_view{ work_url_ }.substr(p_frag_, raw_url_.size());
      work_url_ = std::string_view{ work_url_ }.substr(0, p_frag_);
    }

    size_t p_query_ = std::string_view{ work_url_ }.find("?");
    if (p_query_ != std::string::npos) {
      url_t.query_ =
        std::string_view{ work_url_ }.substr(p_query_, work_url_.size());
      work_url_ = std::string_view{ work_url_ }.substr(0, p_query_);
    }

    size_t p_scheme_ = std::string_view{ work_url_ }.find_first_of("://");
    size_t p_slash_;
    if (p_scheme_ != std::string::npos) {
      url_t.scheme_ = std::string_view{ work_url_ }.substr(0, p_scheme_);
      work_url_ =
        std::string_view{ work_url_ }.substr(p_scheme_ + 3, work_url_.size());

      // get domain
      p_slash_ = std::string_view{ work_url_ }.find("/");
      if (p_slash_ != std::string::npos) {
        url_t.domain_ = std::string_view{ work_url_ }.substr(0, p_slash_);
        size_t at_sign_pos_ = { 0 };
        if (getUserInfo(
              at_sign_pos_)) { // Try to recover username:password if any.
          url_t.domain_ =
            std::string_view{ work_url_ }.substr(at_sign_pos_ + 1, p_slash_);
        }
      }

      if (p_slash_ < work_url_.size()) {
        url_t.path_ =
          std::string_view{ work_url_ }.substr(p_slash_, work_url_.size());
      }
    }
  }
}

/*!
 * \brief SLPUrlParts::hasEscape
 * \param text_
 * \return true|false
 */
bool
SLPUrlParts::hasEscape(const std::string text_)
{
  size_t esc0_ = text_.find("%");
  return esc0_ != std::string::npos ? true : false;
}

/*!
 * \private
 * \brief If there are, returns the user information, which are:
 * username:password.
 *
 * \param size_t pos_ Position of at_sign, if found.
 * \return true|false If userinfo was found.
 *
 * \verbatim
 * RFC 2396 3.2.2. Server-based Naming Authority
 * The user information, if present, is followed by a
 * commercial at-sign "@".
 *
 *    userinfo      = *( unreserved | escaped |
 *                       ";" | ":" | "&" | "=" | "+" | "$" | "," )
 * \endverbatim
 */
bool
SLPUrlParts::getUserInfo(size_t& pos_)
{
  SquidLogParser slp;

  if (size_t at_sign_pos_ = url_t.domain_.find_first_of("@");
      at_sign_pos_ != std::string::npos) {
    std::string userinfo_ = url_t.domain_.substr(0, at_sign_pos_);
    pos_ = at_sign_pos_; // retorn position.
    size_t f_ = { 0 };
    std::string temp_ = {};
    if (hasEscape(userinfo_)) {
      temp_ = slp.UrlDecode(userinfo_);
    } else {
      temp_ = userinfo_;
    }

    if (f_ = temp_.find(":"); f_ != std::string::npos) {
      url_t.username_ = temp_.substr(0, f_);
      url_t.password_ = temp_.substr(f_ + 1, temp_.size());
      return true;
    }
  }
  return false;
}

} // namespace squidlogparser
