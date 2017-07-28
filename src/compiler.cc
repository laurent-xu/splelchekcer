#include <iostream>
#include <string>
#include <fstream>
#include "trie.hh"

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

    auto input = std::string(argv[1]);
    auto output = std::string(argv[2]);

    auto in_stream = std::ifstream(input);
    if (!in_stream.is_open())
    {
      std::cerr << "Couldn't open the file: " << input << std::endl;
      return 1;
    }
    auto trie = Trie();
    trie.build(in_stream);
    in_stream.close();

    trie.to_dot(std::cout);

    auto out_stream = std::ofstream(output);
    if (!out_stream.is_open())
    {
      std::cerr << "Couldn't open the file: " << output << std::endl;
      return 1;
    }
    //compact_trie ct;
    //trie->serialize(out_stream);
    out_stream.close();

    return 0;
}
