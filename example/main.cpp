#include <exception>
#include <filesystem>
#include <iostream>
#include <vector>

#include "pgm/args.hpp"

int main(int argc, char* argv[])
try
{
    namespace fs = std::filesystem;
    std::string name{ fs::path{argv[0]}.filename() };

    // 1 & 2. Define options and positional parameters.
    pgm::args args
    {
        { "-v", "--verbose", pgm::mul,          "increase verbosity" },
        {       "--info", "FLAGS",              "fine-grained informational verbosity" },
        {       "--debug", "FLAGS",             "fine-grained debug verbosity" },
        { "-q", "--quiet",                      "suppress non-error messages" },
        { "-r", "--recursive",                  "recurse into directories" },
        { "-l",                                 "copy symlinks as symlinks" },
        { "-L",                                 "transform symlink into referent file/dir" },
        {       "--chmod", "CHMOD",             "affect file and/or directory permissions" },
        { "-f", "--filter", "RULES", pgm::mul,  "add a file-filtering RULE" },
        { "-V", "--version",                    "print the version and exit" },
        { "-h", "--help",                       "show this help" },

        { "SRC", pgm::mul,                      "source file(s) or directory(s)" },
        { "DEST",                               "destination file or directory" },
    };

    // 3. Parse command line arguments.
    std::exception_ptr ep;
    try { args.parse(argc, argv); }
    catch(...) { ep = std::current_exception(); }

    // 4. Examine options and positional parameters.
    if(args["--help"])
    {
        auto preamble =
R"(sync is a dummy file transfer program created solely for demonstrating
capabilities of pgm::args.)";

        auto epilogue =
R"(You must specify at least one source file or directory and a destination to
copy to. For example:

    sync *.c /dest/path/

In theory, this would transfer all files matching the pattern *.c from the
current directory to the directory /dest/path/. However, since this is a dummy
program, nothing will actually be transferred.)";

        std::cout << args.usage(name, preamble, { }, epilogue) << std::endl;
    }
    else if(args["--version"])
    {
        std::cout << name << " 0.42" << std::endl;
    }
    else if(ep)
    {
        std::rethrow_exception(ep);
    }
    else
    {
        auto verbose_level = args["-v"].count();

        auto quiet = !!args["--quiet"];
        auto recurse = !!args["-r"];

        auto copy_links  = !!args["-l"];
        auto deref_links = !!args["-L"];

        if(copy_links && deref_links) throw pgm::invalid_argument{
            "options '-l' and '-L' are mutually exclusive"
        };

        auto chmod = args["--chmod"].value_or("0644");

        std::vector<std::string> rules;
        for(auto const& rule : args["--filter"].values()) rules.push_back(rule);

        std::vector<std::string> sources;
        for(auto const& source : args["SRC"].values()) sources.push_back(source);

        auto dest = args["DEST"].value();

        // do_useful_stuff();
    }

    return 0;
}
catch(const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return 1;
};
