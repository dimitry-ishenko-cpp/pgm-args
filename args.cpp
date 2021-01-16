////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2021 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "args.hpp"

#include <algorithm>
#include <cctype>

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
    auto is_valid = [](char c) { return c != '!' && c != '?' && c != '*' && std::isgraph(c); };

    if(s.size() && s[0] != '-' && std::all_of(s.begin(), s.end(), is_valid))
        return true;
    else throw invalid_definition{ "bad param name", s };
}

////////////////////////////////////////////////////////////////////////////////
void strip_specifiers(string& name, bool& multiple, bool& opt_val, bool& required)
{
    for(auto n = 0; n < 3; ++n)
    {
        if(name.empty()) break;

        if(name.back() == '*')
        {
            if(!multiple) multiple = true;
            else throw invalid_definition{ "duplicate specifier", name };

            name.pop_back();
        }
        else if(name.back() == '?')
        {
            if(!opt_val) opt_val = true;
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

    if(name.empty() && opt_val) throw invalid_definition{ "bad specifier", name };
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
        // this is a positional parameter
        // and not an option
        required = true;
        name = std::move(name_code_or_full);

        if(name.back() == '*')
        {
            multiple = true;
            name.pop_back();
        }
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
        // this is option parameter
        // and/or option specifiers
        name = std::move(full_or_name);

        strip_specifiers(name, multiple, val_opt, required);
        if(name.size()) valid_name(name);
    }
}

////////////////////////////////////////////////////////////////////////////////
arg::arg(string code_, string full_, string name_, string description_) :
    description{ std::move(description_) }
{
    if(valid_code(code_)) code = std::move(code_);
    else throw invalid_definition{ "bad option name", code_ };

    if(valid_full(full_)) full = std::move(full_);
    else throw invalid_definition{ "bad option name", full_ };

    // this is option parameter
    // and/or option specifiers
    name = std::move(name_);

    strip_specifiers(name, multiple, val_opt, required);
    if(name.size()) valid_name(name);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
namespace
{

auto find_option(const std::vector<arg>& options, const string& name)
{
    auto is_match = [&](const arg& o) { return o.code == name || o.full == name; };
    return std::find_if(options.begin(), options.end(), is_match);

}

auto find_param(const std::vector<arg>& params, const string& name)
{
    auto is_match = [&](const arg& p) { return p.name == name; };
    return std::find_if(params.begin(), params.end(), is_match);
}

}

////////////////////////////////////////////////////////////////////////////////
args::args(std::initializer_list<arg> il)
{
    for(auto& arg : il) (*this) << std::move(arg);
}

////////////////////////////////////////////////////////////////////////////////
args& args::operator<<(pgm::arg arg)
{
    bool new_opt = false;
    if(arg.code.size())
    {
        if(find_option(opts, arg.code) == opts.end())
            new_opt = true;
        else throw invalid_definition{ "duplicate option name", arg.code };
    }

    if(arg.full.size())
    {
        if(find_option(opts, arg.full) == opts.end())
            new_opt = true;
        else throw invalid_definition{ "duplicate option name", arg.full };
    }

    if(!new_opt)
    {
        if(find_param(params, arg.name) == params.end())
        {
            if(arg.multiple)
                for(auto const& p : params)
                    if(p.multiple)
                        throw invalid_definition{ "second multi-value param", arg.name };

            params.push_back(std::move(arg));
        }
        else throw invalid_definition{ "duplicate param name", arg.name };
    }
    else opts.push_back(std::move(arg));

    return (*this);
}

////////////////////////////////////////////////////////////////////////////////
const arg& args::find(const std::string& arg) const
{
    if(auto it = find_option(opts, arg); it != opts.end()) return *it;
    if(auto it = find_param(params, arg); it != params.end()) return *it;

    throw invalid_argument{ "Invalid option or param name", arg };
}

////////////////////////////////////////////////////////////////////////////////
void args::parse(int argc, char* argv[])
{

}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
