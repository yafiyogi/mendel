/*

  MIT License

  Copyright (c) 2025 Yafiyogi

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

#include "spdlog/spdlog.h"

#include "configure_mqtt.h"

#include "mqtt_publisher.hpp"

namespace yafiyogi::mendel {

using namespace std::string_view_literals;

namespace {

constexpr size_type spin_max = 400;

}

mqtt_publisher::mqtt_publisher(mqtt_config & p_config,
                               actions::ActionResultQueueReader && p_queue):
  m_host(std::move(p_config.host)),
  m_port(p_config.port),
  m_queue(std::move(p_queue)),
  m_qos(p_config.qos),
  m_retain(p_config.retain)
{
  int mqtt_version = MQTT_PROTOCOL_V5;
  opts_set(MOSQ_OPT_PROTOCOL_VERSION, &mqtt_version);

  int nodelay_flag = 1;
  opts_set(MOSQ_OPT_TCP_NODELAY, &nodelay_flag);

  // int quickack_flag = 1;
  // opts_set(MOSQ_OPT_TCP_QUICKACK, &quickack_flag);
}

size_type mqtt_publisher::ProcessInQueue(size_type spin,
                                         actions::ActionResultVector & p_queue_data)
{
  p_queue_data.clear(yy_data::ClearAction::Keep);
  while(m_queue.QSwapOut(p_queue_data))
  {
    spin = spin_max; // Reset spin count.

    int mid = 0;
    for(const auto & value : p_queue_data)
    {
      m_in_flight.fetch_add(size_type{1});
      spdlog::debug("Publisher: topic=[{}] payload=[{}]",
                    value.topic,
                    value.data);

      if(auto rv = publish(&mid,
                           value.topic.c_str(),
                           static_cast<int>(value.data.size()),
                           value.data.c_str(),
                           m_qos,
                           m_retain);
         MOSQ_ERR_SUCCESS != rv)
      {
        spdlog::warn("mqtt_publisher::ProcessInQueue(): 4 rv=[{}] msg=[{}]",
                     rv,
                     mosquitto_strerror(rv));
      }
    }
  }

  return spin;
}

void mqtt_publisher::Run(std::stop_token p_stop_token)
{
  reconnect_delay_set(std::chrono::duration_cast<std::chrono::seconds>(default_reconnect_delay).count(),
                      std::chrono::duration_cast<std::chrono::seconds>(default_reconnect_delay).count(),
                      false);
  connect(m_host.c_str(),
          m_port,
          std::chrono::duration_cast<std::chrono::seconds>(default_keepalive).count());

  loop_start();

  actions::ActionResultVector l_queue_data{};
  size_type spin = 1; // Set to 1 to prevent spinning at startup.

  while(!p_stop_token.stop_requested())
  {
    spin = ProcessInQueue(spin, l_queue_data);

    if(0 == --spin)
    {
      spin = spin_max; // Reset spin count.
      m_queue.QWait(p_stop_token, [this] { return !m_queue.QEmpty();});
    }
  }

  ProcessInQueue(spin_max, l_queue_data);

  spin = spin_max;
  while((--spin > 0) && m_in_flight.load(std::memory_order_acquire) > 0)
  {
    std::this_thread::sleep_for(default_shutdown_spin_delay);
  }

  disconnect();

  loop_stop();

  while(is_connected())
  {
    std::this_thread::sleep_for(default_disconnect_sleep);
  }
}

bool mqtt_publisher::is_connected() noexcept
{
  return m_is_connected.load(std::memory_order_acquire);
}

void mqtt_publisher::on_connect(int rc)
{
  m_is_connected.store(true, std::memory_order_release);

  spdlog::debug("{}[{}]"sv, "MQTT Publisher Connected status="sv, rc);
}

void mqtt_publisher::on_publish(int /* mid */)
{
  m_in_flight.fetch_sub(size_type{1});
}

void mqtt_publisher::on_disconnect(int rc)
{
  m_is_connected.store(false, std::memory_order_release);

  spdlog::info("{}[{}]"sv, "MQTT Disconnected status="sv, rc);
}

} // namespace yafiyogi::mendel
