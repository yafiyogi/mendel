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
#include <string_view>

#include "spdlog/spdlog.h"

#include "yy_cpp/yy_flat_set.h"
#include "yy_cpp/yy_make_lookup.h"
#include "yy_cpp/yy_string_case.h"
#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_type_traits.h"
#include "yy_cpp/yy_utility.h"
#include "yy_cpp/yy_vector_util.h"
#include "yy_cpp/yy_yaml_util.h"

#include "yy_prometheus/yy_prometheus_configure.h"

#include "configure_label_actions.h"
#include "configure_values.h"
#include "label_action.h"
#include "label_action_replace_path.h"
#include "mqtt_handler.h"
#include "values_config.h"
#include "values_metric.h"

#include "label_action_copy.h"
#include "label_action_drop.h"
#include "label_action_keep.h"
#include "label_action_replace_path.h"

#include "value_action_keep.h"
#include "value_action_switch.h"

namespace yafiyogi::values {

using namespace std::string_view_literals;

namespace {

enum class LabelActionType {Copy, Drop, Keep, ReplacePath};

constexpr const auto g_label_action_types =
  yy_data::make_lookup<std::string_view, LabelActionType>(LabelActionType::Keep,
                                                          {{CopyLabelAction::action_name, LabelActionType::Copy},
                                                           {DropLabelAction::action_name, LabelActionType::Drop},
                                                           {KeepLabelAction::action_name, LabelActionType::Keep},
                                                           {ReplacePathLabelAction::action_name, LabelActionType::ReplacePath}});

enum class ValueActionType {Keep, Switch};

constexpr const auto g_value_action_types =
  yy_data::make_lookup<std::string_view, ValueActionType>({{KeepValueAction::action_name, ValueActionType::Keep},
                                                           {SwitchValueAction::action_name, ValueActionType::Switch}});

LabelActions configure_label_actions(const YAML::Node & yaml_label_actions)
{
  LabelActions label_actions;
  label_actions.reserve(yaml_label_actions.size());

  for(const auto & yaml_label_action : yaml_label_actions)
  {
    auto action_name{yy_util::to_lower(yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_label_action["action"sv])))};
    LabelActionPtr action;

    spdlog::info("       - action [{}]."sv, action_name);
    spdlog::trace("          [line {}]."sv, yaml_label_action.Mark().line + 1);
    switch(g_label_action_types.lookup(action_name))
    {
      case LabelActionType::Copy:
      {
        std::string_view source{yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_label_action["source"sv]))};
        std::string_view target{yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_label_action["target"sv]))};
        if(!source.empty()
           || !target.empty())
        {
          action = yy_util::static_unique_cast<LabelAction>(std::make_unique<CopyLabelAction>(std::string{source},
                                                                                              std::string{target}));
        }
      }
      break;

      case LabelActionType::Drop:
      {
        std::string_view target{yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_label_action["target"sv]))};
        if(!target.empty())
        {
          action = yy_util::static_unique_cast<LabelAction>(std::make_unique<DropLabelAction>(std::string{target}));
        }
      }
      break;

      case LabelActionType::Keep:
      {
        std::string_view target{yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_label_action["target"sv]))};
        if(!target.empty())
        {
          action = yy_util::static_unique_cast<LabelAction>(std::make_unique<KeepLabelAction>(std::string{target}));
        }
      }
      break;

      case LabelActionType::ReplacePath:
      {
        std::string_view target{yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_label_action["target"sv]))};

        if(!target.empty())
        {
          auto create_topics = [&yaml_label_action]() {
            return configure_label_action_replace_path(yaml_label_action["replace"sv]);
          };

          action = yy_util::static_unique_cast<LabelAction>(std::make_unique<ReplacePathLabelAction>(std::string{target},
                                                                                                     create_topics()));
        }
      }
      break;

      default:
        spdlog::warn("Unrecognized action [{}]"sv, action_name);
        spdlog::trace("  [line {}]."sv, yaml_label_action.Mark().line + 1);
        break;
    }

    if(action)
    {
      label_actions.emplace_back(std::move(action));
    }
  }

  return label_actions;
}

