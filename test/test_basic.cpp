////////////////////////////////////////////////////////////////////////////////
#include "pgm/args.hpp"
#include "test_env.hpp"

#include <gtest/gtest.h>

////////////////////////////////////////////////////////////////////////////////
struct options_basic : testing::Test
{
    pgm::args args
    {
        { "-h", "--help",    "" },
        { "-q", "--quiet",   "" },
        { "-V", "--version", "" },
        { "-v", "--verbose", "" },
    };
};

TEST_F(options_basic, none)
{
    env env{ "pgm" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_FALSE(args["-v"]);
    EXPECT_FALSE(args["--quiet"]);
    EXPECT_FALSE(args["-h"]);
    EXPECT_FALSE(args["--version"]);
}

TEST_F(options_basic, short_only)
{
    env env{ "pgm", "-v", "-h" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_TRUE (args["-v"]);
    EXPECT_FALSE(args["--quiet"]);
    EXPECT_TRUE (args["-h"]);
    EXPECT_FALSE(args["--version"]);

    EXPECT_EQ(args["-v"].count(), 1);
    EXPECT_EQ(args["-h"].value(), "");
}

TEST_F(options_basic, long_only)
{
    env env{ "pgm", "--verbose", "--version" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_TRUE (args["--verbose"]);
    EXPECT_FALSE(args["-q"]);
    EXPECT_FALSE(args["-h"]);
    EXPECT_TRUE (args["--version"]);
}

TEST_F(options_basic, mixed)
{
    env env{ "pgm", "-q", "--help", "-V" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_FALSE(args["-v"]);
    EXPECT_TRUE (args["--quiet"]);
    EXPECT_TRUE (args["-h"]);
    EXPECT_TRUE (args["--version"]);
}

TEST_F(options_basic, grouped)
{
    env env{ "pgm", "-vhV" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_TRUE (args["-v"]);
    EXPECT_FALSE(args["-q"]);
    EXPECT_TRUE (args["-h"]);
    EXPECT_TRUE (args["-V"]);
}

TEST_F(options_basic, unrecognized)
{
    env env1{ "pgm", "--unknown" };
    EXPECT_THROW({ args.parse(env1.argc, env1.argv); }, pgm::invalid_argument);

    env env2{ "pgm", "-x" };
    EXPECT_THROW({ args.parse(env2.argc, env2.argv); }, pgm::invalid_argument);
}

TEST_F(options_basic, duplicate)
{
    env env{ "pgm", "-v", "--verbose" };
    EXPECT_THROW({ args.parse(env.argc, env.argv); }, pgm::invalid_argument);
}

////////////////////////////////////////////////////////////////////////////////
struct options_values : testing::Test
{
    pgm::args args
    {
        {       "--config", "PATH", "" },
        { "-i", "--input",  "FILE", "" },
        { "-n",             "NUM",  "" },
        { "-o", "--output", "FILE", "" },
        { "param", pgm::opt,        "" },
    };
};

TEST_F(options_values, separate)
{
    env env{ "pgm", "-o", "out", "--input", "in" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ   (args["-o"].value(),     "out");
    EXPECT_EQ   (args["--input"].value(), "in");
    EXPECT_FALSE(args["-n"]);
    EXPECT_FALSE(args["--config"]);
}

TEST_F(options_values, attached_short)
{
    env env{ "pgm", "-oout", "-n42" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["-o"].value(), "out");
    EXPECT_EQ(args["-n"].value(), "42");
}

TEST_F(options_values, long_equals)
{
    env env{ "pgm", "--output=out", "--config=/etc/foo" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["--output"].value(), "out");
    EXPECT_EQ(args["--config"].value(), "/etc/foo");
}

TEST_F(options_values, missing_value)
{
    env env1{ "pgm", "-o" };
    EXPECT_THROW({ args.parse(env1.argc, env1.argv); }, pgm::missing_argument);

    env env2{ "pgm", "--input" };
    EXPECT_THROW({ args.parse(env2.argc, env2.argv); }, pgm::missing_argument);
}

TEST_F(options_values, value_looks_like_option)
{
    env env{ "pgm", "-o", "-n", "42" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ   (args["-o"].value(), "-n");
    EXPECT_FALSE(args["-n"]);
    EXPECT_EQ   (args["param"].value(), "42");
}

////////////////////////////////////////////////////////////////////////////////
struct options_optval : testing::Test
{
    pgm::args args
    {
        { "-c", "--color", "WHEN", pgm::optval, "" },
        { "-d",            "WHAT", pgm::optval, "" },
    };
};

TEST_F(options_optval, present)
{
    env env{ "pgm", "--color", "auto", "-d3" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["--color"].value(), "auto");
    EXPECT_EQ(args["-d"].value(), "3");
}

TEST_F(options_optval, omitted)
{
    env env{ "pgm", "-c" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_TRUE (args["-c"]);
    EXPECT_EQ   (args["-c"].value(), "");
    EXPECT_FALSE(args["-d"]);
}

TEST_F(options_optval, omitted_long)
{
    env env{ "pgm", "--color" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_TRUE(args["--color"]);
    EXPECT_EQ  (args["--color"].value(), "");
}

TEST_F(options_optval, next_is_option)
{
    env env{ "pgm", "-c", "-d" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["-c"].value(), "");
    EXPECT_EQ(args["-d"].value(), "");
}

////////////////////////////////////////////////////////////////////////////////
struct options_mul : testing::Test
{
    pgm::args args
    {
        { "-D", "DEFINE",           pgm::mul, "" },
        { "-I", "--include", "DIR", pgm::mul, "" },
        { "-v", "--verbose",        pgm::mul, "" },
    };
};

TEST_F(options_mul, multiple)
{
    env env{ "pgm", "-I", "abc", "-Idef", "--include=ghi", "-D", "FOO", "-D", "BAR" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["-I"].count( ), 3);
    EXPECT_EQ(args["-I"].value(0), "abc");
    EXPECT_EQ(args["-I"].value(1), "def");
    EXPECT_EQ(args["-I"].value(2), "ghi");

    EXPECT_EQ(args["-D"].count( ), 2);
    EXPECT_EQ(args["-D"].value(0), "FOO");
    EXPECT_EQ(args["-D"].value(1), "BAR");
}

TEST_F(options_mul, flag_count)
{
    env env{ "pgm", "-v", "-vv", "--verbose" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["-v"].count(), 4);
}

////////////////////////////////////////////////////////////////////////////////
struct options_req : testing::Test
{
    pgm::args args
    {
        { "-o", "--output", "FILE", pgm::req, "" },
        { "-v", "--verbose", "" },
    };
};

TEST_F(options_req, present)
{
    env env1{ "pgm", "--output", "file" };
    EXPECT_NO_THROW({ args.parse(env1.argc, env1.argv); });

    EXPECT_EQ(args["-o"].value(), "file");
    EXPECT_FALSE(args["-v"]);

    env env2{ "pgm", "-v" };
    EXPECT_THROW({ args.parse(env2.argc, env2.argv); }, pgm::missing_argument);
}

////////////////////////////////////////////////////////////////////////////////
struct options_token : testing::Test
{
    pgm::args args
    {
        { "-a", "" },
        { "-b", "val", "" },
        { "param", pgm::mul, "" },
    };
};

TEST_F(options_token, terminates)
{
    env env{ "pgm", "-a", "--", "-b", "val", "pos" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_TRUE (args["-a"]);
    EXPECT_FALSE(args["-b"]);
    EXPECT_EQ   (args["param"].value(), "-b");
}

TEST_F(options_token, value_after)
{
    env env{ "pgm", "-b", "--", "val" };
    EXPECT_THROW({ args.parse(env.argc, env.argv); }, pgm::missing_argument);
}
