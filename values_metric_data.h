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

#include <cstdint>

#include <string>

#include "yy_cpp/yy_vector.h"

#include "value_type.h"
#include "values_metric_id.h"
#include "values_labels.h"

namespace yafiyogi::values {

class MetricData final
{
  public:
    MetricData(MetricId && p_id) noexcept;

    constexpr MetricData() noexcept = default;
    constexpr MetricData(const MetricData &) noexcept = default;
    constexpr MetricData(MetricData && p_other) noexcept:
      m_id(std::move(p_other.m_id)),
      m_timestamp(p_other.m_timestamp),
      m_value(std::move(p_other.m_value))
    {
      p_other.m_timestamp = 0;
    }

    constexpr MetricData & operator=(const MetricData &) noexcept = default;
    constexpr MetricData & operator=(MetricData && p_other) noexcept
    {
      if(this != &p_other)
      {
        m_id = std::move(p_other.m_id);
        m_timestamp = p_other.m_timestamp;
        p_other.m_timestamp = 0;
        m_value = std::move(p_other.m_value);
      }
      return *this;
    }

    constexpr bool operator<(const MetricData & other) const noexcept
    {
      return m_id < other.m_id;
    }

    constexpr bool operator==(const MetricData & other) const noexcept
    {
      return m_id == other.m_id;
    }

    constexpr MetricId & Id() noexcept
    {
      return m_id;
    }

    constexpr const MetricId & Id() const noexcept
    {
      return m_id;
    }

    constexpr values::Labels & Labels() noexcept
    {
      return m_id.Labels();
    }

    constexpr const values::Labels & Labels() const noexcept
    {
      return m_id.Labels();
    }

    constexpr void Labels(const values::Labels & p_labels) noexcept
    {
      m_id.Labels(p_labels);
    }

    constexpr int64_t Timestamp() const noexcept
    {
      return m_timestamp;
    }

    constexpr void Timestamp(int64_t p_timestamp) noexcept
    {
      m_timestamp = p_timestamp;
    }

    constexpr const std::string_view Value() const noexcept
    {
      return m_value;
    }

    constexpr void Value(std::string_view p_value) noexcept
    {
      m_value = p_value;
    }

    constexpr ValueType Type() const noexcept
    {
      return m_value_type;
    }

    constexpr void Type(ValueType p_value_type) noexcept
    {
      m_value_type = p_value_type;
    }

  private:
    MetricId m_id;
    int64_t m_timestamp = 0;
    std::string m_value{};
    ValueType m_value_type = ValueType::Unknown;
};

using MetricDataVector = yy_quad::simple_vector<MetricData>;

} // namespace yafiyogi::values
