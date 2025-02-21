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

#include "fmt/format.h"
#include "fmt/compile.h"
#include "spdlog/spdlog.h"

#include "yy_cpp/yy_flat_set.h"
#include "yy_cpp/yy_make_lookup.h"
#include "yy_cpp/yy_string_case.h"
#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_yaml_util.h"

#include "mqtt_handler.h"
#include "mqtt_handler_json.h"
#include "mqtt_handler_value.h"
#include "values_config.h"

#include "configure_mqtt_handlers.h"

namespace yafiyogi::mendel {

using namespace std::string_view_literals;
using namespace fmt::literals;

namespace {

const boost::json::parse_options g_json_options{ .numbers = boost::json::number_precision::none};

constexpr auto handler_types =
  yy_data::make_lookup<std::string_view, MqttHandler::type>(MqttHandler::type::Json,
                                                            {{"json"sv, MqttHandler::type::Json},
                                                             {"text"sv, MqttHandler::type::Text},
                                                             {"value"sv, MqttHandler::type::Value}});

MqttHandler::type decode_type(const YAML::Node & yaml_type)
{

  std::string type_name{yy_util::to_lower(yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_type)))};

  return handler_types.lookup(type_name);
}

MqttHandlerPtr configure_json_handler(std::string_view p_id,
                                      const YAML::Node & yaml_json_handler,
                                      values::MetricsMap & values_metrics)
{
  MqttHandlerPtr mqtt_json_handler{};
  auto yaml_properties = yaml_json_handler["properties"sv];
  if(yaml_properties && (0 != yaml_properties.size()))
  {
    MqttJsonHandler::builder_type json_pointer_builder{};
    int metrics_count = 0;
    std::string json_pointer{};
    std::string_view property{};

    auto do_add_property = [&property, &json_pointer, &json_pointer_builder, &metrics_count]
                           (values::Metrics * visitor_values_metrics, auto /* pos */) {
      if(nullptr != visitor_values_metrics)
      {
        auto [builder_metrics, added] = json_pointer_builder.add_pointer(json_pointer,
                                                                         values::Metrics{});
        if(nullptr != builder_metrics)
        {
          for(auto & metric : *visitor_values_metrics)
          {
            if(metric && (metric->Property() == property))
            {
              ++metrics_count;
              spdlog::info("       metric [{}] added."sv,
                           metric->Id().Id());
              builder_metrics->emplace_back(std::move(metric));
            }
          }
        }
      }
    };

    yy_data::flat_set<std::string_view> properties{};
    properties.reserve(yaml_properties.size());
    for(const auto & yaml_property : yaml_properties)
    {
      if(yaml_property.IsScalar())
      {
        property = yy_util::trim(yaml_property.as<std::string_view>());
        json_pointer = fmt::format("/{}"_cf, property);
      }
      else
      {
        property = yy_util::trim(yaml_property["id"sv].as<std::string_view>());
        json_pointer = yy_json::json_pointer_trim(yaml_property["json"sv].as<std::string_view>());
      }
      spdlog::info("     - property [{}] path=[{}]:"sv,
                   property,
                   json_pointer);
      spdlog::trace("        [line {}]."sv,
                    yaml_property.Mark().line + 1);

      if(!json_pointer.empty()
         && !property.empty())
      {
        // Avoid duplicates.
        if(auto [ignore, inserted] = properties.emplace(property);
           inserted)
        {
          std::ignore = values_metrics.find_value(do_add_property, p_id);
        }
        else
        {
          spdlog::warn("   * already added!"sv);
        }
      }
    }

    if(metrics_count > 0)
    {
      auto create_json_pointer_config = [&json_pointer_builder]() {
        return json_pointer_builder.create(g_json_options.max_depth);
      };

      mqtt_json_handler = std::make_unique<MqttJsonHandler>(p_id,
                                                            g_json_options,
                                                            create_json_pointer_config(),
                                                            metrics_count);
    }
  }

  return mqtt_json_handler;
}

MqttHandlerPtr configure_text_handler(std::string_view /* p_id */,
                                      const YAML::Node & /* yaml_text_handler */,
                                      values::MetricsMap & /* values_metrics */)
{
  return MqttHandlerPtr{};
}

MqttHandlerPtr configure_value_handler(std::string_view p_id,
                                       const YAML::Node & /* yaml_value_handler */,
                                       values::MetricsMap & values_metrics)
{
  values::Metrics handler_metrics{} ;
  auto do_add_property = [&handler_metrics]
                         (values::Metrics * visitor_values_metrics, auto /* pos */) {
    if(nullptr != visitor_values_metrics)
    {
      handler_metrics.reserve(visitor_values_metrics->size());

      for(auto & metric : *visitor_values_metrics)
      {
        if(metric)
        {
          spdlog::info("       metric [{}] added."sv,
                       metric->Id().Id());
          handler_metrics.emplace_back(std::move(metric));
        }
      }
    }
  };

  std::ignore = values_metrics.find_value(do_add_property, p_id);

  MqttHandlerPtr handler{};
  if(!handler_metrics.empty())
  {
    handler = std::make_unique<MqttValueHandler>(p_id, std::move(handler_metrics));
  }
  return handler;
}

} // anonymous namespace

MqttHandlerStore configure_mqtt_handlers(const YAML::Node & yaml_handlers,
                                         values::MetricsMap & values_config)
{
  MqttHandlerStore handler_store{};

  if(!yaml_handlers.IsSequence())
  {
    spdlog::error("Configuration error: expecting a sequence of handlers!"sv);
  }
  else
  {
    handler_store.reserve(yaml_handlers.size());

    for(const auto & yaml_handler: yaml_handlers)
    {
      std::string_view l_id{yy_util::trim(yaml_handler["id"sv].as<std::string_view>())};
      const MqttHandler::type type = decode_type(yaml_handler["type"sv]);

      spdlog::info(" Configuring MQTT Handler id [{}]:"sv, l_id);
      spdlog::trace("  [line {}]."sv, yaml_handler.Mark().line + 1);
      MqttHandlerPtr handler;

      spdlog::info("   - type [{}]"sv, yaml_handler["type"sv].as<std::string_view>());
      switch(type)
      {
        case MqttHandler::type::Json:
          handler = configure_json_handler(l_id, yaml_handler, values_config);
          break;

        case MqttHandler::type::Text:
          handler = configure_text_handler(l_id, yaml_handler, values_config);
          break;

        case MqttHandler::type::Value:
          handler = configure_value_handler(l_id, yaml_handler, values_config);
          break;
      }

      if(handler)
      {
        if(const auto & id = handler->Id();
           !id.empty())
        {
          if(auto [handler_pos, emplaced] = handler_store.emplace(std::move(id), std::move(handler));
             !emplaced)
          {
            if(auto [key, added] = handler_store[handler_pos];
               !added)
            {
              spdlog::trace("Handler id [{}] already created. Ignoring [line {}]"sv,
                            key,
                            yaml_handler.Mark().line + 1);
            }
          }
        }
      }
      else
      {
        spdlog::warn("MQTT Handler id [{}] not created!"sv, l_id);
      }
    }
  }

  return handler_store;
}

} // namespace yafiyogi::mendel
