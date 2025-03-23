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

#include "values_labels.h"
#include "values_metric_data_queue.hpp"
#include "mqtt_topics.h"

namespace yafiyogi::mendel {

class CacheHandler;
using CacheHandlerPtr = std::shared_ptr<CacheHandler>;

class mqtt_config;
class mqtt_client_config;

class mqtt_client final:
      public mosqpp::mosquittopp
{
  public:
    explicit mqtt_client(mqtt_config & config,
                         mqtt_client_config & p_client_config,
                         values::MetricDataQueueWriter && p_cache_queue);

    mqtt_client() = delete;
    mqtt_client(const mqtt_client &) = delete;
    constexpr mqtt_client(mqtt_client &&) noexcept = default;

    mqtt_client & operator=(const mqtt_client &) = delete;
    constexpr mqtt_client & operator=(mqtt_client &&) noexcept = default;

    void run();
    void stop();
    bool is_connected() noexcept;
    void on_connect(int rc) override;
    void on_disconnect(int rc) override;
    void on_message(const struct mosquitto_message * message) override;
    void on_subscribe(int mid,
                      int qos_count,
                      const int * granted_qos) override;

  private:
    static constexpr std::chrono::seconds default_keepalive_seconds{60};
    static constexpr std::chrono::seconds default_reconnect_delay_seconds{15};
    static constexpr std::chrono::milliseconds default_disconnect_sleep{500};

    Topics m_topics{};
    Subscriptions m_subscriptions{};
    std::string m_host{};
    int m_port = yy_mqtt::mqtt_default_port;
    yy_mqtt::TopicLevelsView m_path{};
    values::MetricDataVector m_metric_data{};
    values::MetricDataQueueWriter m_cache_queue{};
    std::atomic<bool> m_is_connected = false;
    std::atomic<bool> m_stop = false;
};

} // namespace yafiyogi::mendel
