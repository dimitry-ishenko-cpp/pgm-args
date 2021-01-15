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
bool ends_with(const string& s, const string& end)
{
    if(s.size() >= end.size())
        return s.substr(s.size() - end.size()) == end;
    else return false;
}

////////////////////////////////////////////////////////////////////////////////
void remove_end(string& s, const string& end)
{
    s.erase(s.size() - end.size());
}

////////////////////////////////////////////////////////////////////////////////
bool is_valid(char c) { return c == '-' || std::isalnum(c); }

bool is_valid(const string& s)
{
    return s.size() && s[0] != '-' && std::all_of(
        s.begin(), s.end(), static_cast<bool (*)(char)>(&is_valid)
    );
}

////////////////////////////////////////////////////////////////////////////////
bool is_code(const string& s)
{
    return s.size() == 2 && s[0] == '-' && s[1] != '-';
}

////////////////////////////////////////////////////////////////////////////////
bool is_full(const string& s)
{
    return s.size() > 2 && s[0] == '-' && s[1] == '-' && s[2] != '-';
}

////////////////////////////////////////////////////////////////////////////////
void to_code(string s, string& code)
{
    code = std::move(s);
    if(!is_valid(code.substr(1))) throw invalid_definition{ "bad option name", code };
}

////////////////////////////////////////////////////////////////////////////////
void to_full(string s, string& full)
{
    full = std::move(s);
    if(!is_valid(full.substr(2))) throw invalid_definition{ "bad option name", full };
}

////////////////////////////////////////////////////////////////////////////////
void to_name(const string& s, string& name, bool& multiple)
{
    name = s;

    if(ends_with(name, "..."))
    {
        multiple = true;
        remove_end(name, "...");
    }
    if(!is_valid(name)) throw invalid_definition{ "bad parameter name", s };
}

////////////////////////////////////////////////////////////////////////////////
void to_name(const string& s, string& name, bool& multiple, bool& opt_val, bool& required)
{
    name = s;

    for(auto n = 0; n < 3; ++n)
        if(ends_with(name, "..."))
        {
            if(multiple) throw invalid_definition{ "duplicate specifier", s };
            multiple = true;
            remove_end(name, "...");
        }
        else if(ends_with(name, "?"))
        {
            if(opt_val ) throw invalid_definition{ "duplicate specifier", s };
            opt_val = true;
            remove_end(name, "?");
        }
        else if(ends_with(name, "!"))
        {
            if(required) throw invalid_definition{ "duplicate specifier", s };
            required = true;
            remove_end(name, "!");
        }
        else break;

    if(name.empty())
    {
        if(opt_val) throw invalid_definition{ "bad specifier", s };
    }
    else if(!is_valid(name)) throw invalid_definition{ "bad parameter name", s };
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
arg::arg(string name_code_or_full, string description_) :
    description{ std::move(description_) }
{
    if(is_code(name_code_or_full))
        to_code(std::move(name_code_or_full), code);

    else if(is_full(name_code_or_full))
        to_full(std::move(name_code_or_full), full);
    else
    {
        to_name(name_code_or_full, name, multiple);
        required = true;
    }
}

////////////////////////////////////////////////////////////////////////////////
arg::arg(string code_or_full, string full_or_name, string description_) :
    description{ std::move(description_) }
{
    if(is_code(code_or_full))
        to_code(std::move(code_or_full), code);

    else if(is_full(code_or_full))
        to_full(std::move(code_or_full), full);

    else throw invalid_definition{ "bad option name", code_or_full };

    if(full.empty() && is_full(full_or_name))
        to_full(std::move(full_or_name), full);

    else to_name(full_or_name, name, multiple, val_opt, required);
}

////////////////////////////////////////////////////////////////////////////////
arg::arg(string code_, string full_, string name_, string description_) :
    description{ std::move(description_) }
{
    if(is_code(code_))
        to_code(std::move(code_), code);

    else throw invalid_definition{ "bad option name", code_ };

    if(is_full(full_))
        to_full(std::move(full_), full);

    else throw invalid_definition{ "bad option name", full_ };

    to_name(name_, name, multiple, val_opt, required);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
namespace
{

auto find_option(const std::vector<arg>& options, const string& s)
{
    return std::find_if(options.begin(), options.end(),
        [&](const arg& opt) { return opt.code == s || opt.full == s; }
    );

}

auto find_param(const std::vector<arg>& params, const string& s)
{
    return std::find_if(params.begin(), params.end(),
        [&](const arg& param) { return param.name == s; }
    );
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
    if(arg.code.size() || arg.full.size())
    {
        if(arg.code.size() && find_option(opts, arg.code) != opts.end())
            throw invalid_definition{
                "duplicate option name", arg.code
            };

        if(arg.full.size() && find_option(opts, arg.full) != opts.end())
            throw invalid_definition{
                "duplicate option name", arg.full
            };

        opts.push_back(std::move(arg));
    }
    else
    {
        if(find_param(params, arg.name) != params.end())
            throw invalid_definition{
                "duplicate parameter name", arg.name
            };

        if(arg.multiple)
            for(auto const& param : params)
                if(param.multiple) throw invalid_definition{
                    "2nd parameter with with multiple values", arg.name
                };

        params.push_back(std::move(arg));
    }

    return (*this);
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
