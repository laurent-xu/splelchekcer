#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <cassert>

struct edge_t;
/**
 * \struct Node of trie
 */
struct node_t
{
  using edge_ptr_t = edge_t*;
  /**
   * List of edges
   */
  std::vector<edge_ptr_t> edges;
  /**
   * If this node finishes a word this is the frequency otherwise it's 0.
   */
  size_t freq = 0;
};

/**
 * \struct Edge of trie
 */
struct edge_t
{
  using node_ptr_t = node_t*;
  /**
   * Destination node
   */
  node_ptr_t dst = nullptr;
  /**
   * Label over an edge
   */
  std::string label;
};

class compact_trie;
class trie
{
  public:
    node_t root_;
    using edge_ptr_t = node_t::edge_ptr_t;
    /**
     * \brief This parses a trie from an std::istream
     */
    void build(std::istream& is)
    {
      std::string word;
      size_t freq;

      while (!is.eof())
      {
        is >> word;
        is >> freq;
        if (is.good())
        {
          add_node(root_, make_edge(word, freq));
        }
      }
    }

    /**
     * \brief Output the trie in the dot format
     */
    void to_dot(std::ostream& out) const
    {
      out << "digraph Trie {" << std::endl;
      to_dot(root_, out);
      out << "}" << std::endl;
    }

  private:
    friend class compact_trie;
    /**
     * \brief Create a new edge
     */
    static edge_ptr_t make_edge(const std::string& word, size_t freq)
    {
        edge_ptr_t edge = new edge_t;
        edge->dst = new node_t;
        edge->label = word;
        edge->dst->freq = freq;
        return edge;
    }

    /**
     * \brief Add an edge to a node
     */
    void add_node(node_t& node, edge_ptr_t edge)
    {
      assert("" != edge->label);
      for (auto& child: node.edges)
      {
        // the final node already exists
        if (edge->label == child->label)
        {
          auto new_dst = child->dst;
          assert(0 == new_dst->freq);
          new_dst->freq = edge->dst->freq;
          return;
        }

        edge_ptr_t shorter;
        edge_ptr_t longer;
        if (edge->label.size() > child->label.size())
        {
          shorter = child;
          longer = edge;
        }
        else
        {
          shorter = edge;
          longer = child;
        }

        size_t pos = 0;
        for (pos = 0; pos < shorter->label.size()
             && shorter->label[pos] == longer->label[pos]; ++pos)
          continue;
        if (pos > 0)
        {
          assert(pos < longer->label.size());
          longer->label = longer->label.substr(pos);
          // shorter is included in longer
          if (pos == shorter->label.size())
          {
            child = shorter;
            add_node(*shorter->dst, longer);
          }
          // The shortest node needs for a split
          else
          {
            auto prefix = shorter->label.substr(0, pos);
            shorter->label = shorter->label.substr(pos);
            child = make_edge(prefix, 0);
            add_node(*child->dst, shorter);
            add_node(*child->dst, longer);
          }
          return;
        }
      }
      node.edges.push_back(edge);
    }

    static void to_dot(const node_t& n, std::ostream& out)
    {
      out << "node" << &n << " [label=\"" << n.freq << "\"];" << std::endl;
      for (const auto& child: n.edges)
      {
        out << "node" << &n << " -> node" << child->dst
            << " [label=\""<< child->label << "\"];" << std::endl;
        to_dot(*child->dst, out);
      }
    }
};
