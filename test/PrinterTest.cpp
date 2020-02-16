#include <gtest/gtest.h>

#include "libgqlsql.h"

TEST (Printer, GQL) { 
    auto p=new GQL_SQL::GQLParser::ParserGQL("Master");

#define T(s,e) EXPECT_TRUE(p->parse(s)) << p->res().errormsg; EXPECT_EQ(e,p->res().result); EXPECT_TRUE(p->parse(e)); EXPECT_EQ(e,p->res().result);

    T("select *","SELECT *")
    T("select 2 * (max(empSalary) / max(empTax)) where id<20","SELECT (2*(max(empSalary)/max(empTax))) WHERE (id<20)");
    T("select upper(name), year(startDate)","SELECT upper(name), year(startDate)")
    T("select * Where year(asd)>2000","SELECT * WHERE (year(asd)>2000)")
    T("select A,B+C LABEL A 'hello',B+C 'there'","SELECT A, (B+C) LABEL A 'hello', (B+C) 'there'")
    T("select * LABEL A 'hello',B 'there'","SELECT * LABEL A 'hello', B 'there'")
    T("select year(a),month(a),day(a),(hour(a)*60+minute(a))*60+second(a),millisecond(a),quarter(a),dayOfWeek(a)",
      "SELECT year(a), month(a), day(a), ((((hour(a)*60)+minute(a))*60)+second(a)), millisecond(a), quarter(a), dayOfWeek(a)")
    T("LABEL A 'hello',B 'there'","SELECT * LABEL A 'hello', B 'there'")
    T("select * where name <= 'm' and `A B`>12 and not d < date \"1980-01-01\" group by `9i`, ``,`a.o` piVOT abc,def ORDER BY max(A),B/a+4*CCD LIMIT 6 OFFSET 9 LABEL A 'hello',B 'there' FORMAT A+B '###000',`c c` 'yy-dd' options no_values",
      "SELECT * WHERE (((name<='m') and (`A B`>12)) and (not (d<date '1980-01-01'))) GROUP BY `9i`, ``, `a.o` PIVOT abc, def ORDER BY max(A), ((B/a)+(4*CCD)) LIMIT 6 OFFSET 9 LABEL A 'hello', B 'there' FORMAT (A+B) '###000', `c c` 'yy-dd' OPTIONS no_values")
    T("SELECT last,max(xx) GROUP by last","SELECT last, max(xx) GROUP BY last")
    T("SELECT last,max(xx) GROUP by last PIVOT dept","SELECT last, max(xx) GROUP BY last PIVOT dept")

    p=new GQL_SQL::GQLParser::ParserGQL("Master",std::set<std::string>(),true);
    T("select log(c1)","SELECT log(c1)");
    
}

TEST (Printer, Error) {
    auto p=new GQL_SQL::GQLParser::ParserGQL("Master");

#define E(s,e) EXPECT_FALSE(p->parse(s)); EXPECT_EQ(e,p->res().errormsg);

    E("selct *","invalid_query: pos 1: expected the end of statement but got 'selct' instead")
    E("select not(12)","invalid_query: function 'not' is not known");
    E("select day(12,34)","invalid_query: function 'day' takes 1 argument(s), but got 2");
    E("select day(12,34),*","invalid_query: pos 19: unexpected token '*'");
    E("select log(c1)","invalid_query: function 'log' is not known");
    E("select * order by a order by b","invalid_query: pos 21: expected the end of statement but got 'order' instead");
 
}

int main(int argc, char **argv) {
      ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}

