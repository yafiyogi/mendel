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

#include <string>

#include "yy_mqtt/yy_mqtt_util.h"

#include "values_labels.h"

namespace yafiyogi::values {

const std::string g_empty_str{};

Labels::Labels(const std::string & p_sort_order) noexcept:
  m_sort_order(p_sort_order)
{
}

Labels::Labels(std::string && p_sort_order) noexcept:
  m_sort_order(std::move(p_sort_order))
{
}

void Labels::clear() noexcept
{
  m_labels.clear();
}

void Labels::set_label(std::string_view p_label,
                       std::string_view p_value)
{
  m_labels.emplace_or_assign(std::string{p_label},
                             std::string{p_value});
}

void Labels::set_sort_order(const std::string & p_sort_order) noexcept
{
  m_sort_order = p_sort_order;
}

void Labels::set_sort_order(std::string && p_sort_order) noexcept
{
  m_sort_order = std::move(p_sort_order);
}

const std::string & Labels::get_label(const std::string_view p_label) const noexcept
{
  const std::string * label = &g_empty_str;

  auto do_get_value = [&label](const std::string * p_visitor_label, auto) {
    if(nullptr != p_visitor_label)
    {
      label = p_visitor_label;
    }
  };

  std::ignore = get_label(p_label, do_get_value);

  return *label;
}

void Labels::erase(const std::string_view p_label)
{
  m_labels.erase(p_label);
}

bool Labels::operator<(const Labels & other) const noexcept
{
  if(m_sort_order.empty())
  {
    return m_labels < other.m_labels;
  }

  return get_label(m_sort_order).compare(other.get_label(m_sort_order)) < 0;
}

bool Labels::operator==(const Labels & other) const noexcept
{
  if(m_sort_order.empty())
  {
    return m_labels == other.m_labels;
  }

  return get_label(m_sort_order).compare(other.get_label(m_sort_order)) == 0;
}

} // namespace yafiyogi::values
