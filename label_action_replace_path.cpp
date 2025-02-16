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

#include <cstddef>

#include <string>
#include <memory>

#include "yy_cpp/yy_variant_util.h"
#include "values_labels.h"
#include "label_action.h"

#include "replacement_format.h"

#include "label_action_replace_path.h"

namespace yafiyogi::values {

ReplacePathLabelAction::ReplacePathLabelAction(std::string && p_label_name,
                                               ReplacementTopics && p_topics) noexcept:
  m_label_name(std::move(p_label_name)),
  m_label_value(),
  m_topics(std::move(p_topics))
{
}

void ReplacePathLabelAction::Apply(const Labels & /* labels */,
                                   const yy_mqtt::TopicLevelsView & p_levels,
                                   Labels & metric_labels) noexcept
{
  if(auto payloads = m_topics.find(metric_labels.get_label(g_label_topic));
     !payloads.empty())
  {
    for(const auto * replacements : payloads)
    {
      for(const auto & format : *replacements)
      {
        m_label_value.clear();

        auto format_fn = [&p_levels, this](const auto & formatter)
        {
          formatter(p_levels, m_label_value);
        };

        for(const auto & fmt_elem : format)
        {
          std::visit(format_fn,
                     fmt_elem);
        }

        metric_labels.set_label(m_label_name, m_label_value);
      }
    }
  }
}


} // namespace yafiyogi::values
