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

#include <mutex>
#include <string_view>

#include "yy_cpp/yy_flat_map.h"

#include "values_metric_id_trie.hpp"

namespace yafiyogi::values {

class MetricData;
class MetricDataVector;

class StoreBuilder final
{
  public:
    using store_builder_type = metric_id_trie<double>;
    store_builder_type m_store_builder;

    void Add(const MetricData & p_metric_data);
    void Add(const MetricDataVector & p_metric_data);
};

class Store final
{
  public:
    template<typename Visitor>
    [[nodiscard]]
    constexpr bool Find(Visitor && p_visitor,
                        const MetricId & p_metric) noexcept
    {
      return m_store.find(std::forward<Visitor>(p_visitor), p_metric);
    }

    template<typename Visitor>
    [[nodiscard]]
    constexpr bool Find(Visitor && p_visitor,
                        std::string_view p_metric) const noexcept
    {
      return m_store.find(std::forward<Visitor>(p_visitor), p_metric);
    }

  private:
    using store_type = StoreBuilder::store_builder_type::automaton_type;
    store_type m_store{};
};

} // namespace yafiyogi::values
