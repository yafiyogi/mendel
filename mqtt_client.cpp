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

#include <chrono>
#include <string_view>

#include "spdlog/spdlog.h"

#include "yy_mqtt/yy_mqtt_util.h"

#include "yy_values/yy_values_labels.hpp"
#include "yy_values/yy_values_metric_labels.hpp"

#include "action.hpp"
#include "configure_mqtt.h"
#include "configure_mqtt_client.h"
#include "mqtt_handler.h"

#include "mqtt_client.h"

namespace yafiyogi::mendel {

using namespace std::string_view_literals;

mqtt_client::mqtt_client(mqtt_config & p_config,
                         mqtt_client_config & p_client_config,
                         values::MetricDataQueueWriter && p_cache_queue):
  mosqpp::mosquittopp(),
  m_topics(std::move(p_client_config.topics)),
  m_subscriptions(std::move(p_client_config.subscriptions)),
  m_host(std::move(p_config.host)),
  m_port(p_config.port),
  m_cache_queue(std::move(p_cache_queue))
{
  int mqtt_version = MQTT_PROTOCOL_V5;
  opts_set(MOSQ_OPT_PROTOCOL_VERSION, &mqtt_version);

  int nodelay_flag = 1;
  opts_set(MOSQ_OPT_TCP_NODELAY, &nodelay_flag);

  // int quickack_flag = 1;
  // opts_set(MOSQ_OPT_TCP_QUICKACK, &quickack_flag);
}

void mqtt_client::run()
{
  connect(m_host.c_str(),
          m_port,
          std::chrono::duration_cast<std::chrono::seconds>(default_keepalive_seconds).count());

  try
  {
    while(!m_stop.load(std::memory_order_acquire))
    {
      if(auto rc = loop();
         0 != rc)
      {
        std::this_thread::sleep_for(default_reconnect_delay_seconds);
        spdlog::info(" MQTT Client reconnect [{}]"sv, rc);
        reconnect();
      }
    }
  }
  catch(const std::exception & ex)
  {
    spdlog::critical("Exception caught [{}]"sv, ex.what());
  }
  catch(...)
  {
    spdlog::critical("Exception caught!"sv);
  }

  disconnect();

  while(is_connected())
  {
    std::this_thread::sleep_for(default_disconnect_sleep);
  }
}

void mqtt_client::on_connect(int rc)
{
  m_is_connected.store(true, std::memory_order_release);

  spdlog::debug("{}[{}]"sv, "MQTT Connected status="sv, rc);
  spdlog::info(" {}"sv, "Subscribing to:"sv);

  yy_quad::simple_vector<char *> subs{};
  subs.reserve(m_subscriptions.size());

  for(auto & sub : m_subscriptions)
  {
    spdlog::info(" {}[{}]"sv, "\t - "sv, sub);

    subs.emplace_back(sub.data());
  }

  mosqpp::mosquittopp::subscribe_multiple(nullptr,
                                          static_cast<int>(subs.size()),
                                          subs.data(),
                                          0,
                                          0,
                                          nullptr);
}

struct ActionData final
{
    bool operator<(const actions::ActionPtr & other) const noexcept
    {
      return action < other;
    }

    bool operator==(const actions::ActionPtr & other) const noexcept
    {
      return action == other;
    }

    actions::ActionPtr action;
    yy_values::MetricDataVector data;
};

void mqtt_client::on_message(const struct mosquitto_message * message)
{
  std::string_view topic{yy_mqtt::topic_trim(message->topic)};
  if(auto payloads = m_topics.find(topic);
     !payloads.empty())
  {
    spdlog::debug("Client Processing [{}] payloads=[{}]"sv, topic, payloads.size());
    yy_mqtt::topic_tokenize_view(m_path, topic);

    const std::string_view data{static_cast<std::string_view::value_type *>(message->payload),
                                static_cast<std::string_view::size_type>(message->payloadlen)};

    timestamp_type ts{std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now()).time_since_epoch()};

    size_type metric_count = 0;
    m_metric_data.clear(yy_data::ClearAction::Keep);

    yy_values::MetricDataVectorPtr metric_data{&m_metric_data};
    for(auto & handlers : payloads)
    {
      for(auto & handler : *handlers)
      {
        metric_count += handler->MetricCount();
        m_metric_data.reserve(metric_count);

        handler->Event(data, topic, m_path, ts, metric_data);
      }
    }

    m_cache_queue.QSwapIn(m_metric_data);
  }
}

void mqtt_client::stop()
{
  m_stop.store(true, std::memory_order_release);
}

bool mqtt_client::is_connected() noexcept
{
  return m_is_connected.load(std::memory_order_acquire);
}

void mqtt_client::on_subscribe(int /* mid */,
                               int /* qos_count */,
                               const int * /* granted_qos */)
{
  spdlog::info(" MQTT Client Subscribed."sv);
}

void mqtt_client::on_disconnect(int rc)
{
  spdlog::warn(" {}[{}]"sv, "MQTT Client Disconnected status="sv, rc);

  m_is_connected.store(false, std::memory_order_release);
}

} // namespace yafiyogi::mendel
