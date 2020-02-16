/** \file 
 * 
 * \brief Interface for GQL -> SQL connector
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
 *
 */

#ifndef _LIBGQLSQL_H_
#define _LIBGQLSQL_H_

#include <exception>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <jsoncpp/json/json.h>
#include <mysql++/mysql++.h>
#include <pqxx/pqxx>

#include "libs/uriparser2/uriparser2.h"

#if defined(__GNUC__) || defined(__clang__)
#define pure __attribute__((pure))
#else
//! Mark functions as pure for compilers that support such an extension
#define pure
#endif

namespace GQL_SQL {
    //! Valid list of errors as per the GQL data source protocol v0.6
    /** https://developers.google.com/chart/interactive/docs/dev/implementing_data_source */
    enum class ErrorReasons {
        UNDEFINED=0,                    ///< Not yet set, invalid choice.
        NOT_MODIFIED=1,                 ///< The data has not changed since the last request. 
                                        ///< If this is the reason for the error, you should
                                        ///< not have a value for table.
                                        ///< \a Note: not used by this library

        USER_NOT_AUTHENTICATED=2,       ///<If the data source requires authentication and it has
                                        ///< not been done, specify this value. The client will
                                        ///< then display an alert with the value of message.
                                        ///< \a NOTE: authentication is currently not supported and
                                        ///< considered to be outside the scope of this library

        UNKNOWN_DATA_SOURCE_ID=3,       ///< not used
        ACCESS_DENIED=4,                ///< Returned when accessing a table that is off-limits or
                                        ///< does not exist
        UNSUPPORTED_QUERY_OPERATION=5,  ///< not used
        INVALID_QUERY=6,                ///< Returned if a non existing column is referenced
        INVALID_REQUEST=7,              ///< Returned if any code or library function throws an exception
        INTERNAL_ERROR=8,               ///< not used
        NOT_SUPPORTED=9,                ///< not used
        ILLEGAL_FORMATTING_PATTERNS=10, ///< Returned if a formatting patterns cannot be parsed
        OTHER=11,                       ///< not used
    };
    
    //! convert ErrorReasons enum to string for printing and debugging
    std::ostream& operator<<(std::ostream& outs, ErrorReasons);

    //! Thrown by the parser and SQL connection when unspeakable things happen.
    struct GQLError : std::exception {
        GQLError(ErrorReasons _er,std::string _msg) : er_(_er), msg_(_msg) { }
        virtual const char* what() const noexcept override;
        ///< overwrite the standard std::exception::what() function to reutnr
        ///< the complete error message.
        inline ErrorReasons er() const noexcept pure { return er_; }
        ///< GQL Error reason.
        inline std::string msg() const noexcept pure { return msg_; }
        ///< error message.
    protected:
        ErrorReasons er_=ErrorReasons::UNDEFINED;
        ///< error reason as set in constructor, not mutable, but cannot be const.
        std::string msg_;
        ///< msg as set in constructor, not mutable, but cannot be const.
        mutable std::string what_;
        ///< need a place to store the string returned by what, set on demand in const
        ///< functions so must be mutable
    };

    //! Status of query returned to the caller
    enum class Status {
        OK=0,             ///< all ok
        SYNTAX_ERROR=1,   ///< parse error, invalid query
        UNKNOWN_TABLE=2,  ///< accessing unknown table
        NOT_SUPPORTED=3,  ///< currentl not used
        ERROR=99,
    };

    std::ostream& operator<<(std::ostream& outs, Status);
    ///< convert Status enum to string for printing and debugging

    //! Describes each column of the result
    struct Column {
        std::string id;     ///< id of the column
        std::string label;  ///< label for the colume (empty if not set)
        std::string format; ///< format used for this column (icu style)
    };


    //! Result of parsing the query and all the information required to execute it
    struct Result {
        std::string target;      ///< Target SQL engine
        Status status;           ///< Parsing result
        std::string gql;         ///< Original gql query
        std::string result;      ///< SQL query compatible with the chosen target
        std::string errormsg;    ///< if the status is not Stats::OK contains an error message
        int errorpos=0;          ///< pointer to the error position in the original query
        std::vector<Column> cols;///< Description of every column of the result
    };

