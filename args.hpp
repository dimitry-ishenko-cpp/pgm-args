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

////////////////////////////////////////////////////////////////////////////////
struct param
{
    param(const std::string& name, std::string description);

protected:
    std::string name_;
    std::string description_;

    bool required_ = true;
    bool opt_val_ = false;
    bool poly_ = false;

    param(std::string description);
};

////////////////////////////////////////////////////////////////////////////////
struct option : param
{
    option(std::string code_or_full, std::string description);
    option(std::string code_or_full, std::string full_or_param, std::string description);
    option(std::string code, std::string full, std::string name, std::string description);

protected:
    std::string code_, full_;
};

////////////////////////////////////////////////////////////////////////////////
struct invalid_argument : std::invalid_argument
{
    using std::invalid_argument::invalid_argument;
};

struct invalid_definition : invalid_argument
{
    invalid_definition(const std::string& why, const std::string& arg) :
        invalid_argument{ "Invalid option or parameter definition: " + why + " " + arg }
    { }
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
