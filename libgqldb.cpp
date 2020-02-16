/** \file
 * \brief generic GQL DB interactions
 * \author Claudio Fleiner
 * \copyright 2018 Claudio Fleiner
 *
 * **License:** 
 *
 * > This program is free software: you can redistribute it and/or modify
 * > it under the terms of the GNU Affero General Public License as published by
 * > the Free Software Foundation, either version 3 of the License, or
 * > (at your option) any later version.
 * >
 * > This program is distributed in the hope that it will be useful,
 * > but WITHOUT ANY WARRANTY; without even the implied warranty of
 * > MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * > GNU Affero General Public License for more details.
 * >
 * > You should have received a copy of the GNU Affero General Public License
 * > along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <math.h>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <exception>
#include <glog/logging.h>
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string.hpp>
#include <unicode/utypes.h>
#include <unicode/unistr.h>
#include <unicode/numfmt.h>
#include <unicode/gregocal.h>
#include <unicode/dcfmtsym.h>
#include <unicode/datefmt.h>
#include <unicode/smpdtfmt.h>
#include <unicode/dtfmtsym.h>
#include <unicode/decimfmt.h>
#include <unicode/locid.h>
#include <unicode/uclean.h>

#include "libgqlsql.h"

namespace GQL_SQL {

std::ostream& operator<<(std::ostream& outs, ErrorReasons er)
{
    switch(er) {
    case ErrorReasons::UNDEFINED: outs << "???";break;
    case ErrorReasons::NOT_MODIFIED: outs << "not_modified";break;
    case ErrorReasons::USER_NOT_AUTHENTICATED: outs << "user_not_authenticated";break;
    case ErrorReasons::UNKNOWN_DATA_SOURCE_ID: outs << "unknown_data_source_id";break;
    case ErrorReasons::ACCESS_DENIED: outs << "access_denied";break;
    case ErrorReasons::UNSUPPORTED_QUERY_OPERATION: outs << "unsupported_query_operation";break;
    case ErrorReasons::INVALID_QUERY: outs << "invalid_query";break;
    case ErrorReasons::INVALID_REQUEST: outs << "invalid_request";break;
    case ErrorReasons::INTERNAL_ERROR: outs << "internal_error";break;
    case ErrorReasons::NOT_SUPPORTED: outs << "not_supported";break;
    case ErrorReasons::ILLEGAL_FORMATTING_PATTERNS: outs << "illegal_formatting_patterns";break;
    case ErrorReasons::OTHER: outs << "other";break;
    }
    return outs;
}

namespace DBQuery {

void DB::pivotTable(const Json::Value &tbl,Json::Value &res) const
{
    VLOG(1) << "pivotTable: " << parser_->query()->pivot.size() << std::endl;
    if(!parser_->query()->hasPivotClause()) { return; }

    // each pivot row is at the beginning of the select and ordered

    // first get all the combination for the pivot colums. Those must
    // be the first ones.
    std::map<std::string,uint64_t> cls;
    std::vector<std::string> clsname;
    // map pivot keys to column number to use

    auto &cols=tbl["cols"];
    auto &rows=tbl["rows"];
    // FixMe: should use separate query in order speed up large queries
    // For now scan the table twice, once to get all combination, and then
    // a second time to create the new table (even this could be combined
    // but may require going back and adding in new values]

    VLOG(1) << "rows: " << rows.size() << std::endl;
    for(uint32_t r=0;r<rows.size();r++) {
        std::string key;
        auto rw=rows[r]["c"];
        for(uint32_t c=0;c<parser_->query()->pivot.size();c++) {
            if(c>0) { key+=","; }
            if(rw[c].isMember("f")) {
                key+=rw[c]["f"].asString();
             } else {
                key+=rw[c]["v"].asString();
             }
        }
        if(cls.count(key)==0) {
            auto s=cls.size();
            cls[key]=s;
            VLOG(1) << "KEY:" << key << " = " << s << std::endl;
            clsname.push_back(key);
        }
    }

    std::map<std::string,uint64_t> groups;
    // map group column name to position
    for(auto n:parser_->query()->group) {
        auto s=groups.size();
        groups[n.token]=s;
        VLOG(1) << "GROUP: " << n.token << std::endl;
    }

    res["cols"]=Json::Value();
    Json::Value &newcols=res["cols"];

    for(uint32_t c=static_cast<uint32_t>(parser_->query()->pivot.size()+parser_->query()->group.size());c<cols.size();c++) {
        uint32_t index=newcols.size();
        if(groups.count(cols[c]["id"].asString())) {
            newcols[index]=cols[c];
        } else {
            for(auto n:clsname) {
                Json::Value nc=cols[c];
                auto lb=nc["label"].asString();
                if(lb.size()==0) { lb=nc["id"].asString(); }
                nc["id"]=n+" "+nc["id"].asString();
                nc["label"]=n+" "+lb;
                newcols[index++]=nc;
            }
        }
    }

    std::map<std::string,Json::Value> lines;
    uint32_t grstart=static_cast<uint32_t>(parser_->query()->pivot.size());
    uint32_t grend=static_cast<uint32_t>(grstart+parser_->query()->group.size());
    std::vector<std::string> order;

    for(uint32_t r=0;r<rows.size();r++) {
        std::string key;
        auto rw=rows[r]["c"];
        for(uint32_t c=grstart;c<grend;c++) {
            key+="\t";
            if(rw[c].isMember("f")) {
                key+=rw[c]["f"].asString();
            } else {
                key+=rw[c]["v"].asString();
            }
        }

        if(lines.count(key)==0) {
            Json::Value v;
            for(uint32_t c=grend;c<cols.size();c++) {
                uint32_t index=v.size();
                if(groups.count(cols[c]["id"].asString())) {
                    v[index]=rw[c];
                } else {
                    for(auto n:clsname) {
                        Json::Value nc;
                        nc["v"]=Json::Value::null;
                        v[index++]=nc;
                    }
                }
            }

            lines[key]=v;
            order.push_back(key);
        }

        std::string pkey="";
        auto row=rows[r]["c"];
        for(uint32_t c=0;c<parser_->query()->pivot.size();c++) {
            if(c>0) { pkey+=","; }
            if(row[c].isMember("f")) {
                pkey+=row[c]["f"].asString();
             } else {
                pkey+=row[c]["v"].asString();
             }
        }
        Json::Value line=lines[key];

        uint32_t index=0;
        VLOG(2) << "grend=" << grend << std::endl;
        for(uint32_t c=grend;c<cols.size();c++) {
            if(groups.count(cols[c]["id"].asString())) {
                index++;
            } else {
                VLOG(2) << "Set INDEX " << index << "+" << cls[pkey] << "=" << index+cls[pkey] << " = " << row[c] << std::endl;
                line[static_cast<int>(index+cls[pkey])]=row[c];
                index+=cls.size();
            }
        }
        lines[key]=line;
    }

    res["rows"]=Json::Value();
    Json::Value &newrows=res["rows"];
    for(auto o:order) {
        Json::Value c;
        c["c"]=lines[o];
        newrows[newrows.size()]=c;
    }
}

DB::DB(Json::Value i)
{
    type_=i["type"].asString();
    if(i.isMember("user")) { user_=i["user"].asString(); }
    if(i.isMember("password")) { password_=i["password"].asString(); }
    if(i.isMember("default")) { deftable_=i["default"].asString(); }
    if(i.isMember("server")) { server_=i["server"].asString(); }
    if(i.isMember("port")) { port_=i["port"].asUInt(); }
    if(i.isMember("db")) { db_=i["db"].asString(); }
    if(i.isMember("extended")) { extendedFunctions_=i["db"].asBool(); }
    if(i.isMember("tables")) {
        for(auto n:i["tables"]) {
            tables_.insert(n.asString());
        }
    }
}

DB::DB(const URI &uri)
{
    type_=uri.scheme();
    user_=uri.user();
    password_=uri.pass();
    server_=uri.host();
    port_=uri.port();
    db_=uri.path();
    if(db_[0]=='/') { db_=db_.substr(1); }
}

/// Used to guess boolean strings when applying boolen format
static std::set<std::string> validTrue {"1","true","True","TRUE" };
/// Used to guess boolean strings when applying boolen format
static std::set<std::string> validFalse {"0","false","False","FALSE",""} ;


bool PostgreSQL::isConnected() const 
{
    return connection_&&connection_->is_open();
}

static const std::string GENERAL="General"; ///< general format constant

static const int PATTERN_BOOLEAN=1;         ///< Boolean pattern 
static const int PATTERN_NUMBER=2;          ///< Number pattern 
static const int PATTERN_DATE=4;            ///< Date pattern 
static const int PATTERN_TIME=8;            ///< Time pattern 


// return true if this pattern could be a boolean
// pattern: has exactly one :
static bool isBooleanPattern(const std::string &str)
{
    auto pos=str.find(":");
    if(pos==std::string::npos) { return false; }
    pos=str.find(":",pos+1);
    return pos==std::string::npos;
}

/**
 * return true if this pattern contains a time format character.
 * Currently known are:
 * h        hour in am/pm (1~12)    (Number)            12
 * H        hour in day (0~23)      (Number)            0
 * m        minute in hour          (Number)            30
 * s        second in minute        (Number)            55
 * S        fractional second       (Number)            978
 * a        am/pm marker            (Text)              PM
 * k        hour in day (1~24)      (Number)            24
 * K        hour in am/pm (0~11)    (Number)            0
 * A        milliseconds in day     (Number)            69540000
 * z        time zone               (Time)              Pacific Standard Time
 * Z        time zone (RFC 822)     (Number)            -0800
 * v        time zone (generic)     (Text)              Pacific Time
 * V        time zone (abreviation) (Text)              PT
 * VVVV     time zone (location)    (Text)              United States (Los Angeles)
 */
