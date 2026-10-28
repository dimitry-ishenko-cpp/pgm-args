////////////////////////////////////////////////////////////////////////////////
#include "pgm/args.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

////////////////////////////////////////////////////////////////////////////////
struct conf : testing::Test
{
    pgm::args args
    {
        { "-d", "--debug",   "X", pgm::optval, "" },
        {       "--include", "X", pgm::mul, "" },
        { "-o", "--output",  "X", "" },
        { "-v", "--verbose",      "" },
    };

    const fs::path path = fs::temp_directory_path() / "pgm-args-test.conf";
    void write(std::string_view content) { std::ofstream{path} << content; }

    void TearDown() override { fs::remove(path); }
};

////////////////////////////////////////////////////////////////////////////////
TEST_F(conf, basic)
{
    write(R"(
# comment
verbose
output = /tmp/out.txt
include = foo.h
include = "bar baz.h"  # trailing comment
[section]
)");

    EXPECT_NO_THROW({ args.parse(path); });

    EXPECT_TRUE(args["--verbose"]);
    EXPECT_EQ  (args["--output" ].value(), "/tmp/out.txt");
    EXPECT_EQ  (args["--include"].count(), 2);
    EXPECT_EQ  (args["--include"].value(0), "foo.h");
    EXPECT_EQ  (args["--include"].value(1), "bar baz.h");
}

TEST_F(conf, quoted_hash)
{
    write(R"(output = "file#name.txt")");
    EXPECT_NO_THROW({ args.parse(path); });
    EXPECT_EQ(args["--output"].value(), "file#name.txt");
}

TEST_F(conf, optval)
{
    write("debug");
    EXPECT_NO_THROW({ args.parse(path); });
    EXPECT_EQ(args["--debug"].value(), "");

    args.reset();
    write("debug = 3");
    EXPECT_NO_THROW({ args.parse(path); });
    EXPECT_EQ(args["--debug"].value(), "3");
}

TEST_F(conf, missing_quote)
{
    write("output = \"unclosed");
    EXPECT_THROW({ args.parse(path); }, pgm::invalid_argument);
}

TEST_F(conf, two_words)
{
    write("output = foo bar");
    EXPECT_THROW({ args.parse(path); }, pgm::invalid_argument);
}

TEST_F(conf, missing_equal)
{
    write("output foo");
    EXPECT_THROW({ args.parse(path); }, pgm::invalid_argument);
}

TEST_F(conf, missing_value)
{
    write("output =");
    EXPECT_THROW({ args.parse(path); }, pgm::missing_argument);
}

TEST_F(conf, unrecognized)
{
    write("unknown = 1");
    EXPECT_THROW({ args.parse(path); }, pgm::invalid_argument);
}

TEST_F(conf, flag_with_value)
{
    write("verbose = yes");
    EXPECT_THROW({ args.parse(path); }, pgm::invalid_argument);
}

TEST_F(conf, duplicate)
{
    write(R"(
output = a
output = b
)");
    EXPECT_THROW({ args.parse(path); }, pgm::invalid_argument);
}

TEST_F(conf, missing_file)
{
    EXPECT_THROW({ args.parse("no-such-file.conf"); }, std::runtime_error);
}
