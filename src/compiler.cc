#include <iostream>
#include <string>

static void parse(const std::string&)
{
}

static void write(const std::string&)
{
}

void usage(const std::string& bin_name)
{
    std::cerr << "Usage: " << bin_name
              << " `path to word dictionary' `path to compiled trie'";
    std::abort();
}


int main(int argc, char* argv[])
{
    if (argc != 3)
        usage(argv[0]);
    {
        auto input = std::string(argv[1]);
        auto output = std::string(argv[2]);
        parse(input);
        write(output);
    }
    return 0;
}
