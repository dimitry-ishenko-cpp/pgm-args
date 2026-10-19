////////////////////////////////////////////////////////////////////////////////
#include <deque>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
class env
{
    std::deque<std::string> args_;
    std::vector<char*> argv_{ nullptr };

public:
    int argc = 0;
    char** argv = argv_.data();

    env(std::initializer_list<std::string_view> args) { for (auto&& arg : args) add_arg(arg); }

    void add_arg(std::string_view arg)
    {
        args_.emplace_back(arg);
        argv_.back() = args_.back().data();
        argv_.push_back(nullptr);

        argc = args_.size();
        argv = argv_.data();
    }

    void add_arg(std::string_view name, std::string_view value) {
        add_arg(name); add_arg(value);
    }

    void add_arg_multi(std::string_view name, std::initializer_list<std::string_view> values) {
        for (auto value : values) add_arg(name, value);
    }
};