    std::ostream& operator<<(std::ostream& outs, const Result &);
    ///< pretty print for Result object

    //! Data structures used for parsing GQL strings
    namespace GQLParser {
        //! List of aggregate functions
        enum class AggFunction { AVG, COUNT, MAX, MIN, SUM };


        //! List of all tokens used by the parser and defined as reserved words in GQL
        /** Includes some combined tokens to simplify the parser */
        enum class TokenType {
            _UNDEFINED,
            STRING,NUMBER,
            IDENTIFIER,
            EOL,
            ERROR,

            AND,
            ASC,
            BY,
            DATE,
            DATETIME,
            DESC,
            GQL_FALSE,
            FORMAT,
            GROUP,
            LABEL,
            LIMIT,
            NOT,
            OFFSET,
            OPTIONS,
            OR,
            ORDER,
            PIVOT,
            SELECT,
            TIMEOFDAY,
            TIMESTAMP,
            GQL_TRUE,
            WHERE,

            EQ,NE,LT,GT,GE,LE,
            PLUS,MINUS,TIMES,DIV,
            P_OPEN,P_CLOSE,

            COMMA,

            // non keywords
            IS_NULL,IS_NOT_NULL,STARTS,ENDS,CONTAINS,MATCHES,LIKE
        };

        //! Exception thrown when there is a problem parsing a string
        class SyntaxError : public GQLError {
            public:
                SyntaxError(const SyntaxError &) = default;
                ///< Default copy constructor, required due to explicit destructor

                ///! Main constuctor for this error.
                SyntaxError(const std::string &_w, size_t _p) 
                    : GQLError(ErrorReasons::INVALID_QUERY,std::string("pos ")+std::to_string(_p)+std::string(": ")+_w),
                      pos_(_p) { }

                inline size_t pos() const { return pos_; }
                ///< accessor for pos value

                virtual ~SyntaxError();
                ///< need at least on virtual function to have a place for the vtable.
            protected:
                size_t pos_=0; ///< position of the syntax error in the query being parsed
        };

        //! A token is the smallest element in a GQL query
        class Token final {
            public:
                Token()=default;
                ///< default noargs constructor
                Token(size_t _pos,TokenType _tt,std::string _token="") noexcept : pos(_pos),tt(_tt), token(_token) { }
                ///< construct token with string position, type and string

                //! return true if this is either a string, number or boolean literal
                inline bool isLiteral() const noexcept pure {
                    return    tt==TokenType::STRING 
                           || tt==TokenType::NUMBER
                           || tt==TokenType::GQL_TRUE
                           || tt==TokenType::GQL_FALSE;
                }
                //! return true if two token are exactly equal
                inline bool operator==(const Token &t) const noexcept pure {
                    return pos==t.pos&&tt==t.tt&&token==t.token;
                }
                //! return true if two token are not exactly equal
                inline bool operator!=(const Token &t) const noexcept pure {
                    return !(*this==t);
                }
                //! return true if this token is an identifier with the given name
                inline bool isIdent(const std::string &_id) const noexcept pure {
                    return tt==TokenType::IDENTIFIER&&_id==token;
                }

                size_t pos=0;                      ///< position (character) of this token in the query string
                TokenType tt=TokenType::_UNDEFINED;///< token type (see enum TokenType)
                std::string token;                 ///< original string for this token.
        };
        std::ostream& operator<<(std::ostream& outs, const Token &);
        ///< pretty print Token

        //! Tokenizer class splits up a query string into tokens
        class Tokenizer {
            public:
                Tokenizer(std::string _str) noexcept : str_(_str) { }
                ///< Start tokenizer with the given query string 
                
                Token next();
                ///< Return the next token.
                
                inline Token current() const noexcept pure { return current_; }
                ///< Return the current token

            private:
                std::string nexti();
                ///< Returns the next token that starts at pos_, ignores any initial white space.
                ///< Returns an emtpy string for EOT

                Token nextt();
                ///< Get the next token and convert it to an actual Token.
                ///< Helper function for next().

                std::string str_;
                ///< String to parse
                unsigned int pos_=0;
                ///< Current position in the string, updated during parsing
                Token current_;
                ///< Current token.
        };