inline static bool isTimePattern(const std::string &str)
{
    return str.find_first_of("hHmsSakKAzZvV")!=std::string::npos;
}


/**
 * return true if this pattern contains a date format character.
 * G        era designator          (Text)              AD
 * y        year                    (Number)            1996
 * Y        year (week of year)     (Number)            1997
 * u        extended year           (Number)            4601
 * Q        Quarter                 (Text & Number)     Q2 & 02
 * M        month in year           (Text & Number)     July & 07
 * d        day in month            (Number)            10
 * E        day of week             (Text)              Tuesday
 * e        day of week (local 1~7) (Text & Number)     Tues & 2
 * D        day in year             (Number)            189
 * F        day of week in month    (Number)            2 (2nd Wed in July)
 * w        week in year            (Number)            27
 * W        week in month           (Number)            2
 * g        Julian day              (Number)            2451334
 * q        stand alone quarter     (Text & Number)     Q2 & 02
 * L        stand alone month       (Text & Number)     July & 07
 * c        stand alone day of week (Text & Number)     Tuesday & 2
 */
inline static bool isDatePattern(const std::string &str)
{
    return str.find_first_of("GyYuQMdEeDFwWgqLc")!=std::string::npos;
}

/// Try to guess the pattern type of _pattern and return a bitmask of all
/// the possible interpretation options.
static int patternType(const std::string &_pattern, UErrorCode &_status)
{
    int res=0;
    _status=U_ZERO_ERROR;
    if(isBooleanPattern(_pattern)) { res|=PATTERN_BOOLEAN; }
    UErrorCode decuce=U_ZERO_ERROR;
    icu::UnicodeString upattern=icu::UnicodeString::fromUTF8(_pattern);
    DecimalFormat decf(upattern, decuce);
    VLOG(1) << "DecFormat: " << _pattern << " -> " << u_errorName(decuce) << std::endl;
    if(U_SUCCESS(decuce)) { res|=PATTERN_NUMBER; }
    UErrorCode dateuce=U_ZERO_ERROR;
    SimpleDateFormat datef(upattern, dateuce);
    VLOG(1) << "DateFormat: " << _pattern << " -> " << u_errorName(dateuce) << std::endl;
    if(U_SUCCESS(dateuce)) {
        if(isDatePattern(_pattern)) { res|=PATTERN_DATE; }
        if(isTimePattern(_pattern)) { res|=PATTERN_TIME; }
    }
    if(res==0) { _status=decuce; }
    return res;
}

