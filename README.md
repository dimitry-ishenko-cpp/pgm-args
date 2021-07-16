# pgm::args – Define & Process Program Arguments in C++

This simple module allows one to easily define, parse and examine program
options and positional parameters in a C++ program. :notes:

Here is a ~~concise~~ example that shows capabilities of **pgm::args**:

```cpp
// 0. Include module header.
#include "pgm/args.hpp"

// ...

int main(int argc, char* argv[])
try
{
    std::string name{ argv[0] };

    // 1&2. Define options and positional parameters.
    pgm::args args
    {
        { "-a", "--address", "IP", "Specify IP address to bind to. Default: 0.0.0.0
                                   "(bind to all)."                                 },
        { "-p", "--port", "N",     "Specify port number to listen on. Default: 42." },
        { "-h", "--help",          "Print this help screen and exit."               },
        { "-v", "--version",       "Show version number and exit."                  },
        { "path?+",                "List of files to read data from."               },
    };

    // 3. Parse command line arguments.
    args.parse(argc, argv);

    // 4. Examine options and positional parameters.
    if(args["--help"])
        std::cout << args.usage(name,
            "This is a concise example that shows capabilities of pgm::args."
        ) << std::endl;

    else if(args["--version"])
        std::cout << name << " 0.42" << std::endl;

    else
    {
        auto address{ args["--address"].value_or("0.0.0.0") };
        auto port{ args["--port"].value_or("42") };

        std::cout << "Listening on " << address << ":" << port << std::endl;

        for(auto const& path : args["path"].values())
        {
            std::cout << "Reading from " << path << std::endl;
            do_some_useful_stuff();
        }

        // ...
    }

    return 0;
}
catch(const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return 1;
};
```

## Installation

The recommended installation method is to add **pgm::args** as a sub-module into your
project. :tada:

```shell
$ git submodule add --depth 1 https://github.com/dimitry-ishenko-cpp/pgm-args.git pgm
```

## Description

### :one: Options

Options can be constructed with _short_ name, _long_ name,
_parameter_ name (if the option accepts values) and _description_.

- [ ] :herb: Short name consists of a single dash (`-`) followed by one
  alpha-numeric character.

- [ ] :herb: Long name consists of two dashes (`--`) followed by one or more
  alpha-numeric characters and dashes.

