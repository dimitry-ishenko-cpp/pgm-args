#include <iostream>
#include "pgm/args.hpp"

int main(int argc, char* argv[])
{
    pgm::args args
    {
        { "-v", "--version", "Show version and exit." },
        { "-h", "--help",    "Show this help screen and exit." },
    };

    args.parse(argc, argv);

    if(args["--version"])
    {
        std::cout << argv[0] << " 0.42" << std::endl;
        return 0;
    }
    if(args["--help"])
    {
        std::cout << args.usage(argv[0]) << std::endl;
        return 0;
    }

    std::cout << "Hello world!" << std::endl;
    std::cout << "Use -h or --help to see the help screen." << std::endl;
    return 0;
}
