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

#include "spdlog/spdlog.h"

#include "yy_cpp/yy_find_iter_util.hpp"

#include "values_metric_id_fmt.hpp"

#include "action_kalman.hpp"
#include "values_store.hpp"

namespace yafiyogi::mendel::actions {

KalmanAction::KalmanAction(std::string_view p_id,
                           std::string_view p_output_topic,
                           const KalmanOptions & p_options):
  m_id(std::move(p_id)),
  m_output_topic(std::move(p_output_topic))
{
  // Initialise outputs.
  size_type idx_n = 0;
  p_options.visit([&idx_n, this](auto & /* input_id */, const auto & output_id) {
    if(auto output{yy_data::find_iter(m_outputs, output_id)};
       !output.found)
    {
      spdlog::debug("   output: [{}]", output_id);

      m_outputs.emplace(output.iter, output_id, idx_n);
      ++idx_n;
    }
  });

  // Compact outputs.
  OutputMap{m_outputs}.swap(m_outputs);

  // Initialize inputs.
  size_type idx_m = 0;
  p_options.visit([&idx_m, this](const auto & input_id, const auto & output_id) {
    if(auto input{yy_data::find_iter(m_inputs, input_id)};
       !input.found)
    {
      if(auto [output, output_found] = yy_data::find_iter(m_outputs, output_id);
         output_found)
      {
        spdlog::debug("   input: [{}] out=[{}]",
                      input_id,
                      output->var);

        m_inputs.emplace(input.iter, input_id, idx_m, output->output_idx);
        ++idx_m;
      }
    }
  });

  // Compact inputs.
  InputMap{m_inputs}.swap(m_inputs);

  const size_type m = m_inputs.size();
  const size_type n = m_outputs.size();

  m_ekf = yy_maths::ekf{m, n};
  m_observations.resize(m);
}

void KalmanAction::Run(const ParamVector & p_params,
                       const values::Store & /* values_store */) noexcept
{
  // Zero mapping sensor-function Jacobian matrix h.
  m_h = zero_matrix{m_ekf.M(), m_ekf.N()};
  // Zero vector predicted values hx
  m_hx = zero_vector{m_ekf.M()};

  bool do_calc = false;

  // Only update changed values.
  for(auto param : p_params)
  {
    if(auto [input, found] = yy_data::find_iter(m_inputs, param->Id());
       found)
    {
      auto & [var, input_idx, output_idx] = *input;

      auto set_observation = [&var, input_idx, this](double value) {
        spdlog::debug("kalman: [{}] [{}]=[{:.2f}]", m_id, var, value);
        m_observations(input_idx) = value;
      };

      std::visit(set_observation, param->Binary());

      m_h(input_idx, output_idx) = 1.0;
      m_hx(input_idx) = m_ekf.X(output_idx);
      do_calc = true;
    }
  }

  if(do_calc)
  {
    spdlog::debug("kalman: [{}] previous [{:.2f}]", m_id, fmt::join(m_ekf.X(), " "));
    m_ekf.predict();

    spdlog::debug("kalman: [{}]   inputs [{:.2f}]", m_id, fmt::join(m_observations, " "));
    m_ekf.update(m_observations, m_h, m_hx);
    spdlog::debug("kalman: [{}]  outputs [{:.2f}]", m_id, fmt::join(m_ekf.X(), " "));
  }
}

} // namespace yafiyogi::mendel::actions
