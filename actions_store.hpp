/*

  MIT License

  Copyright (c) 2025 Yafiyogi

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

#include "yy_cpp/yy_observer_ptr.hpp"

#include "action.hpp"
#include "values_metric_id_trie.hpp"

namespace yafiyogi::actions {

class Store
{
  public:
    using value_type = yy_quad::simple_vector<actions::ActionObsPtr>;
    using value_ptr = yy_data::observer_ptr<value_type>;
    using store_builder_type = values::metric_id_trie<value_type>;
    using store_type = store_builder_type::automaton_type;
    using actions_type = yy_quad::simple_vector<actions::ActionPtr>;

    Store(store_type && p_store, actions_type && p_actions);

    constexpr Store() noexcept = default;
    Store(const Store &) = delete;
    constexpr Store(Store &&) noexcept = default;

    Store & operator=(const Store &) = delete;
    constexpr Store & operator=(Store &&) noexcept = default;

    template<typename Visitor>
    [[nodiscard]]
    constexpr bool Find(Visitor && p_visitor,
                        const values::MetricId & p_metric) noexcept
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
    store_type m_store{};
    actions_type m_actions{};
};

using StorePtr = std::unique_ptr<Store>;
using StoreObsPtr = yy_data::observer_ptr<Store>;

class StoreBuilder
{
  public:
    using store_builder_type = Store::store_builder_type;
    using store_type = Store::store_type;
    using action_pointers = Store::value_type;
    using actions_type = Store::actions_type;
    using Inputs = yy_quad::simple_vector<std::string>;

    void Add(ActionPtr action, Inputs & inputs);
    StorePtr Create();

  private:
    using actions = yy_quad::simple_vector<ActionPtr>;

    store_builder_type m_store_builder{};
    actions_type m_actions{};
};


} // namespace yafiyogi::actions
