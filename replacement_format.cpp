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

#include "fmt/format.h"
#include "fmt/compile.h"

#include "replacement_format.h"

namespace yafiyogi::values {

using namespace fmt::literals;

static constexpr auto level_format{"{}{}"_cf};
void FormatLevel::operator()(const yy_mqtt::TopicLevelsView & p_path,
                             std::string & p_label_value) const noexcept
{
  if(m_idx < p_path.size())
  {
    const auto & level = p_path[m_idx];

    p_label_value.reserve(p_label_value.size() + m_prefix.size() + level.size());
    fmt::format_to(std::back_inserter(p_label_value), level_format, m_prefix, level);
  }
}

void FormatPrefix::operator()(const yy_mqtt::TopicLevelsView & /* p_path */,
                              std::string & p_label_value) const noexcept
{
  p_label_value.reserve(p_label_value.size() + m_prefix.size());
  p_label_value.append(m_prefix);
}

} // namespace yafiyogi::values
