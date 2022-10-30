# pgm::args – Define & Process Program Arguments in C++

This simple header-only library allows you to easily define, parse and examine
program options and positional parameters in a C++ program. :notes:

Here is a very simple example of how to use **pgm::args**:

```cpp
// example1.cpp

#include <iostream>
#include <pgm/args.hpp>

int main(int argc, char* argv[])
{
    pgm::args args
    {
        { "-v", "--version", "Show version and exit." },
        { "-h", "--help",    "Show this help screen and exit." },
    };

    args.parse(argc, argv);

    if (args["--version"])
    {
        std::cout << argv[0] << " version " << 0.42 << std::endl;
        return 0;
    }
    if (args["--help"])
    {
        std::cout << args.usage(argv[0]) << std::endl;
        return 0;
    }

    std::cout << "Hello world!" << std::endl;
    std::cout << "Use -h or --help to see the help screen." << std::endl;
    return 0;
}
```

Output:
```console
user@linux:~$ ./example1
Hello world!
Use -h or --help to see the help screen.

user@linux:~$ ./example1 -h
Usage: ./example1 [option]...

Options:
-v, --version    Show version and exit.
-h, --help       Show this help screen and exit.

user@linux:~$ ./example1 -v
./example1 version 0.42

user@linux:~$
```

## Installing pgm::args

You can install **`pgm::args`** in one of the following ways:

1. Install binary package, if you are on Debian/Ubuntu/etc:

   ```console
   $ v=0.3 p=libpgm-args-dev_${v}_all.deb
   $ wget https://github.com/dimitry-ishenko-cpp/pgm-args/releases/download/v${v}/${p}
   $ sudo apt install ./${p}
   ```

2. Install from source:

   ```console
   $ mkdir build
   $ cd build
   $ cmake ..
   $ make all test
   $ sudo make install
   ```

3. Add as a sub-module to your project:

   ```console
   $ git submodule add https://github.com/dimitry-ishenko-cpp/pgm-args.git pgm
   ```

## Using pgm::args

### :zero: Understand the Nomenclature

Program _arguments_ are whitespace-separated tokens given on the command line
when executing a program. In other words, when you execute:

```console
user@linux:~$ foo bar baz
```

`foo` is the program, and `bar` and `baz` are the _arguments_. Following rules
apply to program arguments:

:key: Arguments beginning with the hyphen delimiter `-` are called _options_.

:key: There are two kinds of options:

* _short options_, consisting of a single hyphen followed by an alpha-numeric
  character, eg:  
  `-a` `-b` `-c`

* _long options_, consisting of two hyphens followed by one or more
  alpha-numeric characters and hyphens, eg:  
  `--output` `--log-file`

:key: Several short options maybe grouped in one token:

```bash
foo -abc
# is equivalent to
foo -a -b -c
```

:key: Some options may require a value. For example, the `--output` option may
require path to a file where to write the data. These values are called "option
parameters" or "option arguments" in some literature. Here they are called
_option values_.

:key: _Option value_ may appear as a separate token after the option, or may be
combined with the option in one token. For example:

```bash
foo -opath
# is equivalent to
foo -o path
```

Likewise, for long options:

```bash
foo --output=path
# is equivalent to
foo --output path
```

Note the equal sign `=` delimiter between the option and its value.

:key: Options typically precede other non-option arguments, which are called
_positional parameters_.

:key: The special `--` token terminates all options. Any arguments following
`--` will be treated as positional parameters, even though they start with the
hyphen. In the following example:

```bash
foo -a --bar baz -- -c --qux
```

`-a` and `--bar` are treated as options, and `baz`, `-c` and `--qux` are treated
as positional parameters (unless option `--bar` requires a value, in which case
`baz` will be treated as an option value).

:key: A token consisting of a single hyphen `-` is treated as an ordinary
non-option argument.

---

### :one: Include the Header

**pgm::args** is a header-only library. To use it, simply include `pgm/args.hpp`
header at the beginning of your program:

```cpp
#include <pgm/args.hpp>
...
```

---

### :two: Define Options and Positional Parameters

Options and positional parameters are encapsulated by the **`pgm::arg`** class,
which has the following constructors:

