#pragma once
#include <limits>
#include <algorithm>
#include <iomanip>
#include "pack_trie.hh"

struct match
{
  std::string word;
  size_t freq;
  size_t distance;
};

class distance
{
  public:
    ~distance()
    {
      if (table != nullptr)
        delete[] table;
    }

    std::vector<match> run(std::string&& word,
                           int max_distance,
                           compact_info* start_trie)
    {
      if (table != nullptr)
        delete[] table;
      word_size = word.size();
      row_len = word_size + 1;
      col_len = word_size + max_distance + 2;
      table_len = row_len * col_len;
      table = new unsigned[row_len * col_len];

      current.clear();
      matched_words.clear();
      this->word = std::move(word);
      this->max_distance = max_distance;
      init_table();

#ifdef DEBUG
      std::cerr << "init table" << std::endl;
      print_table();
#endif

      add_edges(start_trie);


      return matched_words;
    }

    size_t exact_match_aux(const std::string& word, size_t idx,
                           compact_info* first_sibling, const size_t word_len)
    {
      for (auto edge: *first_sibling)
      {
        size_t current_idx = idx;
        char* label_ptr = reinterpret_cast<char*>(&edge->label[0]);

        while (*label_ptr && current_idx < word_len
               && *label_ptr == word[current_idx])
        {
          label_ptr++;
          current_idx++;
        }

        if (*label_ptr == 0)
        {
          if (word_len == current_idx)
            return edge->freq;
          else if (edge->has_children())
            return exact_match_aux(word, current_idx,
                                   edge->get_children(), word_len);
        }
        else if (edge->label[0] == word[idx])
          return 0;
      }
      return 0;
    }

    size_t exact_match(const std::string& word, compact_info* start_trie)
    {
      return exact_match_aux(word, 0, start_trie, word.size());
    }

    size_t dist(size_t row)
    {
      return at(row_len - 1, row);
    }

    void print_table()
    {
      std::cerr << "table" << std::endl;
      for (size_t i = 0; i < col_len; ++i)
      {
        for (size_t j = 0; j < row_len; ++j)
        {
          std::cerr << std::setw(5);
          std::cerr << at(j, i);
        }
        std::cerr << std::endl;
      }
      std::cin.get();
    }

    bool add_char(char c)
    {
      current.push_back(c);
      int row = current.size();
      bool continue_searching = false;
      size_t start = std::max({1, row - max_distance});
      size_t end = std::min({static_cast<int>(row_len),
                            row + max_distance + 1});
      for (size_t i = start; i < end; ++i)
      {
        unsigned above = at(i, row - 1) + 1;
        unsigned left = at(i - 1, row) + 1;
        unsigned above_left = at(i - 1, row - 1) + (c == word[i - 1] ? 0 : 1);
        unsigned min = std::min({above, left, above_left});
        if (row >= 2 && i >= 2 && c == word[i - 2]
            && current[row - 2] == word[i - 1])
          min = std::min(min, at(i - 2, row - 2) + 1);
        if (min <= static_cast<unsigned>(max_distance))
        {
          continue_searching = true;
        }
        at(i, row) = min;
      }
      return continue_searching;
    }

    void add_edges(compact_info* first_sibling)
    {
      size_t current_word_len = current.size();
      auto lol = current;
      for (auto edge: *first_sibling)
      {
        bool continue_searching = false;
        char* label_ptr = &edge->label[0];
        while (*label_ptr)
        {
          continue_searching = add_char(*label_ptr);
          if (!continue_searching)
            break;
          ++label_ptr;
        }
        size_t distance = dist(current.size());
        size_t freq = edge->freq;
        if (distance <= static_cast<unsigned>(max_distance) && freq > 0)
          matched_words.push_back({current, freq, distance});
        if (continue_searching && edge->has_children())
          add_edges(edge->get_children());
        current.resize(current_word_len);
      }
    }

    void init_table()
    {
      for (size_t i = 0; i < table_len; ++i)
        table[i] = max_distance + 1;
      for (size_t i = 0; i < row_len; ++i)
      {
        at(i, 0) = i;
      }
      for (size_t i = 0; i < col_len; ++i)
        at(0, i) = i;
    }

    unsigned& at(int col, int row)
    {
      return table[row * row_len + col];
    }

  private:
    std::string word;
    int max_distance;
    size_t word_size = 0;
    size_t row_len = 0;
    size_t col_len = 0;
    size_t table_len = 0;
    unsigned* table = nullptr;
    std::string current;
    std::vector<match> matched_words;
};
