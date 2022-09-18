#include <iostream>

// 1. Include the header.
#include "pgm/args.hpp"

int main(int argc, char* argv[])
{
    // 2. Define options and positional parameters.
    pgm::args args
    {
        { "-v", "--version", "Show version and exit." },
        { "-h", "--help",    "Show this help screen and exit." },
    };

    // 3. Parse command line arguments.
    args.parse(argc, argv);

    // 4. Examine options and positional parameters.
    if(args["--help"])
        std::cout << args.usage(argv[0]) << std::endl;

    else if(args["--version"])
        std::cout << argv[0] << " 0.42" << std::endl;

    else
    {
        std::cout << "Hello world!" << std::endl;
        std::cout << "Use -h or --help to see the help screen." << std::endl;
    }
    return 0;
}