/// Set the result of the query to the given error.
static void setError(Json::Value &_tbl,ErrorReasons _er,const std::string &_msg) 
{
    _tbl.removeMember("table");
    _tbl["status"]="error";
    _tbl["errors"]=Json::Value();
    _tbl["errors"][0]=Json::Value();
    _tbl["errors"][0]["reason"]=to_string(_er);
    _tbl["errors"][0]["message"]=_msg;
    return;
}

/// Maps how an input type is converted according tot he pattern that got detected.
/// Each line is another possible combination of PATTERN_BOOLEAN, PATTERN_NUMBER, etc.
static std::string typeConvert[][6] = {
    /* for     "string",     "boolean",   "number",     "timeofday",  "date",       "datetime"        */
    /*  0 */ { "",           "",           "",           "",           "",           ""            }, // 0 (error case)
    /*  1 */ { TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN  }, // BOOLEAN
    /*  2 */ { TYPE_NUMBER,  TYPE_NUMBER,  TYPE_NUMBER,  TYPE_NUMBER,  TYPE_NUMBER,  TYPE_NUMBER   }, // NUMBER
    /*  3 */ { TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN  }, // NUMBER|BOOLEAN
    /*  4 */ { TYPE_DATE,    TYPE_BOOLEAN, TYPE_DATE,    TYPE_DATE,    TYPE_DATE,    TYPE_DATE     }, // DATE
    /*  5 */ { TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN  }, // DATE|BOOLEAN
    /*  6 */ { TYPE_DATE,    TYPE_NUMBER,  TYPE_NUMBER,  TYPE_DATE,    TYPE_TIME,    TYPE_DATETIME }, // DATE|NUMBER
    /*  7 */ { TYPE_DATE,    TYPE_BOOLEAN, TYPE_NUMBER,  TYPE_DATE,    TYPE_TIME,    TYPE_DATETIME }, // DATE|NUMBER|BOOLEAN
    /*  8 */ { TYPE_TIME,    TYPE_TIME,    TYPE_TIME,    TYPE_DATE,    TYPE_TIME,    TYPE_DATETIME }, // TIME
    /*  9 */ { TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_BOOLEAN  }, // TIME|BOOLEAN
    /* 10 */ { TYPE_TIME,    TYPE_NUMBER,  TYPE_NUMBER,  TYPE_TIME,    TYPE_TIME,    TYPE_TIME     }, // TIME|NUMBER
    /* 11 */ { TYPE_TIME,    TYPE_BOOLEAN, TYPE_NUMBER,  TYPE_TIME,    TYPE_TIME,    TYPE_TIME     }, // TIME|NUMBER|BOOLEAN
    /* 12 */ { TYPE_DATETIME,TYPE_DATETIME,TYPE_DATETIME,TYPE_DATETIME,TYPE_DATETIME,TYPE_DATETIME }, // TIME|DATE
    /* 13 */ { TYPE_DATETIME,TYPE_BOOLEAN, TYPE_DATETIME,TYPE_DATETIME,TYPE_DATETIME,TYPE_DATETIME }, // TIME|DATE|BOOLEAN
    /* 14 */ { TYPE_DATETIME,TYPE_NUMBER,  TYPE_NUMBER,  TYPE_DATETIME,TYPE_DATETIME,TYPE_DATETIME }, // TIME|DATE|NUMBER
    /* 15 */ { TYPE_DATETIME,TYPE_BOOLEAN, TYPE_BOOLEAN, TYPE_DATETIME,TYPE_DATETIME,TYPE_DATETIME }, // TIME|DATE|NUMBER|BOOLEAN
};

