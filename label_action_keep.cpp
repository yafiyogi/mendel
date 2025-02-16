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
#include <memory>

#include "values_labels.h"
#include "label_action_keep.h"

namespace yafiyogi::values {

KeepLabelAction::KeepLabelAction(std::string && p_label) noexcept:
  m_label(std::move(p_label))
{
}

void KeepLabelAction::Apply(const Labels & labels,
                            const yy_mqtt::TopicLevelsView & /* p_levels */,
                            Labels & metric_labels) noexcept
{
  auto do_keep_label = [this, &metric_labels](const std::string * label_value, auto) {
    metric_labels.set_label(m_label, *label_value);
  };

  std::ignore = labels.get_label(m_label, do_keep_label);
}


} // namespace yafiyogi::values
