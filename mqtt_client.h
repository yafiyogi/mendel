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

#include <atomic>
#include <memory>
#include <string>
#include <string_view>

#include <memory>

#include "mosquittopp.h"

#include "yy_mqtt/yy_mqtt_constants.h"

#include "cache_handler.hpp"
#include "values_labels.h"

#include "mqtt_topics.h"

namespace yafiyogi::mendel {

class CacheHandler;
using CacheHandlerPtr = std::shared_ptr<CacheHandler>;

class mqtt_config;

class mqtt_client final:
      public mosqpp::mosquittopp
{
  public:
    explicit mqtt_client(mqtt_config & config,
                         CacheHandlerPtr p_cache_handler);

    mqtt_client() = delete;
    mqtt_client(const mqtt_client &) = delete;
    constexpr mqtt_client(mqtt_client &&) noexcept = default;

    mqtt_client & operator=(const mqtt_client &) = delete;
    constexpr mqtt_client & operator=(mqtt_client &&) noexcept = default;


    void connect();
    bool is_connected() noexcept
    {
      return m_is_connected;
    }

    void on_connect(int rc) override;
    void on_disconnect(int rc) override;
    void on_message(const struct mosquitto_message * message) override;
    void on_subscribe(int mid,
                      int qos_count,
                      const int * granted_qos) override;

  private:
    static constexpr int default_keepalive_seconds = 60;
    static constexpr int ringbuffer_capacity = 64;

    Topics m_topics{};
    Subscriptions m_subscriptions{};
    std::string m_id{};
    std::string m_host{};
    int m_port = yy_mqtt::mqtt_default_port;
    std::atomic<bool> m_is_connected = false;
    values::Labels m_labels{};
    yy_mqtt::TopicLevelsView m_path{};
    CacheHandlerPtr m_cache_handler_ptr{};
    yy_data::observer_ptr<CacheHandler> m_cache_handler{};
    values::MetricDataVector m_metric_data{};
};

} // namespace yafiyogi::mendel
