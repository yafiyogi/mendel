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

#include <memory>
#include <string>

#include "fmt/format.h"

#include "yy_cpp/yy_variant_util.h"

#include "label_action.h"
#include "values_labels.h"
#include "values_metric_labels.hpp"

#include "replacement_format.h"

#include "label_action_replace_path.h"

namespace yafiyogi::values {

ReplacePathLabelAction::ReplacePathLabelAction(std::string && p_label_name,
                                               ReplacementTopics && p_topics) noexcept:
  m_label_name(std::move(p_label_name)),
  m_topics(std::move(p_topics))
{
}

void ReplacePathLabelAction::Apply(const Labels & p_labels_in,
                                   const yy_mqtt::TopicLevelsView & p_levels_in,
                                   Labels & p_labels_out) noexcept
{
  Apply(p_labels_in, p_levels_in, p_labels_out.set_label(m_label_name, std::string_view{}));
}

void ReplacePathLabelAction::Apply(const Labels & p_labels_in,
                                   const yy_mqtt::TopicLevelsView & p_levels_in,
                                   std::string & p_label_out) noexcept
{
  if(auto payloads = m_topics.find(p_labels_in.get_label(g_label_topic));
     !payloads.empty())
  {
    const auto & topic_format = *payloads[0];

    p_label_out.clear();

    auto format_fn = [&p_levels_in, &p_label_out](const auto & formatter) {
      formatter(p_levels_in, p_label_out);
    };

    for(const auto & format_element : topic_format)
    {
      std::visit(format_fn,
                 format_element);
    }
  }
}



} // namespace yafiyogi::values
