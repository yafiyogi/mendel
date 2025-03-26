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

#include "spdlog/spdlog.h"

#include "action.hpp"
#include "actions_handler.hpp"
#include "values_metric_id_fmt.hpp"
#include "values_metric_labels.hpp"

namespace yafiyogi::mendel {

using namespace std::string_view_literals;

namespace {

class NullAction final:
      public actions::Action
{
  public:
    void Run(const actions::ParamVector & /* params */,
             actions::ActionResultVector & /* p_results */,
             values::Store & /* store */,
             timestamp_type /* timestamp */) noexcept override
    {
    }

    const std::string_view Id() const noexcept override
    {
      return "(null)"sv;
    }

    const std::string_view Name() const noexcept override
    {
      return "Null Action"sv;
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
             actions::ActionResultVector & p_results,
             timestamp_type timestamp)
    {
      spdlog::debug("Processing action=[{}] id=[{}]",
                    action->Name(),
                    action->Id());
      action->Run(params, p_results, p_value_store, timestamp);
    }

    actions::ActionObsPtr action = g_null_action;
    actions::ParamVector params{};
};

using ActionValueVector = yy_quad::simple_vector<ActionValue>;

} // anonymous namespace

ActionsHandler::ActionsHandler(actions::StorePtr p_actions_store,
                               values::StorePtr p_values_store,
                               values::MetricDataQueueReader && p_queue_in,
                               actions::ActionResultQueueWriter && p_queue_out):
  m_actions_store(std::move(p_actions_store)),
  m_values_store(std::move(p_values_store)),
  m_queue_in(std::move(p_queue_in)),
  m_queue_out(std::move(p_queue_out))
{
}

void ActionsHandler::Run(std::stop_token p_stop_token)
{
  values::MetricDataVector l_data_in;

  actions::Store & l_actions_store = *m_actions_store;
  values::Store & l_values_store = *m_values_store;

  size_type spin = 1; // Set to 1 to prevent spinning at startup.

  actions::ActionResultVector l_action_values{};
  ActionValueVector l_actions{};

  while(!p_stop_token.stop_requested())
  {
    while(m_queue_in.QSwapOut(l_data_in))
    {
      spin = spin_max; // Reset spin count.

      l_action_values.clear(yy_data::ClearAction::Keep);
      l_actions.clear(yy_data::ClearAction::Keep);

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

            auto & params = action_iter->params;

            if(auto [param_iter, param_found] = yy_data::find_iter(params, data.Id(), actions::actions_detail::compare_param);
               !param_found)
            {
              values::MetricDataObsPtr param{&data};

              params.emplace(param_iter, param);
            }
          }
        };

        std::ignore = l_actions_store.Find(add_actions_n_data, data.Id());
      }

      timestamp_type timestamp{std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::utc_clock::now()).time_since_epoch()};
      for(auto & action : l_actions)
      {
        action.Run(l_values_store, l_action_values, timestamp);
      }

      m_queue_out.QSwapIn(l_action_values);
    }

    if(0 == --spin)
    {
      spin = spin_max; // Reset spin count.
      m_queue_in.QWait(p_stop_token, [this] { return !m_queue_in.QEmpty();});
    }
  }
}

} // namespace yafiyogi::mendel