        //! The Query class describes a parsed GQL query.
        /** It contains all the information from the original query string. */
        class Query {
            public:
                typedef std::shared_ptr<Query> Ptr;
                ///< shared pointer for the Query class
                typedef std::shared_ptr<const Query> CPtr;
                ///< shared pointer for the Query class, for const ptr

                //! This recursive class describes the AST of an expression.
                /** Used for select expression, order and where clause */
                class Expr final {
                    public:
                        typedef std::shared_ptr<Expr> Ptr;
                        ///< shared pointer for the Expr class
                        typedef std::shared_ptr<const Expr> CPtr;
                        ///< shared pointer for the Expr class, for const ptr

                        //! Create expression with up to two sub expressions.
                        static inline Ptr make(TokenType _tp,const std::string &_data="",
                                 const std::shared_ptr<const Expr> _ex1=0,
                                 const std::shared_ptr<const Expr> _ex2=0) 
                                        { return Ptr(new Expr(_tp,_data,_ex1,_ex2)); }
                                // make_shared requires a public constructor, so cannot
                                // be used here. However, this enforces that all Expr
                                // objects are created correctly.

                        inline TokenType tp() const { return tp_; }
                        ///< Accessor for TokenType for this expression
                        inline const std::string &data() const { return data_; }
                        ///< data (string) for this expression, i.e. function name, identifier, etc
                        inline bool noarg() const { return noarg_; }
                        ///< true if this expression has no arguments and is not a function

                        inline void noargSet() { noarg_=1; }
                        ///< mark this expression as an identifier, i.e. not a function with no args
                        inline void push(std::shared_ptr<const Expr> e) { if(e) { sub_.push_back(e); } }
                        ///< add an additional sub expression
                        inline const std::vector<std::shared_ptr<const Expr>> &sub() const { return sub_; }
                        ///< access the list of sub expressions

                        bool operator==(const Expr &other) const;
                        ///< Returns true if both expressions are exactly the same
                        inline bool operator!=(const Expr &other) const { return !operator==(other); }
                        ///< Returns false if both expressions are exactly the same
                        std::string to_string(bool parens=1) const;
                        ///< Stringify the expression (returns a valid gql expression)
                    private:
                        //! Create expression with up to two sub expressions.
                        Expr(TokenType _tp,const std::string &_data="",
                             const std::shared_ptr<const Expr> _ex1=0,
                             const std::shared_ptr<const Expr> _ex2=0) 
                                    : tp_(_tp), data_(_data) { push(_ex1);push(_ex2); }

                    protected:
                        TokenType tp_;     ///< TokenType that describes this expression
                        std::string data_; ///< string to describe this expression (function name, literal, identifier, operator)
                        bool noarg_=0;     ///< 1 for function calls with no args as opposed to identifier that
                                           ///< that are not functions 
                        std::vector<std::shared_ptr<const Expr>> sub_;
                                           ///< ordered list of sub expressions

                };

                //! Describes a single select expression
                class SelectExpr {
                    public:
                        bool aggregate=false;
                        ///< True if this is an aggregate expression
                        Expr::CPtr expr;
                        ///< Actual expression
                        std::string label;
                        ///< label for this expression
                        std::string aggcolumn;
                        ///< if this as an aggregation expression this contains the column
                        std::string format;
                        ///< Format string (if defined)
                        std::string to_string() const { return "Expr: "+expr->to_string()+" label:"+label+" format:"+format; }
                        ///< Returns the expression as a pretty string
                };

                //! Describes a single "order by" expression of the query
                class OrderExpr {
                    public:
                        Expr::CPtr expr;
                        ///< Actual expression
                        bool desc=false;
                        ///< True if the order is descending
                };


            public:
                ///< returns true if there is a pivot clause
                inline bool hasPivotClause() const { return pivot.size()>0; }
                std::vector<SelectExpr> select;
                ///< contains each selext expression, except when using '*', in which
                ///< case it contains the optional label and format information only,
                ///< with the expression being the ID of the column
                Expr::CPtr where=0;
                ///< The where expression or nullptr if there is none
                std::vector<Token> group;
                ///< List of 'group by' identifier (columns)
                std::vector<Token> pivot;
                ///< List of pivot identifiers (colums)
                std::vector<OrderExpr> order;
                ///< List of order expressions
                unsigned long limit=0;
                ///< Limit or 0 if not set
                unsigned long offset=0;
                ///< Offset if set, 0 otherwise

