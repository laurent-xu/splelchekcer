#pragma once
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "trie.hh"

struct compact_info;
struct compact_iterator
{
  public:
    compact_iterator(char* pos) : pos(pos) {}

    compact_iterator& operator++()
    {
      size_t next_edge = *reinterpret_cast<size_t*>(pos + sizeof(size_t));
      pos = next_edge ? pos + next_edge : 0;
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
    char* pos;
};

struct compact_info
{
  size_t freq;
  size_t next_edge;
  size_t offset;
  char label[1];

  compact_iterator begin()
  {
    char* ptr = reinterpret_cast<char*>(this);
    return compact_iterator(ptr);
  }

  compact_iterator end()
  {
    return compact_iterator(0);
  }

  compact_iterator get_children_it()
  {
    if (0 == offset)
      return end();
    char* ptr = reinterpret_cast<char*>(this);
    return compact_iterator(ptr + offset);
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
      data_ = reinterpret_cast<char*>(file_);
    }

    trie to_trie()
    {
      auto result = trie();
      if (data_ == nullptr)
      {
        std::cerr << "Open a file first" << std::endl;
        std::exit(1);
      }

      compact_info *root_edges = reinterpret_cast<compact_info*>(data_);
      result.root_.edges = to_trie_node(root_edges);
      return result;
    }

    static std::streampos serialize(std::ostream& os,
                            const std::vector<edge_t*>& edges)
    {
      auto start_pos = os.tellp();
      size_t empty_size_t = 0;
      std::vector<std::pair<std::streampos, std::streampos>> offsets_pos;
      for (size_t i = 0; i < edges.size(); ++i)
      {
        auto& edge = *edges[i];
        auto start_edge_pos = os.tellp();
        os.write(reinterpret_cast<const char*>(&edge.dst->freq),
                  sizeof(size_t));
        auto next_edge_pos = os.tellp();
        os.write(reinterpret_cast<const char*>(&empty_size_t), sizeof(size_t));
        auto offset_pos = os.tellp();
        os.write(reinterpret_cast<const char*>(&empty_size_t), sizeof(size_t));
        os.write(edge.label.c_str(), edge.label.size() + 1);
        auto end_pos = os.tellp();
        os.seekp(next_edge_pos);
        size_t next_edge_offset = end_pos - start_edge_pos;
        if (i == edges.size() - 1)
          next_edge_offset = 0;
        os.write(reinterpret_cast<const char*>(&next_edge_offset),
                 sizeof(size_t));
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
                   sizeof(size_t));
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

  private:
    int fd_ = -1;
    char* data_ = nullptr;
    void* file_ = nullptr;
    size_t file_size_ = -1;
};

