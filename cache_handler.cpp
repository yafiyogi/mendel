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

#include "spdlog/spdlog.h"

#include "values_metric_id_fmt.hpp"

#include "actions_handler.hpp"
#include "values_metric_labels.hpp"

#include "cache_handler.hpp"

namespace yafiyogi::mendel {
namespace {

constexpr size_type spin_max = 400;

} // anonymous namespace

CacheHandler::CacheHandler(ActionsHandlerPtr p_actions_handler,
                           values::StorePtr p_values_store):
  m_actions_handler(std::move(p_actions_handler)),
  m_values_store(std::move(p_values_store))
{
}

void CacheHandler::Run(std::stop_token p_stop_token)
{
  auto & l_actions_handler = *m_actions_handler;
  values::Store & l_values_store = *m_values_store;

  values::MetricDataVector l_data_in;
  values::MetricDataVector l_data_out;

  size_type spin = 1; // Set to 1 to prevent spinning at startup.
  while(!p_stop_token.stop_requested())
  {
    while(m_queue.swap_out(l_data_in))
    {
      spin = spin_max; // Reset spin count.

      l_data_out.clear(yy_data::ClearAction::Keep);

      bool send_data = false;
      for(auto & data : l_data_in)
      {
        auto & metric_id = data.Id();

        std::string_view value_str{data.Value()};

        auto add_value_to_store = [&send_data, &l_data_out, &data, value_str](value_ptr p_value) {
          double value;
          if(const auto [_, ec] = std::from_chars(value_str.begin(),
                                                  value_str.end(),
                                                  value);
             std::errc{} == ec)
          {
            data.Binary(value);
            send_data = (value != p_value->exchange(value, std::memory_order_release)) || send_data;

            l_data_out.emplace_back(std::move(data));
          }
        };

        std::ignore = l_values_store.Find(add_value_to_store, metric_id);
      }

      l_data_in.clear(yy_data::ClearAction::Keep);

      if(send_data && !l_data_out.empty())
      {
        l_actions_handler.QWrite(l_data_out);
      }
    }

    if(0 == --spin)
    {
      spin = spin_max; // Reset spin count.
      m_queue.wait(p_stop_token, [this] { return !m_queue.empty();});
    }
  }
}


bool CacheHandler::QWrite(values::MetricDataVector & p_data)
{
  if(p_data.empty())
  {
    return true;
  }

  return m_queue.swap_in(p_data);
}

} // namespace yafiyogi::mendel
