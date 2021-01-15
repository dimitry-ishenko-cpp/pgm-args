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

    bool has_code() const { return code_.size(); }
    auto const& code() const { return code_; }

    bool has_full() const { return full_.size(); }
    auto const& full() const { return full_; }

    bool has_name() const { return name_.size(); }
    auto const& name() const { return name_; }

    auto const& description() const { return description_; }

    bool is_option() const { return has_code() || has_full(); }
    bool is_param() const { return !is_option(); }

    bool is_required() const { return required_; }
    bool is_opt_val () const { return opt_val_ ; }
    bool is_multiple() const { return multiple_; }

    auto count() const { return values_.size(); }
    auto const& values() const { return values_; }

    auto const& value() const { return values().at(0); }
    auto const& value(size_t n) const { return values()[n]; }
    auto const& value_or(const string& def) { return count() ? value() : def; }

    void add_value(string);

private:
    string code_, full_, name_;
    string description_;

    bool required_ = false;
    bool opt_val_  = false;
    bool multiple_ = false;

    std::vector<string> values_;
};

////////////////////////////////////////////////////////////////////////////////
struct args
{
    explicit args(std::initializer_list<arg> = { });
    args& operator<<(arg);

    void parse(int argc, char* argv[]);

private:
    std::vector<arg> options_, params_;
    bool has_multiple_ = false;
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