```cpp
pgm::arg(short_name, [spec,] description);             // (1) short option
pgm::arg(long_name,  [spec,] description);             // (2) long option
pgm::arg(param_name, [spec,] description);             // (3) positional param

pgm::arg(short_name, long_name,  [spec,] description); // (4) option with short & long name
pgm::arg(short_name, value_name, [spec,] description); // (5) short option that takes a value
pgm::arg(long_name,  value_name, [spec,] description); // (6) long option that takes a value

pgm::arg(short_name, long_name, value_name, [spec,] description); // (7) short & long name + takes val
```

Where:

:rose: **`short_name`** is a short option name consisting of a single hyphen
followed by one
[alpha-numeric](https://en.cppreference.com/w/cpp/string/byte/isalnum)
character, eg:

```cpp
pgm::arg{ "-a", "..." }; // (1)
pgm::arg{ "-b", "..." }; // (1)
pgm::arg{ "-c", "..." }; // (1)
```

:rose: **`long_name`** is a long option name consisting of two hyphens followed
by one or more
[alpha-numeric](https://en.cppreference.com/w/cpp/string/byte/isalnum)
characters and hyphens, eg:

```cpp
pgm::arg{ "--file-name",     "..." }; // (2)
pgm::arg{ "--output",        "..." }; // (2)

pgm::arg{ "-h", "--help",    "..." }; // (4)
pgm::arg{ "-v", "--version", "..." }; // (4)
```

:rose: **`param_name`** is a positional parameter name that can contain any
[graphic](https://en.cppreference.com/w/cpp/string/byte/isgraph) characters, eg:

```cpp
pgm::arg{ "source",      "..." }; // (3)
pgm::arg{ "x+y",         "..." }; // (3)
pgm::arg{ "foo/bar/baz", "..." }; // (3)
```

:rose: **`value_name`** is an option value name that can contain any
[graphic](https://en.cppreference.com/w/cpp/string/byte/isgraph) characters, eg:

```cpp
pgm::arg{ "-i", "file-name",        "..." }; // (5)
pgm::arg{ "-l", "log-file",         "..." }; // (5)

pgm::arg{ "--filter", "name",       "..." }; // (6)
pgm::arg{ "--set-time", "HH:MM",    "..." }; // (6)

pgm::arg{ "-w", "--wait", "time",   "..." }; // (7)
pgm::arg{ "-d", "--debug", "level", "..." }; // (7)
```

:rose: **`description`** is a human-readable description of the option or
parameter, eg:

```cpp
pgm::arg{ "-h", "--help",    "Show this help screen and exit." }; // (4)
pgm::arg{ "-v", "--version", "Show version number and exit."   }; // (4)

pgm::arg{ "source",          "Path to file with source data."  }; // (3)
pgm::arg{ "foo/bar/baz",     "Lorem ipsum dolor sit amet."     }; // (3)
```

:rose: **`spec`** is an option/param specification consisting of one or more of
the following flags combined using the vertical pipe `|` delimiter:

| flag              | option             | param              | meaning |
|:-----------------:|:------------------:|:------------------:|:--------|
| **`pgm::req`**    | :heavy_check_mark: |                    | mandatory (or **req**uired) option<sup>1</sup>
| **`pgm::mul`**    | :heavy_check_mark: | :heavy_check_mark: | option may be specified **mul**tiple times; <br>positional parameter can accept **mul**tiple values<sup>2</sup> |
| **`pgm::optval`** | :heavy_check_mark: |                    | option **val**ue is **opt**ional (ie, can be omitted) |
| **`pgm::opt`**    |                    | :heavy_check_mark: | positional parameter is **opt**ional<sup>3</sup>  |

<sup>1</sup> Options are optional by default.  
<sup>2</sup> There can be at most one positional parameter marked with `pgm::mul`.  
<sup>3</sup> Parameters are mandatory by default.

Invalid flags (such as, specifying `pgm::optval` for a positional parameter) are
ignored.

---

The **`pgm::args`** class represents a collection of options and positional
parameters (instances of **`pgm::arg`**) supported by your program, and provides
facilities to parse the command line and examine the results.

Options and parameters can be added directly via its constructor:

```cpp
pgm::args(std::initializer_list<pgm::arg> args);
```

or using the `add()` function:

```cpp
void add(pgm::arg arg);

template<typename... Ts>
void add(Ts&&... values); // emplace-style
```

Below are some examples:
```cpp
// construct an instance of pgm::args and add options -v and -h
pgm::args args
{
    { "-v", "--version", "Show version and exit." },
    { "-h", "--help",    "Show this help screen and exit." },
};

// construct option -d and add it to the args
auto arg = pgm::arg{"-d", "--debug", pgm::mul, "Increase debug level"};
args.add(std::move(arg));

// add positional param 'file' directly to the args (emplace-style)
args.add("file", pgm::opt, "Path to file");
```

Invalid and duplicate option/parameter definitions will result in the :poop:
`pgm::invalid_definition` exception being thrown.

---

### :three: Parse the Command Line

Having defined your options and parameters, you can now call the `parse()`
member function of **`pgm::args`** passing it `argc` and `argv`:

```cpp
int main(int argc, char* argv[])
{
    pgm::args args{ ... };
    args.parse(argc, argv);
    ...
}
```

The `parse()` function will examine the command line and extract all options,
their values and positional parameter values that were passed to the program.
This function may throw one of two exceptions:

* :poop: **`pgm::invalid_argument`** will be thrown for any unrecognized or
  duplicate option (unless marked as `pgm::mul`), or for an extraneous
  positional parameter.

* :poop: **`pgm::missing_argument`** will be thrown for any missing option
  marked as `pgm::req`, or for a missing positional parameter _not_ marked as
  `pgm::opt`.

---

### :four: Examine Options and Positional Parameters

Next, use the subscript `operator[]` to examine :eyes: parsed options and
parameters. Options can be referred to by their short name or the long name,
while positional parameters are referred to by their name.

```cpp
int main(int argc, char* argv[])
{
    pgm::args args
    {
        { "--conf", "path",            "Path to alternate config file." },
        { "-d", pgm::mul,              "Increase level of debug messages." },
        { "-q", "--quiet",             "Show error messages only." },
        { "-h", "--help",              "Show this help screen and exit." },
        { "dirs", pgm::opt | pgm::mul, "List of directories." },
    };
    args.parse(argc, argv);
    ...
}
```

The subscript `operator[]` returns const ref to an instance of
**`pgm::argval`**, which allows you to:

* :rose: Call the `count()` function to examine how many times said option/param
  was specified on the command line.

  ```cpp
  auto debug_level = args["-d"].count();
  ```

* :rose: Use `operator bool()` or call the `empty()` function to examine whether
  said option/param was specified at all.

  ```cpp
  if (args["-h"]) show_usage_and_exit();

  bool use_default_conf = args["--conf"].empty();
  ```

  `operator bool()` of **`pgm::argval`** is marked as explicit, so you may have
  to use `static_cast` or double logical negation `!!` to force boolean
  context in certain situations, eg:

  ```cpp
  auto quiet = static_cast<bool>(args["-q"]);
  // or alternatively
  auto quiet = !!args["-q"];
  if (quiet) festina_lente();
  ```

  If there are certain "high priority" options, such as `--help`, which you
  would like to process in all situations, you can do the following:

  ```cpp
  std::exception_ptr ep;
  try { args.parse(argc, argv); }
  catch (...) { ep = std::current_exception(); }

  if (args["--help"]) show_usage();
  else if (ep) std::rethrow_exception(ep);
  else
  {
      // process remaining options/params
  }
  ```

* :rose: Call the `value()` function to access the option/param value. If the
  option doesn't take values, empty string will be returned.

  _NOTE_: This function is equivalent to calling `value(0)` (see below) and will
  throw :poop: **`std::out_of_range`**, if the option/param was not specified on
  the command line.

  Alternatively, you can call `value_or(...)` and specify a default value to be
  returned, when the option/param was _not_ specified.

  ```cpp
  auto conf = args["--conf"].value(); // may throw
  // or with default value
  auto conf = args["--conf"].value_or("/etc");
  ```

* :rose: Call the `values()` function to get all values of a multi-value (ie,
  marked with `pgm::mul`) option or positional parameter.

  ```cpp
  for (auto const& dir : args["dirs"].values())
  {
      do_stuff_with(dir);
  }
  ```
* :rose: Call the `value(n)` function to access n-th value of a multi-value
  option/param.

  ```cpp
  if (args["--foo"].value(2) == "bar") process_bar();
  ```

  _NOTE_: This function will throw the :poop: **`std::out_of_range`** exception,
  if `n` is not valid.

---

### :five: Display Usage

Finally, the **`pgm::args`** class provides the `usage()` member function, which
displays program details including all options and positional parameters in a
nicely formatted manner. :notes:

It has a signature of:

```cpp
std::string usage(program, preamble = "", prologue = "", epilogue = "");
```

and returns text in roughly the following format:

<pre>
<b>&lt;preamble></b>

Usage: <b>&lt;program></b> [option...] params...

<b>&lt;prologue></b>

Options:
...

Parameters:
...

<b>&lt;epilogue></b>
</pre>

`preamble`, `prologue` and `epilogue` are all optional.

---

### :six: Example

Here is a more complete example of using **pgm::args**:

```cpp
// example2.cpp

#include <exception>
#include <filesystem>
#include <iostream>
#include <pgm/args.hpp>
#include <vector>

void show_usage(const pgm::args& args, std::string_view name);
void show_version(std::string_view name);

void transfer(std::string_view source, std::string_view dest);

int main(int argc, char* argv[])
try
{
    namespace fs = std::filesystem;
    auto name = fs::path{argv[0]}.filename().string();

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

    std::exception_ptr ep;
    try { args.parse(argc, argv); }
    catch (...) { ep = std::current_exception(); }

    if (args["--help"])
        show_usage(args, name);

    else if (args["--version"])
        show_version(name);

    else if (ep)
        std::rethrow_exception(ep);

    else // normal program flow
    {
        auto verbose_level = args["-v"].count();

        auto quiet = !!args["--quiet"]; // !! to force bool-context
        auto recurse = !!args["-r"];

        auto copy_links  = !!args["-l"];
        auto deref_links = !!args["-L"];

        if (copy_links && deref_links) throw pgm::invalid_argument{
            "options '-l' and '-L' are mutually exclusive"
        };

        auto chmod = args["--chmod"].value_or("0644");

        std::vector<std::string> rules;
        for (auto const& rule : args["--filter"].values()) rules.push_back(rule);

        std::vector<std::string> sources;
        for (auto const& source : args["SRC"].values()) sources.push_back(source);

        auto dest = args["DEST"].value();

        // "transfer" files
        for (auto const& source : sources)
        {
            if (!quiet) std::cout << "Sending " << source << " to " << dest << std::endl;
            transfer(source, dest);
        }
    }

    return 0;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return 1;
};

void show_usage(const pgm::args& args, std::string_view name)
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

void show_version(std::string_view name)
{
    std::cout << name << " version " << 0.42 << std::endl;
}

void transfer(std::string_view source, std::string_view dest)
{
    //
}
```

Output:
```console
user@linux:~$ ./example2
Missing argument: param 'SRC' is required.

user@linux:~$ ./example2 -h
sync is a dummy file transfer program created solely for demonstrating
capabilities of pgm::args.

Usage: example2 [option]... <SRC>... <DEST>

Options:
-v, --verbose           increase verbosity
    --info=<FLAGS>      fine-grained informational verbosity
    --debug=<FLAGS>     fine-grained debug verbosity
-q, --quiet             suppress non-error messages
-r, --recursive         recurse into directories
-l                      copy symlinks as symlinks
-L                      transform symlink into referent file/dir
    --chmod=<CHMOD>     affect file and/or directory permissions
-f, --filter=<RULES>    add a file-filtering RULE
-V, --version           print the version and exit
-h, --help              show this help

Parameters:
SRC                     source file(s) or directory(s)
DEST                    destination file or directory

You must specify at least one source file or directory and a destination to
copy to. For example:

    sync *.c /dest/path/

In theory, this would transfer all files matching the pattern *.c from the
current directory to the directory /dest/path/. However, since this is a dummy
program, nothing will actually be transferred.

user@linux:~$ ./example2 foo bar baz
Sending foo to baz
Sending bar to baz
```

Share and enjoy. :tada:

## Authors

* **Dimitry Ishenko** - dimitry (dot) ishenko (at) (gee) mail (dot) com

## License

This project is distributed under the GNU GPL license. See the
[LICENSE.md](LICENSE.md) file for details.
