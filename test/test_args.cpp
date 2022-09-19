////////////////////////////////////////////////////////////////////////////////
#include "pgm/args.hpp"

#include <gtest/gtest.h>
#include <tuple>

template<int N>
auto to_argcv(const char* const (&args)[N])
{
    return std::make_tuple(N, const_cast<char**>(args));
}

////////////////////////////////////////////////////////////////////////////////
struct params_0 : testing::Test
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

TEST_F(params_0, not_enough_0)
{
    auto [argc, argv] = to_argcv({ "pgm" });
    EXPECT_THROW({ args.parse(argc, argv); }, pgm::missing_argument);
}

TEST_F(params_0, not_enough_1)
{
    auto [argc, argv] = to_argcv({ "pgm", "p1" });
    EXPECT_THROW({ args.parse(argc, argv); }, pgm::missing_argument);
}

TEST_F(params_0, not_enough_2)
{
    auto [argc, argv] = to_argcv({ "pgm", "p1", "p3" });
    EXPECT_THROW({ args.parse(argc, argv); }, pgm::missing_argument);
}

TEST_F(params_0, req_only)
{
    auto [argc, argv] = to_argcv({ "pgm", "p1", "p3", "p5" });

    EXPECT_NO_THROW({ args.parse(argc, argv); });
    EXPECT_EQ  (args["p1"].value(), "p1");
    EXPECT_TRUE(args["p2"].empty());
    EXPECT_EQ  (args["p3"].value(), "p3");
    EXPECT_TRUE(args["p4"].empty());
    EXPECT_EQ  (args["p5"].value(), "p5");
}

TEST_F(params_0, opt_1)
{
    auto [argc, argv] = to_argcv({ "pgm", "p1", "p2", "p3", "p5" });

    EXPECT_NO_THROW({ args.parse(argc, argv); });
    EXPECT_EQ  (args["p1"].value(), "p1");
    EXPECT_EQ  (args["p2"].value(), "p2");
    EXPECT_EQ  (args["p3"].value(), "p3");
    EXPECT_TRUE(args["p4"].empty());
    EXPECT_EQ  (args["p5"].value(), "p5");
}

TEST_F(params_0, all)
{
    auto [argc, argv] = to_argcv({
        "pgm", "p1", "p2", "p3", "p4", "p5"
    });

    EXPECT_NO_THROW({ args.parse(argc, argv); });
    EXPECT_EQ(args["p1"].value(), "p1");
    EXPECT_EQ(args["p2"].value(), "p2");
    EXPECT_EQ(args["p3"].value(), "p3");
    EXPECT_EQ(args["p4"].value(), "p4");
    EXPECT_EQ(args["p5"].value(), "p5");
}

////////////////////////////////////////////////////////////////////////////////
struct params_1 : public testing::Test
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

TEST_F(params_1, not_enough_0)
{
    auto [argc, argv] = to_argcv({ "pgm" });
    EXPECT_THROW({ args.parse(argc, argv); }, pgm::missing_argument);
}

TEST_F(params_1, not_enough_1)
{
    auto [argc, argv] = to_argcv({ "pgm", "p2" });
    EXPECT_THROW({ args.parse(argc, argv); }, pgm::missing_argument);
}

TEST_F(params_1, req_only)
{
    auto [argc, argv] = to_argcv({ "pgm", "p2", "p4" });

    EXPECT_NO_THROW({ args.parse(argc, argv); });
    EXPECT_TRUE(args["p1"].empty());
    EXPECT_EQ  (args["p2"].value(), "p2");
    EXPECT_TRUE(args["p3"].empty());
    EXPECT_EQ  (args["p4"].value(), "p4");
    EXPECT_TRUE(args["p5"].empty());
}

TEST_F(params_1, opt_1)
{
    auto [argc, argv] = to_argcv({ "pgm", "p1", "p2", "p4" });

    EXPECT_NO_THROW({ args.parse(argc, argv); });
    EXPECT_EQ  (args["p1"].value(), "p1");
    EXPECT_EQ  (args["p2"].value(), "p2");
    EXPECT_TRUE(args["p3"].empty());
    EXPECT_EQ  (args["p4"].value(), "p4");
    EXPECT_TRUE(args["p5"].empty());
}

TEST_F(params_1, opt_2)
{
    auto [argc, argv] = to_argcv({ "pgm", "p1", "p2", "p3", "p4" });

    EXPECT_NO_THROW({ args.parse(argc, argv); });
    EXPECT_EQ  (args["p1"].value(), "p1");
    EXPECT_EQ  (args["p2"].value(), "p2");
    EXPECT_EQ  (args["p3"].value(), "p3");
    EXPECT_EQ  (args["p4"].value(), "p4");
    EXPECT_TRUE(args["p5"].empty());
}

TEST_F(params_1, all)
{
    auto [argc, argv] = to_argcv({
        "pgm", "p1", "p2", "p3", "p4", "p5"
    });

    EXPECT_NO_THROW({ args.parse(argc, argv); });
    EXPECT_EQ(args["p1"].value(), "p1");
    EXPECT_EQ(args["p2"].value(), "p2");
    EXPECT_EQ(args["p3"].value(), "p3");
    EXPECT_EQ(args["p4"].value(), "p4");
    EXPECT_EQ(args["p5"].value(), "p5");
}

TEST_F(params_1, mul_1)
{
    auto [argc, argv] = to_argcv({
        "pgm", "p1.0", "p1.1", "p2", "p3", "p4", "p5"
    });

    EXPECT_NO_THROW({ args.parse(argc, argv); });
    EXPECT_EQ(args["p1"].count(), 2);
    EXPECT_EQ(args["p1"].value(0), "p1.0");
    EXPECT_EQ(args["p1"].value(1), "p1.1");
    EXPECT_EQ(args["p2"].value(), "p2");
    EXPECT_EQ(args["p3"].value(), "p3");
    EXPECT_EQ(args["p4"].value(), "p4");
    EXPECT_EQ(args["p5"].value(), "p5");
}

TEST_F(params_1, mul_3)
{
    auto [argc, argv] = to_argcv({
        "pgm", "p1.0", "p1.1", "p1.2", "p1.3", "p2", "p3", "p4", "p5"
    });

    EXPECT_NO_THROW({ args.parse(argc, argv); });
    EXPECT_EQ(args["p1"].count(), 4);
    EXPECT_EQ(args["p1"].value(0), "p1.0");
    EXPECT_EQ(args["p1"].value(1), "p1.1");
    EXPECT_EQ(args["p1"].value(2), "p1.2");
    EXPECT_EQ(args["p1"].value(3), "p1.3");
    EXPECT_EQ(args["p2"].value(), "p2");
    EXPECT_EQ(args["p3"].value(), "p3");
    EXPECT_EQ(args["p4"].value(), "p4");
    EXPECT_EQ(args["p5"].value(), "p5");
}
