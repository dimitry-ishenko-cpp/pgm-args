////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2024 Dimitry Ishenko
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

////////////////////////////////////////////////////////////////////////////////
// parsed values for an option or positional parameter
struct argval
{
    auto count() const { return data_.size(); }
    auto empty() const { return data_.empty(); }

    explicit operator bool() const { return !empty(); }

    auto const& values() const { return data_; }
    auto const& value() const { return data_.at(0); }
    auto const& value(std::size_t n) const { return data_.at(n); }

    auto value_or(std::string_view def) const { return empty() ? std::string{def} : value(); }

private:
    std::vector<std::string> data_;

    friend class args;
    void add(std::string val) { data_.push_back(std::move(val)); }
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
    std::string short_;       // short option name
    std::string long_;        // long option name
    std::string valname_;     // name of the option value; eg, --opt-name=<value>
    std::string description_; // description

    bool req_ = false;        // mandatory (required) option
    bool mul_ = false;        // can be specified multiple times
    bool optval_ = false;     // option value is optional

    argval values_;           // parsed value(s)
};

////////////////////////////////////////////////////////////////////////////////
// positional parameter
struct param
{
    std::string name_;        // param name
    std::string description_; // description

    bool opt_ = false;        // optional param
    bool mul_ = false;        // can be specified multiple times

    argval values_;           // parsed value(s)
};

////////////////////////////////////////////////////////////////////////////////
// program argument (either option or param)
struct arg
{
    arg(std::string name1, std::string description) :
        arg{std::move(name1), spec{ }, std::move(description)}
    { }
    arg(std::string name1, std::string name2, std::string description) :
        arg{std::move(name1), std::move(name2), spec{ }, std::move(description)}
    { }
    arg(std::string name1, std::string name2, std::string name3, std::string description) :
        arg{std::move(name1), std::move(name2), std::move(name3), spec{ }, std::move(description)}
    { }

    arg(std::string name1, spec, std::string description);
    arg(std::string name1, std::string name2, spec, std::string description);
    arg(std::string name1, std::string name2, std::string name3, spec, std::string description);

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
        for(auto& el : il) add(std::move(el));
    }

    void add(arg);

    template<typename... Ts>
    void add(Ts&&... vs) { add(arg{ std::forward<Ts>(vs)... }); }

    argval const& operator[](std::string_view) const;

    void parse(int argc, char* argv[]);
    std::string usage(std::string_view program, std::string_view preamble = { }, std::string_view prologue = { }, std::string_view epilogue = { }) const;

private:
    std::vector<option> options_;
    std::vector<param> params_;

    void add_option(option);
    void add_param(param);
};

////////////////////////////////////////////////////////////////////////////////
struct argument_exception : std::invalid_argument
{
    argument_exception(std::string_view what, std::string_view why) :
        std::invalid_argument{std::string{what}+": "+std::string{why}+"."}
    { }
};

struct invalid_definition : argument_exception
{
    invalid_definition(std::string_view why) :
        argument_exception{"Invalid definition", why}
    { }
};

struct invalid_argument : argument_exception
{
    invalid_argument(std::string_view why) :
        argument_exception{"Invalid argument", why}
    { }
};

struct missing_argument : argument_exception
{
    missing_argument(std::string_view why) :
        argument_exception{"Missing argument", why}
    { }
};

////////////////////////////////////////////////////////////////////////////////
}

#include "args.ipp"

////////////////////////////////////////////////////////////////////////////////
#endif
