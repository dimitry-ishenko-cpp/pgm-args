////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2021 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef PGM_ARGS_HPP
#define PGM_ARGS_HPP

////////////////////////////////////////////////////////////////////////////////
#include <stdexcept>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace pgm
{

////////////////////////////////////////////////////////////////////////////////
struct arg
{
    arg(std::string name_code_or_full, std::string description);
    arg(std::string code_or_full, std::string full_or_name, std::string description);
    arg(std::string code, std::string full, std::string name, std::string description);

    bool is_option() const { return code_.empty() && full_.empty(); }
    bool is_param() const { return !is_option(); }

private:
    std::string code_, full_, name_;
    std::string description_;

    bool required_ = false;
    bool opt_val_ = false;
    bool poly_ = false;
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
