////////////////////////////////////////////////////////////////////////////////
#include "pgm/args.hpp"
#include "test_env.hpp"

#include <gtest/gtest.h>

////////////////////////////////////////////////////////////////////////////////
struct params : testing::Test
{
    pgm::args args
    {
        { "p1", "" },
        { "p2", pgm::opt, "" },
        { "p3", "" },
        { "p4", pgm::opt, "" },
        { "p5", "" },
    };
};

TEST_F(params, not_enough_0)
{
    env env{ "pgm" };
    EXPECT_THROW({ args.parse(env.argc, env.argv); }, pgm::missing_argument);
}

TEST_F(params, not_enough_1)
{
    env env{ "pgm", "p1" };
    EXPECT_THROW({ args.parse(env.argc, env.argv); }, pgm::missing_argument);
}

TEST_F(params, not_enough_2)
{
    env env{ "pgm", "p1", "p3" };
    EXPECT_THROW({ args.parse(env.argc, env.argv); }, pgm::missing_argument);
}

TEST_F(params, req_only)
{
    env env{ "pgm", "p1", "p3", "p5" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ  (args["p1"].value(), "p1");
    EXPECT_TRUE(args["p2"].empty());
    EXPECT_EQ  (args["p3"].value(), "p3");
    EXPECT_TRUE(args["p4"].empty());
    EXPECT_EQ  (args["p5"].value(), "p5");
}

TEST_F(params, opt_1)
{
    env env{ "pgm", "p1", "p2", "p3", "p5" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ  (args["p1"].value(), "p1");
    EXPECT_EQ  (args["p2"].value(), "p2");
    EXPECT_EQ  (args["p3"].value(), "p3");
    EXPECT_TRUE(args["p4"].empty());
    EXPECT_EQ  (args["p5"].value(), "p5");
}

TEST_F(params, all)
{
    env env{ "pgm", "p1", "p2", "p3", "p4", "p5" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["p1"].value(), "p1");
    EXPECT_EQ(args["p2"].value(), "p2");
    EXPECT_EQ(args["p3"].value(), "p3");
    EXPECT_EQ(args["p4"].value(), "p4");
    EXPECT_EQ(args["p5"].value(), "p5");
}

////////////////////////////////////////////////////////////////////////////////
struct params_mul : public testing::Test
{
    pgm::args args
    {
        { "p1", pgm::opt | pgm::mul, "" },
        { "p2", "" },
        { "p3", pgm::opt, "" },
        { "p4", "" },
        { "p5", pgm::opt, "" },
    };
};

TEST_F(params_mul, not_enough_0)
{
    env env{ "pgm" };
    EXPECT_THROW({ args.parse(env.argc, env.argv); }, pgm::missing_argument);
}

TEST_F(params_mul, not_enough_1)
{
    env env{ "pgm", "p2" };
    EXPECT_THROW({ args.parse(env.argc, env.argv); }, pgm::missing_argument);
}

TEST_F(params_mul, req_only)
{
    env env{ "pgm", "p2", "p4" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_TRUE(args["p1"].empty());
    EXPECT_EQ  (args["p2"].value(), "p2");
    EXPECT_TRUE(args["p3"].empty());
    EXPECT_EQ  (args["p4"].value(), "p4");
    EXPECT_TRUE(args["p5"].empty());
}

TEST_F(params_mul, opt_1)
{
    env env{ "pgm", "p1", "p2", "p4" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ  (args["p1"].value(), "p1");
    EXPECT_EQ  (args["p2"].value(), "p2");
    EXPECT_TRUE(args["p3"].empty());
    EXPECT_EQ  (args["p4"].value(), "p4");
    EXPECT_TRUE(args["p5"].empty());
}

TEST_F(params_mul, opt_2)
{
    env env{ "pgm", "p1", "p2", "p3", "p4" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ  (args["p1"].value(), "p1");
    EXPECT_EQ  (args["p2"].value(), "p2");
    EXPECT_EQ  (args["p3"].value(), "p3");
    EXPECT_EQ  (args["p4"].value(), "p4");
    EXPECT_TRUE(args["p5"].empty());
}

TEST_F(params_mul, all)
{
    env env{ "pgm", "p1", "p2", "p3", "p4", "p5" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["p1"].value(), "p1");
    EXPECT_EQ(args["p2"].value(), "p2");
    EXPECT_EQ(args["p3"].value(), "p3");
    EXPECT_EQ(args["p4"].value(), "p4");
    EXPECT_EQ(args["p5"].value(), "p5");
}

TEST_F(params_mul, mul_1)
{
    env env{ "pgm", "p1.0", "p1.1", "p2", "p3", "p4", "p5" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["p1"].count( ), 2);
    EXPECT_EQ(args["p1"].value(0), "p1.0");
    EXPECT_EQ(args["p1"].value(1), "p1.1");
    EXPECT_EQ(args["p2"].value( ), "p2");
    EXPECT_EQ(args["p3"].value( ), "p3");
    EXPECT_EQ(args["p4"].value( ), "p4");
    EXPECT_EQ(args["p5"].value( ), "p5");
}

TEST_F(params_mul, mul_3)
{
    env env{ "pgm", "p1.0", "p1.1", "p1.2", "p1.3", "p2", "p3", "p4", "p5" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["p1"].count( ), 4);
    EXPECT_EQ(args["p1"].value(0), "p1.0");
    EXPECT_EQ(args["p1"].value(1), "p1.1");
    EXPECT_EQ(args["p1"].value(2), "p1.2");
    EXPECT_EQ(args["p1"].value(3), "p1.3");
    EXPECT_EQ(args["p2"].value( ), "p2");
    EXPECT_EQ(args["p3"].value( ), "p3");
    EXPECT_EQ(args["p4"].value( ), "p4");
    EXPECT_EQ(args["p5"].value( ), "p5");
}
