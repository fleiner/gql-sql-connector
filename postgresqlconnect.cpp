/** \file
 * \brief PostgreSQL connector for GQL
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

/// Strings used to exract the given part of a date or time in postgresql
const static std::set<std::string> extracts { "year","month","day","hour","minute" };


/// Quote an identifier for PostgreSQL
static std::string quoteIdentPostgreSQL(const std::string &s)
{
    return "\""+s+"\"";
}

/// Quote an string for PostgreSQL
static std::string quoteString(const std::string &s)
{
    char quote='"';

    if(s.find(quote)) { quote='\''; }
    return quote+s+quote;
}

/// Convert an expression to a valid PostgreSQL expression.
static std::string postgresqlExpr(GQL_SQL::GQLParser::Query::Expr::CPtr qe,std::set<std::string>&tables)
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

    case GQL_SQL::GQLParser::TokenType::GQL_FALSE:
        r="FALSE";
        break;
    case GQL_SQL::GQLParser::TokenType::GQL_TRUE:
        r="TRUE";
        break;

    case GQL_SQL::GQLParser::TokenType::IS_NULL:
        r="("+postgresqlExpr(qe->sub()[0],tables)+" IS NULL)";
        break;

    case GQL_SQL::GQLParser::TokenType::IS_NOT_NULL:
        r="("+postgresqlExpr(qe->sub()[0],tables)+" IS NOT NULL)";
        break;


    case GQL_SQL::GQLParser::TokenType::PLUS:
    case GQL_SQL::GQLParser::TokenType::MINUS:
        if(qe->sub().size()==1) {
            if(qe->tp()==GQL_SQL::GQLParser::TokenType::PLUS) {
                r+=postgresqlExpr(qe->sub()[0],tables);
            } else {
                r+="(-"+postgresqlExpr(qe->sub()[0],tables)+")";
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
        r+="("+postgresqlExpr(qe->sub()[0],tables)+qe->data()+postgresqlExpr(qe->sub()[1],tables)+")";
        break;

    case GQL_SQL::GQLParser::TokenType::DATE:
        r+="date "+qe->sub()[0]->to_string();
        break;

    case GQL_SQL::GQLParser::TokenType::DATETIME:
        r+="timestamp "+qe->sub()[0]->to_string();
        break;

    case GQL_SQL::GQLParser::TokenType::TIMEOFDAY:
        r+="time "+qe->sub()[0]->to_string();
        break;

    case GQL_SQL::GQLParser::TokenType::LIKE:
    case GQL_SQL::GQLParser::TokenType::AND:
    case GQL_SQL::GQLParser::TokenType::OR:
        assert(qe->sub().size()==2);
        r+="("+postgresqlExpr(qe->sub()[0],tables)+" "+qe->data()+" "+postgresqlExpr(qe->sub()[1],tables)+")";
        break;
    case GQL_SQL::GQLParser::TokenType::NOT:
        assert(qe->sub().size()==1);
        r+="(not "+postgresqlExpr(qe->sub()[0],tables)+")";
        break;
    case GQL_SQL::GQLParser::TokenType::NUMBER:
        r=qe->data();
        break;
    case GQL_SQL::GQLParser::TokenType::STRING:
        r=quoteString(qe->data());
        break;
        
    case GQL_SQL::GQLParser::TokenType::STARTS:
        r="(LEFT(";
        r+=postgresqlExpr(qe->sub()[0],tables);
        r+=",LENGTH(";
        r+=postgresqlExpr(qe->sub()[1],tables);
        r+="))=";
        r+=postgresqlExpr(qe->sub()[1],tables);
        r+=")";
        break;

    case GQL_SQL::GQLParser::TokenType::MATCHES:
        r="(";
        r+=postgresqlExpr(qe->sub()[0],tables);
        r+=" ~ CONCAT('^',";
        r+=postgresqlExpr(qe->sub()[1],tables);
        r+=",'$'))";
        break;

    case GQL_SQL::GQLParser::TokenType::ENDS:
        r="(RIGHT(";
        r+=postgresqlExpr(qe->sub()[0],tables);
        r+=",LENGTH(";
        r+=postgresqlExpr(qe->sub()[1],tables);
        r+="))=";
        r+=postgresqlExpr(qe->sub()[1],tables);
        r+=")";
        break;

    case GQL_SQL::GQLParser::TokenType::CONTAINS:
        r="(POSITION(";
        r+=postgresqlExpr(qe->sub()[1],tables);
        r+=" IN ";
        r+=postgresqlExpr(qe->sub()[0],tables);
        r+=")>0)";
        break;

    case GQL_SQL::GQLParser::TokenType::IDENTIFIER:
        if(qe->sub().size()==0 && !qe->noarg()) {
            auto dot=qe->data().find(".");
            if(dot!=std::string::npos && qe->data().rfind(".")==dot) {
                auto tbl=qe->data().substr(0,dot);
                tables.insert(tbl);
                r=quoteIdentPostgreSQL(tbl);
                r+=".";
                r+=quoteIdentPostgreSQL(qe->data().substr(dot+1));
            } else {
                r=quoteIdentPostgreSQL(qe->data());
                tables.insert("");
            }
        } else {
            std::string suffix="";
            std::string sep=", ";
            if(qe->data()=="starts") {
                r="(INSTR";
                suffix="=1)";
            } else if(qe->data()=="contains") {
                r="(INSTR";
                suffix=">0)";
            } else if(extracts.count(qe->data())) {
                r="(EXTRACT("+qe->data()+" FROM";
                suffix="))";
            } else if(qe->data()=="second") {
                r="floor(EXTRACT(SECONDS FROM";
                suffix="))";
            } else if(qe->data()=="millisecond") {
                r="(EXTRACT(MILLISECONDS FROM";
                suffix=")::int % 1000)";
            } else if(qe->data()=="dateDiff") {
                r="(EXTRACT(DAY FROM ";
                sep="-";
                suffix="))";
            } else if(qe->data()=="toDate") {
                r="date";
            } else {
                r=qe->data();
            }
            r+="(";
            int first=1;
            for(auto e:qe->sub()) {
                if(!first) { r+=sep; }
                first=0;
                r+=postgresqlExpr(e,tables);
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

/// Create the PostgreSQL query string that can then be sent to a MySQL DB
void GQL_SQL::GQLParser::ParserPostgreSQL::createResult()
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
            r+=" "+quoteIdentPostgreSQL(i.token);
        }
        if(query_->pivot.size()) {
            for(auto i:query_->group) {
                r+=", "+quoteIdentPostgreSQL(i.token);
            }
        }
        
        for(unsigned int i=0;i<query_->select.size();i++) {
            if(r!="select ") { r+=", "; }
            auto s=query_->select[i];
            r+=postgresqlExpr(s.expr,tables);
        }
    }
    std::string qstring="";
    if(query_->where) {
        qstring=" where ";
        qstring+=postgresqlExpr(query_->where,tables);
    }
    r+=" from ";
    bool addedTable=false;
    if(tables.size()==0||tables.count("")>0||tables.count(defTable_)>0) {
        r+="\""+defTable_+"\"";
        tables.erase("");
        tables.erase(defTable_);
        addedTable=true;
    }

    for(auto t:tables) {
        if(allowedTables_.count(t)==0) {
            throw GQLError(ErrorReasons::ACCESS_DENIED,"table \""+t+"\" does not exists or is not accessible");
        }
        if(addedTable) { r+=", "; }
        addedTable=true;
        r+="\""+t+"\"";
    }
    r+=qstring;


    // For a manual pivot we need to group by the pivot columns
    bool first=1;
    for(auto i:query_->pivot) {
        if(!first) { r+=","; }
        else { r+=" GROUP BY ";first=false; }
        first=false;
        r+=" "+quoteIdentPostgreSQL(i.token);
    }

    for(auto i:query_->group) {
        if(!first) { r+=","; }
        else { r+=" GROUP BY ";first=false; }
        first=false;
        r+=" "+quoteIdentPostgreSQL(i.token);
    }

    first=1;
    for(const auto &o:query_->order) {
        if(!first) { r+=", "; }
        else { r+=" ORDER BY "; }
        first=false;
        r+=postgresqlExpr(o.expr,tables);
        if(o.desc) { r+=" DESC"; }
    }
    if(query_->limit) {
        r+=" LIMIT "+std::to_string(query_->limit);
    }
    if(query_->offset) {
        r+=" OFFSET "+std::to_string(query_->offset);
    }
    res_.result=r;
}

/// Connect to a PostgreSQL database
void GQL_SQL::DBQuery::PostgreSQL::connect() 
{
    if(port_==0) { port_=5432; }
    try {
        std::string options="host="+server_;
        options+=" port="+std::to_string(port_);
        options+=" dbname="+db_;
        options+=" user="+user_;
        options+=" password="+password_;
        connection_=new pqxx::connection(options);
        parser_=std::shared_ptr<GQLParser::Parser>(new GQLParser::ParserPostgreSQL(deftable_,tables_,extendedFunctions_));
        
        pqxx::work txn{*connection_};
        pqxx::result r=txn.exec("select column_name from INFORMATION_SCHEMA.COLUMNS where table_name = '"+deftable_+"'");
        defTableColumns_.clear();
        for (auto row: r) {
            defTableColumns_.insert(row[0].as<std::string>());
        }
        txn.commit();
    } catch(const pqxx::broken_connection &) {
        // ignore for now, isConnected() returns false and will be dealt
        // with during execute() to set a reasonable error.
    }
}

GQL_SQL::DBQuery::PostgreSQL::~PostgreSQL() {
    if(connection_) {
	delete connection_;
    }
}

/// Various types returned by the PostgreSQL C++ interface
enum class PG_TYPES {
    STRING=0,
    INTEGER=1,
    BOOL=2,
    FLOAT=3,
    DATE=4, // YYYY-MM-DD
    TIME=5, // YYYY-MM-DD
    DATETIME=6, // YYYY-MM-DD
};

/// Map PostgreSQL type numbers to types used in this code
const static std::map<uint32_t,PG_TYPES> pgtypes {
#include "postgresql.pg_type"
};

/// Return the data from the query in json format
void GQL_SQL::DBQuery::PostgreSQL::getdata(const std::string &q,Json::Value &tbl) const
{
    LOG(INFO) << "search: " << q;
    pqxx::work txn{*connection_};
    pqxx::result rows=txn.exec(q);
    tbl["cols"]=Json::Value();
    Json::Value &cols=tbl["cols"];
    cols.resize(rows.columns());
    std::vector<PG_TYPES> convs;
    convs.resize(rows.columns());

    for (uint32_t i = 0; i < rows.columns(); i++) {
        cols[i]["id"]=rows.column_name(i);
        convs[i]=pgtypes.find(rows.column_type(i))->second;
        switch(convs[i]) {
        case PG_TYPES::STRING:
            cols[i]["type"]=TYPE_STRING;
            break;
        case PG_TYPES::INTEGER:
            cols[i]["type"]=TYPE_NUMBER;
            break;
        case PG_TYPES::BOOL:
            cols[i]["type"]=TYPE_BOOLEAN;
            break;
        case PG_TYPES::FLOAT:
            cols[i]["type"]=TYPE_NUMBER;
            break;
        case PG_TYPES::DATE:
            cols[i]["type"]=TYPE_DATE;
            break;
        case PG_TYPES::TIME:
            cols[i]["type"]=TYPE_TIME;
            break;
        case PG_TYPES::DATETIME:
            cols[i]["type"]=TYPE_DATETIME;
            break;
        }
        VLOG(1)<< "type=" << rows.column_type(i) 
                    << "  id=" << rows.column_name(i);
    }

    icu::UnicodeString upattern=icu::UnicodeString::fromUTF8("yyyy-MM-dd HH:mm:ss.SSS");
    icu::UnicodeString upatterntz=icu::UnicodeString::fromUTF8("yyyy-MM-dd HH:mm:ssX");
    UErrorCode dateuce=U_ZERO_ERROR;
    SimpleDateFormat datetimefmt(upattern, dateuce);
    SimpleDateFormat datetimefmttz(upatterntz, dateuce);
    upattern=icu::UnicodeString::fromUTF8("yyyy-MM-dd");
    SimpleDateFormat datefmt(upattern, dateuce);
    tbl["rows"]=Json::Value();
    Json::Value &res=tbl["rows"];
    res.resize(static_cast<uint32_t>(rows.size()));
    int rcnt=0;
    for(auto r:rows) {
        res[rcnt]=Json::Value();
        res[rcnt]["c"]=Json::Value();
        Json::Value &v=res[rcnt]["c"];
        v.resize(r.size());
        uint32_t ccnt=0;
        for(auto c:r) {
            v[ccnt]=Json::Value();
            if(c.is_null()) {
                v[ccnt]["v"]=Json::Value::null;
                switch(convs[ccnt]) {
                case PG_TYPES::DATETIME:
                case PG_TYPES::DATE:
                    v[ccnt]["v"]=MIN_DATE;
                    break;
                case PG_TYPES::TIME:
                case PG_TYPES::INTEGER:
                case PG_TYPES::BOOL:
                    v[ccnt]["v"]=0;
                    break;
                case PG_TYPES::FLOAT:
                    v[ccnt]["v"]=0.0;
                    break;
                case PG_TYPES::STRING:
                    break;
                }

            } else {
                switch(convs[ccnt]) {
                case PG_TYPES::STRING:
                    v[ccnt]["v"]=c.as<std::string>();
                    break;
                case PG_TYPES::BOOL:
                    v[ccnt]["v"]=Json::Value(c.as<bool>());
                    break;
                case PG_TYPES::INTEGER:
                    v[ccnt]["v"]=Json::Value(static_cast<Json::Int64>(c.as<int64_t>()));
                    break;
                case PG_TYPES::FLOAT:
                    v[ccnt]["v"]=(c.as<double>());
                    break;
                case PG_TYPES::TIME:
                    {
                        struct tm t;
                        memset(&t,0,sizeof(t));
                        int hour=0,min=0,sec=0,milli=0;
                        sscanf(c.c_str(),"%d:%d:%d.%d",&hour,&min,&sec,&milli);
                        v[ccnt]["v"]=(hour*3600+min*60+sec)+milli/1000.0;
                    }
                    break;
                case PG_TYPES::DATE:
                    {
                        struct tm t;
                        memset(&t,0,sizeof(t));
                        ParsePosition pp(0);
                        UErrorCode udres=U_ZERO_ERROR;
                        auto ud=datefmt.parse(icu::UnicodeString::fromUTF8(c.as<std::string>()),udres);
                        if(c.as<std::string>()=="0000-00-00") {
                            v[ccnt]["v"]=MIN_DATE;
                        } else if(U_SUCCESS(udres)) {
                            v[ccnt]["v"]=ud/1000.0;
                        } else {
                            v[ccnt]["v"]=0; // FIXME? throw error instead?
                        }
                    }
                    break;
                case PG_TYPES::DATETIME:
                    {
                    struct tm t;
                    memset(&t,0,sizeof(t));
                    ParsePosition pp(0);
                    UErrorCode udres=U_ZERO_ERROR;
                    std::string cs=c.as<std::string>();
                    auto ucs=icu::UnicodeString::fromUTF8(cs);

                    auto ud=datetimefmt.parse(ucs,udres);
                    if(!U_SUCCESS(udres) && (cs[cs.length()-3]=='+'||cs[cs.length()-3]=='-')) {
                        udres=U_ZERO_ERROR;
                        ud=datetimefmttz.parse(ucs,udres);
                    }
                    if(c.as<std::string>()=="0000-00-00 00:00:00.000") {
                        v[ccnt]["v"]=MIN_DATE;
                    } else if(U_SUCCESS(udres)) {
                        v[ccnt]["v"]=ud/1000.0;
                    } else {
                        v[ccnt]["v"]=0; // FIXME? throw error instead?
                    }
                    }
                    break;
                }
            }
            ++ccnt;
        }
        ++rcnt;
    }

}

/// We are the PostgreSQL connector
std::string GQL_SQL::GQLParser::ParserPostgreSQL::target() const { return "PostgreSQL"; }

GQL_SQL::GQLParser::ParserPostgreSQL::~ParserPostgreSQL() { }