/** Format all values as requested in the GQL query.
 * Uses some heuristics to decide how to do the various conversions.
 *
 * Format: the db may return strings for some of the types,
 *         in which case the format pattern must be used to
 *         find the correct type. The table above shows the conversion
 *
 * - for date/time return date, datetime or timeofday. If the pattern has only time
 *   chars the type will be set to timeofday, if there are only date chars date, and
 *   if both can be found datetime is returned
 */
static void setLabelFormat(Json::Value &cols,::GQL_SQL::GQLParser::Query::CPtr query) 
{
    if(query->selectStar) {
        for(auto s:query->select) {
            if(s.label!=""||s.format!="") {
                std::string id=s.expr->to_string();
                for (uint32_t i = 0; i < cols.size(); i++) {
                    if(cols[i]["id"]==id) {
                        if(s.label!="") {
                            cols[i]["label"]=s.label;
                        }
                        if(s.format!="") {
                            cols[i]["pattern"]=s.format;
                        }
                        break;
                    }
                }
            }
        }
    } else {
        int cnt=static_cast<int>(query->pivot.size()?query->pivot.size()+query->group.size():0);
        // FIXME: this assumes that all backends need manual pivoting!!
        // If this ever changes cnt must be set to 0 when doing DB pivoting
        // (other changes include the handling of the row formatting)
        
        for(auto s:query->select) {
            if(s.label!="") {
                cols[cnt]["label"]=s.label;
            }
            if(s.format!="") {
                cols[cnt]["pattern"]=s.format;
            }
            if(s.expr->sub().size()==0&&s.expr->tp()==GQLParser::TokenType::IDENTIFIER) {
                cols[cnt]["id"]=s.expr->data();
            } else {
                cols[cnt]["id"]=s.expr->to_string();
            }
            cnt++;
        }
    }

    for(uint32_t cnt=0;cnt<cols.size();cnt++) {
        auto c=cols[cnt];
        c["format"]=Json::Value::null;
        if(!c.isMember("label")) {
            c["label"]="";
        }
        if(!c.isMember("pattern")) {
            c["pattern"]=GENERAL;
        } else {
            UErrorCode uce=U_ZERO_ERROR;
            int pat=patternType(c["pattern"].asString(),uce);
            VLOG(2) << "Pattern: " << pat << " - " << c["format"] << " - " <<  c["pattern"];
            
            if(pat==0) {
                throw GQLError(ErrorReasons::ILLEGAL_FORMATTING_PATTERNS,c["pattern"].asString()+": "+u_errorName(uce));
            } else {
                if(c["type"]==TYPE_STRING) { c["format"]=typeConvert[pat][0]; }
                else if(c["type"]==TYPE_BOOLEAN) { c["format"]=typeConvert[pat][1]; }
                else if(c["type"]==TYPE_NUMBER) { c["format"]=typeConvert[pat][2]; }
                else if(c["type"]==TYPE_DATE) { c["format"]=typeConvert[pat][3]; }
                else if(c["type"]==TYPE_TIME) { c["format"]=typeConvert[pat][4]; }
                else if(c["type"]==TYPE_DATETIME) { c["format"]=typeConvert[pat][5]; }
            }
            VLOG(2) << "Format: " << pat << " - " <<  c["format"];
        }
        if(!c["format"].isNull()) {
            auto f=c["format"];
            c["format"]=c["type"];
            c["type"]=f;
            // type now has the correct output format, while format has the MYSQL format
        }
        cols[cnt]=c;
    }
}

