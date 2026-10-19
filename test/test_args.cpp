////////////////////////////////////////////////////////////////////////////////
#include "pgm/args.hpp"

#include <deque>
#include <initializer_list>
#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <vector>

class environment
{
    std::deque<std::string> args_;
    std::vector<char*> argv_{ nullptr };

public:
    int argc = 0;
    char** argv = argv_.data();

    environment(std::initializer_list<std::string_view> args) { for (auto&& arg : args) add_arg(arg); }

    void add_arg(std::string_view arg)
    {
        args_.emplace_back(arg);
        argv_.back() = args_.back().data();
        argv_.push_back(nullptr);

        argc = args_.size();
        argv = argv_.data();
    }

    void add_arg(std::string_view name, std::string_view value) {
        add_arg(name); add_arg(value);
    }

    void add_arg_multi(std::string_view name, std::initializer_list<std::string_view> values) {
        for (auto value : values) add_arg(name, value);
    }
};

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
    environment env{ "pgm" };
    EXPECT_THROW({ args.parse(env.argc, env.argv); }, pgm::missing_argument);
}

TEST_F(params_0, not_enough_1)
{
    environment env{ "pgm", "p1" };
    EXPECT_THROW({ args.parse(env.argc, env.argv); }, pgm::missing_argument);
}

TEST_F(params_0, not_enough_2)
{
    environment env{ "pgm", "p1", "p3" };
    EXPECT_THROW({ args.parse(env.argc, env.argv); }, pgm::missing_argument);
}

TEST_F(params_0, req_only)
{
    environment env{ "pgm", "p1", "p3", "p5" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ  (args["p1"].value(), "p1");
    EXPECT_TRUE(args["p2"].empty());
    EXPECT_EQ  (args["p3"].value(), "p3");
    EXPECT_TRUE(args["p4"].empty());
    EXPECT_EQ  (args["p5"].value(), "p5");
}

TEST_F(params_0, opt_1)
{
    environment env{ "pgm", "p1", "p2", "p3", "p5" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ  (args["p1"].value(), "p1");
    EXPECT_EQ  (args["p2"].value(), "p2");
    EXPECT_EQ  (args["p3"].value(), "p3");
    EXPECT_TRUE(args["p4"].empty());
    EXPECT_EQ  (args["p5"].value(), "p5");
}

TEST_F(params_0, all)
{
    environment env{ "pgm", "p1", "p2", "p3", "p4", "p5" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

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
    environment env{ "pgm" };
    EXPECT_THROW({ args.parse(env.argc, env.argv); }, pgm::missing_argument);
}

TEST_F(params_1, not_enough_1)
{
    environment env{ "pgm", "p2" };
    EXPECT_THROW({ args.parse(env.argc, env.argv); }, pgm::missing_argument);
}

TEST_F(params_1, req_only)
{
    environment env{ "pgm", "p2", "p4" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_TRUE(args["p1"].empty());
    EXPECT_EQ  (args["p2"].value(), "p2");
    EXPECT_TRUE(args["p3"].empty());
    EXPECT_EQ  (args["p4"].value(), "p4");
    EXPECT_TRUE(args["p5"].empty());
}

TEST_F(params_1, opt_1)
{
    environment env{ "pgm", "p1", "p2", "p4" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ  (args["p1"].value(), "p1");
    EXPECT_EQ  (args["p2"].value(), "p2");
    EXPECT_TRUE(args["p3"].empty());
    EXPECT_EQ  (args["p4"].value(), "p4");
    EXPECT_TRUE(args["p5"].empty());
}

TEST_F(params_1, opt_2)
{
    environment env{ "pgm", "p1", "p2", "p3", "p4" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ  (args["p1"].value(), "p1");
    EXPECT_EQ  (args["p2"].value(), "p2");
    EXPECT_EQ  (args["p3"].value(), "p3");
    EXPECT_EQ  (args["p4"].value(), "p4");
    EXPECT_TRUE(args["p5"].empty());
}

TEST_F(params_1, all)
{
    environment env{ "pgm", "p1", "p2", "p3", "p4", "p5" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["p1"].value(), "p1");
    EXPECT_EQ(args["p2"].value(), "p2");
    EXPECT_EQ(args["p3"].value(), "p3");
    EXPECT_EQ(args["p4"].value(), "p4");
    EXPECT_EQ(args["p5"].value(), "p5");
}

TEST_F(params_1, mul_1)
{
    environment env{ "pgm", "p1.0", "p1.1", "p2", "p3", "p4", "p5" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["p1"].count( ), 2);
    EXPECT_EQ(args["p1"].value(0), "p1.0");
    EXPECT_EQ(args["p1"].value(1), "p1.1");
    EXPECT_EQ(args["p2"].value( ), "p2");
    EXPECT_EQ(args["p3"].value( ), "p3");
    EXPECT_EQ(args["p4"].value( ), "p4");
    EXPECT_EQ(args["p5"].value( ), "p5");
}

TEST_F(params_1, mul_3)
{
    environment env{ "pgm", "p1.0", "p1.1", "p1.2", "p1.3", "p2", "p3", "p4", "p5" };
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
