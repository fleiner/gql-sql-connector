/** \file 
 * 
 * \brief Implements the CGI interface for the GQL->SQL connector
 *
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
 * **References:**
 *
 * Supports the queries as defined here:
 * https://developers.google.com/chart/interactive/docs/dev/implementing_data_source#response-format
 */

#if __cplusplus==201402L
#include <codecvt>
#else
#include <boost/locale/encoding_utf.hpp>
#endif
#include "libgqlsql.h"
#include "glog/logging.h"

void GQL_SQL::DBQuery::outputJson(std::ostream &o,const Json::Value &tbl)
{
    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(tbl, &o);
}

/// convert a a string using quotes compatible with CSV format
static std::string outputQField(const Json::Value &r)
{
    std::string rs=r.asString();
    std::string res="\"";
    while(1) {
        auto pos=rs.find('"');
        if(pos==std::string::npos) {
            res+=rs;
            break;
        }
        res+=rs.substr(0,pos+1)+'"';
        rs=rs.substr(pos+1);
    }
    return res+'"';
}

/// convert a a string compatible for TSV format by removing any tabs
static std::string outputTField(const Json::Value &r)
{
    std::string rs=r.asString();
    std::string res;
    while(1) {
        auto pos=rs.find('\t');
        if(pos==std::string::npos) {
            res+=rs;
            break;
        }
        res+=rs.substr(0,pos);
        rs=rs.substr(pos+1);
    }
    return res;
}


/// convert a a string to a valid html string by replacing '<', '>' and '&'
/// with the corresponding &XXX; token.
static std::string toHtml(std::string r)
{
    std::string res="";
    size_t pos=0;
    while(true) {
        auto next=r.find_first_of("&<>",pos);
        if(next==std::string::npos) {
            if(pos==0) { return r; }
            res+=r.substr(pos);
            break;
        }
        res+=r.substr(pos,next-pos);
        switch(r[next]) {
        case '&': res+="&amp;";break;
        case '<': res+="&lt;";break;
        case '>': res+="&gt;";break;
        }
        pos=next+1;
    }
    return res;
}


/// Output the result of a query in HTML format
void GQL_SQL::DBQuery::outputHtml(std::ostream &o,const std::string name,const Json::Value &res)
{
    o << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\">" << std::endl
      << "<html>" << std::endl
      << "<head>" << std::endl
      << "<META http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">" << std::endl
      << "<title>" << toHtml(name) << "</title>" << std::endl
      << "</head>" << std::endl
      << "<body>" << std::endl;

    if(res.isMember("errors")) {
        for(auto e:res["errors"]) {
            o << "<h1 color='#f00'>"
              << toHtml(e["reason"].asString())
              << ": "
              << toHtml(e["message"].asString())
              << "</h1></body>"
              << std::endl;
        }
        return;
    }

    o  << "<table border=\"1\" cellpadding=\"2\" cellspacing=\"0\">" << std::endl
       << "<tr style=\"font-weight: bold; background-color: #aaa;\">" << std::endl;

    auto tbl=res["table"];
    for(auto c:tbl["cols"]) {
        o << "<td>";
        if(c.isMember("label") && c["label"]!="") { o << toHtml(c["label"].asString()); }
        else { o << toHtml(c["id"].asString()); }
        o << "</td>";
    }
    o << std::endl << "</tr>" << std::endl;;
    
    static const std::string trcolor[] = { "#f0f0f0","#ffffff" };

    int cnt=0;
    for(auto r:tbl["rows"]) {
        o << "<tr style=\"background-color: " << trcolor[cnt%2] << "\">" << std::endl;
        for(auto c:r["c"]) {
            o << "<td>";
            if(c.isMember("f")) {
                o << toHtml(c["f"].asString());
            } else {
                o << toHtml(c["v"].asString());
            }
            o << "</td>";
        }
        o << std::endl << "</tr>" << std::endl;;
        cnt++;
    }
    o << "</table>" << std::endl
      << "</body>" << std::endl
      << "</html>" << std::endl;
}

#if __cplusplus<201402L
// if not c++14 we need a workaround on Linux
static std::wstring utf8_to_wstring(const std::string& str)
{
    return boost::locale::conv::utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
}
#endif

/// Output a string in UTF16 format
static void out16(std::ostream &o,const std::string &r)
{
#if __cplusplus==201402L
    static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
    auto utf16=utf16conv.from_bytes(r);
#else
    auto utf16=utf8_to_wstring(r);
#endif

    for(auto c:utf16) {
        o << char(c/256) << char(c%256);
    }
}