- [ ] :herb: Parameter name can contain any
  [graphic](https://en.cppreference.com/w/cpp/string/byte/isgraph) characters.

- [ ] :herb: Either the _short_ or the _long_ name (but not both) can be omitted.

- [ ] :herb: If the _parameter_ name is omitted, it means the option doesn't
  accept values.

- [ ] :herb: The _description_ is mandatory.

```cpp
pgm::args args
{
  { "-f", "--file-name",  "Option with short and long names." },

  {       "--tik-tok",    "Option with long name only." },

  { "-p", "path",         "Option with short name and parameter path." },

  { "-c", "--count", "N", "Option with parameter N." },
};
```

Parameter name can have one or more specifiers after it:

- [ ] :key: `?` indicates that option value can be omitted;
- [ ] :key: `!` designates the option as mandatory;
- [ ] :key: `+` means the option can be specified multiple times.

```cpp
pgm::args
{
    { "-d", "--debug", "level?",    "Level of debug info to show. Default: 1." },

    {       "--log-file", "path!",  "Mandatory path to a log file." },

    { "-v", "+",                    "Increase verbosity. (Can be specified multiple times.)" },

    { "-o", "--option", "value?!+", "Specifiers can be combined." },
};
```

If an option doesn't accept values, the specifiers are added by themselves
instead of the parameter name (see option `-v` above).

Several specifiers can also be combined even if it makes little or no sense.
:alien:

---

### :two: Positional Parameters

Positional parameters are constructed with _name_ and _description_, both of
which are mandatory.

- [ ] :herb: Positional arameter name can contain any
  [graphic](https://en.cppreference.com/w/cpp/string/byte/isgraph) characters.

Positional parameter name can have one or more specifiers after it:

- [ ] :key: `?` indicates an optional parameter that can be omitted;
- [ ] :key: `+` means this parameter can be specified multiple values.

```cpp
pgm::args args
{
    // ...

    { "input-path",   "Mandatory input file path." },

    { "output-path?", "Optional output file path." },

    { "aux-path?+",   "Zero or more auxiliary file paths." },
};
```

- [ ] :herb: Optional parameters (with `?`) can only be followed by other
  optional parameters.

- [ ] :herb: The `+` specifier may only be used with the last positional
  parameter.

Specifying duplicate or invalid option or positional parameter will
result in :poop: `pgm::invalid_definition` exception being thrown by the `pgm::args`
constructor or the `add()` function.

---

### :three: Parse

After options and/or positional parameters have been defined, call the `parse()`
function to parse the command line arguments.

The `parse()` function can throw one of the several following exceptions:

- [ ] :poop: `pgm::invalid_argument` if invalid option or parameter was specified;

- [ ] :poop: `pgm::duplicate_option` if an option (not marked with `+`) was specified
  multiple times;

- [ ] :poop: `pgm::missing_option` if an option was marked as required (with `!`),
  but wasn't specified;

- [ ] :poop: `pgm::extra_value` if an option value was specified, but the option
  doesn't accept values;

- [ ] :poop: `pgm::missing_value` if an option requires a value, but none were
  specified;

- [ ] :poop: `pgm::missing_argument` if a non-optional positional parameter was not
  specified on the command line.

---

### :four: Examine

Next, use the subscript `operator[]` of the `pgm::args` class to examine :eyes:
parsed options and parameters. Options are referred to by either the short or
the long name, while positional parameters are referred to by their name.

The subscript `operator[]` returns const reference to an instance of `pgm::arg`
or throws :poop: `pgm::invalid_argument` exception.

Using the `pgm::arg` instance you can examine an option or positional
parameter:

- [ ] :rose: `count()` returns how many times the option/param was encountered
  during parsing;

  ```cpp
  auto level = args["-v"].count();
  ```

- [ ] :rose: `operator bool()` returns `true` if the option/param was found least
  once;

  ```cpp
  if(args["--help"]) show_usage_and_exit();
  if(args["--version"]) show_version_and_exit();
  ```
  
  If your program includes one or more mandatory options or positional parameters,
  but there are certain options (eg, `--help` and `--version`) that should be
  handled even when those are missing, you can do the following:
  
  ```cpp
  std::exception_ptr ep;
  try { args.parse(argc, argv); }
  catch(...) { ep = std::current_exception(); }

  if(args["--help"]) show_usage();
  else if(args["--version"]) show_version();
  else if(ep) std::rethrow_exception(ep);
  else
  {
      // process remaining options/parameters
  }
  ```

- [ ] :rose: `empty()` is the opposite of `operator bool()` – it returns `true`
  when the option/param was *not* found during parsing;

  ```cpp
  if(args["--conf"].empty()) run_with_default_conf();
  ```

- [ ] :rose: `value()` returns option/param value; if the option doesn't accept
  values, it will return an empty string;

  ```cpp
  std::fstream fs{ args["path"].value() };
  ```

- [ ] :rose: `value_or(default_value)` returns option/param value or
  `default_value`, if the option/param was not found during paring;

  ```cpp
  auto address{ args["--address"].value_or("192.168.1.1") };
  ```

- [ ] :rose: `values()` returns `std::vector` of all option/param values; useful
  for multi-value (marked with `+`) options/params;

  ```cpp
  for(auto const& path : args["aux-path"].values()) read_data(path);
  ```

- [ ] :rose: `value(n)` return n-th value of a multi-value option/param.

  ```cpp
  if(args["--foo"].value(2) == "bar") do_baz();
  ```

---

### :five: Usage

Finally, the `pgm::args` class provides the `usage()` function, which can be
used to display all options and positional parameters nicely formatted. :notes:

```cpp
if(args["--help"])
    std::cout << args.usage(name,
        "This is a concise example that shows capabilities of pgm::args."
    ) << std::endl;
```

Output:
```shell
dimitry@laptop:~$ ./example -h
Usage: ./example [option]... [path]...

  -a, --address=<IP>  Specify IP address to bind to.
                      Default: 0.0.0.0 (bind to all).
  -p, --port=<N>      Specify port number to listen on.
                      Default: 42.
  -h, --help          Print this help screen and exit.
  -v, --version       Show version number and exit.

  <path>              List of files to read data from.

This is a concise example that shows capabilities of pgm::args.
```

Share and enjoy. :tada:

## Authors

* **Dimitry Ishenko** - dimitry (dot) ishenko (at) (gee) mail (dot) com

## License

This project is distributed under the GNU GPL license. See the
[LICENSE.md](LICENSE.md) file for details.
