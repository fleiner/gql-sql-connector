/** \file
 * \brief GQL parser implementation
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

#include "libgqlsql.h"

namespace GQL_SQL {


std::ostream& operator << ( std::ostream& outs, Status s)
{
    switch(s) {
    case Status::OK: outs << "Status::OK";break;
    case Status::SYNTAX_ERROR: outs << "Status::SYNTAX_ERROR";break;
    case Status::UNKNOWN_TABLE: outs << "Status::UNKNOWN_TABLE";break;
    case Status::NOT_SUPPORTED: outs << "Status::NOT_SUPPORTED";break;
    case Status::ERROR: outs << "Status::ERROR";break;
    }
    return outs;
}


std::ostream& operator << ( std::ostream& outs, const Result &res)
{
    if(res.status!=Status::OK) {
        outs << res.status << ", pos " << res.errorpos
             << ": " << res.errormsg << std::endl;
    } else {
        outs << res.target << ": " << res.result << std::endl;
        int cnt=1;
        for(auto c:res.cols) {
            outs << "Col " << (cnt++) << ": " << c.label << " format " << c.format << std::endl;
        }
    }
    return outs;
}

namespace GQLParser {

SyntaxError::~SyntaxError() { }

/// Map strings to token types for keywords.
const static std::unordered_map<std::string,TokenType> keyword {
        { "and",TokenType::AND },
        { "asc",TokenType::ASC },
        { "by",TokenType::BY },
        { "date",TokenType::DATE },
        { "datetime",TokenType::DATETIME },
        { "desc",TokenType::DESC },
        { "false",TokenType::GQL_FALSE },
        { "format",TokenType::FORMAT },
        { "group",TokenType::GROUP },
        { "label",TokenType::LABEL },
        { "limit",TokenType::LIMIT },
        { "not",TokenType::NOT },
        { "offset",TokenType::OFFSET },
        { "options",TokenType::OPTIONS },
        { "or",TokenType::OR },
        { "order",TokenType::ORDER },
        { "pivot",TokenType::PIVOT },
        { "select",TokenType::SELECT },
        { "timeofday",TokenType::TIMEOFDAY },
        { "timestamp",TokenType::TIMESTAMP },
        { "true",TokenType::GQL_TRUE },
        { "where",TokenType::WHERE },

        { "=",TokenType::EQ },
        { "!=",TokenType::NE },
        { "<>",TokenType::NE },
        { "<",TokenType::LT },
        { "<=",TokenType::LE },
        { ">=",TokenType::GE },
        { ">",TokenType::GT },

        { "-",TokenType::MINUS },
        { "+",TokenType::PLUS },
        { "*",TokenType::TIMES },
        { "/",TokenType::DIV },
        { "(",TokenType::P_OPEN },
        { ")",TokenType::P_CLOSE },

        { ",",TokenType::COMMA },
};


Token Tokenizer::next() {
    current_=nextt();
    return current_;
}

Token Tokenizer::nextt() {
    while(pos_<str_.length() && (str_[pos_]==' ' || str_[pos_]=='\t' || str_[pos_]=='\n')) { pos_++; }
    auto startpos=pos_+1;
    auto n=nexti();
    if(n=="") { return Token(startpos,TokenType::EOL); }

    if(n[0]=='"' || n[0]=='\'') {
        if(n[n.length()-1]!=n[0]) {
            return Token(startpos,TokenType::ERROR,n);
        }
        return Token(startpos,TokenType::STRING,n.substr(1,n.length()-2));
    }
    if(n[0]=='`') {
        if(n[n.length()-1]!=n[0]) {
            return Token(startpos,TokenType::ERROR,n);
        }
        return Token(startpos,TokenType::IDENTIFIER,n.substr(1,n.length()-2));
    }

    std::string lower=boost::algorithm::to_lower_copy(n);
    if((n[0]>='0'&&n[0]<='9')||n[0]=='.') {
        return Token(startpos,TokenType::NUMBER,lower);
    }

    if(keyword.count(lower)>0) {
        return Token(startpos,keyword.at(lower),lower);
    }

    for(auto c:n) {
        if(!((c>='0'&&c<='9')||(c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_')) {
            return Token(startpos,TokenType::ERROR,n);
        }
    }
    return Token(startpos,TokenType::IDENTIFIER,n);
}

/** true if the character is valid for an identifier */
inline static bool isid(char c) noexcept {
    return (c>='0'&&c<='9')||(c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_';
}

/** true if the character is a valid number character.
 * Scientific notation is not supported by GQL, so the only valid
 * characers are 0-9 and .
 */
inline static bool isnumber(char c) noexcept {
    return (c>='0'&&c<='9')||c=='.';
}

std::string Tokenizer::nexti() {
    while(pos_<str_.length() && (str_[pos_]==' ' || str_[pos_]=='\t' || str_[pos_]=='\n')) { pos_++; }
    if(pos_>=str_.length()) { return ""; }

    // parse a string literal
    if(str_[pos_]=='\'' || str_[pos_]=='"' || str_[pos_]=='`') {
        auto first=pos_;
        auto start=str_[pos_];
        pos_++;
        while(pos_<str_.length()&&str_[pos_]!=start) {
            pos_++;
        }
        pos_++;
        return str_.substr(first,pos_-first);
    }


    // parse a numeric literal
    if(isnumber(str_[pos_])) {
        auto start=pos_;
        while(isnumber(str_[pos_])) { pos_++; }
        return str_.substr(start,pos_-start);
    }

    // parse a id
    // Note that at this point the current character cannot be a number
    if(isid(str_[pos_])) {
        auto start=pos_;
        while(isid(str_[pos_])) { pos_++; }
        return str_.substr(start,pos_-start);
    }

    pos_++;
    // check for special multi-character key words ('<=' etc)
    if(str_.length()>pos_) {
        auto op2=str_.substr(pos_-1,2);
        if(keyword.count(op2)>0) { pos_++;return op2; }
    }

    return str_.substr(pos_-1,1);
}


std::ostream& operator<<(std::ostream& outs, const Token &t)
{
    outs << "Token(" << t.pos << ',' << t.tt << ',' << t.token << ')';
    return outs;
}

std::ostream& operator<<(std::ostream& outs, TokenType t)
{
    switch(t) {
    case TokenType::_UNDEFINED: outs << "???";break;
    case TokenType::STRING: outs << "STRING";break;
    case TokenType::NUMBER: outs << "NUMBER";break;
    case TokenType::IDENTIFIER: outs << "IDENTIFIER";break;
    case TokenType::EOL: outs << "EOL";break;
    case TokenType::ERROR: outs << "ERROR";break;
    case TokenType::AND: outs << "AND";break;
    case TokenType::ASC: outs << "ASC";break;
    case TokenType::BY: outs << "BY";break;
    case TokenType::DATE: outs << "DATE";break;
    case TokenType::DATETIME: outs << "DATETIME";break;
    case TokenType::DESC: outs << "DESC";break;
    case TokenType::GQL_FALSE: outs << "FALSE";break;
    case TokenType::FORMAT: outs << "FORMAT";break;
    case TokenType::GROUP: outs << "GROUP";break;
    case TokenType::LABEL: outs << "LABEL";break;
    case TokenType::LIMIT: outs << "LIMIT";break;
    case TokenType::NOT: outs << "NOT";break;
    case TokenType::OFFSET: outs << "OFFSET";break;
    case TokenType::OPTIONS: outs << "OPTIONS";break;
    case TokenType::OR: outs << "OR";break;
    case TokenType::ORDER: outs << "ORDER";break;
    case TokenType::PIVOT: outs << "PIVOT";break;
    case TokenType::SELECT: outs << "SELECT";break;
    case TokenType::TIMEOFDAY: outs << "TIMEOFDAY";break;
    case TokenType::TIMESTAMP: outs << "TIMESTAMP";break;
    case TokenType::GQL_TRUE: outs << "TRUE";break;
    case TokenType::WHERE: outs << "WHERE";break;
    case TokenType::EQ: outs << "EQ";break;
    case TokenType::NE: outs << "NE";break;
    case TokenType::LT: outs << "LT";break;
    case TokenType::GT: outs << "GT";break;
    case TokenType::GE: outs << "GE";break;
    case TokenType::LE: outs << "LE";break;
    case TokenType::PLUS: outs << "PLUS";break;
    case TokenType::MINUS: outs << "MINUS";break;
    case TokenType::TIMES: outs << "TIMES";break;
    case TokenType::DIV: outs << "DIV";break;
    case TokenType::P_OPEN: outs << "P_OPEN";break;
    case TokenType::P_CLOSE: outs << "P_CLOSE";break;
    case TokenType::COMMA: outs << "COMMA";break;

    case TokenType::IS_NULL: outs << "IS_NULL";break;
    case TokenType::IS_NOT_NULL: outs << "IS_NOT_NULL";break;
    case TokenType::STARTS: outs << "STARTS";break;
    case TokenType::ENDS: outs << "ENDS";break;
    case TokenType::CONTAINS: outs << "CONTAINS";break;
    case TokenType::MATCHES: outs << "MATCHES";break;
    case TokenType::LIKE: outs << "LIKE";break;

    }
    return outs;
}


bool GQLParser::Parser::parse(const std::string &cmd)
{
    query_=0;
    res_=Result();
    try {
        query_=Query::parse(cmd,extendedFunctions_);
        res_.target=target();
        createResult();
        return 1;
    } catch(const SyntaxError &ex) {
        res_.errormsg=ex.what();
        res_.errorpos=ex.pos();
    } catch(const GQLError &ex) {
        res_.errormsg=ex.what();
        res_.errorpos=0;
    }
    return 0;
}

GQLParser::Parser::~Parser() { }

bool Query::Expr::operator==(const Query::Expr &other) const
{
    bool n=tp_==other.tp_
           && data_==other.data_
           && noarg_==other.noarg_
           && sub_.size()==other.sub_.size();
    if(!n) { return n; }
    for(unsigned int i=0;i<sub_.size();i++) {
        if((*sub_[i])!=(*other.sub_[i])) { return 0; }
    }
    return 1;
}

/// Helper function to throw an error
[[noreturn]] static void throw_error(const Tokenizer &tok,const std::string &instead="")
{
    if(instead=="") {
        std::string res="unexpected token '"+tok.current().token+"'";
        throw SyntaxError(res,tok.current().pos);
    } else {
        std::string res="expected "+instead+" but got '"+tok.current().token+"' instead";
        throw SyntaxError(res,tok.current().pos);
    }
}

/// Consume a well known token, throw error if the current token is of a different type.
static void consume(Tokenizer &tok,TokenType tt) {
    if(tok.current().tt!=tt) { throw_error(tok, "expected "+GQL_SQL::to_string(tt)); }
    tok.next();
}

/// Consume nothing and return false.
static bool consume_opt(Tokenizer &) { return false; }

/// Consume one out of a set of token types
/// \returns false if the current token type does not correspond to any of the arguments
/// could not be found. 
template<typename... Args>
static bool consume_opt(Tokenizer &tok,TokenType tt,Args... args) {
    if(tok.current().tt==tt) {
        tok.next();
        return true;
    }
    return consume_opt(tok,args...);
}


// The following functions implement a simple LL(1) parser using
// functions calls.

static Query::Expr::CPtr parseExpr(Tokenizer &tok);

/// parse a value like literal, function call, expression in parenthesis, ....
static Query::Expr::CPtr parseVal(Tokenizer &tok)
{
    Query::Expr::CPtr r=0;
    TokenType tt=tok.current().tt;
    std::string token=tok.current().token;
    switch(tok.current().tt) {
    case GQL_SQL::GQLParser::TokenType::LIKE:
    case GQL_SQL::GQLParser::TokenType::MATCHES:
    case GQL_SQL::GQLParser::TokenType::STARTS:
    case GQL_SQL::GQLParser::TokenType::ENDS:
    case GQL_SQL::GQLParser::TokenType::CONTAINS:
    case GQL_SQL::GQLParser::TokenType::COMMA:
    case GQL_SQL::GQLParser::TokenType::IS_NULL:
    case GQL_SQL::GQLParser::TokenType::IS_NOT_NULL:
    case GQL_SQL::GQLParser::TokenType::ERROR:
    case GQL_SQL::GQLParser::TokenType::EOL:
    case GQL_SQL::GQLParser::TokenType::ASC:
    case GQL_SQL::GQLParser::TokenType::DESC:
    case GQL_SQL::GQLParser::TokenType::BY:
    case GQL_SQL::GQLParser::TokenType::FORMAT:
    case GQL_SQL::GQLParser::TokenType::GROUP:
    case GQL_SQL::GQLParser::TokenType::ORDER:
    case GQL_SQL::GQLParser::TokenType::LABEL:
    case GQL_SQL::GQLParser::TokenType::LIMIT:
    case GQL_SQL::GQLParser::TokenType::OPTIONS:
    case GQL_SQL::GQLParser::TokenType::OFFSET:
    case GQL_SQL::GQLParser::TokenType::AND:
    case GQL_SQL::GQLParser::TokenType::OR:
    case GQL_SQL::GQLParser::TokenType::PIVOT:
    case GQL_SQL::GQLParser::TokenType::SELECT:
    case GQL_SQL::GQLParser::TokenType::TIMESTAMP:
    case GQL_SQL::GQLParser::TokenType::WHERE:
    case GQL_SQL::GQLParser::TokenType::EQ:
    case GQL_SQL::GQLParser::TokenType::NE:
    case GQL_SQL::GQLParser::TokenType::LT:
    case GQL_SQL::GQLParser::TokenType::GT:
    case GQL_SQL::GQLParser::TokenType::GE:
    case GQL_SQL::GQLParser::TokenType::LE:
    case GQL_SQL::GQLParser::TokenType::TIMES:
    case GQL_SQL::GQLParser::TokenType::P_CLOSE:
    case GQL_SQL::GQLParser::TokenType::DIV:
    case GQL_SQL::GQLParser::TokenType::NOT:
        return 0;

    case GQL_SQL::GQLParser::TokenType::_UNDEFINED:
        LOG(FATAL) << "got unusable token type _UNDEFINED";

    case TokenType::PLUS:
    case TokenType::MINUS:
        {
            tok.next();
            auto e=parseVal(tok);
            if(!e) { throw_error(tok); }
            r=Query::Expr::make(tt,token,e);
        }
        break;

    case TokenType::P_OPEN:
        {
            tok.next();
            auto e=parseExpr(tok);
            if(!e) { throw_error(tok); }
            consume(tok,TokenType::P_CLOSE);
            return e;
        }

    case TokenType::GQL_TRUE:
    case TokenType::GQL_FALSE:
    case TokenType::STRING:
    case TokenType::NUMBER:
        tok.next();
        r=Query::Expr::make(tt,token);
        break;

    case TokenType::DATE:
    case TokenType::TIMEOFDAY:
    case TokenType::DATETIME:
        tok.next();
        r=Query::Expr::make(tt,token,
                            Query::Expr::make(tok.current().tt,tok.current().token));
        consume(tok,TokenType::STRING);
        break;

    case TokenType::IDENTIFIER:
        {
            Token fname=tok.current();
            tok.next();
            auto e=Query::Expr::make(tt,token);
            if(consume_opt(tok,TokenType::P_OPEN)) {
                if(Query::isAggFunc(fname.token)) {
                    e->push(Query::Expr::make(tok.current().tt,tok.current().token));
                    consume(tok,TokenType::IDENTIFIER);
                } else if(tok.current().tt!=TokenType::P_CLOSE) {
                    do {
                        auto e2=parseExpr(tok);
                        if(!e2) { throw_error(tok); }
                        e->push(e2);
                    } while(consume_opt(tok,TokenType::COMMA));
                }
                if(e->sub().size()==0) { e->noargSet(); }
                consume(tok,TokenType::P_CLOSE);
            }
            r=e;
        }
        break;
    }
    return r;
}

/// parse a comparison like '<=' or boolean expression like 'starts with'
static Query::Expr::CPtr parseComp(Tokenizer &tok)
{
    auto r=parseVal(tok);
    if(!r) { return 0; }
    Token cur=tok.current();
    if(cur.tt==TokenType::LE
          ||cur.tt==TokenType::LT
          ||cur.tt==TokenType::GE
          ||cur.tt==TokenType::GT
          ||cur.tt==TokenType::EQ
          ||cur.tt==TokenType::NE) {
        while(consume_opt(tok,TokenType::PLUS,TokenType::LE,TokenType::LT,TokenType::GE,
                          TokenType::GT,TokenType::EQ,TokenType::NE)) {
            auto e=parseVal(tok);
            if(!e) { throw_error(tok); }
            r=Query::Expr::make(cur.tt,cur.token,r,e);
            cur=tok.current();
        }
        return r;
    } else if(cur.isIdent("is")) {
        cur=tok.next();
        if(cur.isIdent("null")) {
            tok.next();
            return Query::Expr::make(TokenType::IS_NULL,"is null",r);
        }
        if(consume_opt(tok,TokenType::NOT)) {
            cur=tok.current();
            if(cur.isIdent("null")) {
                tok.next();
                return Query::Expr::make(TokenType::IS_NOT_NULL,"is not null",r);
            }
        }
        throw_error(tok,"'null' or 'not'");
    } else if(cur.isIdent("contains")
       ||cur.isIdent("matches")
       ||cur.isIdent("like")) {
        TokenType tt=cur.isIdent("contains")?TokenType::CONTAINS:
                     cur.isIdent("matches")?TokenType::MATCHES:
                                            TokenType::LIKE;

        tok.next();
        auto r2=parseComp(tok);
        if(!r2) { throw_error(tok); }
        return Query::Expr::make(tt,cur.token,r,r2);

    } else if(cur.isIdent("starts")
       ||cur.isIdent("ends")) {
        TokenType tt=cur.isIdent("starts")?TokenType::STARTS:TokenType::ENDS;
        auto ptok=cur;
        cur=tok.next();
        if(cur.isIdent("with")) {
            tok.next();
            auto r2=parseComp(tok);
            if(!r2) { throw_error(tok); }
            return Query::Expr::make(tt,ptok.token,r,r2);
        }
        throw_error(tok,"expected 'with'");
    }
    return r;
}

/// parse a 'not X' expression
static Query::Expr::CPtr parseNot(Tokenizer &tok)
{
    TokenType tt=tok.current().tt;
    std::string token=tok.current().token;
    bool cnt=0;
    while(tok.current().tt==TokenType::NOT) {
        consume(tok,TokenType::NOT);
        cnt=1-cnt;
    }
    auto r=parseComp(tok);
    if(cnt) {
        r=Query::Expr::make(tt,token,r);
    }
    return r;
}
 
/// parse the 'AND' and 'OR' expresssion
static Query::Expr::CPtr parseBool(Tokenizer &tok)
{
    auto r=parseNot(tok);
    if(!r) { return 0; }
    Token token=tok.current();
    while(consume_opt(tok,TokenType::AND,TokenType::OR)) {
        auto e=parseNot(tok);
        if(!e) { throw_error(tok); }
        r=Query::Expr::make(token.tt,token.token,r,e);
        token=tok.current();
    }
    return r;
}

/// parse multiplication and division
static Query::Expr::CPtr parseMult(Tokenizer &tok)
{
    auto r=parseBool(tok);
    if(!r) { return 0; }
    Token token=tok.current();
    while(consume_opt(tok,TokenType::TIMES,TokenType::DIV)) {
        auto e=parseBool(tok);
        if(!e) { throw_error(tok); }
        r=Query::Expr::make(token.tt,token.token,r,e);
        token=tok.current();
    }
    return r;
}

/// parse add and subtract
static Query::Expr::CPtr parseAdd(Tokenizer &tok)
{
    auto r=parseMult(tok);
    if(!r) { return 0; }
    Token token=tok.current();
    while(consume_opt(tok,TokenType::PLUS,TokenType::MINUS)) {
        auto e=parseMult(tok);
        if(!e) { throw_error(tok); }
        r=Query::Expr::make(token.tt,token.token,r,e);
        token=tok.current();
    }
    return r;
}

/// parse an expression
static Query::Expr::CPtr parseExpr(Tokenizer &tok)
{
    return parseAdd(tok);
}
/// this is the end of the expression parser

/// parse a set of identifiers
static std::vector<Token>  parseIdents(Tokenizer &tok)
{
    std::vector<Token> res;
    do {
        res.push_back(tok.current());
        consume(tok,TokenType::IDENTIFIER);
    } while(consume_opt(tok,TokenType::COMMA));
    return res;
}


/// parse the 'select' clause, including the special 'select *'
static void parseSelect(Query *q, Tokenizer &tok)
{
    q->selectStar=1;
    if(!consume_opt(tok,TokenType::SELECT)) { return; }
    if(consume_opt(tok,TokenType::TIMES)) { return; }
    q->selectStar=0;
    do {
        auto e=parseExpr(tok);
        if(!e) { throw_error(tok); }
        Query::SelectExpr se;
        se.expr=e;
        q->select.push_back(se);
    } while(consume_opt(tok,TokenType::COMMA));
}

/// parse the 'where' clause
static void parseWhere(Query *q, Tokenizer &tok)
{
    if(!consume_opt(tok,TokenType::WHERE)) { return; }
    q->where=parseExpr(tok);
}

/// parse the 'group by' clause
static void parseGroupBy(Query *q, Tokenizer &tok)
{
    if(!consume_opt(tok,TokenType::GROUP)) { return; }
    consume(tok,TokenType::BY);
    q->group=parseIdents(tok);
}

/// parse the 'pivot' clause
static void parsePivot(Query *q, Tokenizer &tok)
{
    if(!consume_opt(tok,TokenType::PIVOT)) { return; }
    q->pivot=parseIdents(tok);
}

/// parse the 'order' clause
static void parseOrder(Query *q, Tokenizer &tok)
{
    if(!consume_opt(tok,TokenType::ORDER)) { return; }
    consume(tok,TokenType::BY);
    do {
        auto e=parseExpr(tok);
        if(!e) { throw_error(tok); }
        Query::OrderExpr oe;
        oe.expr=e;
        oe.desc=consume_opt(tok,TokenType::DESC);
        q->order.push_back(oe);
    } while(consume_opt(tok,TokenType::COMMA));

    parseExpr(tok);
}

/// parse the 'limit' clause
static void parseLimit(Query *q, Tokenizer &tok)
{
    if(!consume_opt(tok,TokenType::LIMIT)) { return; }
    std::string n=tok.current().token;
    consume(tok,TokenType::NUMBER);
    q->limit=std::stoull(n);
}


/// parse the 'offset' clause
static void parseOffset(Query *q, Tokenizer &tok)
{
    if(!consume_opt(tok,TokenType::OFFSET)) { return; }
    std::string n=tok.current().token;
    consume(tok,TokenType::NUMBER);
    q->offset=std::stoull(n);
}

/// parse the 'label' clause and match it with the 'select' expression
static void parseLabel(Query *q, Tokenizer &tok)
{
    if(!consume_opt(tok,TokenType::LABEL)) { return; }
    do {
        auto e=parseExpr(tok);
        if(!e) { throw_error(tok); }
        std::string label=tok.current().token;
        consume(tok,TokenType::STRING);
        if(q->selectStar) {
            Query::SelectExpr se;
            se.expr=e;
            se.label=label;
            q->select.push_back(se);
        } else {
            bool found=0;
            for(auto &ex:q->select) {
                if((*ex.expr)==(*e)) {
                    ex.label=label;
                    found=1;
                    break;
                }
            }
            if(!found) {
                throw_error(tok,"label Expression "+e->to_string()+" not found");
            }
        }
    } while(consume_opt(tok,TokenType::COMMA));

}

/// parse the 'format' clause and match it with the 'select' expression
static void parseFormat(Query *q, Tokenizer &tok)
{
    if(!consume_opt(tok,TokenType::FORMAT)) { return; }
    do {
        auto e=parseExpr(tok);
        if(!e) { throw_error(tok); }
        std::string format=tok.current().token;
        consume(tok,TokenType::STRING);

        bool found=0;
        for(auto &ex:q->select) {
            if((*ex.expr)==(*e)) {
                ex.format=format;
                found=1;
                break;
            }
        }
        if(!found) {
            if(q->selectStar) {
                Query::SelectExpr se;
                se.expr=e;
                se.format=format;
                q->select.push_back(se);
            } else {
                throw_error(tok,"Format Expression "+e->to_string()+" not found");
            }
        }
    } while(consume_opt(tok,TokenType::COMMA));
}

/// parse the 'options' clause
static void parseOptions(Query *q, Tokenizer &tok)
{
    if(!consume_opt(tok,TokenType::OPTIONS)) { return; }
    if(tok.current().tt==TokenType::IDENTIFIER) {
        if(tok.current().token=="no_format") {
            q->no_format=1;
        } else if(tok.current().token=="no_values") {
            q->no_values=1;
        } else {
            throw_error(tok,"expected no_format or no_values");
        }
    }
    consume(tok,TokenType::IDENTIFIER);
}

/// return the number of arguments a fucntion call must have.
/// This is not used if 'extendedFunctions' are allowed, in which
/// case the parser does not validate function calls at all.
static int32_t funcArgs(const std::string &_name)
{
    static const std::map<std::string,uint16_t> functions {
        { "year", 1 },
        { "month", 1 },
        { "day", 1 },
        { "hour", 1 },
        { "minute", 1 },
        { "second", 1 },
        { "millisecond", 1 },
        { "quarter", 1 },
        { "dayOfWeek", 1 },
        { "now", 1 },
        { "dateDiff", 2 },
        { "toDate", 1 },
        { "upper", 1 },
        { "lower", 1 },
        { "now", 0 },

        { "like", 2 },
        { "matches", 2 },
        { "contains", 2 },
        { "ends", 2 },
        { "starts", 2 },

        { "and", 2},
        { "or", 2},

        { "+", 2},
        { "-", 2},
        { "/", 2},
        { "*", 2},

        { "=", 2},
        { "!=", 2},
        { "<", 2},
        { ">", 2},
        { "<=", 2},
        { ">=", 2},

        { "avg", 1 },
        { "count", 1 },
        { "max", 1 },
        { "min", 1 },
        { "sum", 1 },

        { "is null", 1 },
        { "is not null", 1 },
    };

    auto it=functions.find(_name);
    if(it==functions.end()) { return -1; }
    return it->second;
}

/// Do some simple semantic validations for a single expressions
static void checkSemantics(const Query *q,Query::Expr::CPtr ex)
{
    if(!ex) { return; }
    if(ex->sub().size()==0&&!ex->noarg()) { return; }
    if(!q->extendedFunctions) {
        int32_t args=funcArgs(ex->data());
        if(args<0) { throw GQLError(ErrorReasons::INVALID_QUERY,"function '"+ex->data()+"' is not known"); }
        if(static_cast<uint32_t>(args)!=ex->sub().size()) {
            throw GQLError(ErrorReasons::INVALID_QUERY,"function '"+ex->data()+"' takes "+std::to_string(args)
                           +" argument(s), but got "+std::to_string(ex->sub().size()));
        }
    }
}

/// Do some simple semantic validations for all expressions
static void checkSemantics(const Query *q)
{
    for(auto &s:q->select) { checkSemantics(q,s.expr); }
    checkSemantics(q,q->where);
    for(auto &o:q->order) { checkSemantics(q,o.expr); }
}

/// Parse a GQL string and return a query object with the result.
Query::CPtr Query::parse(const std::string &_query, bool _extendedFunctions)
{
    Query *q=new Query();
    q->extendedFunctions=_extendedFunctions;
    Tokenizer tok(_query);
    tok.next();

    parseSelect(q,tok);
    parseWhere(q,tok);
    parseGroupBy(q,tok);
    parsePivot(q,tok);
    parseOrder(q,tok);
    parseLimit(q,tok);
    parseOffset(q,tok);
    parseLabel(q,tok);
    parseFormat(q,tok);
    parseOptions(q,tok);

    if(tok.current().tt!=TokenType::EOL) { throw_error(tok,"the end of statement"); }
    checkSemantics(q);

    return std::shared_ptr<Query>(q);
}

bool Query::isAggFunc(const std::string &_name)
{
    static const std::set<std::string> aggfunc { "avg","count","max","min","sum" };
    return aggfunc.count(_name)>0;
}

/// quote identifiers if needed (for GQL output)
static std::string quoteIdent(const std::string &s)
{
    bool needQuote=s.size()==0||keyword.count(s)>0||(s[0]>='0'&&s[0]<='9');
    if(!needQuote) {
        for(auto c:s) {
            needQuote=!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_');
            if(needQuote) { break; }
        }
    }
    if(needQuote) {
        return "`"+s+"`";
    }
    return s;
}

/// quote a string for GQL output
static std::string quoteString(const std::string &s)
{
    char quote='"';

    if(s.find(quote)) { quote='\''; }
    return quote+s+quote;
}

/// human readable form for a query using GQL syntax
std::string Query::to_string() const
{
    std::string r="";
    if(selectStar) {
        r="SELECT *";
    } else if(select.size()>0) {
        r="SELECT ";
        for(unsigned int i=0;i<select.size();i++) {
            if(i>0) { r+=", "; }
            r+=select[i].expr->to_string();
        }
    }
    if(where) {
        r+=" WHERE "+where->to_string();
    }
    if(group.size()>0) {
        r+=" GROUP BY";
        bool first=1;
        for(auto i:group) {
            if(!first) { r+=","; }
            first=0;
            r+=" "+quoteIdent(i.token);
        }
    }
    if(pivot.size()>0) {
        r+=" PIVOT";
        bool first=1;
        for(auto i:pivot) {
            if(!first) { r+=","; }
            first=0;
            r+=" "+quoteIdent(i.token);
        }
    }
    if(order.size()>0) {
        r+=" ORDER BY";
        bool first=1;
        for(auto i:order) {
            if(!first) { r+=","; }
            first=0;
            r+=" "+i.expr->to_string();
            if(i.desc) { r+=" DESC"; }
        }
    }
    if(limit) { r+=" LIMIT "+std::to_string(limit); }
    if(offset) { r+=" OFFSET "+std::to_string(offset); }

    bool label=false;
    for(unsigned int i=0;i<select.size();i++) {
        if(select[i].label!="") {
            if(label) {
                r+=", ";
            } else {
                r+=" LABEL ";
                label=true;
            }
            r+=select[i].expr->to_string();
            r+=" "+quoteString(select[i].label);
        }
    }
    bool format=false;
    for(unsigned int i=0;i<select.size();i++) {
        if(select[i].format!="") {
            if(format) {
                r+=", ";
            } else {
                r+=" FORMAT ";
                format=true;
            }
            r+=select[i].expr->to_string();
            r+=" "+quoteString(select[i].format);
        }
    }
    if(no_values) {
        r+=" OPTIONS no_values";
    } else if(no_format) {
        r+=" OPTIONS no_format";
    }
    return r;
}

/// return expression in GQL syntax as a string
std::string Query::Expr::to_string(bool parens) const
{
    std::string r;
    switch(tp()) {
    case GQLParser::TokenType::PLUS:
    case GQLParser::TokenType::MINUS:
        if(sub().size()==1) {
            if(tp()==GQLParser::TokenType::PLUS) {
                r+=sub()[0]->to_string();
            } else {
                r+="(-"+sub()[0]->to_string()+")";
            }
            break;
        }
        FALLTHROUGH
    case GQLParser::TokenType::TIMES:
    case GQLParser::TokenType::DIV:
    case GQLParser::TokenType::LT:
    case GQLParser::TokenType::LE:
    case GQLParser::TokenType::GT:
    case GQLParser::TokenType::GE:
    case GQLParser::TokenType::EQ:
    case GQLParser::TokenType::NE:
        assert(sub().size()==2);
        if(parens) { r+="("; }
        r+=(sub()[0]->to_string())+data()+(sub()[1]->to_string());
        if(parens) { r+=")"; }
        break;
    case GQLParser::TokenType::AND:
    case GQLParser::TokenType::OR:
        if(parens) { r+="("; }
        assert(sub().size()==2);
        r+=(sub()[0]->to_string())+" "+data()+" "+(sub()[1]->to_string());
        if(parens) { r+=")"; }
        break;
    case GQLParser::TokenType::NOT:
        assert(sub().size()==1);
        if(parens) { r+="("; }
        r+="not "+(sub()[0]->to_string());
        if(parens) { r+=")"; }
        break;
    case GQLParser::TokenType::NUMBER:
        r=data();
        break;
    case GQLParser::TokenType::STRING:
        r=quoteString(data());
        break;

    case GQLParser::TokenType::DATE:
    case GQLParser::TokenType::TIMEOFDAY:
    case GQLParser::TokenType::DATETIME:
        assert(sub().size()==1);
        r+=data()+" "+sub()[0]->to_string();
        break;

    case GQLParser::TokenType::_UNDEFINED:
    case GQLParser::TokenType::EOL:
        LOG(FATAL) << "unsuable token type " << tp();

    case GQLParser::TokenType::ERROR:
    case GQLParser::TokenType::ASC:
    case GQLParser::TokenType::BY:
    case GQLParser::TokenType::IDENTIFIER:
    case GQLParser::TokenType::DESC:
    case GQLParser::TokenType::GQL_FALSE:
    case GQLParser::TokenType::GQL_TRUE:
    case GQLParser::TokenType::FORMAT:
    case GQLParser::TokenType::LABEL:
    case GQLParser::TokenType::LIMIT:
    case GQLParser::TokenType::GROUP:
    case GQLParser::TokenType::OFFSET:
    case GQLParser::TokenType::OPTIONS:
    case GQLParser::TokenType::ORDER:
    case GQLParser::TokenType::PIVOT:
    case GQLParser::TokenType::SELECT:
    case GQLParser::TokenType::TIMESTAMP:
    case GQLParser::TokenType::WHERE:
    case GQLParser::TokenType::P_OPEN:
    case GQLParser::TokenType::P_CLOSE:
    case GQLParser::TokenType::COMMA:
    case GQLParser::TokenType::IS_NULL:
    case GQLParser::TokenType::IS_NOT_NULL:
    case GQLParser::TokenType::STARTS:
    case GQLParser::TokenType::ENDS:
    case GQLParser::TokenType::CONTAINS:
    case GQLParser::TokenType::MATCHES:
    case GQLParser::TokenType::LIKE:
        r=quoteIdent(data());
        if(sub().size()==0 && !noarg()) {
        } else {
            r+="(";
            int first=1;
            for(auto e:sub()) {
                if(!first) { r+=", "; }
                first=0;
                r+=e->to_string(0);
            }
            if(r[r.size()-1]==' ') {
                r=r.substr(0,r.length()-1);
            }
            r+=")";
        }
    }
    return r;
}

std::ostream& operator<<(std::ostream& outs, const Query::Expr &ex)
{
    outs <<  ex.to_string();
    return outs;
}

} // namespace GQLParser
} // namespace GQL_SQL
