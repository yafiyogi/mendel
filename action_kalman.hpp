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
#include "yy_values/yy_values_metric_id.hpp"

#include "action.hpp"

namespace yafiyogi::actions {
namespace kalman_action_detail {

struct OutputMapping
{
    constexpr bool operator<(const std::string & value) const noexcept
    {
      return property < value;
    }

    constexpr bool operator==(const std::string & value) const noexcept
    {
      return property == value;
    }

    std::string property{};
    yy_values::MetricId value_id{};
    size_type output_idx = 0;
};

struct InputMapping
{
    constexpr bool operator<(const yy_values::MetricId & value) const noexcept
    {
      return value_id < value;
    }

    constexpr bool operator==(const yy_values::MetricId & value) const noexcept
    {
      return value_id == value;
    }

    yy_values::MetricId value_id{};
    size_type input_idx = 0;
    size_type output_idx = 0;
    bool initialized = false;

    static int compare(const InputMapping & mapping,
                       const yy_values::MetricId & id)
    {
      return mapping.value_id.compare(id);
    }
};

} // namespace kalman_action_detail

struct KalmanOption
{
    yy_values::MetricId input{};
    std::string output{};
    yy_maths::ekf::value_type accuracy{yy_maths::ekf::EPS};

    static int compare(const KalmanOption & option,
                       const yy_values::MetricId & id)
    {
      return option.input.compare(id);
    }
};

using KalmanOptions = yy_quad::simple_vector<KalmanOption>;

class KalmanAction final:
      public Action
{
  public:
    using ekf = yy_maths::ekf;
    using value_type = ekf::value_type;
    using size_type = ekf::size_type;

    static constexpr value_type EPS = yy_maths::ekf::EPS;

    KalmanAction(std::string_view p_id,
                 std::string_view p_output_topic,
                 std::string_view p_output_value_id,
                 const KalmanOptions & p_options);
    void Run(const ParamVector & p_params,
             ActionResultVector & p_results,
             values::Store & p_values_store,
             timestamp_type p_timestamp) noexcept override;

    const std::string_view Id() const noexcept override;
    const std::string_view Name() const noexcept override;

  private:
    using matrix = ekf::matrix;
    using zero_matrix = ekf::zero_matrix;
    using vector = ekf::vector;
    using zero_vector = ekf::zero_vector;

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

    ActionResult m_result{};
};

} // namespace yafiyogi::actions
