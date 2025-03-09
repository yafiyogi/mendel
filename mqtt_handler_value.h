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

#pragma once

#include <string_view>

#include "mqtt_handler.h"
#include "value_type.h"
#include "values_metric.h"
#include "values_metric_data.h"

namespace yafiyogi::mendel {

class MqttValueHandler:
      public MqttHandler
{
  public:
    using MetricDataVector = values::MetricDataVector;

    explicit MqttValueHandler(std::string_view p_handler_id,
                              values::Metrics && p_metrics) noexcept;
    constexpr MqttValueHandler() noexcept = default;
    MqttValueHandler(const MqttValueHandler &) = delete;
    constexpr MqttValueHandler(MqttValueHandler &&) noexcept = default;

    MqttValueHandler & operator=(const MqttValueHandler &) = delete;
    constexpr MqttValueHandler & operator=(MqttValueHandler &&) noexcept = default;

    void Event(std::string_view p_mqtt_data,
               const std::string_view p_topic,
               const yy_mqtt::TopicLevelsView & p_levels,
               const int64_t p_timestamp,
               values::MetricDataVectorPtr p_metric_data) noexcept override;
  private:
    values::Metrics m_metrics{};
    MetricDataVector m_metric_data{};
};

} // namespace yafiyogi::mendel
