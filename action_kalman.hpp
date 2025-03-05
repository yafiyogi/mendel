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

#include "yy_cpp/yy_flat_map.h"
#include "yy_cpp/yy_vector.h"
#include "yy_maths/yy_ekf.hpp"

#include "action.hpp"

namespace yafiyogi::mendel::actions {
namespace kalman_action_detail {

struct OutputMapping
{
    constexpr bool operator<(const std::string & value) const noexcept
    {
      return var < value;
    }

    constexpr bool operator==(const std::string & value) const noexcept
    {
      return var == value;
    }

    std::string var{};
    size_type output_idx = 0;
};

struct InputMapping
{
    constexpr bool operator<(const std::string & value) const noexcept
    {
      return var < value;
    }

    constexpr bool operator==(const std::string & value) const noexcept
    {
      return var == value;
    }

    std::string var{};
    size_type input_idx = 0;
    size_type output_idx = 0;
};

} // namespace kalman_action_detail

using KalmanOptions = yy_data::flat_map<std::string, std::string>;

class KalmanAction final:
      public Action
{
  public:
    KalmanAction(std::string_view p_id,
                 std::string_view p_output_topic,
                 const KalmanOptions & p_options);
    void Run(const values::Store & store) noexcept override;

    constexpr const std::string & Id() const noexcept
    {
      return m_id;
    }

  private:
    using ekf = yy_maths::ekf;
    using matrix = ekf::matrix;
    using zero_matrix = ekf::zero_matrix;
    using vector = ekf::vector;
    using zero_vector = ekf::zero_vector;
    using size_type = ekf::size_type;

    std::string m_id;
    std::string m_output_topic{};

    ekf m_ekf{};
    vector m_observations{};
    matrix m_h{};
    vector m_hx{};

    using OutputMap = yy_quad::simple_vector<kalman_action_detail::OutputMapping>;
    OutputMap m_outputs{};

    using InputMap = yy_quad::simple_vector<kalman_action_detail::InputMapping>;

    InputMap m_inputs{};
};

} // namespace yafiyogi::mendel::actions
