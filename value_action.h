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

#include "value_action_fwd.h"
#include "value_type.h"

namespace yafiyogi::values {

class MetricData;

class ValueAction
{
  public:
    constexpr ValueAction() noexcept = default;
    constexpr ValueAction(const ValueAction &) noexcept = default;
    constexpr ValueAction(ValueAction &&) noexcept = default;
    constexpr virtual ~ValueAction() noexcept = default;

    constexpr ValueAction & operator=(const ValueAction &) noexcept = default;
    constexpr ValueAction & operator=(ValueAction &&) noexcept = default;

    virtual void Apply(MetricData & p_metric_data,
                       ValueType p_value_type) noexcept = 0;
    virtual std::string_view Name() const noexcept = 0;
};

} // namespace yafiyogi::values