                bool no_format=false;
                ///< True if the no_format option is set
                bool no_values=false;
                ///< True if the no_value option is set

                static bool isAggFunc(const std::string &_name);
                ///< return true if this is an aggregate function, false otherwise
                ///< return number of arguments for given function, or -1 for unknown functions

                bool selectStar=0;
                ///< True if this is a "select everything" query
                bool extendedFunctions=0;
                ///< True if the parser accepts unknown functions which are passed to the
                ///< SQL engine

                /**! Parse the given GQL query and create an AST
                 * @param _query   valid GQL query
                 * @param _extendedFunctions whether or not to allow non GQL functions
                 *                           to be passed to the SQL engine.
                 * Note that this returns a const pointer and non of the attributes
                 * are expected to be changed.
                 */
                static CPtr parse(const std::string &_query,bool _extendedFunctions=0);

                std::string to_string() const;
                ///< returna GQL conforming string of the query
            private:
                Query() { }
                ///< Constructor, hidden, only way to create an object is via parse()
        };


        std::ostream& operator<<(std::ostream& outs, TokenType);
        ///< Pretty print TokenType
        std::ostream& operator<<(std::ostream& outs, const Query::Expr &);
        ///< Pretty print an expression

        //! Class that does all the parsing of a GQL string
        /** derived classes then convert the parsed data to the required string for
         * the chosen DB backend */
        class Parser {
            public:
                /**! Create a parser
                 * @param _defTable default table to be queried
                 * @param _allowedTables if not empty list of tables that can be queried
                 *                        using the extension tablename.columname
                 * @param _extendedFunctions Allow any function call by passing them down
                 *                           to the underlying SQL engine.
                 */
                // this really should be protected, but would not make it possible
                // to inherit it, so public as per C++ standard. It is still not
                // possible to instantiate this class due to the target() function.
                Parser(const std::string &_defTable="",
                       const std::set<std::string> &_allowedTables=std::set<std::string>(),
                       bool _extendedFunctions=false) :
                    defTable_(_defTable), allowedTables_(_allowedTables),
                    extendedFunctions_(_extendedFunctions) { }
                virtual ~Parser();

                inline const std::string &defTable() const { return defTable_; }
                ///< Return the name of the table used for this query
                inline const std::set<std::string> &allowedTables() const { return allowedTables_; }
                ///< Return set of table names that may be used in the query


                bool parse(const std::string &);
                ///< Parse the given string and create the objects describing it
                inline const Result &res() const { return res_; }
                ///< Returns the result of the parsing
                inline const Query::CPtr query() const { return query_; }
                ///< Returns the query after parsing the data, in the form of an object tree
                virtual std::string target() const=0;
                ///< Return a string describing the target for this parser/translator


            protected:
                Parser(const Parser &) = delete;
                ///< no copy constructor. Create new object if needed

                std::string defTable_;
                ///< defaut table name used for any column that does not specify a table
                std::set<std::string> allowedTables_;
                ///< list of accepted table names
                std::string error_="";
                ///< set to the last error detected, not cleared if there was no error.
                Query::CPtr query_;
                ///< result of last query parsed
                bool extendedFunctions_=0;
                ///< Allow function names that are not defined in the GQL spec
                ///< and pass them through to the underlying SQL engine.
                Result res_;
                ///< Result of the parsing, contains the SQL query for the selected target

            private:
                virtual void createResult() = 0;
                ///< Create the SQL string for the requested target. Overwritten by
                ///< derived classes
        };

        //! Returns a GQL string from a GQl query. This is only really useful for testing.
        class ParserGQL final : public Parser {
            // mostly for testing
            public:
                using Parser::Parser;
                ///< Inherit constructor
                virtual std::string target() const override;
                ///< return the target as a human reabable string
                virtual ~ParserGQL() override;
            private:
                virtual void createResult() override;
                ///< Create the GQL query (this is expected to be the same as the input query)
        };

