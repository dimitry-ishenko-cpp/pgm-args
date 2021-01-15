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

private:
    std::vector<arg> opts, params;
};

////////////////////////////////////////////////////////////////////////////////
struct invalid_argument : std::invalid_argument
{
    using std::invalid_argument::invalid_argument;
};

struct invalid_definition : invalid_argument
{
    invalid_definition(const string& why, const string& arg) :
        invalid_argument{ "Invalid definition: " + why + " '" + arg + "'" }
    { }
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
