#include <gtest/gtest.h>

#include "libgqlsql.h"

using GQL_SQL::GQLParser::Token;
using GQL_SQL::GQLParser::TokenType;
using GQL_SQL::GQLParser::Tokenizer;


TEST (Tokenizer, KeywordCases) { 
    auto t=Tokenizer("and Or pIVot    order\t\nselect");
    EXPECT_EQ(Token(1,TokenType::AND,"and"),t.next());
    EXPECT_EQ(Token(1,TokenType::AND,"and"),t.current());
    EXPECT_EQ(Token(5,TokenType::OR,"or"),t.next());
    EXPECT_EQ(Token(8,TokenType::PIVOT,"pivot"),t.next());
    EXPECT_EQ(Token(17,TokenType::ORDER,"order"),t.next());
    EXPECT_EQ(Token(24,TokenType::SELECT,"select"),t.next());
    EXPECT_EQ(Token(30,TokenType::EOL,""),t.next());
    EXPECT_EQ(Token(30,TokenType::EOL,""),t.next());
    EXPECT_EQ(Token(30,TokenType::EOL,""),t.next());
}

TEST (Tokenizer, Idents) { 
    auto t=Tokenizer(" hello \t\tthere how do you  do");
    EXPECT_EQ(Token(2,TokenType::IDENTIFIER,"hello"),t.next());
    EXPECT_EQ(Token(10,TokenType::IDENTIFIER,"there"),t.next());
    EXPECT_EQ(Token(16,TokenType::IDENTIFIER,"how"),t.next());
    EXPECT_EQ(Token(20,TokenType::IDENTIFIER,"do"),t.next());
    EXPECT_EQ(Token(23,TokenType::IDENTIFIER,"you"),t.next());
    EXPECT_EQ(Token(28,TokenType::IDENTIFIER,"do"),t.next());
    EXPECT_EQ(Token(30,TokenType::EOL,""),t.next());
}

TEST (Tokenizer, Expression) { 
    auto t=Tokenizer("4.5+-5*(23<=4=<4)");
    EXPECT_EQ(Token(1,TokenType::NUMBER,"4.5"),t.next());
    EXPECT_EQ(Token(4,TokenType::PLUS,"+"),t.next());
    EXPECT_EQ(Token(5,TokenType::MINUS,"-"),t.next());
    EXPECT_EQ(Token(6,TokenType::NUMBER,"5"),t.next());
    EXPECT_EQ(Token(7,TokenType::TIMES,"*"),t.next());
    EXPECT_EQ(Token(8,TokenType::P_OPEN,"("),t.next());
    EXPECT_EQ(Token(9,TokenType::NUMBER,"23"),t.next());
    EXPECT_EQ(Token(11,TokenType::LE,"<="),t.next());
    EXPECT_EQ(Token(13,TokenType::NUMBER,"4"),t.next());
    EXPECT_EQ(Token(14,TokenType::EQ,"="),t.next());
    EXPECT_EQ(Token(15,TokenType::LT,"<"),t.next());
    EXPECT_EQ(Token(16,TokenType::NUMBER,"4"),t.next());
    EXPECT_EQ(Token(17,TokenType::P_CLOSE,")"),t.next());
    EXPECT_EQ(Token(18,TokenType::EOL,""),t.next());
}

TEST (Tokenizer, Strings) { 
    auto t=Tokenizer(" 'hello \"you\"' \"and\n\tsome\" `'\" `");
    EXPECT_EQ(Token(2,TokenType::STRING,"hello \"you\""),t.next());
    EXPECT_EQ(Token(16,TokenType::STRING,"and\n\tsome"),t.next());
    EXPECT_EQ(Token(28,TokenType::IDENTIFIER,"'\" "),t.next());
    EXPECT_EQ(Token(33,TokenType::EOL,""),t.next());
}

int main(int argc, char **argv) {
      ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}
