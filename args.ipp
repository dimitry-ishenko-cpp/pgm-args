////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2022 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef PGM_ARGS_IPP
#define PGM_ARGS_IPP

////////////////////////////////////////////////////////////////////////////////
#include <algorithm>
#include <cctype> // std::isalnum, std::isgraph
#include <deque>
#include <iomanip>
#include <optional>
#include <sstream>
#include <tuple>

////////////////////////////////////////////////////////////////////////////////
namespace pgm
{

////////////////////////////////////////////////////////////////////////////////
constexpr auto operator|(spec lhs, spec rhs)
{
    return static_cast<spec>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

////////////////////////////////////////////////////////////////////////////////
namespace
{

using std::begin, std::end, std::size;

// check if `s` is a valid short option
constexpr bool is_short_option(string_view s)
{
    return size(s) == 2 && s[0] == '-' && std::isalnum(s[1]);
}

// check if `s` is a valid long option
constexpr bool is_long_option(string_view s)
{
    return size(s) > 2 && s[0] == '-' && s[1] == '-' && s[2] != '-'
        && std::all_of(begin(s) + 2, end(s), [](char c){ return c == '-' || std::isalnum(c); });
}

// check if `s` is a valid option value
constexpr bool is_valname(string_view s)
{
    return size(s) > 0 && s[0] != '-'
        && std::all_of(begin(s), end(s), [](char c){ return std::isgraph(c); });
}

// check if `s` is a valid positional param name
constexpr bool is_param_name(string_view s)
{
    return size(s) > 0 && s[0] != '-'
        && std::all_of(begin(s), end(s), [](char c){ return std::isgraph(c); });
}

////////////////////////////////////////////////////////////////////////////////
inline auto make_option(string short_, string long_, string valname, spec spc, string description)
{
    return option{
        move(short_), move(long_), move(valname), move(description),
        static_cast<bool>(spc & req),
        static_cast<bool>(spc & mul),
        static_cast<bool>(spc & optval),
    };
}

////////////////////////////////////////////////////////////////////////////////
inline auto make_param(string name, spec spc, string description)
{
    return param{
        move(name), move(description),
        static_cast<bool>(spc & opt),
        static_cast<bool>(spc & mul),
    };
}

// return quoted `name`
inline auto q(const string& name) { return "'" + name + "'"; }

}

////////////////////////////////////////////////////////////////////////////////
inline arg::arg(string name1, spec spc, string description)
{
    if(is_short_option(name1))
        val_ = make_option(move(name1), { }, { }, spc, move(description));

    else if(is_long_option(name1))
        val_ = make_option({ }, move(name1), { }, spc, move(description));

    else if(is_param_name(name1))
        val_ = make_param(move(name1), spc, move(description));

    else throw invalid_definition{"bad option or param name " + q(name1)};
}

////////////////////////////////////////////////////////////////////////////////
inline arg::arg(string name1, string name2, spec spc, string description)
{
    if(is_short_option(name1))
    {
        if(is_long_option(name2))
            val_ = make_option(move(name1), move(name2), { }, spc, move(description));

        else if(is_valname(name2))
            val_ = make_option(move(name1), { }, move(name2), spc, move(description));

        else throw invalid_definition{"bad long option or option value name " + q(name2)};
    }
    else if(is_long_option(name1))
    {
        if(is_valname(name2))
            val_ = make_option({ }, move(name1), move(name2), spc, move(description));

        else throw invalid_definition{"bad option value name " + q(name2)};
    }
    else throw invalid_definition{"bad short or long option name " + q(name1)};
}

////////////////////////////////////////////////////////////////////////////////
inline arg::arg(string name1, string name2, string name3, spec spc, string description)
{
    if(!is_short_option(name1))
        throw invalid_definition{"bad short option name " + q(name1)};

    else if(!is_long_option(name2))
        throw invalid_definition{"bad long option name " + q(name2)};

    else if(!is_valname(name3))
        throw invalid_definition{"bad option value name " + q(name3)};

    else val_ = make_option(move(name1), move(name2), move(name3), spc, move(description));
}

////////////////////////////////////////////////////////////////////////////////
inline void args::add(arg new_)
{
    if(new_.is_option())
        add_option(move(new_.to_option()));

    else if(new_.is_param())
        add_param(move(new_.to_param()));

    else throw invalid_definition{"neither option nor param"};
}

////////////////////////////////////////////////////////////////////////////////
inline void args::add_option(option new_)
{
    if(new_.short_.size())
    {
        for(auto const& el : options_) if(el.short_ == new_.short_)
            throw invalid_definition{"duplicate option " + q(new_.short_)};
    }

    if(new_.long_.size())
    {
        for(auto const& el : options_) if(el.long_ == new_.long_)
            throw invalid_definition{"duplicate option " + q(new_.long_)};
    }

    options_.push_back(move(new_));
}

////////////////////////////////////////////////////////////////////////////////
inline void args::add_param(param new_)
{
    for(auto const& el : params_) if(el.name_ == new_.name_)
        throw invalid_definition{"duplicate param " + q(new_.name_)};

    for(auto const& el : params_) if(new_.mul_ && el.mul_)
        throw invalid_argument{"more than one multi-value param " + q(new_.name_)};

    params_.push_back(move(new_));
}

////////////////////////////////////////////////////////////////////////////////
inline argval const& args::operator[](string_view name) const
{
    if(!name.empty())
    {
        for(auto const& el : options_)
            if(el.short_ == name || el.long_ == name) return el.values_;

        for(auto const& el : params_) if(el.name_ == name) return el.values_;
    }
    throw invalid_argument{"unrecognized option or param " + q(string{name})};
}

////////////////////////////////////////////////////////////////////////////////
namespace
{

inline auto pop(std::deque<string>& q)
{
    auto el = move(q.front());
    q.pop_front();
    return el;
}

inline bool is_not_option(string_view s)
{
    return s.empty() || s == "-" || s[0] != '-';
}

}

////////////////////////////////////////////////////////////////////////////////
inline void args::parse(int argc, char* argv[])
{
    std::deque<string> args;
    for(auto n = 1; n < argc; ++n) args.emplace_back(argv[n]);

    bool had_token = false;
    std::deque<string> saved;

    while(args.size())
    {
        auto arg = pop(args);

        if(had_token || is_not_option(arg)) // param ("", "-" or re: "[^-].+")
            saved.push_back(move(arg));     // process at the end

        else if(arg == "--") // end-of-options token
            had_token = true;

        else // option (re: "-.+")
        {
            string name;
            std::optional<string> value;

            if(arg[1] == '-') // long option (re: "--.+")
            {
                auto p = arg.find('=', 2);
                if(p != string::npos) // with value (re: "--[^=]+=.?")
                {
                    name = arg.substr(0, p);
                    value = arg.substr(p + 1);
                }
                else name = arg; // without value (re: "--[^=]+")
            }
            else // short option (re: -[^-].?)
            {
                name = arg.substr(0, 2);
                if(arg.size() > 2) value = arg.substr(2); // with value (re: "-[^-].+")
            }

            // find matching definition
            auto pred = [&](auto const& el){ return el.short_ == name || el.long_ == name; };

            auto it = std::find_if(options_.begin(), options_.end(), pred);
            if(it == options_.end()) throw invalid_argument{"unrecognized option " + q(name)};

            else if(it->valname_.empty()) // doesn't take values
            {
                if(value) // but we have one
                {
                    if(name.size() == 2) // short option
                    {
                        // maybe this is a group of short options? (eg, -abc)
                        // push them to the front of the queue
                        args.push_front("-" + *value);
                    }
                    else throw invalid_argument{q(name) + " doesn't take values"};
                }
                value = ""; // indicate presence
            }
            else if(it->optval_) // optional value
            {
                if(!value) // and we don't have one
                {
                    // take the next arg, if it's not an option
                    if(args.size() && is_not_option(args[0]))
                        value = pop(args);
                    else value = ""; // or indicate presence
                }
            }
            else // requires value
            {
                if(!value) // but we don't have one
                {
                    // take the next arg unless it's "--"
                    if(args.size() && args[0] != "--")
                        value = pop(args);
                    else throw missing_argument{q(name) + " requires a value"};
                }
            }

            if(!it->mul_ && !it->values_.empty())
                throw invalid_argument{"duplicate option " + q(name)};

            it->values_.add(move(*value));
        }
    }

    // check required options
    for(auto const& el : options_)
        if(el.req_ && el.values_.empty()) throw missing_argument{
            "option " + q(el.short_.empty() ? el.long_ : el.long_.empty() ? el.short_ : el.short_+", "+el.long_) + " is required"
        };

    // process params
    auto req_n = std::count_if(params_.begin(), params_.end(),
        [&](auto const& el){ return !el.opt_; }
    ); // # of non-opt params to fill

    for(auto it = params_.begin(), end = params_.end(); it != end; ++it)
    {
        if(!it->opt_) --req_n;
        else if(saved.size() <= req_n) continue; // not enough for opt params

        if(saved.size())
        {
            do it->values_.add(pop(saved));
            while(it->mul_ && saved.size() >= end - it); // munch extra values
        }
        else throw missing_argument{"param " + q(it->name_) + " is required"};
    }

    if(saved.size()) throw invalid_argument{"extra param " + q(saved[0])};
}

////////////////////////////////////////////////////////////////////////////////
inline string args::usage(string_view program, string_view preamble, string_view prologue, string_view epilogue) const
{
    string short_fill = std::any_of(options_.begin(), options_.end(),
        [](auto const& el){ return el.short_.size(); }
    ) ? "    " : ""; // filler for "-o, "

    size_t cell_0_max = 0;
    std::vector<std::tuple<string, string>> rows;

    ////////////////////
    if(preamble.size())
    {
        rows.emplace_back(preamble, "");
        rows.emplace_back("", "");
    }

    ////////////////////
    string cell_0 = "Usage: " + string{program};

    if(options_.size()) cell_0 += " [option]...";
    if(params_.size())
    {
        for(auto const& el : params_)
        {
            if(el.opt_) cell_0 += " [" + el.name_ + "]";
            else cell_0 += " <" + el.name_ + ">";

            if(el.mul_) cell_0 += "...";
        }
    }
    rows.emplace_back(move(cell_0), "");

    ////////////////////
    if(prologue.size())
    {
        rows.emplace_back("", "");
        rows.emplace_back(prologue, "");
    }

    ////////////////////
    if(options_.size())
    {
        rows.emplace_back("", "");
        rows.emplace_back("Options:", "");

        for(auto const& el : options_)
        {
            string cell_0;
            if(el.short_.size())
            {
                cell_0 += el.short_; // "-o"
                if(el.long_.size())
                {
                    cell_0 += ", " + el.long_; // "-o, --opt-name"
                    if(el.valname_.size()) cell_0 += "="; // "-o, --opt-name="
                }
                else if(el.valname_.size()) cell_0 += " "; // "-o "
            }
            else
            {
                cell_0 += short_fill + el.long_; // "    --opt-name"
                if(el.valname_.size()) cell_0 += "="; // "    --opt-name="
            }

            if(el.valname_.size())
            {
                if(el.optval_) cell_0 += "[" + el.valname_ + "]"; // "...[val]"
                else cell_0 += "<" + el.valname_ + ">"; // "...<val>"
            }

            cell_0_max = std::max(cell_0_max, cell_0.size());

            std::istringstream iss{el.description_ + "\n"};
            string cell_1;
            std::getline(iss, cell_1);

            rows.emplace_back(move(cell_0), move(cell_1));
            while(std::getline(iss, cell_1)) rows.emplace_back("", move(cell_1));
        }
    }

    ////////////////////
    if(params_.size())
    {
        rows.emplace_back("", "");
        rows.emplace_back("Parameters:", "");

        for(auto const& el : params_)
        {
            cell_0_max = std::max(cell_0_max, el.name_.size());

            std::istringstream iss{el.description_ + "\n"};
            string cell_1;
            std::getline(iss, cell_1);

            rows.emplace_back(el.name_, move(cell_1));
            while(std::getline(iss, cell_1)) rows.emplace_back("", move(cell_1));
        }
    }

    ////////////////////
    if(epilogue.size())
    {
        rows.emplace_back("", "");
        rows.emplace_back(epilogue, "");
    }

    ////////////////////
    std::ostringstream oss;
    oss << std::left;

    for(auto const& [ cell_0, cell_1 ] : rows)
    {
        oss << std::setw(cell_0_max) << cell_0;
        if(cell_1.size()) oss << "    " << cell_1;
        oss << "\n";
    }

    auto overall = oss.str();
    overall.pop_back(); // remove trailing '\n'

    return overall;
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