/// Apply a number format pattern to a double
static std::string applyPattern(double flt,const NumberFormat *fmt)
{
    UnicodeString s2;
    UErrorCode uce=U_ZERO_ERROR;
    fmt->format(flt, s2, uce);
    if(!U_SUCCESS(uce)) {
        throw GQLError(ErrorReasons::ILLEGAL_FORMATTING_PATTERNS,"cannot convert '"+std::to_string(flt)+"'");
    }
    std::string result;
    s2.toUTF8String(result);
    return result;
}


/// Apply a number format pattern to an integer
static std::string applyPattern(int64_t flt,const NumberFormat *fmt)
{
    UnicodeString s2;
    UErrorCode uce=U_ZERO_ERROR;
    fmt->format(flt, s2, uce);
    if(uce!=U_ZERO_ERROR) {
        throw GQLError(ErrorReasons::ILLEGAL_FORMATTING_PATTERNS,"cannot convert '"+std::to_string(flt)+"'");
    }
    std::string result;
    s2.toUTF8String(result);
    return result;
}

/// Apply a date format pattern to a date represented as a double
static std::string applyPattern(double flt,const DateFormat *fmt)
{
    UnicodeString s2;
    UErrorCode uce=U_ZERO_ERROR;
    fmt->format(flt, s2, uce);
    if(uce!=U_ZERO_ERROR) {
        throw GQLError(ErrorReasons::ILLEGAL_FORMATTING_PATTERNS,"cannot convert '"+std::to_string(flt)+"'");
    }
    std::string result;
    s2.toUTF8String(result);
    return result;
}

