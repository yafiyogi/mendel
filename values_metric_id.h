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

#pragma once

#include <string>

namespace yafiyogi::values {

class MetricId final
{
  public:
    explicit MetricId(std::string_view p_metric_id) noexcept;
    MetricId(std::string_view p_id,
             std::string_view p_location) noexcept;

    constexpr MetricId() noexcept = default;
    constexpr MetricId(const MetricId &) noexcept = default;
    constexpr MetricId(MetricId &&) noexcept = default;

    constexpr MetricId & operator=(const MetricId &) noexcept = default;
    constexpr MetricId & operator=(MetricId &&) noexcept = default;

    constexpr bool operator<(const MetricId & other) const noexcept
    {
      return std::tie(m_id, m_location) < std::tie(other.m_id, other.m_location);
    }

    constexpr bool operator==(const MetricId & other) const noexcept
    {
      return std::tie(m_id, m_location) == std::tie(other.m_id, other.m_location);
    }

    constexpr int compare(const MetricId & other) const noexcept
    {
      if(int comp = m_id.compare(other.m_id);
         0 != comp)
      {
        return comp;
      }

      return m_location.compare(other.m_location);
    }

    constexpr const std::string & Id() const noexcept
    {
      return m_id;
    }

    constexpr const std::string & Location() const noexcept
    {
      return m_location;
    }

    constexpr void Location(const std::string & p_location) noexcept
    {
      m_location = p_location;
    }

    constexpr void Location(std::string && p_location) noexcept
    {
      m_location = std::move(p_location);
    }

  private:
    std::string m_id{};
    std::string m_location{};
};

} // namespace yafiyogi::values
