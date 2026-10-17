////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "args.hpp"

#include <algorithm>
#include <deque>
#include <iomanip>
#include <optional>
#include <sstream>
#include <tuple>

////////////////////////////////////////////////////////////////////////////////
namespace pgm
{

////////////////////////////////////////////////////////////////////////////////
namespace
{

namespace ranges = std::ranges;

constexpr bool is_alnum(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
constexpr bool is_dash_alnum(char c) { return c == '-' || is_alnum(c); }
constexpr bool is_graph(char c) { return c >= '!' && c <= '~'; }

constexpr bool is_short_option(std::string_view s) {
    return s.size() == 2 && s[0] == '-' && is_alnum(s[1]);
}

constexpr bool is_long_option(std::string_view s) {
    return s.size() > 2 && s[0] == '-' && s[1] == '-' && s[2] != '-' && ranges::all_of(s, is_dash_alnum);
}

constexpr bool is_name(std::string_view s) {
    return s.size() > 0 && s[0] != '-' && ranges::all_of(s, is_graph);
}

constexpr bool is_not_option(std::string_view s) { return s.empty() || s == "-" || s[0] != '-'; }

////////////////////////////////////////////////////////////////////////////////
inline auto make_option(std::string s1, std::string s2, std::string s3, pgm::spec spec, std::string description)
{
    return option{
        std::move(s1), std::move(s2), std::move(s3), std::move(description),
        !!(spec & req), !!(spec & mul), !!(spec & optval),
    };
}

inline auto make_param(std::string s, pgm::spec spec, std::string description)
{
    return param{
        std::move(s), std::move(description),
        !!(spec & opt), !!(spec & mul),
    };
}

inline auto pop(std::deque<std::string>& q)
{
    auto el = std::move(q.front());
    q.pop_front();
    return el;
}

}

////////////////////////////////////////////////////////////////////////////////
inline arg::arg(std::string s, pgm::spec spec, std::string description)
{
    if (is_short_option(s))
        val_ = make_option(std::move(s), {}, {}, spec, std::move(description));

    else if (is_long_option(s))
        val_ = make_option({}, std::move(s), {}, spec, std::move(description));

    else if (is_name(s))
        val_ = make_param(std::move(s), spec, std::move(description));

    else throw invalid_definition{"bad option or param name '" + s + "'"};
}

////////////////////////////////////////////////////////////////////////////////
inline arg::arg(std::string s1, std::string s2, pgm::spec spec, std::string description)
{
    if (is_short_option(s1))
    {
        if (is_long_option(s2))
            val_ = make_option(std::move(s1), std::move(s2), {}, spec, std::move(description));

        else if (is_name(s2))
            val_ = make_option(std::move(s1), {}, std::move(s2), spec, std::move(description));

        else throw invalid_definition{"bad long option or option value name '" + s2 + "'"};
    }
    else if (is_long_option(s1))
    {
        if (is_name(s2))
            val_ = make_option({}, std::move(s1), std::move(s2), spec, std::move(description));

        else throw invalid_definition{"bad option value name '" + s2 + "'"};
    }
    else throw invalid_definition{"bad short or long option name '" + s1 + "'"};
}

////////////////////////////////////////////////////////////////////////////////
inline arg::arg(std::string s1, std::string s2, std::string s3, pgm::spec spec, std::string description)
{
    if (!is_short_option(s1))
        throw invalid_definition{"bad short option name '" + s1 + "'"};

    else if (!is_long_option(s2))
        throw invalid_definition{"bad long option name '" + s2 + "'"};

    else if (!is_name(s3))
        throw invalid_definition{"bad option value name '" + s3 + "'"};

    else val_ = make_option(std::move(s1), std::move(s2), std::move(s3), spec, std::move(description));
}

////////////////////////////////////////////////////////////////////////////////
inline void args::add(arg new_)
{
    if (new_.is_option())
        add_option(std::move(new_.to_option()));

    else if (new_.is_param())
        add_param(std::move(new_.to_param()));

    else throw invalid_definition{"neither option nor param"};
}

////////////////////////////////////////////////////////////////////////////////
inline void args::add_option(option new_)
{
    if (!new_.short_.empty()
        && ranges::find(options_, new_.short_, &option::short_) != options_.end())
            throw invalid_definition{"duplicate option '" + new_.short_ + "'"};

    if (!new_.long_.empty()
        && ranges::find(options_, new_.long_, &option::long_) != options_.end())
            throw invalid_definition{"duplicate option '" + new_.long_ + "'"};

    options_.push_back(std::move(new_));
}

////////////////////////////////////////////////////////////////////////////////
inline void args::add_param(param new_)
{
    if (ranges::find(params_, new_.name_, &param::name_) != params_.end())
        throw invalid_definition{"duplicate param '" + new_.name_ + "'"};

    if (new_.mul_ && ranges::any_of(params_, &param::mul_))
        throw invalid_definition{"more than one multi-value param '" + new_.name_ + "'"};

    params_.push_back(std::move(new_));
}

////////////////////////////////////////////////////////////////////////////////
inline argval const& args::operator[](std::string_view name) const
{
    if (!name.empty())
    {
        for (auto&& el : options_)
            if (el.short_ == name || el.long_ == name) return el.values_;

        for (auto&& el : params_) if (el.name_ == name) return el.values_;
    }
    throw invalid_argument{"unrecognized option or param '" + std::string{name} + "'"};
}

////////////////////////////////////////////////////////////////////////////////
inline void args::parse(int argc, char* argv[])
{
    std::deque<std::string> args;
    for (auto n = 1; n < argc; ++n) args.emplace_back(argv[n]);

    bool had_token = false;
    std::deque<std::string> saved;

    while (args.size())
    {
        auto arg = pop(args);

        if (had_token || is_not_option(arg)) // param ("", "-" or "[^-].+")
            saved.push_back(std::move(arg)); // process afterwards

        else if (arg == "--") // end-of-options token
            had_token = true;

        else // option ("-.+")
        {
            std::string name;
            std::optional<std::string> value;

            if (arg[1] == '-') // long option ("--.+")
            {
                auto p = arg.find('=', 2);
                if (p != arg.npos) // with value ("--[^=]+=.?")
                {
                    name = arg.substr(0, p);
                    value = arg.substr(p + 1);
                }
                else name = arg; // without value ("--[^=]+")
            }
            else // short option (-[^-].?)
            {
                name = arg.substr(0, 2);
                if (arg.size() > 2) value = arg.substr(2); // with value ("-[^-].+")
            }

            // find matching definition
            auto it = ranges::find_if(options_,
                [&](auto&& el){ return el.short_ == name || el.long_ == name; }
            );
            if (it == options_.end())
                throw invalid_argument{"unrecognized option '" + name + "'"};

            else if (it->valname_.empty()) // doesn't take values
            {
                if (value) // but we have one
                {
                    if (name.size() == 2) // short option
                    {
                        // maybe this is a group of short options (eg, -abc)
                        // push them in front of the queue
                        args.push_front("-" + *value);
                    }
                    else throw invalid_argument{"'" + name + "' doesn't take values"};
                }
                value = ""; // indicate presence
            }
            else if (it->optval_) // optional value
            {
                if (!value) // and we don't have one
                {
                    // take the next arg, if it's not an option
                    if (args.size() && is_not_option(args[0]))
                        value = pop(args);
                    else value = ""; // or indicate presence
                }
            }
            else // requires a value
            {
                if (!value) // but we don't have one
                {
                    // take the next arg unless it's "--"
                    if (args.size() && args[0] != "--")
                        value = pop(args);
                    else throw missing_argument{"'" + name + "' requires a value"};
                }
            }

            if (!it->mul_ && !it->values_.empty())
                throw invalid_argument{"duplicate option '" + name + "'"};

            it->values_.add(std::move(*value));
        }
    }

    // check required options
    for (auto&& el : options_)
        if (el.req_ && el.values_.empty()) throw missing_argument{
            "option '" + (
                el.short_.empty() ? el.long_ : el.long_.empty() ? el.short_ : el.short_ + ", " + el.long_
            ) + "' is required"
        };

    // process params
    auto req_n = ranges::count(params_, false, &param::opt_); // # of required params to fill

    for (auto it = params_.begin(), end = params_.end(); it != end; ++it)
    {
        if (!it->opt_) --req_n;
        else if (saved.size() <= req_n) continue; // not enough for optional params

        if (saved.size())
        {
            do it->values_.add(pop(saved));
            while (it->mul_ && saved.size() >= end - it); // munch extra values
        }
        else throw missing_argument{"param '" + it->name_ + "' is required"};
    }

    if (saved.size()) throw invalid_argument{"extra param '" + saved[0] + "'"};
}

////////////////////////////////////////////////////////////////////////////////
inline std::string args::usage(std::string_view program, std::string_view preamble,
    std::string_view prologue, std::string_view epilogue) const
{
    std::string short_fill = ranges::any_of(options_,
        [](auto&& el){ return el.short_.size(); }
    ) ? "    " : ""; // filler for "-o, "

    std::size_t cell_0_max = 0;
    std::vector<std::tuple<std::string, std::string>> rows;

    ////////////////////
    if (preamble.size())
    {
        rows.emplace_back(preamble, "");
        rows.emplace_back("", "");
    }

    ////////////////////
    auto cell_0 = "Usage: " + std::string{program};

    if (options_.size()) cell_0 += " [option]...";
    if (params_.size())
    {
        for (auto&& el : params_)
        {
            if (el.opt_) cell_0 += " [" + el.name_ + "]";
            else cell_0 += " <" + el.name_ + ">";

            if (el.mul_) cell_0 += "...";
        }
    }
    rows.emplace_back(std::move(cell_0), "");

    ////////////////////
    if (prologue.size())
    {
        rows.emplace_back("", "");
        rows.emplace_back(prologue, "");
    }

    ////////////////////
    if (options_.size())
    {
        rows.emplace_back("", "");
        rows.emplace_back("Options:", "");

        for (auto&& el : options_)
        {
            std::string cell_0;
            if (el.short_.size())
            {
                cell_0 += el.short_; // "-o"
                if (el.long_.size())
                {
                    cell_0 += ", " + el.long_; // "-o, --opt-name"
                    if (el.valname_.size()) cell_0 += "="; // "-o, --opt-name="
                }
                else if (el.valname_.size()) cell_0 += " "; // "-o "
            }
            else
            {
                cell_0 += short_fill + el.long_; // "    --opt-name"
                if (el.valname_.size()) cell_0 += "="; // "    --opt-name="
            }

            if (el.valname_.size())
            {
                if (el.optval_) cell_0 += "[" + el.valname_ + "]"; // "...[val]"
                else cell_0 += "<" + el.valname_ + ">"; // "...<val>"
            }

            cell_0_max = std::max(cell_0_max, cell_0.size());

            std::istringstream iss{el.description_ + "\n"};
            std::string cell_1;
            std::getline(iss, cell_1);

            rows.emplace_back(std::move(cell_0), std::move(cell_1));
            while (std::getline(iss, cell_1)) rows.emplace_back("", std::move(cell_1));
        }
    }

    ////////////////////
    if (params_.size())
    {
        rows.emplace_back("", "");
        rows.emplace_back("Parameters:", "");

        for (auto&& el : params_)
        {
            cell_0_max = std::max(cell_0_max, el.name_.size());

            std::istringstream iss{el.description_ + "\n"};
            std::string cell_1;
            std::getline(iss, cell_1);

            rows.emplace_back(el.name_, std::move(cell_1));
            while (std::getline(iss, cell_1)) rows.emplace_back("", std::move(cell_1));
        }
    }

    ////////////////////
    if (epilogue.size())
    {
        rows.emplace_back("", "");
        rows.emplace_back(epilogue, "");
    }

    ////////////////////
    std::ostringstream oss;
    oss << std::left;

    for (auto&& [ cell_0, cell_1 ] : rows)
    {
        oss << std::setw(cell_0_max) << cell_0;
        if (cell_1.size()) oss << "    " << cell_1;
        oss << "\n";
    }

    auto overall = oss.str();
    overall.pop_back(); // remove trailing '\n'

    return overall;
}

////////////////////////////////////////////////////////////////////////////////
}
