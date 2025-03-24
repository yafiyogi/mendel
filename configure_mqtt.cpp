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

#include <string>

#include "spdlog/spdlog.h"

#include "yy_cpp/yy_make_lookup.h"
#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_string_case.h"
#include "yy_cpp/yy_yaml_util.h"

#include "configure_mqtt.h"

namespace yafiyogi::mendel {

using namespace std::string_literals;
using namespace std::string_view_literals;

constexpr auto bool_types =
  yy_data::make_lookup<std::string_view, bool>(true,
                                               {{"yes"sv, true},
                                                {"true"sv, true},
                                                {"on"sv, true},
                                                {"enable"sv, true},
                                                {"no"sv, false},
                                                {"false"sv, false},
                                                {"off"sv, false},
                                                {"disable"sv, false}});

void configure_mqtt(const YAML::Node & yaml_mqtt,
                    mqtt_config & config)
{
  const auto & yaml_host = yaml_mqtt["host"sv];
  if(!yaml_host)
  {
    spdlog::error("Not found mqtt host\n"sv);
  }

  config.host = std::string{yaml_host.as<std::string_view>()};
  config.port = yy_util::yaml_get_value(yaml_mqtt["port"sv], yy_mqtt::mqtt_default_port);
  config.qos = std::max(0, std::min(2, yy_util::yaml_get_value(yaml_mqtt["qos"sv], 0)));
  auto retain{yy_util::to_lower(yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_mqtt["retain"sv], "yes")))};
  config.retain = bool_types.lookup(retain);

  spdlog::info("  host  : [{}]"sv, config.host);
  spdlog::info("  port  : [{}]"sv, config.port);
  spdlog::info("  qos   : [{}]"sv, config.qos);
  spdlog::info("  retain: [{}]"sv, retain);
}

mqtt_config configure_mqtt(const YAML::Node & yaml_mqtt)
{
  mqtt_config config{};

  configure_mqtt(yaml_mqtt, config);

  return config;
}

} // namespace yafiyogi::mendel
