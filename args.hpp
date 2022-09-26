////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2022 Dimitry Ishenko
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
#include <string_view>
#include <variant>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace pgm
{

// pull in often-used names from std::
using std::move;
using std::size_t;
using std::string;
using std::string_view;

////////////////////////////////////////////////////////////////////////////////
// parsed values for an option or positional parameter
struct argval
{
    auto count() const { return data_.size(); }
    auto empty() const { return data_.empty(); }

    explicit operator bool() const { return !empty(); }

    auto const& values() const { return data_; }
    auto const& value() const { return data_.at(0); }
    auto const& value(size_t n) const { return data_.at(n); }

    auto value_or(string_view def) const { return empty() ? string{def} : value(); }

private:
    std::vector<string> data_;

    friend class args;
    void add(string val) { data_.push_back(move(val)); }
};

////////////////////////////////////////////////////////////////////////////////
// option or param spec
enum spec
{
    req    = 1, // mandatory option
    mul    = 2, // option/param can be specified multiple times
    optval = 4, // option value is optional
    opt    = 8, // optional param
};

constexpr auto operator|(spec lhs, spec rhs);

////////////////////////////////////////////////////////////////////////////////
// program option
struct option
{
    string short_;          // short option name
    string long_;           // long option name
    string valname_;        // name of the option value; eg, --opt-name=<value>
    string description_;    // description

    bool req_ = false;      // mandatory (required) option
    bool mul_ = false;      // can be specified multiple times
    bool optval_ = false;   // option value is optional

    argval values_;         // parsed value(s)
};

////////////////////////////////////////////////////////////////////////////////
// positional parameter
struct param
{
    string name_;           // param name
    string description_;    // description

    bool opt_ = false;      // optional param
    bool mul_ = false;      // can be specified multiple times

    argval values_;         // parsed value(s)
};

////////////////////////////////////////////////////////////////////////////////
// program argument (either option or param)
struct arg
{
    arg(string name1, string description) :
        arg{move(name1), spec{ }, move(description)}
    { }
    arg(string name1, string name2, string description) :
        arg{move(name1), move(name2), spec{ }, move(description)}
    { }
    arg(string name1, string name2, string name3, string description) :
        arg{move(name1), move(name2), move(name3), spec{ }, move(description)}
    { }

    arg(string name1, spec, string description);
    arg(string name1, string name2, spec, string description);
    arg(string name1, string name2, string name3, spec, string description);

    auto is_option() const { return std::holds_alternative<option>(val_); }
    auto is_param() const { return std::holds_alternative<param>(val_); }

    auto& to_option() { return std::get<option>(val_); }
    auto& to_param() { return std::get<param>(val_); }

private:
    std::variant<option, param> val_;
};

////////////////////////////////////////////////////////////////////////////////
// program arguments
struct args
{
    args() = default;
    explicit args(std::initializer_list<arg> il)
    {
        for(auto& el : il) add(move(el));
    }

    void add(arg);

    template<typename... Ts>
    void add(Ts&&... vs) { add(arg{ std::forward<Ts>(vs)... }); }

    argval const& operator[](string_view) const;

    void parse(int argc, char* argv[]);
    string usage(string_view program, string_view preamble = { }, string_view prologue = { }, string_view epilogue = { }) const;

private:
    std::vector<option> options_;
    std::vector<param> params_;

    void add_option(option);
    void add_param(param);
};

////////////////////////////////////////////////////////////////////////////////
struct argument_exception : std::invalid_argument
{
    argument_exception(string_view what, string_view why) :
        std::invalid_argument{string{what}+": "+string{why}+"."}
    { }
};

struct invalid_definition : argument_exception
{
    invalid_definition(string_view why) :
        argument_exception{"Invalid definition", why}
    { }
};

struct invalid_argument : argument_exception
{
    invalid_argument(string_view why) :
        argument_exception{"Invalid argument", why}
    { }
};

struct missing_argument : argument_exception
{
    missing_argument(string_view why) :
        argument_exception{"Missing argument", why}
    { }
};

////////////////////////////////////////////////////////////////////////////////
}

#include "args.ipp"

////////////////////////////////////////////////////////////////////////////////
#endif