/// Output the result of a GQL query in TSV (tab separated values) format
void GQL_SQL::DBQuery::outputTsv(std::ostream &o,const Json::Value &res)
{
    o << "\xfe\xff";
    if(res.isMember("errors")) {
        for(auto e:res["errors"]) {
            out16(o,e["reason"].asString());
            out16(o,"\t");
            out16(o,e["message"].asString());
            out16(o,"\n");
        }
    }
    bool first=true;
    auto tbl=res["table"];
    for(auto c:tbl["cols"]) {
        if(!first) { out16(o,"\t"); }
        first=false;
        if(c.isMember("label") && c["label"]!="") { out16(o,outputTField(c["label"].asString())); }
        else { out16(o,outputTField(c["id"].asString())); }
    }
    out16(o,"\n");
    
    for(auto r:tbl["rows"]) {
        bool fc=true;
        for(auto c:r["c"]) {
            if(!fc) { out16(o,"\t"); }
            fc=false;
            if(c.isMember("f")) {
                out16(o,outputTField(c["f"]));
            } else {
                out16(o,outputTField(c["v"]));
            }
        }
        out16(o,"\n");
    }
}

/// Output the result of a GQL query in CSV (comma separated values) format
void GQL_SQL::DBQuery::outputCsv(std::ostream &o,const Json::Value &res)
{
    bool first=true;
    if(res.isMember("errors")) {
        for(auto e:res["errors"]) {
            o << outputQField(e["reason"]);
            o << ",";
            o << outputQField(e["message"]);
            o << std::endl;
        }
        return;
    }
    auto tbl=res["table"];
    for(auto c:tbl["cols"]) {
        if(!first) { o << ','; }
        first=false;
        if(c.isMember("label") && c["label"]!="") { o << outputQField(c["label"]); }
        else { o << outputQField(c["id"]); }
    }
    o << std::endl;
    
    for(auto r:tbl["rows"]) {
        bool fc=true;
        for(auto c:r["c"]) {
            if(!fc) { o <<","; }
            fc=false;
            if(c.isMember("f")) {
                o<< outputQField(c["f"]);
            } else {
                o<< outputQField(c["v"]);
            }
        }
        o << std::endl;
    }
}

/// Store the result of the parsing of the URL fo the Query
struct CgiQuery {
    std::string query;           ///< actual query
    std::string reqId;           ///< Query ID set by user
    std::string version;         ///< version (ignored)
    std::string sig;             ///< signature (ignored)
    std::string out;             ///< output format (either html, json, csv or tsv-excel)
    std::string responseHandler; ///< name of response handler function
    std::string outFileName;     ///< output file to use when browser requests the data

    std::string to_string() const;
    ///< convert to human readable string for debugging
    CgiQuery() { parseQuery(); }
    /// Create the object by parsing teh QUERY_STRING environment variable
private:
    void parseQuery();
    /// parse the QUERY_STRING environment variable
};

// not URL encoded!!
std::string CgiQuery::to_string() const 
{
    return  "tq="+query
           +"&tqx=reqId:"+reqId
           +";version:"+version
           +";sig:"+sig
           +";out:"+out
           +";responseHandler:"+responseHandler
           +";outFileName:"+outFileName;
}


/// Convert URL string to actual string (replaces '+' and %xx with correct character)
static std::string parseUrlFormat(const std::string &orig)
{
    std::string res;
    size_t pos=0;
    while(true) {
        auto next=orig.find_first_of("%+",pos);
        if(next==std::string::npos) {
            if(pos==0) { return orig; }
            res+=orig.substr(pos);
            return res;
        }
        res+=orig.substr(pos,next-pos);
        if(orig[next]=='+') {
            res+=' ';
            pos=next+1;
        } else {
            res+=static_cast<char>(std::stoul(orig.substr(next+1,2),0,16));
            pos=next+3;
        }
    }
}

/// Parse the TQX parameter (various settings for the query)
static void parseTQX(CgiQuery *cq, const std::string &query) 
{
    std::string q=parseUrlFormat(query);
    size_t pos=0;
    while(pos<q.length()) {
        auto next=q.find(";",pos);
        if(next==std::string::npos) { next=q.length(); }
        size_t max=next-pos;
        if(max>=6 && q.substr(pos,6)=="reqId:") {
            cq->reqId=q.substr(pos+6,max-6);
        } else if(max>=8 && q.substr(pos,8)=="version:") {
            cq->version=q.substr(pos+8,max-8);
        } else if(max>=4 && q.substr(pos,4)=="out:") {
            cq->out=q.substr(pos+4,max-4);
        } else if(max>=4 && q.substr(pos,4)=="sig:") {
            cq->sig=q.substr(pos+4,max-4);
        } else if(max>=16 && q.substr(pos,16)=="responseHandler:") {
            cq->responseHandler=q.substr(pos+16,max-16);
        } else if(max>=12 && q.substr(pos,12)=="outFileName:") {
            cq->outFileName=q.substr(pos+12,max-12);
        }
        pos=next+1;
    }
}

