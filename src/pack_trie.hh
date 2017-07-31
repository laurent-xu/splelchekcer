#pragma once
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "trie.hh"

struct compact_info;

using freq_t = unsigned;
using next_edge_t = unsigned;
using offset_t = size_t;

struct compact_iterator
{
  public:
    compact_iterator(void* pos) : pos(pos) {}

    compact_iterator& operator++()
    {
      char* ptr = reinterpret_cast<char*>(pos);
      next_edge_t next_edge = *reinterpret_cast<next_edge_t*>(ptr);
      pos = next_edge ? reinterpret_cast<void*>(ptr + next_edge) : nullptr;
      return *this;
    }

    compact_info* operator*()
    {
      return reinterpret_cast<compact_info*>(pos);
    }

    compact_info* operator->()
    {
      return operator*();
    }

    bool operator==(const compact_iterator& other)
    {
      return other.pos == pos;
    }

    bool operator!=(const compact_iterator& other)
    {
      return other.pos != pos;
    }

  private:
    void* pos;
};

struct compact_info
{
  next_edge_t next_edge;
  freq_t freq;
  offset_t offset;
  char label[1];

  compact_iterator begin()
  {
    void* ptr = reinterpret_cast<void*>(this);
    return compact_iterator(ptr);
  }

  compact_iterator end()
  {
    return compact_iterator(nullptr);
  }

  compact_iterator get_children_it()
  {
    if (0 == offset)
      return end();
    char* ptr = reinterpret_cast<char*>(this);
    return compact_iterator(reinterpret_cast<void*>(ptr + offset));
  }

  compact_info* get_children()
  {
    char* ptr = reinterpret_cast<char*>(this);
    return reinterpret_cast<compact_info*>(ptr + offset);
  }

  bool has_children()
  {
    return 0 != offset;
  }
}
__attribute__((packed));

class compact_trie
{
  public:
    compact_trie() {}
    ~compact_trie()
    {
      munmap(file_, file_size_);
      close(fd_);
    }

    void load(const std::string filename)
    {
      struct stat s;
      fd_ = open(filename.c_str(), 0);
      if (fd_ == -1 || fstat(fd_, &s) < 0)
      {
        std::cerr << "Couldn't open the file: " << filename << std::endl;
        std::exit(1);
      }
      file_size_ = s.st_size;

      file_ = mmap(NULL, file_size_, PROT_READ, MAP_FILE | MAP_SHARED, fd_, 0);
    }

    trie to_trie()
    {
      auto result = trie();
      if (file_ == nullptr)
      {
        std::cerr << "Open a file first" << std::endl;
        std::exit(1);
      }

      compact_info *root_edges = reinterpret_cast<compact_info*>(file_);
      result.root_.edges = to_trie_node(root_edges);
      return result;
    }

    static std::streampos serialize(std::ostream& os,
                            const std::vector<edge_t*>& edges)
    {
      auto start_pos = os.tellp();
      offset_t node_empty = 0;
      next_edge_t edge_empty = 0;
      std::vector<std::pair<std::streampos, std::streampos>> offsets_pos;
      for (size_t i = 0; i < edges.size(); ++i)
      {
        auto& edge = *edges[i];
        auto start_edge_pos = os.tellp();
        auto next_edge_pos = os.tellp();
        os.write(reinterpret_cast<const char*>(&edge_empty),
                 sizeof(next_edge_t));
        os.write(reinterpret_cast<const char*>(&edge.dst->freq),
                 sizeof(freq_t));
        auto offset_pos = os.tellp();
        os.write(reinterpret_cast<const char*>(&node_empty),
                 sizeof(offset_t));
        os.write(edge.label.c_str(), edge.label.size() + 1);
        auto end_pos = os.tellp();
        os.seekp(next_edge_pos);
        size_t next_edge_offset = end_pos - start_edge_pos;
        if (i == edges.size() - 1)
          next_edge_offset = 0;
        os.write(reinterpret_cast<const char*>(&next_edge_offset),
                 sizeof(next_edge_t));
        os.seekp(end_pos);
        offsets_pos.emplace_back(start_edge_pos, offset_pos);
      }

      for (size_t i = 0; i < edges.size(); ++i)
      {
        auto& edge = *edges[i];
        auto start_edge_pos = offsets_pos[i].first;
        auto offset_pos = offsets_pos[i].second;
        if (!edge.dst->edges.empty())
        {
          auto node_pos = serialize(os, edge.dst->edges);
          auto current_pos = os.tellp();
          os.seekp(offset_pos);
          size_t node_offset = node_pos - start_edge_pos;
          os.write(reinterpret_cast<const char*>(&node_offset),
                   sizeof(offset_t));
          os.seekp(current_pos);
        }
      }
      return start_pos;
    }

    std::vector<edge_t*>
    to_trie_node(compact_info* compact)
    {
      std::vector<edge_t*> edges;
      for (auto it = compact->begin(); it != compact->end(); ++it)
      {
        std::string label = std::string(reinterpret_cast<char*>(it->label));
        auto edge = trie::make_edge(label, it->freq);
        edges.push_back(edge);
        if (it->has_children())
          edge->dst->edges = to_trie_node(it->get_children());
      }
      return edges;
    }

    compact_info* get_root()
    {
      return reinterpret_cast<compact_info*>(file_);
    }

  private:
    int fd_ = -1;
    void* file_ = nullptr;
    size_t file_size_ = -1;
};

