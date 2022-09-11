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

////////////////////////////////////////////////////////////////////////////////
struct arg
{
    arg(std::string name_code_or_full, std::string description);
    arg(std::string code_or_full, std::string full_or_name, std::string description);
    arg(std::string code, std::string full, std::string name, std::string description);

    auto const& values() const { return values_; }
    auto count() const { return values().size(); }

    bool empty() const { return !count(); }
    explicit operator bool() const { return count(); }

    auto const& value() const { return values().at(0); }
    auto const& value(size_t n) const { return values()[n]; } // NB: no range check

    auto const& value_or(const std::string& def) const { return count() ? value() : def; }

private:
    friend struct args;

    std::string code_;      // short option name
    std::string full_;      // long option name
    std::string name_;      // param name (option or positional)
    std::string description_;

    bool required_ = false; // required option
    bool val_opt_  = false; // value is optional
    bool multiple_ = false; // can be specified multiple times

    std::vector<std::string> values_;
};

////////////////////////////////////////////////////////////////////////////////
struct args
{
    explicit args(std::initializer_list<arg> = { });

    void add(arg);
    template<typename... Args>
    void add(Args&&... args) { return add(arg{ std::forward<Args>(args)... }); }

    auto const& operator[](const std::string& arg) const { return find(arg); }

    void parse(int argc, char* argv[]);
    std::string usage(const std::string& program, const std::string& description = { });

private:
    std::vector<arg> options, params;

    using const_iterator = decltype (options)::const_iterator;
    using iterator = decltype (options)::iterator;

    const_iterator find_option(const std::string&) const;
    iterator find_option(const std::string&);
    const_iterator find_param(const std::string&) const;

    const arg& find(const std::string&) const;
};

////////////////////////////////////////////////////////////////////////////////
inline auto quoted(const std::string& arg) { return arg.size() ? "'" + arg + "'" : arg; }

////////////////////////////////////////////////////////////////////////////////
struct argument_exception : std::invalid_argument
{
    argument_exception(const std::string& msg, const std::string& arg) :
        std::invalid_argument{ msg + " " + quoted(arg) + "." }
    { }

    argument_exception(const std::string& msg, const std::string& arg1, const std::string& arg2) :
        std::invalid_argument{ msg + " " + quoted(arg1) +
            (arg1.size() && arg2.size() ? " or " : "") + quoted(arg2) + "."
        }
    { }
};

////////////////////////////////////////////////////////////////////////////////
struct invalid_definition : argument_exception
{
    invalid_definition(const std::string& msg, const std::string& arg) :
        argument_exception{ "Invalid definition: " + msg, arg }
    { }
};

////////////////////////////////////////////////////////////////////////////////
struct invalid_argument : argument_exception
{
    invalid_argument(const std::string& arg) :
        argument_exception{ "Invalid argument", arg }
    { }

    using argument_exception::argument_exception;
};

////////////////////////////////////////////////////////////////////////////////
struct missing_argument : argument_exception
{
    missing_argument(const std::string& arg) :
        argument_exception{ "Missing argument", arg }
    { }

    using argument_exception::argument_exception;
};

////////////////////////////////////////////////////////////////////////////////
struct missing_option : missing_argument
{
    missing_option(const std::string& arg1, const std::string& arg2) :
        missing_argument{ "Missing required option", arg1, arg2 }
    { }
};

////////////////////////////////////////////////////////////////////////////////
struct duplicate_option : invalid_argument
{
    duplicate_option(const std::string& arg) :
        invalid_argument{ "Duplicate option", arg }
    { }
};

////////////////////////////////////////////////////////////////////////////////
struct missing_value : missing_argument
{
    missing_value(const std::string& arg) :
        missing_argument{ "Missing option value", arg }
    { }
};

////////////////////////////////////////////////////////////////////////////////
struct extra_value : invalid_argument
{
    extra_value(const std::string& arg) :
        invalid_argument{ "Extra option value", arg }
    { }
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
