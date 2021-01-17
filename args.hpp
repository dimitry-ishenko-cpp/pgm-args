////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2021 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef PGM_ARGS_HPP
#define PGM_ARGS_HPP

////////////////////////////////////////////////////////////////////////////////
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace pgm
{

using std::string;

////////////////////////////////////////////////////////////////////////////////
struct arg
{
    arg(string name_code_or_full, string description);
    arg(string code_or_full, string full_or_name, string description);
    arg(string code, string full, string name, string description);

    string code; // short option name
    string full; // long option name
    string name; // param name (option or positional)
    string description;

    bool required = false; // required option
    bool val_opt  = false; // value is optional
    bool multiple = false; // can be specified multiple times

    std::vector<string> values;
};

////////////////////////////////////////////////////////////////////////////////
struct args
{
    explicit args(std::initializer_list<arg> = { });
    args& operator<<(arg);

    void parse(int argc, char* argv[]);

    auto const& values(const string& arg) const { return find(arg).values; }
    auto count(const string& arg) const { return values(arg).size(); }
    bool empty(const string& arg) const { return !count(arg); }

    auto const& value(const string& arg) const { return values(arg).at(0); }
    auto const& value(const string& arg, size_t n) const { return values(arg)[n]; }

    auto const& value_or(const string& arg, const string& other)
    {
        auto const& vv = values(arg);
        return vv.size() ? vv.at(0) : other;
    }

private:
    std::vector<arg> options, params;
    const arg& find(const std::string&) const;
};

////////////////////////////////////////////////////////////////////////////////
struct argument_exception : std::invalid_argument
{
    argument_exception(const std::string& msg, const std::string& arg) :
        std::invalid_argument(msg + " '" + arg + "'")
    { }
};

struct invalid_definition : argument_exception
{
    invalid_definition(const string& msg, const string& arg) :
        argument_exception("Invalid definition: " + msg, arg)
    { }
};

struct invalid_argument : argument_exception
{
    invalid_argument(const string& arg) :
        argument_exception("Invalid argument", arg)
    { }

    using argument_exception::argument_exception;
};

struct missing_argument : argument_exception
{
    missing_argument(const string& arg) :
        argument_exception("Missing argument", arg)
    { }

    using argument_exception::argument_exception;
};

struct missing_option : missing_argument
{
    missing_option(const string& arg) :
        missing_argument("Missing required option", arg)
    { }
};

struct duplicate_option : invalid_argument
{
    duplicate_option(const string& arg) :
        invalid_argument("Duplicate option", arg)
    { }
};

struct missing_value : missing_argument
{
    missing_value(const string& arg) :
        missing_argument("Missing option value", arg)
    { }
};

struct extra_value : invalid_argument
{
    extra_value(const string& arg) :
        invalid_argument("Extra option value", arg)
    { }
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