/// apply the boolean format to a column
static void applyBooleanFormat(Json::Value &rows,
                               uint32_t c,
                               const std::string &pattern,
                               bool no_values,bool no_format)
{
    std::string truestr="TRUE";
    std::string falsestr="FALSE";
    if(pattern!=GENERAL) {
        auto split=pattern.find(":");
        if(split==std::string::npos) {
            throw GQLError(ErrorReasons::ILLEGAL_FORMATTING_PATTERNS,"pattern '"+pattern+"' is not a valid boolean pattern");
        }
        truestr=pattern.substr(0,split);
        falsestr=pattern.substr(split+1);
    }

    for(uint32_t r=0;r<rows.size();r++) {
        Json::Value &cell=rows[r]["c"][c];
        auto v=cell["v"];
        bool vd;
        if(v.isString()) {
            if(validTrue.count(v.asString())) { vd=1; }
            else if(validFalse.count(v.asString())) { vd=0; }
            else {
                throw GQLError(ErrorReasons::ILLEGAL_FORMATTING_PATTERNS,"cannot convert '"+v.asString()+"' to a boolean");
            }
        } else {
            vd=v.asBool();
        }

        if(no_values) { cell.removeMember("v"); }
        else { cell["v"]=vd; }
        if(no_format) { cell.removeMember("f"); }
        else { cell["f"]=vd?truestr:falsestr; }
    }
}


/// apply a string format to a column
static void applyStringFormat(Json::Value &rows,
                               uint32_t c,
                               bool no_values,bool no_format)
{
    NumberFormat *fmt=0;
    ON_EXIT(if(fmt) { free(fmt); });
    for(uint32_t r=0;r<rows.size();r++) {
        Json::Value &cell=rows[r]["c"][c];
        auto v=cell["v"];
        std::string str;
        if(v.isNumeric()) {
            if(!fmt) {
                UErrorCode status = U_ZERO_ERROR;
                fmt=NumberFormat::createInstance(status);
            }
            str=applyPattern(v.asDouble(),fmt);
            cell["v"]=str;
        } else if(v.isBool()) {
            str=v.asBool()?"TRUE":"FALSE";
            cell["v"]=str;
        } else {
            str=v.asString();
        }
        if(no_values) {
            cell["f"]=cell["v"];
            cell.removeMember("v");
        }
        if(no_format) { cell.removeMember("f"); }
    }
}

/// apply a number format to a column
static void applyNumberFormat(Json::Value &rows,
                               uint32_t c,const std::string &pattern,
                               bool no_values,bool no_format)
{
    NumberFormat *fmt=0;
    ON_EXIT(if(fmt) { free(fmt); });
    UErrorCode status = U_ZERO_ERROR;
    fmt=NumberFormat::createInstance(status);

    if(pattern!=GENERAL) {
        DecimalFormat *d=dynamic_cast<DecimalFormat*>(fmt);
        UnicodeString ustr(pattern.c_str(),static_cast<int>(pattern.size()));
        UParseError upe;
        UErrorCode uce=U_ZERO_ERROR;
        d->applyPattern(ustr,upe,uce);
        LOG_IF(FATAL,!U_SUCCESS(uce)) << u_errorName(uce) << " for \"" << pattern << "\"" << std::endl;
    }

    // now need to apply this to every single entry
    for(uint32_t r=0;r<rows.size();r++) {
        Json::Value &cell=rows[r]["c"][c];
        auto v=cell["v"];
        if(v.isString()) {
            if(!no_values) {
                auto vd=std::stod(v.asString());
                cell["v"]=vd;
            }
        } else if(v.isUInt()||v.isUInt64()) {
            uint64_t vd=v.asLargestUInt();
            if(vd&0x8000000000000000ULL) {
                // uci cannot deal with unsigned 64bit numbers correctly, so
                // if the number is too big use doubles instead
                if(pattern==GENERAL) {
                    cell["f"]=std::to_string(vd);
                } else {
                    cell["f"]=applyPattern(static_cast<double>(vd),fmt);
                }
            } else {
                cell["f"]=applyPattern(static_cast<int64_t>(vd),fmt);
            }
        } else if(v.isIntegral()) {
            int64_t vd=v.asLargestInt();
            cell["f"]=applyPattern(vd,fmt);
        } else {
            auto vd=v.asDouble();
            if(pattern==GENERAL) {
                char buf[30];
                sprintf(buf,"%g",vd);
                cell["f"]=buf;
            } else {
                cell["f"]=applyPattern(vd,fmt);
            }
        }
        if(no_values) { cell.removeMember("v"); }
        if(no_format) { cell.removeMember("f"); }
    }
}

