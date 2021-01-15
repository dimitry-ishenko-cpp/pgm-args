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

    string code, full, name;
    string description;

    bool required = false;
    bool val_opt  = false; // value is optional
    bool multiple = false;

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
    std::vector<arg> opts, params;
    const arg& find(const std::string&) const;
};

////////////////////////////////////////////////////////////////////////////////
struct invalid_argument : std::invalid_argument
{
    invalid_argument(const std::string& msg, const std::string& arg) :
        std::invalid_argument(msg + " '" + arg + "'")
    { }
};

struct bad_definition : invalid_argument
{
    bad_definition(const string& msg, const string& arg) :
        invalid_argument("Bad definition: " + msg, arg)
    { }
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
