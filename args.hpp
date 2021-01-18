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
#include <utility> // std::forward
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

private:
    friend struct args;

    string code_;            // short option name
    string full_;            // long option name
    string name_;            // param name (option or positional)
    string description_;

    bool required_ = false; // required option
    bool val_opt_  = false; // value is optional
    bool multiple_ = false; // can be specified multiple times

    std::vector<string> values_;
};

////////////////////////////////////////////////////////////////////////////////
struct args
{
    explicit args(std::initializer_list<arg> = { });

    void add(arg);
    template<typename... Args>
    void add(Args&&... args) { return add(arg{ std::forward<Args>(args)... }); }

    void parse(int argc, char* argv[]);

    auto const& values(const string& arg) const { return find(arg).values_; }
    auto count(const string& arg) const { return values(arg).size(); }
    bool empty(const string& arg) const { return !count(arg); }

    auto const& value(const string& arg) const { return values(arg).at(0); }
    auto const& value(const string& arg, size_t n) const { return values(arg)[n]; }

    auto const& value_or(const string& arg, const string& other)
    {
        auto const& vv = values(arg);
        return vv.size() ? vv.at(0) : other;
    }

    string usage(const string& program, const string& description = { });

private:
    std::vector<arg> options, params;

    using const_iterator = decltype(options)::const_iterator;
    using iterator = decltype(options)::iterator;

    const_iterator find_option(const string&) const;
    iterator find_option(const string&);
    const_iterator find_param(const string&) const;

    const arg& find(const string&) const;
};

////////////////////////////////////////////////////////////////////////////////
inline auto quoted(const string& arg) { return arg.size() ? "'" + arg + "'" : arg; }

////////////////////////////////////////////////////////////////////////////////
struct argument_exception : std::invalid_argument
{
    argument_exception(const string& msg, const string& arg) :
        std::invalid_argument{ msg + " " + quoted(arg) + "." }
    { }

    argument_exception(const string& msg, const string& arg1, const string& arg2) :
        std::invalid_argument{ msg + " " + quoted(arg1) +
            (arg1.size() && arg2.size() ? " or " : "") + quoted(arg2) + "."
        }
    { }
};

////////////////////////////////////////////////////////////////////////////////
struct invalid_definition : argument_exception
{
    invalid_definition(const string& msg, const string& arg) :
        argument_exception{ "Invalid definition: " + msg, arg }
    { }
};

////////////////////////////////////////////////////////////////////////////////
struct invalid_argument : argument_exception
{
    invalid_argument(const string& arg) :
        argument_exception{ "Invalid argument", arg }
    { }

    using argument_exception::argument_exception;
};

////////////////////////////////////////////////////////////////////////////////
struct missing_argument : argument_exception
{
    missing_argument(const string& arg) :
        argument_exception{ "Missing argument", arg }
    { }

    using argument_exception::argument_exception;
};

////////////////////////////////////////////////////////////////////////////////
struct missing_option : missing_argument
{
    missing_option(const string& arg1, const string& arg2) :
        missing_argument{ "Missing required option", arg1, arg2 }
    { }
};

////////////////////////////////////////////////////////////////////////////////
struct duplicate_option : invalid_argument
{
    duplicate_option(const string& arg) :
        invalid_argument{ "Duplicate option", arg }
    { }
};

////////////////////////////////////////////////////////////////////////////////
struct missing_value : missing_argument
{
    missing_value(const string& arg) :
        missing_argument{ "Missing option value", arg }
    { }
};

////////////////////////////////////////////////////////////////////////////////
struct extra_value : invalid_argument
{
    extra_value(const string& arg) :
        invalid_argument{ "Extra option value", arg }
    { }
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
