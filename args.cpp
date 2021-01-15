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
bool ends_with(const std::string& s, const std::string& end)
{
    if(s.size() >= end.size())
        return s.substr(s.size() - end.size()) == end;
    else return false;
}

////////////////////////////////////////////////////////////////////////////////
void remove_end(std::string& s, const std::string& end)
{
    s.erase(s.size() - end.size());
}

////////////////////////////////////////////////////////////////////////////////
bool is_valid(char c) { return c == '-' || std::isalnum(c); }

bool is_valid(const std::string& s)
{
    return s.size() && s[0] != '-' && std::all_of(
        s.begin(), s.end(), static_cast<bool (*)(char)>(&is_valid)
    );
}

////////////////////////////////////////////////////////////////////////////////
bool is_code(const std::string& s)
{
    return s.size() == 2 && s[0] == '-' && s[1] != '-';
}

////////////////////////////////////////////////////////////////////////////////
bool is_full(const std::string& s)
{
    return s.size() > 2 && s[0] == '-' && s[1] == '-' && s[2] != '-';
}

////////////////////////////////////////////////////////////////////////////////
void to_code(std::string s, std::string& code)
{
    code = std::move(s);
    if(!is_valid(code.substr(1))) throw invalid_definition{ "bad option name", code };
}

////////////////////////////////////////////////////////////////////////////////
void to_full(std::string s, std::string& full)
{
    full = std::move(s);
    if(!is_valid(full.substr(2))) throw invalid_definition{ "bad option name", full };
}

////////////////////////////////////////////////////////////////////////////////
void to_name(const std::string& s, std::string& name, bool& multiple)
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
void to_name(const std::string& s, std::string& name, bool& multiple, bool& opt_val, bool& required)
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
arg::arg(std::string name_code_or_full, std::string description) :
    description_{ std::move(description) }
{
    if(is_code(name_code_or_full))
        to_code(std::move(name_code_or_full), code_);

    else if(is_full(name_code_or_full))
        to_full(std::move(name_code_or_full), full_);
    else
    {
        to_name(name_code_or_full, name_, multiple_);
        required_ = true;
    }
}

////////////////////////////////////////////////////////////////////////////////
arg::arg(std::string code_or_full, std::string full_or_name, std::string description) :
    description_{ std::move(description) }
{
    if(is_code(code_or_full))
        to_code(std::move(code_or_full), code_);

    else if(is_full(code_or_full))
        to_full(std::move(code_or_full), full_);

    else throw invalid_definition{ "bad option name", code_or_full };

    if(full_.empty() && is_full(full_or_name))
        to_full(std::move(full_or_name), full_);

    else to_name(full_or_name, name_, multiple_, opt_val_, required_);
}

////////////////////////////////////////////////////////////////////////////////
arg::arg(std::string code, std::string full, std::string name, std::string description) :
    description_{ std::move(description) }
{
    if(is_code(code))
        to_code(std::move(code), code_);

    else throw invalid_definition{ "bad option name", code };

    if(is_full(full))
        to_full(std::move(full), this->full_);

    else throw invalid_definition{ "bad option name", full };

    to_name(name, name_, multiple_, opt_val_, required_);
}

////////////////////////////////////////////////////////////////////////////////
args::args(std::initializer_list<arg> il)
{
    for(auto& arg : il) (*this) << std::move(arg);
}

////////////////////////////////////////////////////////////////////////////////
args& args::operator<<(pgm::arg arg)
{
    if(arg.is_param())
    {
        if(find_param(arg.name()) != params_.end()) throw invalid_definition{
            "duplicate parameter name", arg.name()
        };
        if(arg.is_multiple())
        {
            if(has_multiple_) throw invalid_definition{
                "2nd parameter with with multiple values", arg.name()
            };
            has_multiple_ = true;
        }
        params_.push_back(std::move(arg));
    }
    else
    {
        if(arg.has_code())
            if(find_option(arg.code()) != options_.end()) throw invalid_definition{
                "duplicate option name", arg.code()
            };

        if(arg.has_full())
            if(find_option(arg.full()) != options_.end()) throw invalid_definition{
                "duplicate option name", arg.full()
            };

        options_.push_back(std::move(arg));
    }

    return (*this);
}

////////////////////////////////////////////////////////////////////////////////
args::iterator args::find_option(const std::string& cf)
{
    return std::find_if(options_.begin(), options_.end(),
        [&](const arg& opt) { return opt.code() == cf || opt.full() == cf; }
    );
}

////////////////////////////////////////////////////////////////////////////////
args::iterator args::find_param(const std::string& name)
{
    return std::find_if(params_.begin(), params_.end(),
        [&](const arg& param) { return param.name() == name; }
    );
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
