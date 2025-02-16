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

#include <cstdint>

#include <memory>
#include <string_view>

#include "spdlog/spdlog.h"

#include "yy_cpp/yy_assert.h"

#include "values_labels.h"
#include "values_metric_data.h"

#include "mqtt_handler_json.h"

namespace yafiyogi::mendel {

using namespace std::string_view_literals;

namespace json_handler_detail {

const values::Labels JsonVisitor::g_empty_labels{};
const yy_mqtt::TopicLevelsView JsonVisitor::g_empty_levels{};

JsonVisitor::JsonVisitor(size_type p_metric_count) noexcept
{
  m_metric_data.reserve(p_metric_count);
}

void JsonVisitor::labels(const values::Labels * p_labels) noexcept
{
  if(nullptr == p_labels)
  {
    p_labels = &g_empty_labels;
  }
  m_labels = p_labels;
}

void JsonVisitor::levels(const yy_mqtt::TopicLevelsView * p_levels) noexcept
{
  if(nullptr == p_levels)
  {
    p_levels = &g_empty_levels;
  }
  m_levels = p_levels;
}

void JsonVisitor::timestamp(const int64_t p_timestamp) noexcept
{
  m_timestamp = p_timestamp;
}

void JsonVisitor::apply(Metrics & p_metrics,
                        std::string_view p_value,
                        values::ValueType p_value_type)
{
  for(auto & metric : p_metrics)
  {
    metric->Event(p_value, *m_labels, *m_levels, m_metric_data, m_timestamp, p_value_type);
  }
}

values::MetricDataVector & JsonVisitor::metric_data() noexcept
{
  return m_metric_data;
}

}

MqttJsonHandler::MqttJsonHandler(std::string_view p_handler_id,
                                 const parser_options_type & p_json_options,
                                 handler_config_type && p_json_handler_config,
                                 size_type p_metric_count) noexcept:
  MqttHandler(p_handler_id, type::Json),
  m_parser(p_json_options, std::move(p_json_handler_config), p_metric_count)
{
}

values::MetricDataVector & MqttJsonHandler::Event(std::string_view p_value,
                                                  const values::Labels & p_labels,
                                                  const yy_mqtt::TopicLevelsView & p_levels,
                                                  const int64_t p_timestamp) noexcept
{
  spdlog::debug("  handler [{}]"sv, Id());

  m_parser.reset();
  auto & handler = m_parser.handler();
  handler.reset();
  auto & visitor = handler.visitor();

  visitor.reset();
  visitor.labels(&p_labels);
  visitor.levels(&p_levels);
  visitor.timestamp(p_timestamp);

  m_parser.write_some(false, p_value.data(), p_value.size(), boost::json::error_code{});

  return visitor.metric_data();
}

} // namespace yafiyogi::mendel
