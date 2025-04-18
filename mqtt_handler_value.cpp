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

#include <string_view>

#include "spdlog/spdlog.h"

#include "yy_values/yy_values_metric.hpp"
#include "yy_values/yy_values_metric_data.hpp"

#include "mqtt_handler_value.h"

namespace yafiyogi::mendel {

using namespace std::string_view_literals;

MqttValueHandler::MqttValueHandler(std::string_view p_handler_id,
                                   yy_values::Metrics && p_metrics,
                                   size_type p_metrics_count) noexcept:
  MqttHandler(p_handler_id, type::Value, p_metrics_count),
  m_metrics(std::move(p_metrics))
{
}

void MqttValueHandler::Event(std::string_view p_mqtt_data,
                             const std::string_view p_topic,
                             const yy_mqtt::TopicLevelsView & p_levels,
                             const timestamp_type p_timestamp,
                             yy_values::MetricDataVectorPtr p_metric_data) noexcept
{
  spdlog::debug("  handler [{}]"sv, Id());

  for(auto & metric : m_metrics)
  {
    metric->Event(p_mqtt_data,
                  p_topic,
                  p_levels,
                  p_timestamp,
                  yy_values::ValueType::Unknown,
                  p_metric_data);
  }
}

} // namespace yafiyogi::mendel