/// Parse the TQ parameter (the actual query)
static void parseTQ(CgiQuery *cq, const std::string &query) 
{
    cq->query=parseUrlFormat(query);
}

void CgiQuery::parseQuery()
{
    // need to split along '&', ignore the ';' as those are used as part
    // of tqx key/value pairs

    std::string tq;
    std::string tqx;

    const char *qenv=getenv("QUERY_STRING");
    if(qenv==nullptr) {
        std::cerr << "CGI query not found (QUERY_STRING env variable not set)" << std::endl;
        exit(1);
    }
    LOG(INFO) << "QUERY: " << qenv;
    std::string query_string=qenv;

    size_t pos=0;
    while(pos<query_string.length()) {
        auto next=query_string.find("&",pos);
        if(next==std::string::npos) {
            next=query_string.length();
        }
        if(query_string.substr(pos,4)=="tqx=") {
            parseTQX(this,query_string.substr(pos+4,next-pos-4));
        } else if(query_string.substr(pos,3)=="tq=") {
            parseTQ(this,query_string.substr(pos+3,next-pos-3));
        }
        pos=next+1;
    }
}

/// Return true for characters that do not need to be encoded in URL's
static inline bool isValidChar(char c)
{
    return (c>='0'&&c<='9')||(c>='A'&&c<='Z')
          ||(c>='a'&&c<='z')||c=='.'||c=='_'
          ||c=='-';
} 

/// Convert a string to URL encoded format
static std::string encodePercent(const std::string &q)
{
    std::string res;
    for(auto c:q) {
        if(isValidChar(c)) {
            res+=c;
        } else {
            static char buf[5];
            sprintf(buf,"%%%02X",c);
            res+=buf;
        }
    }
    return res;

}

/// Handle the CGI query and send the data to stdout
void GQL_SQL::DBQuery::handleCgi(GQL_SQL::DBQuery::DB::Ptr db)
{
    CgiQuery q;

    Json::Value r;
    db->execute(q.query,r);
    std::cout << "Cache-Control: no-cache, no-store, max-age=0, must-revalidate\r\n";
    std::cout << "X-Content-Type-Options: nosniff\r\n";
    std::cout << "X-Robots-Tag: noindex, nofollow, nosnippet\r\n";
    if(q.out=="html") {
        std::cout << "Content-type: text/html; charset=utf-8\r\n\r\n";
        // FIXME: UTF8 would depend on the db, but for now no support for other encodings exists
        outputHtml(std::cout,db->deftable(),r);
    } else if(q.out=="tsv"||q.out=="tsv-excel") {
        if(q.outFileName=="") { q.outFileName="data.tsv"; }
        std::cout << "Content-Disposition: attachment; filename=\"" << encodePercent(q.outFileName)
                  << "\"; filename*=UTF-8''" << encodePercent(q.outFileName) << "\r\n";
        std::cout << "Content-Type: text/tab-separated-values; charset=utf-16\r\n\r\n";
        outputTsv(std::cout,r);
    } else if(q.out=="csv") {
        if(q.outFileName=="") { q.outFileName="data.csv"; }
        std::cout << "Content-Disposition: attachment; filename=\"" << encodePercent(q.outFileName)
                  << "\"; filename*=UTF-8''" << encodePercent(q.outFileName) << "\r\n";
        std::cout << "Content-type: text/csv; charset=utf-8\r\n\r\n";
        outputCsv(std::cout,r);
    } else {
        try {
            r["reqId"]=std::stoll(q.reqId);
        } catch(const std::exception &) {
            r["reqId"]=0;
        }
        if(q.outFileName=="") { q.outFileName="json.txt"; }
        if(q.responseHandler=="") { q.responseHandler="google.visualization.Query.setResponse"; }
        std::cout << "Content-Disposition: attachment; filename=\"" << encodePercent(q.outFileName)
                  << "\"; filename*=UTF-8''" << encodePercent(q.outFileName) << "\r\n";
        std::cout << "Content-type: application/javascript; charset=utf-8\r\n\r\n";
        std::cout << "/*O_o*/\n" << q.responseHandler << "(";
        outputJson(std::cout,r);
        std::cout << ");";
    }
}
