////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2021 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef PGM_ARGS_HPP
#define PGM_ARGS_HPP

////////////////////////////////////////////////////////////////////////////////
#include <algorithm>
#include <cctype> // std::isalnum, std::isgraph
#include <deque>
#include <initializer_list>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace pgm
{

// pull in often-used names from std::
using std::deque;
using std::move;
using std::size_t;
using std::string;
using std::string_view;
using std::vector;

////////////////////////////////////////////////////////////////////////////////
// parsed values for an option or positional parameter
struct argval
{
    auto count() const { return data_.size(); }
    auto empty() const { return data_.empty(); }

    explicit operator bool() { return !empty(); }

    auto const& values() const { return data_; }
    auto const& value() const { return data_.at(0); }
    auto const& value(size_t n) const { return data_.at(n); }

    auto value_or(string_view def) const { return empty() ? string{def} : value(); }

private:
    vector<string> data_;

    friend class args;
    void add(string val) { data_.push_back(move(val)); }
    auto size() const { return data_.size(); }
};

////////////////////////////////////////////////////////////////////////////////
// option or param spec
enum spec
{
    req = 1,    // mandatory option
    mul = 2,    // option/param can be specified multiple times
    optval = 4, // option value is optional
    opt = 8,    // optional param
};

constexpr auto operator|(spec lhs, spec rhs)
{
    return static_cast<spec>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

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

    static auto from(string short_, string long_, string valname, spec spc, string description)
    {
        return option{
            move(short_), move(long_), move(valname), move(description),
            static_cast<bool>(spc & req),
            static_cast<bool>(spc & mul),
            static_cast<bool>(spc & optval),
        };
    }
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

    static auto from(string name, spec spc, string description)
    {
        return param{
            move(name), move(description),
            static_cast<bool>(spc & opt),
            static_cast<bool>(spc & mul),
        };
    }
};

////////////////////////////////////////////////////////////////////////////////
// program argument (either option or param)
struct arg
{
    arg(string name1, string description) : // (1)
        arg{move(name1), spec{ }, move(description)}
    { }
    arg(string name1, string name2, string description) : // (2)
        arg{move(name1), move(name2), spec{ }, move(description)}
    { }
    arg(string name1, string name2, string name3, string description) : // (3)
        arg{move(name1), move(name2), move(name3), spec{ }, move(description)}
    { }

    arg(string name1, spec, string description); // (1)
    arg(string name1, string name2, spec, string description); // (2)
    arg(string name1, string name2, string name3, spec, string description); // (3)

    auto is_option() const { return std::holds_alternative<option>(val_); }
    auto is_param() const { return std::holds_alternative<param>(val_); }

    auto&& to_option() && { return std::get<option>(val_); }
    auto&& to_param() && { return std::get<param>(val_); }

private:
    std::variant<option, param> val_;
};

////////////////////////////////////////////////////////////////////////////////
// program arguments
struct args
{
    args() = default;
    explicit args(std::initializer_list<arg> il) { for(auto& el : il) add(move(el)); }

    void add(arg);

    template<typename... Args>
    void add(Args&&... args) { add(arg{ std::forward<Args>(args)... }); }

    argval const& operator[](string_view) const;

    void parse(int argc, char* argv[]);
    string usage(string_view program, string_view description = { }) const;

private:
    vector<option> options_;
    vector<param> params_;
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

// find container element with member equal to `what`
template<typename Cont, typename T>
inline auto find_equal(Cont&& cont, string T::* memptr, string_view what)
{
    return std::find_if(begin(cont), end(cont),
        [&](T const& el){ return el.*memptr == what; }
    );
}

// find container element with either of two members equal to `what`
template<typename Cont, typename T>
inline auto find_equal(Cont&& cont, string T::* memptr1, string T::* memptr2, string_view what)
{
    return std::find_if(begin(cont), end(cont),
        [&](T const& el){ return el.*memptr1 == what || el.*memptr2 == what; }
    );
}

// return true if container has an element with member equal to `what`
template<typename Cont, typename T>
inline auto has_equal(Cont&& cont, string T::* elem, string_view what)
{
    return find_equal(cont, elem, what) != end(cont);
}

}

////////////////////////////////////////////////////////////////////////////////
inline arg::arg(string name1, spec spc, string description)
{
    if(is_short_option(name1))
        val_ = option::from(move(name1), { }, { }, spc, move(description));

    else if(is_long_option(name1))
        val_ = option::from({ }, move(name1), { }, spc, move(description));

    else if(is_param_name(name1))
        val_ = param::from(move(name1), spc, move(description));

    else throw invalid_definition{"'"+name1+"' not a valid option or param name"};
}

////////////////////////////////////////////////////////////////////////////////
inline arg::arg(string name1, string name2, spec spc, string description)
{
    if(is_short_option(name1))
    {
        if(is_long_option(name2))
            val_ = option::from(move(name1), move(name2), { }, spc, move(description));

        else if(is_valname(name2))
            val_ = option::from(move(name1), { }, move(name2), spc, move(description));

        else throw invalid_definition{"'"+name2+"' not a valid long option or option value name"};
    }
    else if(is_long_option(name1))
    {
        if(is_valname(name2))
            val_ = option::from({ }, move(name1), move(name2), spc, move(description));

        else throw invalid_definition{"'"+name2+"' not a valid option value name"};
    }
    else throw invalid_definition{"'"+name1+"' not a valid short or long option name"};
}

////////////////////////////////////////////////////////////////////////////////
inline arg::arg(string name1, string name2, string name3, spec spc, string description)
{
    if(!is_short_option(name1))
        throw invalid_definition{"'"+name1+"' not a valid short option name"};

    else if(!is_long_option(name2))
        throw invalid_definition{"'"+name2+"' not a valid long option name" };

    else if(!is_valname(name3))
        throw invalid_definition{"'"+name3+"' not a valid option value name"};

    else val_ = option::from(move(name1), move(name2), move(name3), spc, move(description));
}

////////////////////////////////////////////////////////////////////////////////
inline void args::add(pgm::arg arg)
{
    if(arg.is_option())
    {
        auto new_ = move(arg).to_option();

        if(new_.short_.size())
        {
            if(has_equal(options_, &option::short_, new_.short_))
                throw invalid_definition{"duplicate short option '"+new_.short_+"'"};
        }
        if(new_.long_.size())
        {
            if(has_equal(options_, &option::long_, new_.long_))
                throw invalid_definition{"duplicate long option '"+new_.long_+"'"};
        }

        options_.push_back(move(new_));
    }
    else if(arg.is_param())
    {
        auto new_ = move(arg).to_param();

        if(has_equal(params_, &param::name_, new_.name_))
            throw invalid_definition{"duplicate param '"+new_.name_+"'"};

        if(params_.size())
        {
            if(params_.back().mul_) throw invalid_definition{
                "'"+new_.name_+"' after multi-value param"
            };

            if(params_.back().opt_ && !new_.opt_) throw invalid_definition{
                "non-optional '"+new_.name_+"' after optional param"
            };
        }

        params_.push_back(move(new_));
    }
    else throw invalid_definition{"neither option nor param"};
}

////////////////////////////////////////////////////////////////////////////////
inline argval const& args::operator[](string_view name) const
{
    if(!name.empty())
    {
        if(auto it = find_equal(options_, &option::short_, &option::long_, name);
            it != options_.end()
        ) return it->values_;

        if(auto it = find_equal(params_, &param::name_, name);
            it != params_.end()
        ) return it->values_;
    }
    throw invalid_argument{"option or param '"+string{name}+"' not defined"};
}

////////////////////////////////////////////////////////////////////////////////
namespace
{

inline auto pop(deque<string>& q)
{
    auto el = std::move(q.front());
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
    deque<string> args;
    for(auto n = 1; n < argc; ++n) args.emplace_back(argv[n]);

    bool had_token = false;
    deque<string> saved_params;

    while(args.size())
    {
        auto arg = pop(args);

        if(had_token || is_not_option(arg)) // param ("", "-" or re: "[^-].+")
            saved_params.push_back(std::move(arg));

        else if(arg == "--") // end-of-options token
            had_token = true;

        else  // option (re: "-.+")
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

            // see if we have this option
            auto it = find_equal(options_, &option::short_, &option::long_, name);
            if(it == options_.end())
                throw invalid_argument{"option '"+name+"' not defined"};

            if(it->valname_.empty()) // doesn't take values
            {
                if(value) // but we have one
                {
                    if(name.size() == 2) // short option
                    {
                        // assume this was a group of short options (eg, -abc)
                        // and push them to the front of the queue
                        args.push_front("-" + *value);
                    }
                    else throw invalid_argument{"option '"+name+"' doesn't take values"};
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
                    else throw missing_argument{"option '"+name+"' requires a value"};
                }
            }

            if(it->values_.size() && !it->mul_)
                throw invalid_argument{"duplicate option '"+name+"'"};

            it->values_.add(std::move(*value));
        }
    }

    // check required options
    for(auto const& el : options_)
        if(el.req_ && !el.values_.size())
            throw missing_argument{"option "+(
                el.short_.size() && el.long_.size() ?  "'"+el.short_+"/"+el.long_+"'" :
                    el.short_.size() ? "'"+el.short_+"'" : "'"+el.long_+"'"
            )+" is required"};

    // process saved params
    for(auto& el : params_)
    {
        if(saved_params.size())
        {
            do el.values_.add(pop(saved_params));
            while(el.mul_ && saved_params.size());
        }
        else if(!el.opt_) throw missing_argument{"param '"+el.name_+"' is required"};
    }

    if(saved_params.size()) throw invalid_argument{
        "extra param '"+saved_params[0]+"'"
    };
}

inline string args::usage(string_view program, string_view description) const
{
    std::ostringstream os;

    return os.str();
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
