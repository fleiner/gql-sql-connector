#include <gtest/gtest.h>

#include "libgqlsql.h"


TEST(OnExit, Loop) {
    uint32_t cnt=0;
    {
        for(int i=0;i<10;i++) {
            //EXPECT_EQ(i/2,cnt) << "i=" << i;
            const char *p=(i%2)?"hello":0;
            ON_EXIT(if(p) { cnt++; });
            if(i%2) {
                EXPECT_TRUE(p);
            } else {
                EXPECT_FALSE(p);
            }
        }
        EXPECT_EQ(5,cnt);
    }
}

TEST(OnExit, Block) {
    uint32_t cnt=0;
    {
        ON_EXIT(cnt++;);
        {
            EXPECT_EQ(0,cnt);
            ON_EXIT(cnt++;);
            {
                EXPECT_EQ(0,cnt);
                ON_EXIT(cnt++;);
                {
                    EXPECT_EQ(0,cnt);
                    ON_EXIT(cnt++;);
                    EXPECT_EQ(0,cnt);
                }
                EXPECT_EQ(1,cnt);
            }
            EXPECT_EQ(2,cnt);
        }
        EXPECT_EQ(3,cnt);
    }
    EXPECT_EQ(4,cnt);
}



int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