/// apply a date/time format to a column
static void applyDateTimeFormat(const std::string &type,
                                const std::string &format,
                                Json::Value &rows,
                                uint32_t c,std::string pattern,
                                bool no_values,bool no_format)
{
    DateFormat *dfmt=DateFormat::createInstance();
    ON_EXIT(if(dfmt) { free(dfmt); });
    UErrorCode status = U_ZERO_ERROR;
    icu::GregorianCalendar cal(status);

    auto upattern=icu::UnicodeString::fromUTF8("yyyy-MM-dd HH:mm:ss");
    UErrorCode dateuce=U_ZERO_ERROR;
    SimpleDateFormat parseTimestamp(upattern,dateuce);
    upattern=icu::UnicodeString::fromUTF8("yyyy");
    dateuce=U_ZERO_ERROR;
    SimpleDateFormat parseYear(upattern,dateuce);

    if(pattern==GENERAL) {
        if(type==TYPE_DATE) {
            pattern="yyyy/MM/dd";
        } else if(type==TYPE_TIME) {
            pattern="H:mm:ss.SSS";
        } else if(type==TYPE_DATETIME) {
            pattern="yyyy/MM/dd H:mm:ss.SSS";
        }
    }
    if(pattern!=GENERAL) {
        SimpleDateFormat *d=dynamic_cast<SimpleDateFormat*>(dfmt);
        UnicodeString ustr(pattern.c_str(),static_cast<int32_t>(pattern.size()));
        UErrorCode uce=U_ZERO_ERROR;
        d->applyLocalizedPattern(ustr,uce);
        LOG_IF(FATAL,!U_SUCCESS(uce)) << "failed to use date pattern '" << pattern << "'" << std::endl;
    }


    // now need to apply this format to every single entry
    for(uint32_t r=0;r<rows.size();r++) {
        Json::Value &cell=rows[r]["c"][c];
        const Json::Value &v=cell["v"];
        double vd;

        // First convert the cell to a double in the format that icu expects
        if(format=="string") {
            UErrorCode udres=U_ZERO_ERROR;
            UnicodeString uv(v.asString().c_str(),static_cast<int32_t>(v.asString().size()));
            if(v.asString()=="0000-00-00 00:00:00" || v.asString()=="0000") {
                vd=MIN_DATE*1000.0;
            } else if(v.asString().size()==4) {
                vd=parseYear.parse(uv,udres);
            } else {
                vd=parseTimestamp.parse(uv,udres);
            }
        } else if(v.isString()) {
            cell["v"]=v;
            vd=std::stod(v.asString());
        } else if(v.isIntegral()&&!v.isDouble()) {
            vd=v.asInt()*1000.0;
        } else {
            vd=v.asDouble()*1000.0;
        }
        cell["f"]=applyPattern(vd,dfmt);
        if(no_values) {
            cell.removeMember("v");
        } else {
            struct tm tm;
            time_t ts(static_cast<time_t>(floor(vd/1000.0)));
            localtime_r(&ts,&tm);
            char buf[70]; // big enough for largest Date
            // GQL has some strict requirements how dates and times are returned
            if(type==TYPE_DATE) {
                cal.setTime(vd,status);
                sprintf(buf,"Date(%d,%d,%d)",
                        cal.get(UCAL_YEAR,status),
                        cal.get(UCAL_MONTH,status),
                        cal.get(UCAL_DAY_OF_MONTH,status));
                cell["v"]=buf;
            } else if(type==TYPE_DATETIME) {
                cal.setTime(vd,status);
                sprintf(buf,"Date(%d,%d,%d,%d,%d,%d,%ld)",
                        cal.get(UCAL_YEAR,status),
                        cal.get(UCAL_MONTH,status),
                        cal.get(UCAL_DAY_OF_MONTH,status),
                        cal.get(UCAL_HOUR_OF_DAY,status),
                        cal.get(UCAL_MINUTE,status),
                        cal.get(UCAL_SECOND,status),
                        static_cast<long int>(vd)-1000*ts);
                cell["v"]=buf;
            } else {
                cell["v"]=Json::Value();
                Json::Value &ar=cell["v"];
                ar[0]=tm.tm_hour;
                ar[1]=tm.tm_min;
                ar[2]=tm.tm_sec;
                ar[3]=static_cast<unsigned long long>(vd)%1000;
            }
        }
        if(no_format) { cell.removeMember("f"); }
    }
}


