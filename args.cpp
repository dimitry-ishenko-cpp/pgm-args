////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2021 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "args.hpp"

#include <algorithm>
#include <cctype>
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

////////////////////////////////////////////////////////////////////////////////
bool valid_code(const std::string& s)
{
    if(s.size() == 2 && s[0] == '-' && s[1] != '-')
    {
        if(std::isalnum(s[1])) return true;
        else throw invalid_definition{ "bad option name", s };
    }
    else return false;
}

////////////////////////////////////////////////////////////////////////////////
bool valid_full(const std::string& s)
{
    if(s.size() > 2 && s[0] == '-' && s[1] == '-' && s[2] != '-')
    {
        auto is_valid = [](char c) { return c == '-' || std::isalnum(c); };

        if(std::all_of(s.begin() + 2, s.end(), is_valid))
            return true;
        else throw invalid_definition{ "bad option name", s };
    }
    else return false;
}

////////////////////////////////////////////////////////////////////////////////
bool valid_name(const std::string& s)
{
    auto is_valid = [](char c)
    {
        return c != '+' && c != '?' && c != '!' && std::isgraph(c);
    };

    if(s.size() && s[0] != '-' && std::all_of(s.begin(), s.end(), is_valid))
        return true;
    else throw invalid_definition{ "bad param name", s };
}

