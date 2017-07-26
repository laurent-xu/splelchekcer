#include <iostream>
#include <string>

static void parse(const std::string&)
{
}

void usage(const std::string& bin_name)
{
    std::cerr << "Usage: " << bin_name << "`path to compiled trie'";
}


int main(int argc, char* argv[])
{
    if (argc != 2)
        usage(argv[0]);
    else
    {
        auto input = std::string(argv[1]);
        parse(input);
    }
    return 0;
}
