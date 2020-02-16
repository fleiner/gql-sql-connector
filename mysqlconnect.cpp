/** \file
 * \brief MySQL connector for GQL
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
#include <cstdint>
#include <unordered_map>
#include <exception>
#include <jsoncpp/json/json.h>
#include <glog/logging.h>
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

/// Quote an identifier for MySQL
static std::string quoteIdentMYSQL(const std::string &s)
{
    return "`"+s+"`";
}

/// Quote an string for MySQL
static std::string quoteString(const std::string &s)
{
    char quote='"';

    if(s.find(quote)) { quote='\''; }
    return quote+s+quote;
}

/// Convert an expression to a valid MySQL expression.
static std::string mysqlExpr(GQL_SQL::GQLParser::Query::Expr::CPtr qe,std::set<std::string>&tables)
{
    std::string r;
    switch(qe->tp()) {
    case GQL_SQL::GQLParser::TokenType::ASC:
    case GQL_SQL::GQLParser::TokenType::BY:
    case GQL_SQL::GQLParser::TokenType::COMMA:
    case GQL_SQL::GQLParser::TokenType::DESC:
    case GQL_SQL::GQLParser::TokenType::EOL:
    case GQL_SQL::GQLParser::TokenType::ERROR:
    case GQL_SQL::GQLParser::TokenType::FORMAT:
    case GQL_SQL::GQLParser::TokenType::GROUP:
    case GQL_SQL::GQLParser::TokenType::LABEL:
    case GQL_SQL::GQLParser::TokenType::LIMIT:
    case GQL_SQL::GQLParser::TokenType::OFFSET:
    case GQL_SQL::GQLParser::TokenType::OPTIONS:
    case GQL_SQL::GQLParser::TokenType::ORDER:
    case GQL_SQL::GQLParser::TokenType::PIVOT:
    case GQL_SQL::GQLParser::TokenType::P_CLOSE:
    case GQL_SQL::GQLParser::TokenType::P_OPEN:
    case GQL_SQL::GQLParser::TokenType::SELECT:
    case GQL_SQL::GQLParser::TokenType::TIMESTAMP:
    case GQL_SQL::GQLParser::TokenType::WHERE:
    case GQL_SQL::GQLParser::TokenType::_UNDEFINED:
        LOG(FATAL) << "got unusable token type " << qe->tp();

    case GQL_SQL::GQLParser::TokenType::IS_NULL:
        r="("+mysqlExpr(qe->sub()[0],tables)+" IS NULL)";
        break;

    case GQL_SQL::GQLParser::TokenType::IS_NOT_NULL:
        r="("+mysqlExpr(qe->sub()[0],tables)+" IS NOT NULL)";
        break;

    case GQL_SQL::GQLParser::TokenType::GQL_FALSE:
        r="FALSE";
        break;
    case GQL_SQL::GQLParser::TokenType::GQL_TRUE:
        r="TRUE";
        break;

    case GQL_SQL::GQLParser::TokenType::PLUS:
    case GQL_SQL::GQLParser::TokenType::MINUS:
        if(qe->sub().size()==1) {
            if(qe->tp()==GQL_SQL::GQLParser::TokenType::PLUS) {
                r+=mysqlExpr(qe->sub()[0],tables);
            } else {
                r+="(-"+mysqlExpr(qe->sub()[0],tables)+")";
            }
            break;
        }
        FALLTHROUGH
    case GQL_SQL::GQLParser::TokenType::TIMES:
    case GQL_SQL::GQLParser::TokenType::DIV:
    case GQL_SQL::GQLParser::TokenType::LT:
    case GQL_SQL::GQLParser::TokenType::LE:
    case GQL_SQL::GQLParser::TokenType::GT:
    case GQL_SQL::GQLParser::TokenType::GE:
    case GQL_SQL::GQLParser::TokenType::EQ:
    case GQL_SQL::GQLParser::TokenType::NE:
        assert(qe->sub().size()==2);
        r+="("+mysqlExpr(qe->sub()[0],tables)+qe->data()+mysqlExpr(qe->sub()[1],tables)+")";
        break;

    case GQL_SQL::GQLParser::TokenType::DATE:
        r+="date "+qe->sub()[0]->to_string();
        break;

    case GQL_SQL::GQLParser::TokenType::DATETIME:
        r+="CONVERT("+qe->sub()[0]->to_string()+",datetime(3))";
        break;

    case GQL_SQL::GQLParser::TokenType::TIMEOFDAY:
        r+="time "+qe->sub()[0]->to_string();
        break;

    case GQL_SQL::GQLParser::TokenType::LIKE:
    case GQL_SQL::GQLParser::TokenType::AND:
    case GQL_SQL::GQLParser::TokenType::OR:
        assert(qe->sub().size()==2);
        r+="("+mysqlExpr(qe->sub()[0],tables)+" "+qe->data()+" "+mysqlExpr(qe->sub()[1],tables)+")";
        break;
    case GQL_SQL::GQLParser::TokenType::NOT:
        assert(qe->sub().size()==1);
        r+="(not "+mysqlExpr(qe->sub()[0],tables)+")";
        break;
    case GQL_SQL::GQLParser::TokenType::NUMBER:
        r=qe->data();
        break;
    case GQL_SQL::GQLParser::TokenType::STRING:
        r=quoteString(qe->data());
        break;
        
    case GQL_SQL::GQLParser::TokenType::MATCHES:
        r="(";
        r+=mysqlExpr(qe->sub()[0],tables);
        r+=" REGEXP CONCAT('^',";
        r+=mysqlExpr(qe->sub()[1],tables);
        r+=",'$'))";
        break;

    case GQL_SQL::GQLParser::TokenType::ENDS:
        r="(RIGHT(";
        r+=mysqlExpr(qe->sub()[0],tables);
        r+=",LENGTH(";
        r+=mysqlExpr(qe->sub()[1],tables);
        r+="))=";
        r+=mysqlExpr(qe->sub()[1],tables);
        r+=")";
        break;

    case GQL_SQL::GQLParser::TokenType::CONTAINS:
    case GQL_SQL::GQLParser::TokenType::STARTS:
    case GQL_SQL::GQLParser::TokenType::IDENTIFIER:
        if(qe->sub().size()==0 && !qe->noarg()) {
            auto dot=qe->data().find(".");
            if(dot!=std::string::npos && qe->data().rfind(".")==dot) {
                auto tbl=qe->data().substr(0,dot);
                tables.insert(tbl);
                r=quoteIdentMYSQL(tbl);
                r+=".";
                r+=quoteIdentMYSQL(qe->data().substr(dot+1));
            } else {
                r=quoteIdentMYSQL(qe->data());
                tables.insert("");
            }
        } else {
            std::string suffix="";
            if(qe->data()=="starts") {
                r="(INSTR";
                suffix="=1)";
            } else if(qe->data()=="contains") {
                r="(INSTR";
                suffix=">0)";
            } else if(qe->data()=="millisecond") {
                r="(microsecond";
                suffix="/1000)";
            } else if(qe->data()=="toDate") {
                r="date";
            } else {
                r=qe->data();
            }
            r+="(";
            int first=1;
            for(auto e:qe->sub()) {
                if(!first) { r+=", "; }
                first=0;
                r+=mysqlExpr(e,tables);
            }
            if(r[r.size()-1]==' ') {
                r=r.substr(0,r.length()-1);
            }
            r+=")";
            r+=suffix;
        }
    }
    return r;
}

/// Create the MySQL query string that can then be sent to a MySQL DB
void GQL_SQL::GQLParser::ParserMySQL::createResult()
{
    if(!query_) { return; }
    std::string r="";
    std::set<std::string> tables;
    if(query_->selectStar) {
        r="select * ";
    } else if(query_->select.size()>0) {
        r="select ";
        for(auto i:query_->pivot) {
            if(r!="select ") { r+=", "; }
            r+=" "+quoteIdentMYSQL(i.token);
        }
        if(query_->pivot.size()) {
            for(auto i:query_->group) {
                r+=", "+quoteIdentMYSQL(i.token);
            }
        }
        
        for(unsigned int i=0;i<query_->select.size();i++) {
            if(r!="select ") { r+=", "; }
            auto s=query_->select[i];
            r+=mysqlExpr(s.expr,tables);
        }
    }
    std::string qstring="";
    if(query_->where) {
        qstring=" where ";
        qstring+=mysqlExpr(query_->where,tables);
    }
    r+=" from ";
    bool addedTable=false;
    if(tables.size()==0||tables.count("")>0||tables.count(defTable_)>0) {
        r+="`"+defTable_+"`";
        tables.erase("");
        tables.erase(defTable_);
        addedTable=true;
    }
    for(auto t:tables) {
        if(allowedTables_.count(t)==0) {
            throw GQLError(ErrorReasons::ACCESS_DENIED,"table `"+t+"` does not exists or is not accessible");
        }
        if(addedTable) { r+=", "; }
        addedTable=true;
        r+="`"+t+"`";
    }

    r+=qstring;

    // For a manual pivot we need to group by the pivot columns
    bool first=1;
    for(auto i:query_->pivot) {
        if(!first) { r+=","; }
        else { r+=" GROUP BY ";first=false; }
        first=false;
        r+=" "+quoteIdentMYSQL(i.token);
    }

    for(auto i:query_->group) {
        if(!first) { r+=","; }
        else { r+=" GROUP BY ";first=false; }
        first=false;
        r+=" "+quoteIdentMYSQL(i.token);
    }

    first=1;
    for(const auto &o:query_->order) {
        if(!first) { r+=", "; }
        else { r+=" ORDER BY "; }
        first=false;
        r+=mysqlExpr(o.expr,tables);
        if(o.desc) { r+=" DESC"; }
    }
    if(query_->offset) {
        if(query_->limit) {
            r+=" LIMIT "+std::to_string(query_->offset)+", "+std::to_string(query_->limit);
        } else {
            r+=" LIMIT "+std::to_string(query_->offset)+", 18446744073709551615";
        }
    } else if(query_->limit) {
        r+=" LIMIT "+std::to_string(query_->limit);
    }
    res_.result=r;
}

/// We are the MySQL connector
std::string GQL_SQL::GQLParser::ParserMySQL::target() const { return "MySQL"; }

GQL_SQL::GQLParser::ParserMySQL::~ParserMySQL() { }

/// Various types returned by the MySQL C++ interface
enum class MYSQL_TPS {
    STRING=0,
    INT=1,
    UINT=2,
    DOUBLE=3,
    DATE=4, // YYYY-MM-DD
    TIME=5, // YYYY-MM-DD
    DATETIME=6, // YYYY-MM-DD
};

/// Return the data from the query in json format
void GQL_SQL::DBQuery::MySQL::getdata(const std::string &q,Json::Value &tbl) const
{
    LOG(INFO) << "search: " << q;
    auto query=connection_->query(q);
    auto rows=query.store();
    tbl["cols"]=Json::Value();
    Json::Value &cols=tbl["cols"];
    uint32_t colcount=static_cast<uint32_t>(rows.field_names()->size());
    cols.resize(colcount);
    std::vector<MYSQL_TPS> convs;
    convs.resize(colcount);

    for (int32_t i = 0; i < static_cast<int32_t>(colcount); i++) {
        cols[i]["id"]=rows.field_name(i);
        std::string ctype=rows.field_type(i).name();
        std::string stype=rows.field_type(i).sql_name();
        size_t ci=static_cast<size_t>(i);
        if(ctype=="m") { cols[i]["type"]="number";convs[ci]=MYSQL_TPS::UINT; }
        else if(ctype=="s"||ctype=="i"||ctype=="ctype"||ctype=="l"||ctype=="j"||stype.find("TINYINT")!=std::string::npos||stype.find("BIGINT")!=std::string::npos||stype.substr(0,3)=="INT") { cols[i]["type"]="number";convs[ci]=MYSQL_TPS::INT; }
        else if(ctype=="d"||ctype=="f"||stype.find("DOUBLE")!=std::string::npos||stype.find("DECIMAL")!=std::string::npos) { cols[i]["type"]="number";convs[ci]=MYSQL_TPS::DOUBLE; }
        else if(ctype.find("DateTime")!=std::string::npos) { cols[i]["type"]=TYPE_DATETIME;convs[ci]=MYSQL_TPS::DATETIME; }
        else if(ctype.find("Date")!=std::string::npos) { cols[i]["type"]=TYPE_DATE;convs[ci]=MYSQL_TPS::DATE; }
        else if(ctype.find("Time")!=std::string::npos) { cols[i]["type"]=TYPE_TIME;convs[ci]=MYSQL_TPS::TIME; }
        else { cols[i]["type"]="string"; }

#if 0
        LOG(INFO) << "type=" << ctype 
                    << "  sqlname=" << rows.field_type(i).sql_name() 
                    << "  id=" << rows.field_type(i).id() 
                    << "  convs=" << static_cast<int>(convs[ci])
                    << "  " << cols[i]["id"];
#endif
    }

    icu::UnicodeString upattern=icu::UnicodeString::fromUTF8("yyyy-MM-dd HH:mm:ss.SSS");
    UErrorCode dateuce=U_ZERO_ERROR;
    SimpleDateFormat datetimefmt(upattern, dateuce);
    icu::UnicodeString upattern2=icu::UnicodeString::fromUTF8("yyyy-MM-dd HH:mm:ss");
    dateuce=U_ZERO_ERROR;
    SimpleDateFormat datetimefmt2(upattern2, dateuce);
    upattern=icu::UnicodeString::fromUTF8("yyyy-MM-dd");
    SimpleDateFormat datefmt(upattern, dateuce);
    tbl["rows"]=Json::Value();
    Json::Value &res=tbl["rows"];
    res.resize(static_cast<uint32_t>(rows.num_rows()));
    int rcnt=0;
    for(auto r:rows) {
        res[rcnt]=Json::Value();
        res[rcnt]["c"]=Json::Value();
        Json::Value &v=res[rcnt]["c"];
        v.resize(static_cast<uint32_t>(r.size()));
        int ccnt=0;
        for(auto c:r) {
            v[ccnt]=Json::Value();
            if(c.is_null()) {
                v[ccnt]["v"]=Json::Value::null;
            } else {
                switch(convs[static_cast<size_t>(ccnt)]) {
                case MYSQL_TPS::UINT:
                    v[ccnt]["v"]=Json::Value(static_cast<Json::UInt64>(c));
                    break;
                case MYSQL_TPS::INT:
                    v[ccnt]["v"]=Json::Value(static_cast<Json::Int64>(c));
                    break;
                case MYSQL_TPS::DOUBLE:
                    v[ccnt]["v"]=static_cast<double>(c);
                    break;
                case MYSQL_TPS::TIME:
                    {
                        struct tm t;
                        memset(&t,0,sizeof(t));
                        int hour=0,min=0,sec=0,milli=0;
                        sscanf(c.c_str(),"%d:%d:%d.%d",&hour,&min,&sec,&milli);
                        v[ccnt]["v"]=(hour*3600+min*60+sec)+milli/1000.0;
                    }
                    break;
                case MYSQL_TPS::DATE:
                    {
                        struct tm t;
                        memset(&t,0,sizeof(t));
                        ParsePosition pp(0);
                        UErrorCode udres=U_ZERO_ERROR;
                        auto ud=datefmt.parse(c.data(),udres);
                        if(c=="0000-00-00") {
                            v[ccnt]["v"]=MIN_DATE;
                        } else if(U_SUCCESS(udres)) {
                            v[ccnt]["v"]=ud/1000.0;
                        } else {
                            v[ccnt]["v"]=0; // FIXME: throw error instead?
                        }
                    }
                    break;
                case MYSQL_TPS::DATETIME:
                    {
                    struct tm t;
                    memset(&t,0,sizeof(t));
                    ParsePosition pp(0);
                    UErrorCode udres=U_ZERO_ERROR;
                    auto ud=datetimefmt.parse(c.data(),udres);
                    if(!U_SUCCESS(udres)) {
                        udres=U_ZERO_ERROR;
                        ud=datetimefmt2.parse(c.data(),udres);
                    }
                    LOG(INFO) << "c=" << c.data() << "  ud=" << ud << "  " << u_errorName(udres);
                    if(c=="0000-00-00 00:00:00.000") {
                        v[ccnt]["v"]=MIN_DATE;
                    } else if(U_SUCCESS(udres)) {
                        v[ccnt]["v"]=ud/1000.0;
                    } else {
                        v[ccnt]["v"]=0; // FIXME? throw error instead?
                    }
                    }
                    break;
                case MYSQL_TPS::STRING:
                    v[ccnt]["v"]=(c.data());
                }
            }
            ++ccnt;
        }
        ++rcnt;
    }
}

/// Connect to a MySQL database
void GQL_SQL::DBQuery::MySQL::connect() 
{
    if(port_==0) { port_=3306; }
    try {
        mysqlpp::SetCharsetNameOption* scno = new mysqlpp::SetCharsetNameOption("utf8");
        //connection_=new mysqlpp::Connection(db_.c_str()+1,server_.c_str(),user_.c_str(),password_.c_str(),port_);
        connection_=new mysqlpp::Connection();
        connection_->set_option(scno);
        connection_->connect(db_.c_str(),server_.c_str(),user_.c_str(),password_.c_str(),port_);
        parser_=std::shared_ptr<GQLParser::Parser>(new GQLParser::ParserMySQL(deftable_,tables_,extendedFunctions_));
        
        connection_->query("SET NAMES 'utf8';");
        auto query=connection_->query("DESCRIBE `"+deftable_+"`");
        defTableColumns_.clear();

        auto rows=query.store();
        for(const auto &r:rows) {
            defTableColumns_.insert(r[0].data());
        }
    } catch(const mysqlpp::ConnectionFailed &) {
        // ignore for now, isConnected() returns false and will be dealt
        // with during execute() to set a reasonable error.
    }
}

/// Return true if connected to a DB
bool GQL_SQL::DBQuery::MySQL::isConnected() const 
{
    return connection_&&connection_->connected();
}

GQL_SQL::DBQuery::MySQL::~MySQL() {
    if(connection_) {
        if(connection_->connected()) { connection_->disconnect(); }
	delete connection_;
    }
}
