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
#include <string_view>

#include "spdlog/spdlog.h"

#include "yy_cpp/yy_clear_action.h"

#include "values_metric_id_fmt.hpp"

#include "label_action.h"
#include "values_labels.h"
#include "values_metric_labels.hpp"

#include "values_metric.h"

namespace yafiyogi::values {

using namespace std::string_view_literals;

Metric::Metric(MetricId && p_id,
               std::string && p_property,
               LabelActions && p_label_actions,
               ValueActions && p_value_actions,
               LabelActions && p_metric_property_actions):
  m_metric_data(std::move(p_id)),
  m_property(std::move(p_property)),
  m_label_actions(std::move(p_label_actions)),
  m_value_actions(std::move(p_value_actions)),
  m_metric_property_actions(std::move(p_metric_property_actions)),
  m_metric_properties(m_metric_property_actions.size())
{
}

const MetricId & Metric::Id() const noexcept
{
  return m_metric_data.Id();
}

const std::string & Metric::Property() const noexcept
{
  return m_property;
}

void Metric::Event(std::string_view p_value,
                   const std::string_view p_topic,
                   const yy_mqtt::TopicLevelsView & p_levels,
                   const int64_t p_timestamp,
                   ValueType p_value_type,
                   values::MetricDataVectorPtr p_metric_data)
{
  spdlog::debug("    [{}] property=[{}] [{}]"sv,
                Id().Id(),
                m_property,
                p_value);

  m_metric_data.Value(p_value);
  m_metric_data.Type(p_value_type);
  m_metric_data.Timestamp(p_timestamp);

  m_metric_properties.clear(yy_data::ClearAction::Keep);
  m_metric_properties.set_label(g_label_topic, std::string{p_topic});

  for(const auto & action : m_metric_property_actions)
  {
    action->Apply(m_metric_properties, p_levels, m_metric_properties);
  }

  m_metric_data.Id().Location(m_metric_properties.get_label(g_label_location));

  for(auto & l_labels = m_metric_data.Labels();
      const auto & action : m_label_actions)
  {
    action->Apply(m_metric_properties, p_levels, l_labels);
  }

  for(const auto & action : m_value_actions)
  {
    action->Apply(m_metric_data, p_value_type);
  }

  if(spdlog::level::debug == spdlog::get_level())
  {
    m_metric_data.Labels().visit([](const auto & label,
                                    const auto & value) {
      spdlog::debug("      - [{}]:[{}]"sv, label, value);
    });
  }

  p_metric_data->emplace_back(m_metric_data);
}

} // namespace yafiyogi::values