        //! Returs a valid MySQL query from a GQL query given the various options
        class ParserMySQL final : public Parser {
            public:
                //! Inherit constructor
                using Parser::Parser;
                virtual std::string target() const override;
                ///< return the target as a human reabable string
                virtual ~ParserMySQL() override;
            private:
                virtual void createResult() override;
                ///< Create the SQL query for the MySQL/Maria DB
        };

        //! Returs a valid PostgreSQL query from a GQL query given the various options
        class ParserPostgreSQL final : public Parser {
            public:
                using Parser::Parser;
                ///< Inherit constructor
                virtual std::string target() const override;
                ///< return the target as a human reabable string
                virtual ~ParserPostgreSQL() override;
            private:
                virtual void createResult() override;
                ///< Create the SQL query for the PostgreSQL DB
        };

    }


    namespace DBQuery {
        static const std::string TYPE_BOOLEAN="boolean";   ///< boolean column type
        static const std::string TYPE_STRING="string";     ///< string column type
        static const std::string TYPE_NUMBER="number";     ///< number column type
        static const std::string TYPE_DATE="date";         ///< date column type
        static const std::string TYPE_TIME="timeofday";    ///< time column type
        static const std::string TYPE_DATETIME="datetime"; ///< datetime column type
        static const long long MIN_DATE=-62167219200LL-3600*24*2;
        ///< value that gets the icu gregorian calendar to return 0001/01/01 00:00:00
        ///< (cannot go lower than that).

        //! Base class to connect to an SQL DB and run a GQL query
        class DB {
            public:
                typedef std::shared_ptr<DB> Ptr;
                ///< shared pointer for the Query class
                typedef std::shared_ptr<const DB> CPtr;
                ///< shared pointer for the Query class, for const ptr
                
                virtual bool isConnected() const = 0;
                ///< Returns True if there is a working connection to the DB
                virtual void connect()=0;
                ///< connect to the database. 
                ///< Errors are returned in json format through DB::execute()
                ///< and isConnected() will return false if the connection
                ///< failed.

                virtual ~DB();
                ///< destructor

                void execute(const std::string &gql,Json::Value &res) const;
                ///< Execute the given query and returns a json object with the result

                void deftableSet(const std::string &_d) { deftable_=_d; }
                ///< Set table to query
                const std::string deftable() { return deftable_; }
                ///< return table to query.

                inline void tableAdd(const std::string &_t) { tables_.insert(_t); }
                ///< Add a table to the list of tables that may be queried.
                ///< This uses a GQL extension with the format Tablename.Colname
                ///< and works with any SQL backend.
                inline void tableDel(const std::string &_t) { tables_.erase(_t); }
                ///< Remove a table to the list of tables that may be queried
                inline void tableClear() { tables_.clear(); }
                ///< Clear the extended list of tables/
                inline bool tableExists(const std::string &t) const { return tables_.count(t)>0; }
                ///< Returns true if a table is part of the extended table set

                inline void extendedFunctionsSet(bool _v) { extendedFunctions_=_v; }
                ///< Allow/disallow extended functions (pass through SQL functions)
                inline bool extendedFunctions() const { return extendedFunctions_; }
                ///< Query if extended functions are allowed

            protected:
                virtual void getdata(const std::string &r,Json::Value &tbl) const = 0;
                ///< run the SQL query and get all the data in json format
                void pivotTable(const Json::Value &tbl,Json::Value &tres) const;
                ///< Manually implement the pivot command by manipulating the json result.
                ///< In order to avoid expensive deep copies a new table is generated
                ///< and returned which makes this operation very expensive memory wise
                ///< as all data is in memory twice until the operation ends.

                std::shared_ptr<GQLParser::Parser> parser_=0;
                ///< The parser object used
                std::map<std::string,std::string> config_;
                ///< A key/value map of configuration options if a config file was used
                std::set<std::string> tables_;
                ///< set of table that may be queried
                std::string type_;
                ///< SQL database type
                std::string deftable_;
                ///< name of table to query
                std::string user_;
                ///< SQL user name
                std::string password_;
                ///< SQL password
                std::string server_;
                ///< SQL server
                std::string db_;
                ///< SQL db to query
                std::set<std::string> defTableColumns_;
                ///< list of additional tables that may be queried
                uint32_t port_=0;
                ///< TCP port to use for the DB connection
                bool extendedFunctions_=false;
                ///< true if extended SQL functions may be used