////////////////////////////////////////////////////////////////////////////////
void strip_specifiers(std::string& name, bool& multiple, bool& val_opt, bool& required)
{
    for(auto n = 0; n < 3; ++n)
    {
        if(name.empty()) break;

        if(name.back() == '+')
        {
            if(!multiple) multiple = true;
            else throw invalid_definition{ "duplicate specifier", name };

            name.pop_back();
        }
        else if(name.back() == '?')
        {
            if(!val_opt) val_opt = true;
            else throw invalid_definition{ "duplicate specifier", name };

            name.pop_back();
        }
        else if(name.back() == '!')
        {
            if(!required) required = true;
            else throw invalid_definition{ "duplicate specifier", name };

            name.pop_back();
        }
        else break;
    }
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
arg::arg(std::string name_code_or_full, std::string description) :
    description_{ std::move(description) }
{
    if(valid_code(name_code_or_full))
        code_ = std::move(name_code_or_full);

    else if(valid_full(name_code_or_full))
        full_ = std::move(name_code_or_full);

    else
    {
        // this is a positional param name
        name_ = name_code_or_full;
        strip_specifiers(name_, multiple_, val_opt_, required_);

        // required only applies to options
        if(required_) throw invalid_definition{ "bad specifier", name_code_or_full };
        valid_name(name_);
    }
}

////////////////////////////////////////////////////////////////////////////////
arg::arg(std::string code_or_full, std::string full_or_name, std::string description) :
    description_{ std::move(description) }
{
    if(valid_code(code_or_full))
        code_ = std::move(code_or_full);

    else if(valid_full(code_or_full))
        full_ = std::move(code_or_full);

    else throw invalid_definition{ "bad option name", code_or_full };

    if(full_.empty() && valid_full(full_or_name))
        full_ = std::move(full_or_name);

    else
    {
        // this is option param name and/or option specifier(s)
        name_ = full_or_name;
        strip_specifiers(name_, multiple_, val_opt_, required_);

        // option has no param (ie, takes no value)
        if(name_.empty())
        {
            // val_opt only applies to option with param
            if(val_opt_) throw invalid_definition{ "bad specifier", full_or_name };
        }
        else valid_name(name_);
    }
}

////////////////////////////////////////////////////////////////////////////////
arg::arg(std::string code, std::string full, std::string name, std::string description) :
    description_{ std::move(description) }
{
    if(valid_code(code))
        code_ = std::move(code);
    else throw invalid_definition{ "bad option name", code_ };

    if(valid_full(full))
        full_ = std::move(full);
    else throw invalid_definition{ "bad option name", full };

    // this is option param name and/or option specifier(s)
    name_ = name;
    strip_specifiers(name_, multiple_, val_opt_, required_);

    // option has no param (ie, takes no value)
    if(name_.empty())
    {
        // val_opt only applies to option with param
        if(val_opt_) throw invalid_definition{ "bad specifier", name };
    }
    else valid_name(name_);
}

////////////////////////////////////////////////////////////////////////////////
args::args(std::initializer_list<arg> il)
{
    for(auto& arg : il) add(std::move(arg));
}

////////////////////////////////////////////////////////////////////////////////
void args::add(pgm::arg arg)
{
    bool new_opt = false;
    if(arg.code_.size())
    {
        if(find_option(arg.code_) == options.end())
            new_opt = true;
        else throw invalid_definition{ "duplicate option name", arg.code_ };
    }

    if(arg.full_.size())
    {
        if(find_option(arg.full_) == options.end())
            new_opt = true;
        else throw invalid_definition{ "duplicate option name", arg.full_ };
    }

    if(new_opt)
        options.push_back(std::move(arg));

    else
    {
        if(find_param(arg.name_) == params.end())
        {
            if(params.size())
            {
                if(params.back().multiple_)
                    throw invalid_definition{
                        "another param after multi-value param", arg.name_
                    };

                if(params.back().val_opt_ && !arg.val_opt_)
                    throw invalid_definition{
                        "non-optional param after optional one", arg.name_
                    };
            }
            params.push_back(std::move(arg));
        }
        else throw invalid_definition{ "duplicate param name", arg.name_ };
    }
}

////////////////////////////////////////////////////////////////////////////////
auto args::find_option(const std::string& s) const -> const_iterator
{
    return std::find_if(options.begin(), options.end(),
        [&](const arg& opt) { return opt.code_ == s || opt.full_ == s; }
    );
}

////////////////////////////////////////////////////////////////////////////////
auto args::find_option(const std::string& s) -> iterator
{
    return std::find_if(options.begin(), options.end(),
        [&](const arg& opt) { return opt.code_ == s || opt.full_ == s; }
    );
}

////////////////////////////////////////////////////////////////////////////////
auto args::find_param(const std::string& s) const -> const_iterator
{
    return std::find_if(params.begin(), params.end(),
        [&](const arg& par) { return par.name_ == s; }
    );
}

////////////////////////////////////////////////////////////////////////////////
const arg& args::find(const std::string& s) const
{
    if(auto it = find_option(s); it != options.end()) return *it;
    if(auto it = find_param(s); it != params.end()) return *it;

    throw invalid_argument{ s };
}

////////////////////////////////////////////////////////////////////////////////
namespace
{

auto take_front(std::deque<std::string>& d)
{
    auto e{ std::move(d.front()) };
    d.pop_front();
    return e;
}

bool is_param(const std::string& s)
{
    return s.empty() || s == "-" || s[0] != '-';
}

auto split_option(const std::string& arg)
{
    std::string opt;
    std::optional<std::string> val;

    if(arg[1] == '-') // long option (full)
    {
        if(auto p = arg.find('=', 2); p != std::string::npos)
        {
            opt = arg.substr(0, p);
            val = arg.substr(p + 1);
        }
        else opt = arg;
    }
    else // short option (code)
    {
        opt = arg.substr(0, 2);
        if(arg.size() > 2) val = arg.substr(2);
    }

    return std::make_tuple(opt, val);
}

}

////////////////////////////////////////////////////////////////////////////////
void args::parse(int argc, char* argv[])
{
    std::deque<std::string> args, todo;
    for(int n = 1; n < argc; ++n) args.emplace_back(argv[n]);

    bool had_token = false;
    while(args.size())
    {
        auto arg{ take_front(args) };

        if(had_token || is_param(arg))
            todo.push_back(std::move(arg)); // params are processed below

        else if(arg == "--")
            had_token = true;

        else // option?
        {
            auto [ opt, val ] = split_option(arg);

            if(auto it = find_option(opt); it != options.end())
            {
                // option takes no value
                if(it->name_.empty())
                {
                    if(val)
                    {
                        // this could be a group of short options (eg, -abc)
                        if(opt.size() == 2)
                            args.push_front('-' + *val);
                        else throw extra_value{ arg };
                    }
                }
                // option takes optional value
                else if(it->val_opt_)
                {
                    if(!val && args.size() && is_param(args.front()))
                        val = take_front(args);
                }
                // option requires value
                else
                {
                    if(!val)
                    {
                        if(args.size() && args.front() != "--")
                            val = take_front(args);
                        else throw missing_value{ arg };
                    }
                }

                if(it->values_.size() && !it->multiple_)
                    throw duplicate_option{ arg };

                if(!val) val = std::string{ };
                it->values_.push_back(std::move(*val));
            }
            else throw invalid_argument{ arg };
        }
    }

    // check required options
    for(auto const& opt : options)
        if(opt.required_ && opt.values_.empty())
            throw missing_option{ opt.code_, opt.full_ };

    // process collected parameters
    for(auto& par : params)
    {
        if(todo.size())
            do par.values_.push_back(take_front(todo));
            while(par.multiple_ && todo.size());

        else if(!par.val_opt_)
            throw missing_argument{ par.name_ };
    }

    if(todo.size()) throw invalid_argument{ todo.front() };
}

////////////////////////////////////////////////////////////////////////////////
std::string args::usage(const std::string& program, const std::string& description)
{
    std::ostringstream os;
    os << std::left;

    os << "Usage: " << program;
    if(options.size()) os << " [option]...";
    if(params.size())
    {
        for(auto const& par : params)
            os << (par.val_opt_ ? " [" + par.name_ + "]" : " <" + par.name_ + ">");
        if(params.back().multiple_) os << "...";
    }

    std::size_t size = 0;
    std::vector<std::tuple<std::string, std::string>> args;

    for(auto const& opt : options)
    {
        std::string name{ opt.name_ };
        if(name.size())
        {
            name = "<" + name + ">";
            if(opt.full_.size()) name = "=" + name;
            if(opt.val_opt_) name = "[" + name + "]";
            if(!opt.full_.size()) name = " " + name;
        }

        std::string fill;
        if(opt.full_.size()) fill = opt.code_.size() ? ", " : "    ";

        auto arg{ "  " + opt.code_ + fill + opt.full_ + name + "  " };
        size = std::max(size, arg.size());
        args.emplace_back(std::move(arg), opt.description_ + '\n');
    }

    // add blank line between options and params
    if(params.size()) args.emplace_back(" ", " ");

    for(auto const& par : params)
    {
        auto arg{ "  <" + par.name_ + ">  " };
        size = std::max(size, arg.size());
        args.emplace_back(std::move(arg), par.description_ + '\n');
    }

    if(args.size()) os << '\n';
    for(auto const& [ arg, desc ] : args)
    {
        std::string read;
        bool once = true;
        for(std::istringstream is{ desc }; std::getline(is, read); )
        {
            os << '\n' << std::setw(size) << (once ? arg : "") << read;
            once = false;
        }
    }
    if(description.size()) os << "\n\n" << description;

    return os.str();
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
