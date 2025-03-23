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

#include <chrono>
#include <memory>

#include "yy_cpp/yy_observer_ptr.hpp"
#include "yy_cpp/yy_vector.h"

#include "action_result.hpp"
#include "values_metric_data.h"

namespace yafiyogi::values {

class Store;

} // namespace yafiyogi::values

namespace yafiyogi::actions {

class Store;

using ParamVector = yy_quad::simple_vector<values::MetricDataObsPtr>;

class Action
{
  public:
    constexpr Action() noexcept = default;
    constexpr Action(const Action &) noexcept = default;
    constexpr Action(Action &&) noexcept = default;
    virtual ~Action() noexcept = default;

    constexpr Action & operator=(const Action &) noexcept = default;
    constexpr Action & operator=(Action &&) noexcept = default;

    virtual void Run(const ParamVector & p_params,
                     ActionResultVector & p_results,
                     values::Store & p_values_store,
                     timestamp_type p_timestamp) noexcept = 0;
};

using ActionPtr = std::unique_ptr<Action>;
using ActionObsPtr = yy_data::observer_ptr<Action>;

} // namespace yafifogi::mendel::actions
