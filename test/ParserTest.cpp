#include <gtest/gtest.h>

#include "libgqlsql.h"

TEST (Parser, Synatx) { 
    GQL_SQL::GQLParser::Query::parse("select *");
    GQL_SQL::GQLParser::Query::parse("select dept");
    GQL_SQL::GQLParser::Query::parse("select dept, sum(salary) group by dept");
    GQL_SQL::GQLParser::Query::parse("select A, sum(B) group by A");
    GQL_SQL::GQLParser::Query::parse("select dept, salary");
    GQL_SQL::GQLParser::Query::parse("select max(salary)");
    GQL_SQL::GQLParser::Query::parse("select `email address`, name, `date`");
    GQL_SQL::GQLParser::Query::parse("select lunchTime, name");
    GQL_SQL::GQLParser::Query::parse("where name contains 'John'");
    GQL_SQL::GQLParser::Query::parse("where dept starts with 'e'");
    GQL_SQL::GQLParser::Query::parse("where role ends with 'y'");
    GQL_SQL::GQLParser::Query::parse("where country matches '.*ia'");
    GQL_SQL::GQLParser::Query::parse("where name like 'fre%'");
    GQL_SQL::GQLParser::Query::parse("where salary >= 600");
    GQL_SQL::GQLParser::Query::parse("where dept != 'Eng' and date '2005-01-21' < hireDate");
    GQL_SQL::GQLParser::Query::parse("where (dept<>'Eng' and isSenior=true) or (dept='Sales') or seniorityStartTime is null");
    GQL_SQL::GQLParser::Query::parse("select name where salary > 700");
    GQL_SQL::GQLParser::Query::parse("select dept, max(salary) group by dept");
    GQL_SQL::GQLParser::Query::parse("select lunchTime, avg(salary), count(age) group by isSenior,lunchTime");
    GQL_SQL::GQLParser::Query::parse("select sum(salary) pivot dept");
    GQL_SQL::GQLParser::Query::parse("select dept, sum(salary) group by dept pivot lunchTime");
    GQL_SQL::GQLParser::Query::parse("select lunchTime, sum(salary) group by lunchTime pivot dept");
    GQL_SQL::GQLParser::Query::parse("select sum(salary) pivot dept, lunchTime");
    GQL_SQL::GQLParser::Query::parse("select sum(salary), max(lunchTime) pivot dept");
    GQL_SQL::GQLParser::Query::parse("order by dept, salary desc");
    GQL_SQL::GQLParser::Query::parse("select dept, max(salary) group by dept order by max(salary)");
    GQL_SQL::GQLParser::Query::parse("limit 100");
    GQL_SQL::GQLParser::Query::parse("offset 10");
    GQL_SQL::GQLParser::Query::parse("limit 30 offset 210 ");
    GQL_SQL::GQLParser::Query::parse("label dept 'Department', name \"Employee Name\", location 'Employee Location'");
    GQL_SQL::GQLParser::Query::parse("format salary '#,##0.00', hireDate 'dd-MMM-yyyy', isSenior 'Yes!:Not yet'");
    GQL_SQL::GQLParser::Query::parse("select upper(name), year(startDate)");
    GQL_SQL::GQLParser::Query::parse("select empSalary - empTax");
    GQL_SQL::GQLParser::Query::parse("select 2 * (max(empSalary) / max(empTax)) ");
    GQL_SQL::GQLParser::Query::parse("");

}

TEST (Parser, Errors) { 
    EXPECT_THROW(GQL_SQL::GQLParser::Query::parse("select select"),GQL_SQL::GQLParser::SyntaxError);
    EXPECT_THROW(GQL_SQL::GQLParser::Query::parse("max(year(startDate)) "),GQL_SQL::GQLParser::SyntaxError);
    EXPECT_THROW(GQL_SQL::GQLParser::Query::parse("sum(salary + perks) "),GQL_SQL::GQLParser::SyntaxError);
}

int main(int argc, char **argv) {
      ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}