/** 
 * Apply the correct format to all json values.
 * This is somewhat complicated as the DB may not necessarily return a value in the
 * correct format for ICU conversion, so intermediate conversions may be needed.
 */
static void applyFormat(Json::Value &tbl,bool no_values,bool no_format)
{
    Json::Value &cols=tbl["cols"];
    Json::Value &rows=tbl["rows"];

    for(uint32_t c=0;c<cols.size();c++) {
        std::string pattern=cols[c]["pattern"].asString();
        std::string type=cols[c]["type"].asString();
        std::string format=cols[c]["format"].asString();


        if(type==TYPE_NUMBER) {
            applyNumberFormat(rows,c,pattern,no_values,no_format);
        } else if(type==TYPE_BOOLEAN) {
            applyBooleanFormat(rows,c,pattern,no_values,no_format);
        } else if(type==TYPE_STRING) {
            applyStringFormat(rows,c,no_values,no_format);
        } else if(type==TYPE_DATE||type==TYPE_DATETIME||type==TYPE_TIME) {
            applyDateTimeFormat(type,format,rows,c,pattern,no_values,no_format);
        } else {
            throw GQLError(ErrorReasons::INVALID_QUERY,"unknown column type '"+type+"'");
        }
    }
}

/// Execute a query and return the GQL expected json.
void DB::execute(const std::string &gql,Json::Value &res) const
{
    res["version"]="0.7";
    if(!isConnected()) {
        setError(res,ErrorReasons::ACCESS_DENIED,"db connection failed");
    } else {
        try {
            if(parser_->parse(gql)) {
                Result r=parser_->res();
                LOG(INFO) << "Result: " << r;

                res["table"]=Json::Value();
                res["status"]="ok";
                if(parser_->query()->hasPivotClause()) {
                    Json::Value tbl=Json::Value();
                        // in order to avoid copying as much as possible
                        // create the original in a separate table and then
                        // pivot into the actual result
                    getdata(r.result,tbl);
                    setLabelFormat(tbl["cols"],parser_->query());
                    applyFormat(tbl,parser_->query()->no_values,0);
                       // 0: keep format, needed for pivot
                    pivotTable(tbl,res["table"]);
                } else {
                    getdata(r.result,res["table"]);
                    setLabelFormat(res["table"]["cols"],parser_->query());
                    applyFormat(res["table"],parser_->query()->no_values, parser_->query()->no_format);
                }
            } else {
                Result r=parser_->res();
                setError(res,ErrorReasons::INVALID_QUERY,r.errormsg);
            }
        } catch(const GQLError &er) {
            setError(res,er.er(),er.msg());
        } catch(const std::exception &ex) {
            setError(res,ErrorReasons::INVALID_REQUEST,ex.what());
        }
    }
}

DB::~DB() { }


}

const char* GQLError::what() const noexcept
{
    if(what_=="") {
        what_=to_string(er_)+": "+msg_;
    }
    return what_.c_str();
}

}
