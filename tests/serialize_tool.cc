#include <fstream>
#include <iostream>
#include "../src/trie.hh"
#include "../src/pack_trie.hh"

static void to_output(trie& t,
                      const std::string& output,
                      const std::string& dot)
{
    auto out_stream = std::ofstream(output);
    if (!out_stream.is_open())
    {
      std::cerr << "Couldn't open the file: " << output << std::endl;
      std::exit(1);
    }
    compact_trie::serialize(out_stream, t.root_.edges);
    out_stream.close();

    auto dot_stream = std::ofstream(dot);
    if (!dot_stream.is_open())
    {
      std::cerr << "Couldn't open the file: " << output << std::endl;
      std::exit(1);
    }
    t.to_dot(dot_stream);
    dot_stream.close();
}

int main(int argc, char* argv[])
{
    if (argc != 6)
      return 1;

    auto input = std::string(argv[1]);
    auto output = std::string(argv[2]);
    auto output2 = std::string(argv[3]);
    auto to_dot = std::string(argv[4]);
    auto to_dot2 = std::string(argv[5]);

    auto in_stream = std::ifstream(input);
    if (!in_stream.is_open())
    {
      std::cerr << "Couldn't open the file: " << input << std::endl;
      return 1;
    }
    auto t = trie();
    t.build(in_stream);
    in_stream.close();

    to_output(t, output, to_dot);

    auto ct = compact_trie();
    ct.load(output);
    auto t2 = ct.to_trie();
    to_output(t2, output2, to_dot2);
    return 0;
}
