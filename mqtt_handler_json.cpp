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
#include <string_view>

#include "spdlog/spdlog.h"

#include "yy_cpp/yy_assert.h"

#include "yy_values/yy_values_labels.hpp"
#include "yy_values/yy_values_metric_data.hpp"

#include "mqtt_handler_json.h"

namespace yafiyogi::mendel {

using namespace std::string_view_literals;

namespace json_handler_detail {

const yy_mqtt::TopicLevelsView JsonVisitor::g_empty_levels{};

JsonVisitor::JsonVisitor(JsonVisitor && p_other) noexcept:
  m_levels(std::move(p_other.m_levels)),
  m_metric_data(std::move(p_other.m_metric_data)),
  m_timestamp(p_other.m_timestamp),
  m_topic(p_other.m_topic)
{
  p_other.reset();
}

JsonVisitor & JsonVisitor::operator=(JsonVisitor && p_other) noexcept
{
  if(this != &p_other)
  {
    m_levels = p_other.m_levels;
    m_metric_data = std::move(p_other.m_metric_data);
    m_timestamp = p_other.m_timestamp;
    m_topic = p_other.m_topic;

    p_other.reset();
  }

  return *this;
}

void JsonVisitor::levels(const yy_mqtt::TopicLevelsView * p_levels) noexcept
{
  if(nullptr == p_levels)
  {
    p_levels = &g_empty_levels;
  }
  m_levels = p_levels;
}

void JsonVisitor::metric_data(yy_values::MetricDataVectorPtr p_metric_data) noexcept
{
  m_metric_data = p_metric_data;
}

void JsonVisitor::topic(const std::string_view p_topic) noexcept
{
  m_topic = p_topic;
}

void JsonVisitor::timestamp(const timestamp_type p_timestamp) noexcept
{
  m_timestamp = p_timestamp;
}

void JsonVisitor::reset() noexcept
{
  m_levels = &g_empty_levels;
  m_metric_data.release();
  m_timestamp = timestamp_type{};
  m_topic = std::string_view{};
}

void JsonVisitor::apply(Metrics & p_metrics,
                        std::string_view p_value,
                        yy_values::ValueType p_value_type)
{
  for(auto & metric : p_metrics)
  {
    metric->Event(p_value,
                  m_topic,
                  *m_levels,
                  m_timestamp,
                  p_value_type,
                  m_metric_data);
  }
}

}

MqttJsonHandler::MqttJsonHandler(std::string_view p_handler_id,
                                 const parser_options_type & p_json_options,
                                 handler_config_type && p_json_handler_config,
                                 size_type p_metric_count) noexcept:
  MqttHandler(p_handler_id, type::Json, p_metric_count),
  m_parser(p_json_options, std::move(p_json_handler_config))
{
}

void MqttJsonHandler::Event(std::string_view p_mqtt_data,
                            const std::string_view p_topic,
                            const yy_mqtt::TopicLevelsView & p_levels,
                            const timestamp_type p_timestamp,
                            yy_values::MetricDataVectorPtr p_metric_data) noexcept
{
  spdlog::debug("  handler [{}]"sv, Id());

  m_parser.reset();
  auto & handler = m_parser.handler();
  handler.reset();
  auto & visitor = handler.visitor();

  visitor.reset();
  visitor.levels(&p_levels);
  visitor.metric_data(p_metric_data);
  visitor.timestamp(p_timestamp);
  visitor.topic(p_topic);

  m_parser.write_some(false,
                      p_mqtt_data.data(),
                      p_mqtt_data.size(),
                      boost::json::error_code{});
}

} // namespace yafiyogi::mendel
