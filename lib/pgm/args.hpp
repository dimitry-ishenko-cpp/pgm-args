////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

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
// option/param values
struct argval
{
    auto count() const { return data_.size(); }
    auto empty() const { return data_.empty(); }

    explicit operator bool() const { return !empty(); }

    auto& values() const { return data_; }
    auto& value() const { return data_.at(0); }
    auto& value(std::size_t n) const { return data_.at(n); }

    auto value_or(std::string_view def) const { return empty() ? std::string{def} : value(); }

private:
    std::vector<std::string> data_;

    friend class args;
    void add(std::string val) { data_.push_back(std::move(val)); }
};

////////////////////////////////////////////////////////////////////////////////
// option/param spec
enum spec
{
    req    = 1, // required option
    mul    = 2, // option/param can be specified multiple times
    optval = 4, // option value is optional
    opt    = 8, // optional param
};

constexpr auto operator|(spec lhs, spec rhs);

////////////////////////////////////////////////////////////////////////////////
struct option
{
    std::string short_;       // short option name
    std::string long_;        // long option name
    std::string valname_;     // name of the option value; eg, --opt-name=<value>
    std::string description_; // description

    bool req_ = false;        // required option
    bool mul_ = false;        // can be specified multiple times
    bool optval_ = false;     // option value is optional

    argval values_;           // parsed option values
};

////////////////////////////////////////////////////////////////////////////////
struct param
{
    std::string name_;        // param name
    std::string description_; // description

    bool opt_ = false;        // optional param
    bool mul_ = false;        // can be specified multiple times

    argval values_;           // parsed param values
};

////////////////////////////////////////////////////////////////////////////////
struct arg
{
    // `s` can be a short option name, a long option name or a param name
    arg(std::string s, std::string description) :
        arg{std::move(s), spec{}, std::move(description)}
    { }
    arg(std::string s, spec, std::string description);

    // `s1` can be either a short option name or a long option name
    // if `s1` is a short option name, `s2` can be a long option name or an option value name
    // if `s1` is a long  option name, `s2` is an option value name
    arg(std::string s1, std::string s2, std::string description) :
        arg{std::move(s1), std::move(s2), spec{}, std::move(description)}
    { }
    arg(std::string s1, std::string s2, spec, std::string description);

    // `s1` is a short option name, `s2` is a long option name and `s3` is an option value name
    arg(std::string s1, std::string s2, std::string s3, std::string description) :
        arg{std::move(s1), std::move(s2), std::move(s3), spec{}, std::move(description)}
    { }
    arg(std::string s1, std::string s2, std::string s3, spec, std::string description);

    auto is_option() const { return std::holds_alternative<option>(val_); }
    auto is_param() const { return std::holds_alternative<param>(val_); }

    auto& to_option() { return std::get<option>(val_); }
    auto& to_param() { return std::get<param>(val_); }

private:
    std::variant<option, param> val_;
};

////////////////////////////////////////////////////////////////////////////////
struct args
{
    args() = default;
    explicit args(std::initializer_list<arg> il) { for (auto&& el : il) add(std::move(el)); }

    void add(arg);
    void add(auto&&... args) { add(arg{ std::forward<decltype (args)>(args)... }); }

    argval const& operator[](std::string_view) const;

    void parse(int argc, char* argv[]);

    std::string usage(std::string_view program, std::string_view preamble = {},
        std::string_view prologue = {}, std::string_view epilogue = {}) const;

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
        std::invalid_argument{std::string{what} + ": " + std::string{why} + "."}
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
