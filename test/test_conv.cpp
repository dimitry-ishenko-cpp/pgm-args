////////////////////////////////////////////////////////////////////////////////
#include "pgm/args.hpp"
#include "test_env.hpp"

#include <gtest/gtest.h>
#include <initializer_list>
#include <string_view>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
struct convert_basic : testing::Test
{
    pgm::args args
    {
        { "-n", "--num",   "N", "" },
        { "-r", "--real",  "F", "" },
        { "-b", "--bool",  "B", "" },
        { "-m", "--multi", "M", pgm::mul, "" },
    };
};

TEST_F(convert_basic, int)
{
    struct opt_val { std::string_view opt; int val; };
    for (auto [n, v] : std::initializer_list<opt_val>{ {"42",42}, {"-7",-7}, {"+8",+8}, {"0",0}, {"007",007} })
    {
        env env{ "pgm", "-n", n };
        EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

        EXPECT_EQ(args["-n"].value<int>(), v);
        EXPECT_EQ(args["--num"].value<int>(), v);
    }
}

TEST_F(convert_basic, real)
{
    struct opt_val { std::string_view opt; double val; };
    for (auto [n, v] : std::initializer_list<opt_val>{ {"3.25",3.25}, {"-1e3", -1e3}, {"+0.5",+0.5}, {".5",.5}, {"5.",5.}})
    {
        env env{ "pgm", "--real", n };
        EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

        EXPECT_DOUBLE_EQ(args["-r"].value<double>(), v);
    }
}

TEST_F(convert_basic, size_t)
{
    env env{ "pgm", "-n", "1024" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["-n"].value<std::size_t>(), 1024u);
}

TEST_F(convert_basic, bool_true_false)
{
    env env1{ "pgm", "-b", "true" };
    EXPECT_NO_THROW({ args.parse(env1.argc, env1.argv); });

    EXPECT_TRUE(args["-b"].value<bool>());

    env env2{ "pgm", "--bool", "false" };
    EXPECT_NO_THROW({ args.parse(env2.argc, env2.argv); });

    EXPECT_FALSE(args["-b"].value<bool>());
}

TEST_F(convert_basic, bool_variants)
{
    for (auto s : { "yes", "YES", "on", "ON", "1" })
    {
        env env{ "pgm", "-b", s };
        EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });
        EXPECT_TRUE(args["-b"].value<bool>());
    }
}

TEST_F(convert_basic, bool_variants_false)
{
    for (auto s : { "no", "NO", "off", "OFF", "0" })
    {
        env env{ "pgm", "-b", s };
        EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });
        EXPECT_FALSE(args["-b"].value<bool>());
    }
}

TEST_F(convert_basic, multi)
{
    env env{ "pgm", "-m", "1", "-m", "2", "-m", "3" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    auto vals = args["-m"].values<int>();
    EXPECT_EQ(vals, (std::vector<int>{ 1, 2, 3 }));
}

TEST_F(convert_basic, value_or_present)
{
    env env{ "pgm", "-n", "10" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["-n"].value_or(0), 10);
}

TEST_F(convert_basic, value_or_missing)
{
    env env{ "pgm" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["-n"].value_or(7), 7);
    EXPECT_EQ(args["-r"].value_or(1.5), 1.5);
    EXPECT_EQ(args["-b"].value_or(true), true);
}

TEST_F(convert_basic, invalid_int)
{
    for (auto n : { "abc", "", "4x", "x4", "4.5", " 4", "4 ", "0x10", "1e3", "+", "-", "++4", "+-4", "-+4" })
    {
        env env{ "pgm", "-n", n };
        EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

        EXPECT_THROW({ args["-n"].value<int>(); }, pgm::invalid_argument);
    }
}

TEST_F(convert_basic, invalid_real)
{
    for (auto n : { "abc", "", "1.5x", "1,5", " 1.5", "1.5 ", "+", "++1.5", "+-1.5", "1e", "1e999" })
    {
        env env{ "pgm", "-r", n };
        EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

        EXPECT_THROW({ args["-r"].value<double>(); }, pgm::invalid_argument);
    }
}

TEST_F(convert_basic, invalid_bool)
{
    env env{ "pgm", "-b", "maybe" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_THROW({ args["-b"].value<bool>(); }, pgm::invalid_argument);
}

TEST_F(convert_basic, out_of_range)
{
    env env{ "pgm", "-n", "999999999999999999999" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_THROW({ args["-n"].value<int>(); }, pgm::invalid_argument);
}

TEST_F(convert_basic, value_or_malformed_throws)
{
    env env{ "pgm", "-n", "xyz" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_THROW({ args["-n"].value_or(0); }, pgm::invalid_argument);
}

////////////////////////////////////////////////////////////////////////////////
enum class color { red, blue, white };

template<>
inline color pgm::convert<color>(std::string_view s)
{
    if (s == "red"  ) return color::red;
    if (s == "blue" ) return color::blue;
    if (s == "white") return color::white;
    throw pgm::invalid_argument{"invalid color '" + std::string{s} + "'"};
}

struct convert_custom : testing::Test
{
    pgm::args args {
        { "-c", "--color", "NAME", "" },
    };
};

TEST_F(convert_custom, ok)
{
    env env{ "pgm", "--color", "blue" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["-c"].value<color>(), color::blue);
}

TEST_F(convert_custom, value_or)
{
    env env{ "pgm" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_EQ(args["-c"].value_or(color::red), color::red);
}

TEST_F(convert_custom, bad)
{
    env env{ "pgm", "-c", "purple" };
    EXPECT_NO_THROW({ args.parse(env.argc, env.argv); });

    EXPECT_THROW({ args["-c"].value<color>(); }, pgm::invalid_argument);
}
