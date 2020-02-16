/** \file 
 * \brief CLI interface for GQL -> SQL connector
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <locale.h>
#include <getopt.h>
#include <iostream>
#include <fstream>
#include </usr/include/unicode/uloc.h>
#include </usr/include/unicode/ures.h>
#include <unicode/uclean.h>
#include <glog/logging.h>
#include "libs/uriparser2/uriparser2.h"

#include "libgqlsql.h"

static const char *prgname=0;

/// Print usage information and optional error message.
/// @param _error If non empty will be printed and force a non zero exit code.
[[noreturn]] static void usage(const std::string &_error="") 
{
    if(_error!="") {
        std::cerr << "ERROR: " << _error << std::endl;
        std::cerr << "Use '" << prgname << " --help' for more information" << std::endl;
        exit(1);
    }
    std::cerr 
        << "Usage: " << prgname << " -d table -t table[,table]* [-x cmd] db-url|configfile [cmd ...]" << std::endl
        << "    --default (-d): name of default table" << std::endl
        << "    --tables (-t) : list of space, comma or semi-colon separated acceptable tables" << std::endl
        << "    --extended (-e): allow any function to be passed through SQL" << std::endl
        << "    --locale (-l): locale to use" << std::endl
        << "    --format (-f) html|csv|tsv|json: change output format (for cmds only)" << std::endl
        << "    --help (h): print this text" << std::endl
        << std::endl
        << "The remainder of the arguments are used to connect to the db, currently supported:" << std::endl
        << "- mysql: mysql://user@pw:host:port/db" << std::endl
        << "- mariadb: mariadb://user@pw:host:port/db" << std::endl
        << "- postgresql: postgresql://user@pw:host:port/db" << std::endl
        << std::endl
        << "or a configuration file which  must contain a json formatted map with the" << std::endl
        << "following entries (command line options overwrite those):" << std::endl
        << "{" << std::endl
        << "    \"type\":     either 'mysql', 'mariadb' or 'postgresql'" << std::endl
        << "    \"user\":     username" << std::endl
        << "    \"server\":   server" << std::endl
        << "    \"port\":     port (optional)" << std::endl
        << "    \"password\": username" << std::endl
        << "    \"db\":       database" << std::endl
        << "    \"default\":  default table (optional, cmd line arg overwrites)" << std::endl
        << "    \"tables\":   array of acceptable table names (optional), cmd line overwrites" << std::endl
        << "    \"extended\": true if any function name should be accepted." << std::endl
        << "    \"locale\":   locale to use" << std::endl
        << "}" << std::endl
        << std::endl
        << "Additional arguments are taken as GQL search requests and result" << std::endl
        << "in one line of output per search, using the standard GQL json format" << std::endl
        << "unless another format is specified (in those cases using more than a single" << std::endl
        << "command probably creates confusing output.)" << std::endl
        << "With no search commmand cgi mode is enabled and the output is structured" << std::endl
        << "as an HTTP GQL query, reading the query from  the QUERY_STRING environment variable." << std::endl
        << "This will also overwrite the locale that gets used by using the HTTP suggested" << std::endl
        << "locale unless it is set explicitly with the --locale option." << std::endl
        << "Command line arguments overwrite settings in the config file." << std::endl;
        ;
    exit(_error==""?0:1);
}


/// List of all options for getopt()
const static struct option longopt[] = {
    { "help",     no_argument,      0, 'h' },
    { "extended", no_argument,      0, 'e' },
    { "default",  required_argument,0, 'd' },
    { "tables",   required_argument,0, 't' },
    { "locale",   required_argument,0, 'l' },
    { "format",   required_argument,0, 'f' },
    {0,0,0,0}
};


/// Split a string at spaces, commas or semi-colons.
/// Used to split the list of tables into individual table names.
/// @param _str string to split
/// \returns the set of works
static std::set<std::string> split(const std::string &_str)
{
    std::string next;
    std::set<std::string> res;
    for(auto s:_str) {
        if(s==' '||s==','||s==';') {
            if(next.size()>0) {
                res.insert(next);
                next="";
            }
        } else {
            next+=s;
        }
    }
    if(next.size()>0) {
        res.insert(next);
    }
    return res;
}

/// Prep the connection to the DB
static GQL_SQL::DBQuery::DB::Ptr createDB(const std::string &config, const std::string &url) 
{
    GQL_SQL::DBQuery::DB::Ptr db=0;

    if(config!="") {
        std::ifstream f(config);
        if(f.fail()) { usage("config file is not readable"); }
        Json::Value jconfig;

        // if the first character is a '#' ignore the line
        // in order to support #!
        char c;
        f >> c;
        if(c=='#') {
            char buf[201];
            f.getline(buf,200);
        } else {
            f.seekg(0);
        }
        try {
            f >> jconfig;
        } catch(const Json::RuntimeError &x) {
            usage(std::string("Config file not readable: ")+x.what());
        } 
        f.close();
        if(jconfig.isMember("locale")) {
            setlocale(LC_ALL,jconfig["locale"].asString().c_str());
        }
        if(jconfig["type"]=="mysql" || jconfig["type"]=="mariadb") {
            db=GQL_SQL::DBQuery::DB::Ptr(new GQL_SQL::DBQuery::MySQL(jconfig));
        } else if(jconfig["type"]=="postgresql") {
            db=GQL_SQL::DBQuery::DB::Ptr(new GQL_SQL::DBQuery::PostgreSQL(jconfig));
        } else {
            usage(std::string("unknown db type ")+jconfig["type"].asString());
        }
    } else {
        URI u(url.c_str());
        if(u.scheme()=="") {
            usage("DB URL cannot be parsed: "+url);
        }
        if(u.scheme()=="mariadb" || u.scheme()=="mysql") {
            db=GQL_SQL::DBQuery::DB::Ptr(new GQL_SQL::DBQuery::MySQL(u));
        } else if(u.scheme()=="postgresql") {
            db=GQL_SQL::DBQuery::DB::Ptr(new GQL_SQL::DBQuery::PostgreSQL(u));
        } else {
            usage(std::string("unknown scheme ")+u.scheme());
        }
    }

    assert(db);
    return db;
}

/// Set locale using the requested locale from the browser.
/// Uses the HTTP_ACCEPT_LANGUAGE environment variable as per the cgi calling convention.
static void setLocaleFromBrowser()
{
    auto hal=getenv("HTTP_ACCEPT_LANGUAGE");
    if(hal) {
        char resultLocale[200];
        UAcceptResult outResult;
        UErrorCode status=U_ZERO_ERROR;
        auto available = ures_openAvailableLocales(NULL, &status);
        status=U_ZERO_ERROR;
        uloc_acceptLanguageFromHTTP(resultLocale, 200, &outResult, hal, available, &status);
        if(U_SUCCESS(status)) {
            if(!setlocale(LC_ALL,resultLocale)) {
                std::string withUtf8=resultLocale;
                withUtf8+=".UTF8";
                setlocale(LC_ALL,withUtf8.c_str());
                // no errors if either locale failed
            }
        }
    }
}

/// The gqldb main function
/**
 * Help text:
 *
 * \include gqldb-usage.txt
 */
