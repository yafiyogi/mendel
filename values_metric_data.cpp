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

#include "values_metric_data.h"

namespace yafiyogi::values {

MetricData::MetricData(const MetricId & p_id) noexcept:
  m_id(std::move(p_id))
{
}

void MetricData::swap(MetricData & other) noexcept
{
  if(this != &other)
  {
    std::swap(m_id, other.m_id);
    std::swap(m_labels, other.m_labels);
    std::swap(m_timestamp, other.m_timestamp);
    std::swap(m_value, other.m_value);
    std::swap(m_binary, other.m_binary);
    std::swap(m_value_type, other.m_value_type);
  }
}

} // namespace yafiyogi::values
