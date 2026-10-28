////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "args.hpp"

#include <algorithm>
#include <charconv> // std::from_chars
#include <deque>
#include <format>
#include <fstream>
#include <optional>
#include <ranges>
#include <sstream>
#include <tuple>
#include <vector>
#include <type_traits>

////////////////////////////////////////////////////////////////////////////////
namespace pgm
{

////////////////////////////////////////////////////////////////////////////////
namespace
{

namespace ranges = std::ranges;

constexpr bool iequals(std::string_view l, std::string_view r)
{
    auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? char(c - 'A' + 'a') : c; };
    return ranges::equal(l, r, {}, lower, lower);
}

constexpr bool is_alnum(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
constexpr bool is_dash_alnum(char c) { return c == '-' || is_alnum(c); }
constexpr bool is_graph(char c) { return c >= '!' && c <= '~'; }

constexpr bool is_valid_short_option(std::string_view s) {
    return s.size() == 2 && s[0] == '-' && is_alnum(s[1]);
}

constexpr bool is_valid_long_option(std::string_view s) {
    return s.size() > 2 && s[0] == '-' && s[1] == '-' && s[2] != '-' && ranges::all_of(s, is_dash_alnum);
}

constexpr bool is_valid_name(std::string_view s) {
    return s.size() > 0 && s[0] != '-' && ranges::all_of(s, is_graph);
}

constexpr bool is_not_option(std::string_view s) { return s.empty() || s == "-" || s[0] != '-'; }

////////////////////////////////////////////////////////////////////////////////
inline auto make_option(std::string s1, std::string s2, std::string s3, pgm::spec spec, std::string description)
{
    return option{ std::move(s1), std::move(s2), std::move(s3), std::move(description),
        !!(spec & req), !!(spec & mul), !!(spec & optval), {}
    };
}

inline auto make_param(std::string s, pgm::spec spec, std::string description) {
    return param{ std::move(s), std::move(description), !!(spec & opt), !!(spec & mul), {} };
}

inline auto pop(std::deque<std::string>& q)
{
    auto el = std::move(q.front());
    q.pop_front();
    return el;
}

}

////////////////////////////////////////////////////////////////////////////////
template<typename V>
V convert(std::string_view s)
{
    static_assert(std::is_arithmetic_v<V>, "Unsupported value type");

    if constexpr (std::is_same_v<V, bool>)
    {
        if (iequals(s,  "true") || iequals(s, "yes") || iequals(s,  "on") || s == "1") return true;
        if (iequals(s, "false") || iequals(s,  "no") || iequals(s, "off") || s == "0") return false;

        throw invalid_argument{"invalid boolean value '" + std::string{s} + "'"};
    }
    else
    {
        auto digits = s;
        if (digits.starts_with('+') && !digits.starts_with("+-")) digits.remove_prefix(1);

        V val;
        auto [end, ec] = std::from_chars(digits.data(), digits.data() + digits.size(), val);
        if (ec == std::errc{} && end == digits.data() + digits.size()) return val;

        auto msg = "invalid value '" + std::string{s} + "'";
        if (ec == std::errc::result_out_of_range) msg += " - out of range";
        throw invalid_argument{msg};
    }
}

template<typename V>
decltype(auto) argval::value(std::size_t n) const
{
    using U = std::remove_cvref_t<V>;

    if constexpr (std::is_same_v<U, std::string>)
        return data_.at(n); // const std::string&
    else return convert<U>(data_.at(n));
}

template<typename V>
auto argval::value_or(std::size_t n, V def) const
{
    using U = std::remove_cvref_t<V>;

    if constexpr (std::is_convertible_v<U, std::string_view>)
        return n < count() ? data_[n] : std::string{std::move(def)};
    else return n < count() ? convert<U>(data_[n]) : def;
}

template<typename V>
decltype(auto) argval::values() const
{
    using U = std::remove_cvref_t<V>;

    if constexpr (!std::is_same_v<U, std::string>)
    {
        auto view = data_ | std::views::transform(convert<U>);
        return std::vector<U>(view.begin(), view.end());
    }
    else return (data_); // const std::vector<std::string>&
}

////////////////////////////////////////////////////////////////////////////////
inline arg::arg(std::string s, pgm::spec spec, std::string description)
{
    if (is_valid_short_option(s))
        val_ = make_option(std::move(s), {}, {}, spec, std::move(description));

    else if (is_valid_long_option(s))
        val_ = make_option({}, std::move(s), {}, spec, std::move(description));

    else if (is_valid_name(s))
        val_ = make_param(std::move(s), spec, std::move(description));

    else throw invalid_definition{"bad option or param name '" + s + "'"};
}

////////////////////////////////////////////////////////////////////////////////
inline arg::arg(std::string s1, std::string s2, pgm::spec spec, std::string description)
{
    if (is_valid_short_option(s1))
    {
        if (is_valid_long_option(s2))
            val_ = make_option(std::move(s1), std::move(s2), {}, spec, std::move(description));

        else if (is_valid_name(s2))
            val_ = make_option(std::move(s1), {}, std::move(s2), spec, std::move(description));

        else throw invalid_definition{"bad long option or option value name '" + s2 + "'"};
    }
    else if (is_valid_long_option(s1))
    {
        if (is_valid_name(s2))
            val_ = make_option({}, std::move(s1), std::move(s2), spec, std::move(description));

        else throw invalid_definition{"bad option value name '" + s2 + "'"};
    }
    else throw invalid_definition{"bad short or long option name '" + s1 + "'"};
}

////////////////////////////////////////////////////////////////////////////////
inline arg::arg(std::string s1, std::string s2, std::string s3, pgm::spec spec, std::string description)
{
    if (!is_valid_short_option(s1))
        throw invalid_definition{"bad short option name '" + s1 + "'"};

    else if (!is_valid_long_option(s2))
        throw invalid_definition{"bad long option name '" + s2 + "'"};

    else if (!is_valid_name(s3))
        throw invalid_definition{"bad option value name '" + s3 + "'"};

    else val_ = make_option(std::move(s1), std::move(s2), std::move(s3), spec, std::move(description));
}

////////////////////////////////////////////////////////////////////////////////
inline void args::reset()
{
    for (auto& o : options_) o.values_.clear();
    for (auto& p : params_)  p.values_.clear();
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
inline void args::parse(const std::filesystem::path& path)
{
    std::ifstream in{path};
    if (!in) throw std::runtime_error{"cannot open '" + path.string() + "'"};

    struct invalid { std::string why; };
    struct missing { std::string why; };

    std::string line;
    for (int n = 1; std::getline(in, line); ++n)
    try
    {
        std::string name;
        std::optional<std::string> value;

        std::istringstream is{line};

        if ((is >> std::ws).eof()) continue;
        is >> name;
        if (name.starts_with('#') || (name.starts_with('[') && name.ends_with(']'))) continue;

        if (!(is >> std::ws).eof())
        {
            std::string word;
            is >> word;

            if (!word.starts_with('#'))
            {
                if (word != "=") throw invalid{"missing '='"};
                if ((is >> std::ws).eof()) throw missing{"missing value"};

                char ch = is.peek();
                if (ch == '"')
                {
                    if (!(is >> std::quoted(word))) throw invalid{"unclosed quote"};
                }
                else if (ch != '#') is >> word;
                else throw missing{"missing value"};

                value = std::move(word);

                if (!(is >> std::ws).eof())
                {
                    is >> word;
                    if (!word.starts_with('#')) throw invalid{"garbage at end"};
                }
            }
        }

        // find matching definition
        auto match = ranges::find_if(options_,
            [name = "--" + name](auto&& o){ return name == o.long_; }
        );
        if (match == options_.end())
            throw invalid{"unrecognized option '" + name + "'"};

        else if (match->valname_.empty()) // doesn't take values
        {
            // ...but we have one
            if (value) throw invalid{"'" + name + "' doesn't take values"};
            value = ""; // indicate presence
        }
        else // may take a value
        {
            // ...but we don't have one
            if (!value)
            {
                if (!match->optval_) throw missing{"'" + name + "' requires a value"};
                value = ""; // indicate presence
            }
        }

        if (!match->mul_ && !match->values_.empty())
            throw invalid{"duplicate option '" + name + "'"};

        match->values_.add(std::move(*value));
    }
    catch (const invalid& e)
    {
        throw invalid_argument{path.string() + ":" + std::to_string(n) + " - " + e.why};
    }
    catch (const missing& e)
    {
        throw missing_argument{path.string() + ":" + std::to_string(n) + " - " + e.why};
    }
}

////////////////////////////////////////////////////////////////////////////////
inline void args::parse(int argc, char* argv[])
{
    std::deque<std::string> args;
    for (auto n = 1; n < argc; ++n) args.emplace_back(argv[n]);

    bool had_token = false; // end-of-options token ("--")
    std::deque<std::string> store; // for params

    while (args.size())
    {
        auto arg = pop(args);

        if (had_token || is_not_option(arg)) // either "" or "-" or "[^-].+"
            store.push_back(std::move(arg));

        else if (arg == "--") // end-of-options token
            had_token = true;

        else // an option ("-.+")
        {
            std::string name;
            std::optional<std::string> value;

            if (arg[1] == '-') // long option ("--.+")
            {
                if (auto p = arg.find('=', 2); p != arg.npos) // ...with value ("--[^=]+=.?")
                {
                    name = arg.substr(0, p);
                    value = arg.substr(p + 1);
                }
                else name = arg; // ...without value ("--[^=]+")
            }
            else // short option (-[^-].?)
            {
                name = arg.substr(0, 2);
                if (arg.size() > 2) value = arg.substr(2); // ...with value ("-[^-].+")
            }

            // find matching definition
            auto match = ranges::find_if(options_,
                [&](auto&& o){ return name == o.short_ || name == o.long_; }
            );
            if (match == options_.end())
                throw invalid_argument{"unrecognized option '" + name + "'"};

            else if (match->valname_.empty()) // doesn't take values
            {
                if (value) // ...but we have one
                {
                    if (name.size() == 2) // short option
                    {
                        // this might be a group of short options (eg, -abc),
                        // push the rest back into the queue
                        args.push_front("-" + *value);
                    }
                    else throw invalid_argument{"'" + name + "' doesn't take values"};
                }
                value = ""; // indicate presence
            }
            else if (match->optval_) // value is optional
            {
                if (!value) // ...and we don't have one
                {
                    // take the next arg, if it's not an option
                    if (args.size() && is_not_option(args[0]))
                        value = pop(args);
                    else value = ""; // or indicate presence
                }
            }
            else // requires a value
            {
                if (!value) // ...but we don't have one
                {
                    // take the next arg unless it's the end-of-options token
                    if (args.size() && args[0] != "--")
                        value = pop(args);
                    else throw missing_argument{"'" + name + "' requires a value"};
                }
            }

            if (!match->mul_ && !match->values_.empty())
                throw invalid_argument{"duplicate option '" + name + "'"};

            match->values_.add(std::move(*value));
        }
    }

    // check required options
    for (auto&& o : options_)
        if (o.req_ && o.values_.empty())
        {
            auto name = o.short_.empty() ? o.long_
                : o.long_.empty() ? o.short_
                : o.short_ + ", " + o.long_;
            throw missing_argument{"option '" + name + "' is required"};
        }

    // # of non-opt params to fill
    auto need = static_cast<std::size_t>( ranges::count(params_, false, &param::opt_) );

    auto remain = params_.size();
    for (auto&& p : params_)
    {
        --remain;

        if (p.opt_)
        {
            // can we spare a value for this param?
            if (!(store.size() > need)) continue;
        }
        else --need;

        if (store.size())
        {
            // munch as many values as we can
            do p.values_.add(pop(store));
            while (p.mul_ && store.size() > remain);
        }
        else throw missing_argument{"param '" + p.name_ + "' is required"};
    }

    if (store.size()) throw invalid_argument{"extra param '" + store[0] + "'"};
}

////////////////////////////////////////////////////////////////////////////////
inline std::string args::usage(std::string_view program, std::string_view preamble,
    std::string_view prologue, std::string_view epilogue) const
{
    std::size_t width0 = 0;
    std::vector<std::tuple<std::string, std::string>> rows;

    ////////////////////
    if (preamble.size())
    {
        rows.emplace_back(preamble, "");
        rows.emplace_back("", "");
    }

    ////////////////////
    auto col0 = "Usage: " + std::string(program);
    if (options_.size()) col0 += " [option]...";
    for (auto&& p : params_)
    {
        col0 += (p.opt_ ? " [" : " <") + p.name_ + (p.opt_ ? "]" : ">");
        if (p.mul_) col0 += "...";
    }
    rows.emplace_back(std::move(col0), "");

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

        bool fill = ranges::any_of(options_, [](auto&& s){ return s.size(); }, &option::short_);
        for (auto&& o : options_)
        {
            std::string col0;
            if (o.short_.size())
            {
                col0 += o.short_;                              // "-o"
                if (o.long_.size())
                {
                    col0 += ", " + o.long_;                    // "-o, --opt"
                    if (o.valname_.size()) col0 += "=";        // "-o, --opt="
                }
                else if (o.valname_.size()) col0 += " ";       // "-o "
            }
            else
            {
                col0 += (fill ? "    " : "") + o.long_;        // "    --opt"
                if (o.valname_.size()) col0 += "=";            // "    --opt="
            }

            if (!o.valname_.empty())
            {
                if (o.optval_) col0 += "[" + o.valname_ + "]"; // ...[val]
                else           col0 += "<" + o.valname_ + ">"; // ...<val>
            }

            width0 = std::max(col0.size(), width0);

            auto& desc = o.description_; // ref
            for (std::size_t p0 = 0, p1 = 0; p1 != desc.npos; p0 = p1 + 1)
            {
                p1 = desc.find('\n', p0);
                rows.emplace_back(std::move(col0), desc.substr(p0, p1 - p0));
                col0.clear();
            }
        }
    }

    ////////////////////
    if (params_.size())
    {
        rows.emplace_back("", "");
        rows.emplace_back("Parameters:", "");

        for (auto&& p : params_)
        {
            std::string col0 = p.name_; // copy

            width0 = std::max(col0.size(), width0);

            auto& desc = p.description_; // ref
            for (std::size_t p0 = 0, p1 = 0; p1 != desc.npos; p0 = p1 + 1)
            {
                p1 = desc.find('\n', p0);
                rows.emplace_back(std::move(col0), desc.substr(p0, p1 - p0));
                col0.clear();
            }
        }
    }

    ////////////////////
    if (epilogue.size())
    {
        rows.emplace_back("", "");
        rows.emplace_back(epilogue, "");
    }

    ////////////////////
    std::string usage;
    for (auto&& [col0, col1] : rows)
    {
        if (col1.empty()) usage += std::format("{}\n", col0);
        else usage += std::format("{:<{}}    {}\n", col0, width0, col1);
    }
    usage.pop_back();

    return usage;
}

////////////////////////////////////////////////////////////////////////////////
}