int main(int argc, char*argv[])
{
    google::InitGoogleLogging(argv[0]);
    prgname=argv[0];
    int optindex;
    bool cgi=0;
    bool extended=0;
    std::string defTable;
    std::set<std::string> tables;
    std::vector<std::string> cmd;
    std::string config;
    std::string url;
    std::string locale;
    std::string format="";

    optind=1;
    while(1) {
        int opt=getopt_long(argc,argv,"hed:t:l:f:",longopt,&optindex);
        if(opt==-1) { break; }
        switch(opt) {
        case '?': exit(1);
        case 'h': usage();
        case 'e': extended=1;break;
        case 'd': defTable=optarg;break;
        case 't': tables=split(optarg);break;
        case 'l': locale=optarg;break;
        case 'f': format=optarg;break;
        }
    }

    if(optind>=argc) {
        usage("missing db url or config file");
    } else {
        struct stat sbuf;
        if(stat(argv[optind],&sbuf)==0) {
            config=argv[optind];
        } else {
            url=argv[optind];
        }
        optind++;
    }
    cgi=optind>=argc;

    if(format!=""&&format!="json"&&format!="csv"&&format!="tsv"&&format!="html") {
        usage(std::string("unsupported format '")+format+"'");
    }
    if(cgi&&format!="") {
        usage("the --format option is not supported in cgi mode");
    }

    GQL_SQL::DBQuery::DB::Ptr db=createDB(config,url);

    if(locale!="") {
        if(!setlocale(LC_ALL,locale.c_str())) {
            usage(std::string("Cannot set locale '")+locale+"'");
        } 
    } else if(cgi) {
        setLocaleFromBrowser();
    }


    if(defTable!="") { db->deftableSet(defTable); }
    if(extended) { db->extendedFunctionsSet(1); }

    if(tables.size()>0) {
        // overwrite the config file list
        db->tableClear();
        for(auto t:tables) { db->tableAdd(t); }
    }

    db->connect();

    if(cgi) {
        handleCgi(db);
    } else {
        for(int c=optind;c<argc;c++) {
            Json::Value r;
            db->execute(argv[c],r);
            if(format=="json"||format=="") {
                GQL_SQL::DBQuery::outputJson(std::cout,r);
                std::cout << std::endl;
            } else if(format=="html") {
                GQL_SQL::DBQuery::outputHtml(std::cout,defTable,r);
            } else if(format=="csv") {
                GQL_SQL::DBQuery::outputCsv(std::cout,r);
            } else if(format=="tsv") {
                GQL_SQL::DBQuery::outputTsv(std::cout,r);
            }
        }
    }

    u_cleanup();

    return 0;
}

