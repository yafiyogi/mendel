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

#include "yy_cpp/yy_find_iter_util.hpp"
#include "action_kalman.hpp"
#include "values_store.hpp"

namespace yafiyogi::mendel::actions {

KalmanAction::KalmanAction(std::string && p_output_topic,
                           const KalmanOptions & p_options):
  m_output_topic(std::move(p_output_topic))
{
  // Initialise outputs.
  size_type idx_n = 0;
  p_options.visit([&idx_n, this](auto & /* input_id */, const auto & output_id) {
    if(auto output{yy_data::find_iter(m_outputs, output_id)};
       !output.found)
    {
      m_outputs.emplace(output.iter, output_id, idx_n);
      ++idx_n;
    }
  });

  // Compact outputs.
  VarIdxVector{m_outputs}.swap(m_outputs);

  // Initialize inputs.
  size_type idx_m = 0;
  p_options.visit([&idx_m, this](const auto & input_id, const auto & output_id) {
    if(auto input{yy_data::find_iter(m_inputs, input_id)};
       !input.found)
    {
      if(auto output = yy_data::find_iter(m_outputs, output_id);
         output.found)
      {
        m_inputs.emplace(input.iter, input_id, idx_m);
        ++idx_m;
      }
    }
  });

  // Compact inputs.
  VarIdxVector{m_inputs}.swap(m_inputs);

  const size_type m = m_inputs.size();
  const size_type n = m_outputs.size();

  matrix H{m, n};

  p_options.visit([&H, this](const auto & input_id, const auto & output_id) {
    if(auto input = yy_data::find_iter(m_inputs, input_id);
       !input.found)
    {
      if(auto output = yy_data::find_iter(m_outputs, output_id);
         output.found)
      {
        H(input.iter->idx(), output.iter->idx()) = 1.0;
      }
    }
  });

  vector initial{n, 1.0};

  m_ekf = ekf{initial, H};

  m_hx.resize(m);
  m_observations.resize(m);
}

void KalmanAction::Run(const values::Store & store) noexcept
{
  m_ekf.predict();

  m_hx = m_ekf.X();

  size_type count = m_inputs.size();

  for(const auto & [input, idx_m] : m_inputs)
  {
    auto get_value = [idx_m, this](const auto & value) {
      m_observations(idx_m) = value;
    };

    if(!store.Find(get_value, input))
    {
      break;
    }
    --count;
  }

  if(0 == count)
  {
    m_ekf.update(m_observations, m_hx);
  }
}

} // namespace yafiyogi::mendel::actions
