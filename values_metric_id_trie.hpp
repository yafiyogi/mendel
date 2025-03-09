/*

  MIT License

  Copyright (c) 2024-2025 Yafiyogi

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#pragma once

#include <string>
#include <string_view>

#include "yy_cpp/yy_fm_flat_trie_ptr.h"
#include "yy_cpp/yy_span.h"
#include "yy_cpp/yy_tokenizer.h"

#include "values_metric_id.h"

namespace yafiyogi::values {
namespace metric_id_trie_detail {

static constexpr std::string_view MetricIdPathSeparator{":"};
static constexpr char MetricIdPathSeparatorChar = MetricIdPathSeparator[0];

template<typename TrieTraitsType>
class Query final
{
  public:
    using traits = TrieTraitsType;
    using tokenizer_type = typename traits::tokenizer_type;
    using token_type = typename traits::token_type;

    using label_const_l_value_ref = typename traits::label_l_value_ref;
    using label_span_type = typename tokenizer_type::label_span_type;

    using node_type = typename traits::ptr_node_type;
    using node_ptr = typename traits::ptr_node_ptr;
    using const_node_ptr = typename traits::ptr_const_node_ptr;
    using trie_vector = typename traits::ptr_trie_vector;
    using data_vector = typename traits::data_vector;

    using value_type = typename traits::value_type;

    constexpr explicit Query(trie_vector && p_nodes,
                             data_vector && p_data) noexcept:
      m_nodes(std::move(p_nodes)),
      m_data(std::move(p_data))
    {
    }

    constexpr Query() noexcept = default;
    Query(const Query &) = delete;
    constexpr Query(Query &&) noexcept = default;
    constexpr ~Query() noexcept = default;

    Query & operator=(const Query &) = delete;
    constexpr Query & operator=(Query &&) noexcept = default;

    template<typename Visitor>
    [[nodiscard]]
    constexpr bool find(Visitor && p_visitor,
                        const MetricId & p_metric_id) noexcept
    {
      node_ptr node{find_string(node_ptr{m_nodes.data()},
                                yy_quad::make_const_span(p_metric_id.Id()))};

      if(node)
      {
        node = find_string(node,
                           yy_quad::make_const_span(p_metric_id.Location()));
      }

      return visit(std::forward<Visitor>(p_visitor), node);
    }

    template<typename Visitor>
    [[nodiscard]]
    constexpr bool find(Visitor && p_visitor, std::string_view p_location) const noexcept
    {
      const_node_ptr node{find_string(const_node_ptr{m_nodes.data()},
                                      yy_quad::make_const_span(p_location))};

      return visit(std::forward<Visitor>(p_visitor), node);
    }

    [[nodiscard]]
    constexpr bool empty() const noexcept
    {
      return m_data.empty();
    }

  private:
    template<typename Visitor>
    [[nodiscard]]
    static constexpr bool visit(Visitor && visitor, node_ptr node)
    {
      bool has_payload = node && !node->empty();
      if(has_payload)
      {
        visitor(node->data());
      }

      return has_payload;
    }

    template<typename Visitor>
    [[nodiscard]]
    static constexpr bool visit(Visitor && visitor, const_node_ptr node)
    {
      bool has_payload = node && !node->empty();
      if(has_payload)
      {
        visitor(node->data());
      }

      return has_payload;
    }

    [[nodiscard]]
    static constexpr node_ptr find_string(node_ptr node,
                                          token_type token) noexcept
    {
      if(!node)
      {
        return node_ptr{};
      }

      auto next_node_do = [&node](auto edge_node, size_type)
      {
        node = *edge_node;
      };

      tokenizer_type tokenizer{token};

      while(!tokenizer.empty())
      {
        if(token_type label_part{tokenizer.scan()};
           !node->find_edge(next_node_do, label_part))
        {
          return node_ptr{};
        }
      }

      return node;
    }

    [[nodiscard]]
    static constexpr const_node_ptr find_string(const_node_ptr node,
                                                token_type token) noexcept
    {
      if(!node)
      {
        return const_node_ptr{};
      }

      auto next_node_do = [&node](auto edge_node, size_type)
      {
        node = edge_node->get();
      };

      tokenizer_type tokenizer{token};

      while(!tokenizer.empty())
      {
        if(token_type label_part{tokenizer.scan()};
           !node->find_edge(next_node_do, label_part))
        {
          return const_node_ptr{};
        }
      }

      return node;
    }

    trie_vector m_nodes{};
    data_vector m_data{};
};

template<typename LabelType>
using tokenizer_type = yy_trie::label_word_tokenizer<LabelType,
                                                     MetricIdPathSeparatorChar,
                                                     yy_util::tokenizer>;
} // namespace metric_id_trie_detail

template<typename ValueType>
using metric_id_trie = yy_data::fm_flat_trie_ptr<std::string,
                                                 ValueType,
                                                 metric_id_trie_detail::Query,
                                                 metric_id_trie_detail::tokenizer_type>;
} // namespace yafiyogi::values
