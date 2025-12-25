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

#include "fmt/ranges.h"
#include "spdlog/spdlog.h"

#include "yy_cpp/yy_make_lookup.h"
#include "yy_cpp/yy_string_case.h"
#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_vector.h"
#include "yy_cpp/yy_yaml_util.h"

#include "action.hpp"
#include "action_kalman.hpp"
#include "actions_store.hpp"
#include "values_store.hpp"

#include "configure_actions.hpp"

namespace yafiyogi::mendel {

using namespace std::string_view_literals;

namespace {

enum class ActionType : uint8_t
{
  None,
  Kalman
};

using Inputs = yy_quad::simple_vector<std::string>;
using Outputs = yy_quad::simple_vector<std::string>;

constexpr auto handler_types =
  yy_data::make_lookup<std::string_view, ActionType>(
    ActionType::None,
    {
      {"kalman"sv, ActionType::Kalman}
});

ActionType decode_action_type(const YAML::Node & yaml_type)
{
  if(!yaml_type)
  {
    return ActionType::None;
  }

  std::string type_name{yy_util::to_lower(
    yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_type)))};

  return handler_types.lookup(type_name);
}

void configure_kalman(const YAML::Node & yaml_kalman,
                      actions::StoreBuilder & actions_builder,
                      values::StoreBuilder & values_builder)
{
  if(yaml_kalman)
  {
    auto action_id{yy_util::trim(
      yy_util::yaml_get_value<std::string_view>(yaml_kalman["action_id"sv]))};
    spdlog::info("  Configuring Kalman Filter [{}]."sv, action_id);

    actions::KalmanOptions options{};

    auto & yaml_values{yaml_kalman["values"sv]};

    Inputs inputs{};
    inputs.reserve(yaml_values.size());
    Outputs outputs{};

    if(yaml_values.IsMap())
    {
      for(const auto & yaml_value: yaml_values)
      {
        auto input{yy_util::yaml_get_value<std::string_view>(yaml_value.first)};
        auto output{
          yy_util::yaml_get_value<std::string_view>(yaml_value.second)};

        if(!input.empty() && !output.empty())
        {
          options.emplace_back(yy_values::MetricId{input}, std::string{output});

          inputs.emplace_back(std::string{input});
          outputs.emplace_back(std::string{output});
        }
      }
    }
    else if(yaml_values.IsSequence())
    {
      for(const auto & yaml_value: yaml_values)
      {
        auto input{
          yy_util::yaml_get_value<std::string_view>(yaml_value["in"sv])};
        auto output{
          yy_util::yaml_get_value<std::string_view>(yaml_value["out"sv])};

        if(!input.empty() && !output.empty())
        {
          auto optional_accuracy{
            yy_util::yaml_get_optional_value<double>(yaml_value["accuracy"sv])};

          double accuracy = actions::KalmanAction::EPS;
          if(optional_accuracy.has_value())
          {
            accuracy = 1.0 / optional_accuracy.value();
            accuracy /= 100.0;
          }

          options.emplace_back(yy_values::MetricId{input},
                               std::string{output},
                               accuracy);

          inputs.emplace_back(std::string{input});
          outputs.emplace_back(std::string{output});
        }
      }
    }

    if(!options.empty())
    {
      if(auto yaml_output = yaml_kalman["output"sv]; yaml_output)
      {
        auto output_topic{
          yy_util::yaml_get_value<std::string_view>(yaml_output["topic"sv])};
        auto output_value_id{
          yy_util::yaml_get_value<std::string_view>(yaml_output["value_id"sv])};

        actions::ActionPtr action{
          std::make_unique<actions::KalmanAction>(action_id,
                                                  output_topic,
                                                  output_value_id,
                                                  std::move(options))};

        for(auto & input: inputs)
        {
          values_builder.Add(input);
        }

        for(auto & output: outputs)
        {
          values_builder.Add(fmt::format("{}:{}"sv, output_value_id, output));
        }

        actions_builder.Add(std::move(action), inputs);
      }
    }
    else
    {
      spdlog::info("  [{}] has no options. Not adding."sv, action_id);
    }
  }
}

} // anonymous namespace

void configure_actions(const YAML::Node & yaml_actions,
                       actions::StoreBuilder & actions_store,
                       values::StoreBuilder & values_store)
{
  actions::StoreBuilder actions{};

  if(yaml_actions && !yy_util::yaml_is_scalar(yaml_actions))
  {
    for(const auto yaml_action: yaml_actions)
    {
      switch(decode_action_type(yaml_action["type"sv]))
      {
      case ActionType::Kalman:
        configure_kalman(yaml_action, actions_store, values_store);
        break;

      case ActionType::None:
        [[fallthrough]];
      default:
        break;
      }
    }
  }
}

} // namespace yafiyogi::mendel
