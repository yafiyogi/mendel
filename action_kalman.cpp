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

#include <charconv>

#include "fmt/format.h"
#include "fmt/compile.h"
#include "spdlog/spdlog.h"

#include "yy_cpp/yy_find_iter_util.hpp"

#include "yy_maths/yy_matrix_fmt.hpp"

#include "values_metric_id_fmt.hpp"

#include "action_kalman.hpp"
#include "values_store.hpp"

namespace yafiyogi::actions {

using namespace std::string_view_literals;
using namespace fmt::literals;

namespace {

// json format
// {
//   "<property>":<float>,
//   ...
// }

constexpr auto g_json_property_format{"\"{}\":{:.2f},"_cf};
constexpr auto g_timestamp_format{"\"utc_micros\":{}}}"_cf};
constexpr auto g_ouput_value_id{"{}:{}"_cf};

constexpr auto g_predict_interval{timestamp_type{std::chrono::seconds(60)}};

} // anonymous

KalmanAction::KalmanAction(std::string_view p_id,
                           std::string_view p_output_topic,
                           std::string_view p_output_value_id,
                           const KalmanOptions & p_options):
  m_id(std::move(p_id)),
  m_output_topic(std::move(p_output_topic))
{
  auto calc_output_value_id = [&p_output_value_id](auto property) -> values::MetricId {
    return values::MetricId(fmt::format(g_ouput_value_id, p_output_value_id, property));
  };
  // Initialise outputs.
  size_type idx_n = 0;

  for(const auto & [input_id, output_id, accuracy] : p_options)
  {
    if(auto output{yy_data::find_iter(m_outputs, output_id)};
       !output.found)
    {
      auto [iter, _] = m_outputs.emplace(output.iter,
                                         output_id,
                                         calc_output_value_id(output_id),
                                         idx_n);
      spdlog::info("    output: [{}] value_id:[{}]"sv,
                   iter->property,
                   iter->value_id);

      ++idx_n;
    }
  }

  // Compact outputs.
  OutputMap{m_outputs}.swap(m_outputs);

  // Initialize inputs.
  size_type idx_m = 0;
  for(const auto & [input_id, output, accuracy] : p_options)
  {
    if(auto [output_iter, output_found] = yy_data::find_iter(m_outputs, output);
       output_found)
    {
      auto output_idx = static_cast<int>(output_iter->output_idx);
      auto compare_input = [output_idx](const kalman_action_detail::InputMapping & p_mapping,
                                        const values::MetricId & p_input_id) -> int {
        int comp = p_mapping.value_id.compare(p_input_id);

        if(comp == 0)
        {
          comp = static_cast<int>(p_mapping.output_idx) - output_idx;
        }

        return comp;
      };

      if(auto [input_iter, input_found] = yy_data::find_iter(m_inputs, input_id, compare_input);
         !input_found)
      {
        spdlog::info("    input: [{}] out=[{}]"sv,
                     input_id,
                     output_iter->property);

        m_inputs.emplace(input_iter, input_id, idx_m, output_iter->output_idx);
        ++idx_m;
      }
    }
  }

  // Compact inputs.
  InputMap{m_inputs}.swap(m_inputs);

  m_ekf = yy_maths::ekf{m_inputs.size(), m_outputs.size()};
  m_observations.resize(m_ekf.M());

  // Zero mapping sensor-function Jacobian matrix h.
  m_h = zero_matrix{m_ekf.M(), m_ekf.N()};
}

void KalmanAction::Run(const ParamVector & p_params,
                       ActionResultVector & p_results,
                       values::Store & p_values_store,
                       timestamp_type p_timestamp) noexcept
{
  // Zero vector predicted values hx
  m_hx = zero_vector{m_ekf.M()};

  auto set_observation = [](std::string_view source,
                            const values::MetricId input_value_id,
                            value_type & z,
                            value_type value) {
    spdlog::debug("  {} [{}] value [{:.2f}]"sv, source, input_value_id, value);
    z = value;
  };

  for(auto & input : m_inputs)
  {
    if(const auto [param_iter, param_found] = yy_data::find_iter(p_params, input.value_id, actions_detail::compare_param);
       param_found)
    {
      input.initialized = true;

      m_h(input.input_idx, input.output_idx) = 1.0;
      m_hx(input.input_idx) = m_ekf.X(input.output_idx);
      auto & z = m_observations(input.input_idx);

      auto param_set_observation = [&input, &z, &set_observation](value_type value) {
        set_observation("parameter"sv,
                        input.value_id,
                        z,
                        value);
      };

      std::visit(param_set_observation, (*param_iter)->Binary());
    }
    else if(input.initialized)
    {
      auto store_set_observation = [this, &input, &set_observation](auto value) {
        m_hx(input.input_idx) = m_ekf.X(input.output_idx);
        auto & z = m_observations(input.input_idx);

        set_observation("store"sv,
                        input.value_id,
                        z,
                        value->load(std::memory_order_acquire));
      };

      std::ignore = p_values_store.Find(store_set_observation, input.value_id);
    }
  }

  spdlog::debug("  inputs  : [{:.2f}]"sv, m_observations);
  //spdlog::debug("  mappings: [{:.0f}]"sv, m_h);
  spdlog::debug("  previous: [{:.2f}]"sv, m_ekf.X());

  m_ekf.predict();
  m_ekf.update(m_observations, m_h, m_hx);
  spdlog::debug("  outputs : [{:.2f}]"sv, m_ekf.X());

  m_result.topic = m_output_topic;
  m_result.data = "{"sv;

  for(auto & output : m_outputs)
  {
    auto ekf_Xn = m_ekf.X(output.output_idx);

    auto add_value_to_store = [&ekf_Xn](auto p_value) {
      p_value->store(ekf_Xn, std::memory_order_release);
    };

    std::ignore = p_values_store.Find(add_value_to_store, output.value_id);

    fmt::format_to(std::back_inserter(m_result.data),
                   g_json_property_format,
                   output.property,
                   ekf_Xn);
  }

  fmt::format_to(std::back_inserter(m_result.data),
                 g_timestamp_format,
                 std::chrono::duration_cast<std::chrono::microseconds>(p_timestamp).count());

  spdlog::debug("  result  : json=[{}]"sv, m_result.data);

  p_results.swap_data_back(m_result);
}

const std::string_view KalmanAction::Id() const noexcept
{
  return m_id;
}

const std::string_view KalmanAction::Name() const noexcept
{
  return "Kalman Filter"sv;
}

} // namespace yafiyogi::actions
