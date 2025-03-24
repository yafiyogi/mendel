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

#pragma once

#include <atomic>
#include <memory>
#include <stop_token>
#include <string>
#include <string_view>

#include "mosquittopp.h"

#include "actions_result_queue.hpp"

namespace yafiyogi::mendel {

class mqtt_config;

class mqtt_publisher final:
      public mosqpp::mosquittopp
{
  public:
    mqtt_publisher(mqtt_config & p_config,
                   actions::ActionResultQueueReader && p_queue);
    void Run(std::stop_token p_stop_token);
    void stop();
    bool is_connected() noexcept;
    void on_connect(int rc) override;
    void on_publish(int mid) override;
    void on_disconnect(int rc) override;

  private:
    size_type ProcessInQueue(size_type p_spin,
                             actions::ActionResultVector & p_data);

    static constexpr std::chrono::seconds default_keepalive{60};
    static constexpr std::chrono::seconds default_reconnect_delay{15};
    static constexpr std::chrono::milliseconds default_shutdown_spin_delay{1};
    static constexpr std::chrono::milliseconds default_disconnect_sleep{500};

    std::string m_host{};
    int m_port = yy_mqtt::mqtt_default_port;
    actions::ActionResultQueueReader m_queue{};
    int m_qos = 2;
    std::atomic<size_type> m_in_flight = 0;
    std::atomic<bool> m_is_connected = false;
    bool m_retain = true;
};

} // namespace yafiyogi::mendel
