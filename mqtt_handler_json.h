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

#include <cstddef>
#include <cstdint>

#include "boost/json/basic_parser_impl.hpp"

#include "yy_json/yy_json_pointer.h"
#include "yy_mqtt/yy_mqtt_types.h"

#include "mqtt_handler.h"
#include "value_type.h"
#include "values_metric.h"
#include "values_metric_data.h"

namespace yafiyogi::mendel {
namespace json_handler_detail {

class JsonVisitor
{
  public:
    using MetricDataVector = values::MetricDataVector;
    using Metrics = values::Metrics;

    JsonVisitor(size_type p_metric_count) noexcept;

    constexpr JsonVisitor() noexcept = default;
    constexpr JsonVisitor(const JsonVisitor &) noexcept = default;
    constexpr JsonVisitor(JsonVisitor && p_other) noexcept:
      m_labels(p_other.m_labels),
      m_levels(p_other.m_levels),
      m_timestamp(p_other.m_timestamp),
      m_metric_data(p_other.m_metric_data)
    {
      p_other.reset();
    }

    constexpr JsonVisitor & operator=(const JsonVisitor &) noexcept = default;
    constexpr JsonVisitor & operator=(JsonVisitor && p_other) noexcept
    {
      if(this != &p_other)
      {
        m_labels = p_other.m_labels;
        m_levels = p_other.m_levels;
        m_timestamp = p_other.m_timestamp;
        m_metric_data = std::move(p_other.m_metric_data);

        p_other.reset();
      }

      return *this;
    }

    void labels(const values::Labels * p_labels) noexcept;
    void levels(const yy_mqtt::TopicLevelsView * p_levels) noexcept;
    void timestamp(const int64_t p_timestamp) noexcept;
    void apply_str(Metrics & metrics,
                   std::string_view str)
    {
      apply(metrics, str, values::ValueType::String);
    }

    void apply_int64(Metrics & metrics,
                     std::string_view raw,
                     std::int64_t /* num */)
    {
      apply(metrics, raw, values::ValueType::Int);
    }

    void apply_uint64(Metrics & metrics,
                      std::string_view raw,
                      std::uint64_t /* num */)
    {
      apply(metrics, raw, values::ValueType::UInt);
    }

    void apply_double(Metrics & metrics,
                      std::string_view raw,
                      double /* num */)
    {
      apply(metrics, raw, values::ValueType::Float);
    }

    void apply_bool(Metrics & metrics,
                    bool flag)
    {
      apply(metrics, flag ? g_true_str : g_false_str, values::ValueType::Bool);
    }

    constexpr void reset() noexcept
    {
      m_labels = &g_empty_labels;
      m_levels = &g_empty_levels;
      m_metric_data.clear(yy_data::ClearAction::Keep);
      m_timestamp = 0;
    }

    MetricDataVector & metric_data() noexcept;

  private:
    void apply(Metrics & p_metrics,
               std::string_view p_data,
               values::ValueType p_value_type);

    static constexpr const std::string_view g_true_str{"true"};
    static constexpr const std::string_view g_false_str{"false"};
    static const values::Labels g_empty_labels;
    static const yy_mqtt::TopicLevelsView g_empty_levels;

    const values::Labels * m_labels = &g_empty_labels;
    const yy_mqtt::TopicLevelsView * m_levels = &g_empty_levels;
    int64_t m_timestamp = 0;
    MetricDataVector m_metric_data{};
};

} // namespace json_handler_detail

class MqttJsonHandler final:
      public MqttHandler
{
  public:
    using MetricDataVector = values::MetricDataVector;
    using builder_type = yy_json::json_pointer_builder<values::Metrics, json_handler_detail::JsonVisitor>;
    using handler_type = builder_type::handler_type;
    using handler_config_type = handler_type::pointers_config_type;
    using parser_type = boost::json::basic_parser<handler_type>;
    using parser_options_type = boost::json::parse_options;

    explicit MqttJsonHandler(std::string_view p_handler_id,
                             const parser_options_type & p_json_options,
                             handler_config_type && p_json_handler_config,
                             size_type metric_count) noexcept;

    MqttJsonHandler() = delete;
    MqttJsonHandler(const MqttJsonHandler &) = delete;
    constexpr MqttJsonHandler(MqttJsonHandler &&) noexcept = default;

    MqttJsonHandler & operator=(const MqttJsonHandler &) = delete;
    constexpr MqttJsonHandler & operator=(MqttJsonHandler &&) noexcept = default;

    MetricDataVector & Event(std::string_view p_value,
                             const values::Labels & p_labels,
                             const yy_mqtt::TopicLevelsView & p_levels,
                             const int64_t p_timestamp) noexcept override;

  private:
    parser_type m_parser;
};

} // namespace yafiyogi::mendel
