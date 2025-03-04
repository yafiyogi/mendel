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

#include <charconv>

#include "fmt/format.h"

#include "actions_handler.hpp"
#include "mendel_labels.hpp"

#include "cache_handler.hpp"

namespace yafiyogi::mendel {

CacheHandler::CacheHandler(ActionsHandlerPtr p_action_handler,
                           actions::StorePtr p_actions_store,
                           values::StorePtr p_values_store):
  m_action_handler_ptr(std::move(p_action_handler)),
  m_actions_store(std::move(p_actions_store)),
  m_values_store(std::move(p_values_store))
{
  m_action_handler = m_action_handler_ptr.get();
}

void CacheHandler::Run(std::stop_token p_stop_token)
{
  actions::Store & l_actions_store = *m_actions_store;
  values::Store & l_values_store = *m_values_store;

  actions::Store::value_type l_actions;
  values::MetricDataVector l_data;

  auto actions_found = [&l_actions](auto p_actions) {
    auto & found_actions = * p_actions;

    for(auto found_action : found_actions)
    {
      if(auto [iter, found] = find_iter(l_actions, found_action);
         !found)
      {
        l_actions.emplace(iter, found_action);
      }
    }
  };

  while(!p_stop_token.stop_requested())
  {
    while(m_queue.swap_out(l_data))
    {
      for(auto & data : l_data)
      {
        std::string_view value_str{data.Value()};

        auto add_value_to_store = [value_str](value_ptr p_value) {
          double value;
          if(const auto [_, ec] = std::from_chars(value_str.begin(),
                                                  value_str.end(),
                                                  value);
             std::errc{} == ec)
          {
            p_value->store(value, std::memory_order_release);
          }
        };

        if(const values::MetricId & metric_id = data.Id();
           l_actions_store.Find(actions_found, metric_id, g_label_location))
        {
          if(l_values_store.Find(add_value_to_store, metric_id, g_label_location))
          {
          }
        }
      }

      l_data.clear(yy_data::ClearAction::Keep);
    }

    m_action_handler->Write(l_actions);
    l_actions.clear(yy_data::ClearAction::Keep);

    m_queue.wait(p_stop_token, [this] { return !m_queue.empty();});
  }
}

bool CacheHandler::Write(values::MetricDataVector & data)
{
  return m_queue.swap_in(data);
}

} // namespace yafiyogi::mendel
