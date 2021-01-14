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

bool is_name(const std::string& s)
{
    return s.size() > 2 && s[0] == '-' && s[1] == '-' && s[2] != '-';
}

////////////////////////////////////////////////////////////////////////////////
void to_code(std::string s, std::string& code)
{
    code = std::move(s);
    if(!is_valid(code)) throw 0; // ***
}

void to_name(std::string s, std::string& name)
{
    name = std::move(s);
    if(!is_valid(name)) throw 0; // ***
}

void to_param(std::string s, std::string& param, bool& poly, bool& opt_val, bool& required)
{
    param = std::move(s);

    for(auto n = 0; n < 3; ++n)
        if(ends_with(param, "..."))
        {
            if(poly) throw 0; // ***
            poly = true;
            remove_end(param, "...");
        }
        else if(ends_with(param, "?"))
        {
            if(opt_val) throw 0; // ***
            opt_val = true;
            remove_end(param, "?");
        }
        else if(ends_with(param, "!"))
        {
            if(required) throw 0; // ***
            required = true;
            remove_end(param, "!");
        }
        else break;

    if(!is_valid(param)) throw 0; // ***
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
param::param(std::string name, std::string description) :
    name_{ std::move(name) }, description_{ std::move(description) }
{
    if(ends_with(name_, "..."))
    {
        remove_end(name_, "...");
        poly_ = true;
    }
    if(!is_valid(name)) throw 0; // ***
}

param::param(std::string description) :
    description_{ std::move(description) }, required_{ false }
{ }

////////////////////////////////////////////////////////////////////////////////
option::option(std::string code_or_name, std::string description) :
    param{ std::move(description) }
{
    if(is_code(code_or_name))
        to_code(code_or_name, code_);
    else if(is_name(code_or_name))
        to_name(code_or_name, name_);
    else throw 0; // ***
}

////////////////////////////////////////////////////////////////////////////////
option::option(std::string code, std::string name_or_param, std::string description) :
    param{ std::move(description) }
{
    if(is_code(code))
        to_code(code, code_);
    else throw 0; // ***

    if(is_name(name_or_param))
        to_name(name_or_param, name_);
    else to_param(name_or_param, param::name_, poly_, opt_val_, required_);
}

////////////////////////////////////////////////////////////////////////////////
option::option(std::string code, std::string name, std::string param_, std::string description) :
    param{ std::move(description) }
{
    if(is_code(code))
        to_code(code, code_);
    else throw 0; // ***

    if(is_name(name))
        to_name(name, name_);
    else throw 0; // ***

    to_param(param_, param::name_, poly_, opt_val_, required_);
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
