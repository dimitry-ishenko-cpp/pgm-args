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
bool valid_code(const string& s)
{
    if(s.size() == 2 && s[0] == '-' && s[1] != '-')
    {
        if(std::isalnum(s[1])) return true;
        else throw invalid_definition{ "bad option name", s };
    }
    else return false;
}

////////////////////////////////////////////////////////////////////////////////
bool valid_full(const string& s)
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
bool valid_name(const string& s)
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
void strip_specifiers(string& name, bool& multiple, bool& val_opt, bool& required)
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
arg::arg(string name_code_or_full, string description_) :
    description{ std::move(description_) }
{
    if(valid_code(name_code_or_full))
        code = std::move(name_code_or_full);

    else if(valid_full(name_code_or_full))
        full = std::move(name_code_or_full);

    else
    {
        // this is a positional param name
        name = name_code_or_full;
        strip_specifiers(name, multiple, val_opt, required);

        // required only applies to options
        if(required) throw invalid_definition{ "bad specifier", name_code_or_full };
        valid_name(name);
    }
}

////////////////////////////////////////////////////////////////////////////////
arg::arg(string code_or_full, string full_or_name, string description_) :
    description{ std::move(description_) }
{
    if(valid_code(code_or_full))
        code = std::move(code_or_full);

    else if(valid_full(code_or_full))
        full = std::move(code_or_full);

    else throw invalid_definition{ "bad option name", code_or_full };

    if(full.empty() && valid_full(full_or_name))
        full = std::move(full_or_name);

    else
    {
        // this is option param name and/or option specifier(s)
        name = full_or_name;
        strip_specifiers(name, multiple, val_opt, required);

        // option has no param (ie, takes no value)
        if(name.empty())
        {
            // val_opt only applies to option with param
            if(val_opt) throw invalid_definition{ "bad specifier", full_or_name };
        }
        else valid_name(name);
    }
}

////////////////////////////////////////////////////////////////////////////////
arg::arg(string code_, string full_, string name_, string description_) :
    description{ std::move(description_) }
{
    if(valid_code(code_))
        code = std::move(code_);
    else throw invalid_definition{ "bad option name", code_ };

    if(valid_full(full_))
        full = std::move(full_);
    else throw invalid_definition{ "bad option name", full_ };

    // this is option param name and/or option specifier(s)
    name = name_;
    strip_specifiers(name, multiple, val_opt, required);

    // option has no param (ie, takes no value)
    if(name.empty())
    {
        // val_opt only applies to option with param
        if(val_opt) throw invalid_definition{ "bad specifier", name_ };
    }
    else valid_name(name);
}

////////////////////////////////////////////////////////////////////////////////
namespace
{

auto find_option(const std::vector<arg>& opts, const string& s)
{
    auto is_match = [&](const arg& opt) { return opt.code == s || opt.full == s; };
    return std::find_if(opts.begin(), opts.end(), is_match);

}

auto find_option(std::vector<arg>& opts, const string& s)
{
    auto is_match = [&](const arg& opt) { return opt.code == s || opt.full == s; };
    return std::find_if(opts.begin(), opts.end(), is_match);

}

auto find_param(const std::vector<arg>& pars, const string& s)
{
    auto is_match = [&](const arg& par) { return par.name == s; };
    return std::find_if(pars.begin(), pars.end(), is_match);
}

auto take_front(std::deque<string>& d)
{
    auto e{ std::move(d.front()) };
    d.pop_front();
    return e;
}

bool is_param(const string& s)
{
    return s.empty() || s == "-" || s[0] != '-';
}

auto split_option(const string& arg)
{
    string opt;
    std::optional<string> val;

    if(arg[1] == '-') // long option (full)
    {
        if(auto p = arg.find('=', 2); p != string::npos)
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
args::args(std::initializer_list<arg> il)
{
    for(auto& arg : il) add(std::move(arg));
}

////////////////////////////////////////////////////////////////////////////////
void args::add(pgm::arg arg)
{
    bool new_opt = false;
    if(arg.code.size())
    {
        if(find_option(options, arg.code) == options.end())
            new_opt = true;
        else throw invalid_definition{ "duplicate option name", arg.code };
    }

    if(arg.full.size())
    {
        if(find_option(options, arg.full) == options.end())
            new_opt = true;
        else throw invalid_definition{ "duplicate option name", arg.full };
    }

    if(new_opt)
        options.push_back(std::move(arg));

    else
    {
        if(find_param(params, arg.name) == params.end())
        {
            if(params.size())
            {
                if(params.back().multiple)
                    throw invalid_definition{
                        "another param after multi-value param", arg.name
                    };

                if(params.back().val_opt && !arg.val_opt)
                    throw invalid_definition{
                        "non-optional param after optional one", arg.name
                    };
            }
            params.push_back(std::move(arg));
        }
        else throw invalid_definition{ "duplicate param name", arg.name };
    }
}

////////////////////////////////////////////////////////////////////////////////
const arg& args::find(const string& arg) const
{
    if(auto it = find_option(options, arg); it != options.end()) return *it;
    if(auto it = find_param(params, arg); it != params.end()) return *it;

    throw invalid_argument{ arg };
}

////////////////////////////////////////////////////////////////////////////////
void args::parse(int argc, char* argv[])
{
    std::deque<string> args, todo;
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

            if(auto it = find_option(options, opt); it != options.end())
            {
                // option takes no value
                if(it->name.empty())
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
                else if(it->val_opt)
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

                if(it->values.size() && !it->multiple)
                    throw duplicate_option{ arg };

                if(!val) val = string{ };
                it->values.push_back(std::move(*val));
            }
            else throw invalid_argument{ arg };
        }
    }

    // check required options
    for(auto const& opt : options)
        if(opt.required && opt.values.empty())
            throw missing_option{ opt.code, opt.full };

    // process collected parameters
    for(auto& par : params)
    {
        if(todo.size())
            do par.values.push_back(take_front(todo));
            while(par.multiple && todo.size());

        else if(!par.val_opt)
            throw missing_argument{ par.name };
    }

    if(todo.size()) throw invalid_argument{ todo.front() };
}

////////////////////////////////////////////////////////////////////////////////
string args::usage(const string& program, const string& description)
{
    std::ostringstream os;
    os << std::left;

    os << "Usage: " << program;
    if(options.size()) os << " [option]...";
    if(params.size())
    {
        for(auto const& par : params)
            os << (par.val_opt ? " [" + par.name + "]" : " <" + par.name + ">");
        if(params.back().multiple) os << "...";
    }

    std::size_t size = 0;
    std::vector<std::tuple<string, string>> args;

    for(auto const& opt : options)
    {
        string name{ opt.name };
        if(name.size())
        {
            name = "<" + name + ">";
            if(opt.full.size()) name = "=" + name;
            if(opt.val_opt) name = "[" + name + "]";
            if(!opt.full.size()) name = " " + name;
        }

        string fill;
        if(opt.full.size()) fill = opt.code.size() ? ", " : "    ";

        auto arg{ "  " + opt.code + fill + opt.full + name + "  " };
        size = std::max(size, arg.size());
        args.emplace_back(std::move(arg), opt.description + '\n');
    }

    // add blank line between options and params
    if(params.size()) args.emplace_back(" ", " ");

    for(auto const& par : params)
    {
        auto arg{ "  <" + par.name + ">  " };
        size = std::max(size, arg.size());
        args.emplace_back(std::move(arg), par.description + '\n');
    }

    if(args.size()) os << '\n';
    for(auto const& [ arg, desc ] : args)
    {
        string read;
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