ValueActions configure_value_actions(const YAML::Node & yaml_value_actions)
{
  ValueActions value_actions;
  if(!yaml_value_actions)
  {
    return value_actions;
  }

  value_actions.reserve(yaml_value_actions.size());

  for(const auto & yaml_value_action : yaml_value_actions)
  {
    auto action_name{yy_util::to_lower(yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_value_action["action"sv])))};
    ValueActionPtr action;

    spdlog::info("       - action [{}]."sv, action_name);
    spdlog::trace("          [line {}]."sv, yaml_value_action.Mark().line + 1);
    switch(g_value_action_types.lookup(action_name, ValueActionType::Keep))
    {
      case ValueActionType::Keep:
        // Don't add as it does nothing.
        // action = yy_util::static_unique_cast<ValueAction>(std::make_unique<KeepValueAction>());
        break;

      case ValueActionType::Switch:
      {
        auto & yaml_values = yaml_value_action["values"sv];

        if(yaml_values)
        {
          std::optional<std::string> default_value{std::nullopt};
          SwitchValueAction::Switch switch_values;

          if(auto num_cases = yaml_values.size();
             num_cases > 1)
          {
            switch_values.reserve(num_cases - 1);
          }

          for(auto & yaml_case : yaml_values)
          {
            if(!yy_util::yaml_is_scalar(yaml_case))
            {
              if(!default_value.has_value())
              {
                if(auto & yaml_default = yaml_case["default"sv];
                   yaml_default)
                {
                  default_value = yy_util::yaml_get_optional_value<std::string>(yaml_default);
                  if(default_value.has_value())
                  {
                    spdlog::info("         - default: [{}]", default_value.value());
                  }
                }
              }

              if(auto & yaml_input = yaml_case["input"sv];
                 yaml_input)
              {
                if(auto & yaml_output = yaml_case["output"sv];
                   yaml_output)
                {
                  auto input{yy_util::yaml_get_value<std::string_view>(yaml_input)};
                  auto output{yy_util::yaml_get_value<std::string_view>(yaml_output)};

                  spdlog::info("         - input: [{}] output: [{}]", input, output);
                  switch_values.emplace(std::string{input},
                                        std::string{output});
                }
              }
            }
          }

          if(default_value.has_value()
             && !switch_values.empty())
          {
            action = yy_util::static_unique_cast<ValueAction>(std::make_unique<SwitchValueAction>(std::move(default_value.value()),
                                                                                                  std::move(switch_values)));
          }
        }
      }
      break;

      default:
        spdlog::debug("configure_value_actions(): 4c");
        spdlog::warn("Unrecognized action [{}]"sv, action_name);
        spdlog::trace("  [line {}]."sv, yaml_value_action.Mark().line + 1);
        break;
    }

    if(action)
    {
      value_actions.emplace_back(std::move(action));
    }
  }

  return value_actions;
}


} // anonymous namespace

MetricsMap configure_values(const YAML::Node & yaml_values)
{
  MetricsMap metrics{};

  if(yaml_values)
  {
    for(const auto & yaml_value : yaml_values)
    {
      auto & yaml_handlers = yaml_value["handlers"sv];
      yy_data::flat_set<std::string_view> handlers{};
      handlers.reserve(yaml_handlers.size());

      auto value_id{yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_value["value"sv]))};
      spdlog::info(" Configuring Value [{}]."sv,
                   value_id);
      spdlog::trace("  [line {}]."sv,
                    yaml_value.Mark().line + 1);

      for(const auto & yaml_handler : yaml_handlers)
      {
        std::string_view handler_id{yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_handler["handler_id"sv]))};
        spdlog::info("   handler [{}]:"sv, handler_id);
        spdlog::trace("    [line {}]."sv, yaml_handler.Mark().line + 1);

        const auto & yaml_property = yaml_handler["property"sv];

        if(auto [ignore, emplaced] = handlers.emplace(handler_id);
           emplaced)
        {
          auto property_name{yy_util::yaml_get_optional_value<std::string_view>(yaml_property)};

          if(property_name.has_value()
             && !property_name.value().empty())
          {
            spdlog::info("     - value [{}]."sv, property_name.value());
            spdlog::trace("        [line {}]."sv, yaml_property.Mark().line + 1);

            auto create_label_actions = [&yaml_handler]() {
              return configure_label_actions(yaml_handler["label_actions"sv]);
            };

            auto create_value_actions = [&yaml_handler]() {
              return configure_value_actions(yaml_handler["value_actions"sv]);
            };

            auto metric{std::make_shared<Metric>(MetricId{value_id, Labels{}},
                                                 std::string{property_name.value()},
                                                 create_label_actions(),
                                                 create_value_actions())};

            spdlog::info("     - add metric [{}] to handler [{}] property [{}]."sv,
                         metric->Id().Id(),
                         handler_id,
                         metric->Property());

            auto [metrics_pos, ignore_found] = metrics.emplace(std::string{handler_id},
                                                               Metrics{});

            auto [ignore_key, handler_metrics] = metrics[metrics_pos];

            handler_metrics.emplace_back(std::move(metric));
          }
        }
        else
        {
          spdlog::warn("   'property' setting!"sv);
          spdlog::trace("    [line {}]."sv, yaml_value.Mark().line + 1);
        }
      }
    }
  }

  return metrics;
}


} // namespace yafiyogi::values