                DB(Json::Value _init);
                ///< Initialize DB connection using a set of k/v
                DB(const URI &_uri);
                ///< Initialize DB connection using a URL
        };

        //! MySQL connect class
        class MySQL : public DB {
            public:
                MySQL(Json::Value _init) : DB(_init) { }
                ///< Initialize using json k/v config parameters
                MySQL(const URI &_uri) : DB(_uri) { }
                ///< Initialize using a connection URL
                virtual bool isConnected() const override;
                ///< return status of DB connection
                virtual void connect() override;
                ///< connect to the database. Failures should be checked
                ///< by calling isConnected().

                virtual ~MySQL();
                ///< destructor

            protected:
                void getdata(const std::string &q,Json::Value &tbl) const override;
                ///< Query the database and return the data in a json object
            private:
                mysqlpp::Connection *connection_=0;
                ///< MySQL connecion object
        };

        //! PostgerSQL connect class
        class PostgreSQL : public DB {
            public:
                PostgreSQL(Json::Value _init) : DB(_init) { }
                ///< Initialize using json k/v config parameters
                PostgreSQL(const URI &_uri) : DB(_uri) { }
                ///< Initialize using a connection URL
                virtual bool isConnected() const override;
                ///< return status of DB connection
                virtual void connect() override;
                ///< connect to the database

                virtual ~PostgreSQL();
                ///< destructor

            protected:
                void getdata(const std::string &q,Json::Value &tbl) const override;
                ///< Query the database and return the data in a json object
            private:
                pqxx::connection *connection_=0;
                ///< PostgreSQL connecion object
        };


        void handleCgi(DBQuery::DB::Ptr db);
        ///< Get the query and output format from the CGI environment variables.
        ///< Currently uses only HTTP_ACCEPT_LANGUAGE and QUERY_STRING.

        void outputHtml(std::ostream &o,const std::string name,const Json::Value &res);
        ///< output data in html format to the given stream
        void outputCsv(std::ostream &o,const Json::Value &res);
        ///< output data in CSV format to the given stream
        void outputTsv(std::ostream &o,const Json::Value &res);
        ///< output data in TSV format to the given stream
        void outputJson(std::ostream &o,const Json::Value &tbl);
        ///< output data in Json format to the given stream

    }

    //! convert any class to a string by using a stringstream and ostream operator
    template<typename T> const std::string to_string(const T &n) {
        std::stringstream str;
        str << n;
        return str.str();
    }

}

//! Helper class for #ON_EXIT(code)
/** inspired by  see https://gist.github.com/castano/6075672 */
template <typename F> struct ScopeExit {
    //! constructor stores function to execute at exit
    ScopeExit(F _f) : f_(_f) {}
    //! Due to user defined destructor need to define copy constructor
    /** This is needed for the macro below that uses the 'auto' keyword to
     *  have the C++ compiler define the exact type of the function created
     *  (lambda functions have unique types so this cannot easily be avoided)
     */
    ScopeExit(const ScopeExit &_se) = default;
    //! Destructor: execute function
    inline ~ScopeExit() { f_(); }
private:
    const F f_;
};

//! Helper function for #ON_EXIT(code)
template <typename F> inline ScopeExit<F> MakeScopeExit(F f) { return ScopeExit<F>(f); }

/// \cond
/// Helper macro to allow generating a unique identifier each time it is used
#define STRING_JOIN2(arg1, arg2) DO_STRING_JOIN2(arg1, arg2)
#define DO_STRING_JOIN2(arg1, arg2) arg1 ## arg2
/// \endcond

//! Execute code when exiting current scope (uses ScopeExit)
#define ON_EXIT(code) \
    auto STRING_JOIN2(scope_exit_, __LINE__) = MakeScopeExit([&](){code;})

#ifdef __clang__
#  define FALLTHROUGH [[clang::fallthrough]];
#else
#  define FALLTHROUGH [[fallthrough]];
#endif
///< Used to mark cases statements that have no break.
///< This allows certain compilers to warn against missing 
///< break statements.

#endif
