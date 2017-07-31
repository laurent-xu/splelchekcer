#include <iostream>
#include <string>
#include "distance.hh"

static void usage(const std::string& bin_name)
{
    std::cerr << "Usage: " << bin_name << "`path to compiled trie'";
    std::exit(1);
}

static void format_output(std::ostream& os, const::std::string& word,
                          size_t freq)
{
  if (freq > 0)
    os << "[{\"word\":\"" << word << "\",\"freq\":" << freq
       <<",\"distance\":0}]\n";
  else
    os << "[]\n";
}

static void format_output(std::ostream& os, std::vector<match>& matches)
{

  std::sort(matches.begin(),
            matches.end(), [](match a, match b) -> bool {
          if (a.distance != b.distance)
              return a.distance < b.distance;
          if (b.freq != a.freq)
              return b.freq < a.freq;
          return a.word.compare(b.word) < 0;
          });
  os << "[";
  for (unsigned i = 0; i < matches.size(); i++)
  {
      const auto& r = matches[i];
      os << "{\"word\":\"" << r.word
         << "\",\"freq\":" << r.freq << ","
          "\"distance\":" << r.distance << "}";
      if (i != matches.size() - 1)
          os << ',';
  }
  os << "]\n";
}

int main(int argc, char* argv[])
{
    if (argc != 2)
        usage(argv[0]);
    else
    {
        auto input = std::string(argv[1]);
        auto ct = compact_trie();
        ct.load(input);
        auto root_edges = ct.get_root();
        auto dl = distance();
        while (!std::cin.eof())
        {
          std::string tmp;
          std::string word;
          int max_dist = -1;

          std::cin >> tmp;
          std::cin >> max_dist;
          std::cin >> word;

          if (!word.size())
              continue;

          if (max_dist == 0)
          {
            size_t freq = dl.exact_match(word, root_edges);
            format_output(std::cout, word, freq);
          }
          else if (max_dist > 0)
          {
              auto res = dl.run(std::move(word), max_dist, root_edges);
              format_output(std::cout, res);
          }
          else
              format_output(std::cout, word, 0);
        }
    }
    std::cout.flush();
    return 0;
}
