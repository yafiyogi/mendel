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

#include <chrono>

#include "fmt/format.h"

#include "action.hpp"
#include "actions_handler.hpp"
#include "values_metric_labels.hpp"

namespace yafiyogi::mendel {
namespace {

class NullAction final:
      public actions::Action
{
  public:
    void Run(const actions::ParamVector & /* params */,
             values::Store & /* store */,
             timestamp_type /* timestamp */) noexcept override
    {
    }
};

NullAction null_action;
actions::ActionObsPtr g_null_action{&null_action};

constexpr size_type spin_max = 400;

struct ActionValue final
{
    bool operator<(const actions::ActionObsPtr & p_action) const noexcept
    {
      return action < p_action;
    }

    bool operator==(const actions::ActionObsPtr & p_action) const noexcept
    {
      return action == p_action;
    }

    void Run(values::Store & p_value_store,
             timestamp_type timestamp)
    {
      action->Run(values, p_value_store, timestamp);
    }

    actions::ActionObsPtr action = g_null_action;
    actions::ParamVector values{};
};

using ActionValueVector = yy_quad::simple_vector<ActionValue>;


} // anonymous namespace

ActionsHandler::ActionsHandler(actions::StorePtr p_actions_store,
                               values::StorePtr p_values_store,
                               values::MetricDataQueueReader && p_queue):
  m_actions_store(std::move(p_actions_store)),
  m_values_store(std::move(p_values_store)),
  m_queue(std::move(p_queue))
{
}

void ActionsHandler::Run(std::stop_token p_stop_token)
{
  values::MetricDataVector l_data_in;

  actions::Store & l_actions_store = *m_actions_store;
  values::Store & l_values_store = *m_values_store;

  size_type spin = 1; // Set to 1 to prevent spinning at startup.
  while(!p_stop_token.stop_requested())
  {
    while(m_queue.QSwapOut(l_data_in))
    {
      spin = spin_max; // Reset spin count.

      ActionValueVector l_actions{};
      for(auto & data : l_data_in)
      {
        auto add_actions_n_data = [&data, &l_actions](actions::Store::value_ptr actions) {
          for(auto action : *actions)
          {
            auto [action_iter, action_found] = yy_data::find_iter(l_actions, action);
            if(!action_found)
            {
              auto [iter, _] = l_actions.emplace(action_iter, action, actions::ParamVector{});
              action_iter = std::move(iter);
            }

            auto & values = action_iter->values;

            values::MetricDataObsPtr data_ptr{&data};
            if(auto [data_iter, data_found] = yy_data::find_iter(values, data_ptr);
               !data_found)
            {
              values.emplace(data_iter, data_ptr);
            }
          }
       };

        std::ignore = l_actions_store.Find(add_actions_n_data, data.Id());
      }

      timestamp_type timestamp{std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::utc_clock::now()).time_since_epoch()};
      for(auto & action : l_actions)
      {
        action.Run(l_values_store, timestamp);
      }
      l_actions.clear(yy_data::ClearAction::Keep);
    }

    if(0 == --spin)
    {
      spin = spin_max; // Reset spin count.
      m_queue.QWait(p_stop_token, [this] { return !m_queue.QEmpty();});
    }
  }
}

} // namespace yafiyogi::mendel
