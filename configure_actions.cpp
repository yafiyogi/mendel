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

#include "spdlog/spdlog.h"
#include "fmt/ranges.h"

#include "yy_cpp/yy_make_lookup.h"
#include "yy_cpp/yy_string_case.h"
#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_vector.h"
#include "yy_cpp/yy_yaml_util.h"

#include "action.hpp"
#include "action_kalman.hpp"

#include "configure_actions.hpp"

namespace yafiyogi::mendel {

using namespace std::string_view_literals;

namespace {

enum class ActionType:uint8_t
{
  None,
  Kalman
};

using Inputs = yy_quad::simple_vector<std::string>;

constexpr auto handler_types =
  yy_data::make_lookup<std::string_view, ActionType>(ActionType::None,
                                                     {{"kalman"sv, ActionType::Kalman}});

ActionType decode_action_type(const YAML::Node & yaml_type)
{
  if(!yaml_type)
  {
    return ActionType::None;
  }

  std::string type_name{yy_util::to_lower(yy_util::trim(yy_util::yaml_get_value<std::string_view>(yaml_type)))};

  return handler_types.lookup(type_name);
}

void configure_kalman(const YAML::Node & yaml_kalman,
                      actions::StoreBuilder & store)
{
  if(yaml_kalman)
  {
    actions::KalmanOptions options{};

    auto & yaml_values{yaml_kalman["values"sv]};

    Inputs inputs{};
    inputs.reserve(yaml_values.size());

    for(const auto & yaml_value : yaml_values)
    {
      auto input{yy_util::yaml_get_value<std::string_view>(yaml_value.first)};
      auto output{yy_util::yaml_get_value<std::string_view>(yaml_value.second)};

      if(!input.empty() && !output.empty())
      {
        options.emplace(std::string{input}, std::string{output});
        inputs.emplace_back(std::string{input});
      }
    }

    if(!options.empty())
    {
      auto output_topic{yy_util::yaml_get_value<std::string_view>(yaml_kalman["output_topic"sv])};

      ActionPtr action{std::make_unique<actions::KalmanAction>(std::string{output_topic}, std::move(options))};

      store.Add(std::move(action), inputs);
    }
  }
}

} // anonymous namespace

actions::Store configure_actions(const YAML::Node & yaml_actions)
{
  actions::StoreBuilder actions{};

  if(yaml_actions && !yy_util::yaml_is_scalar(yaml_actions))
  {
    for(const auto yaml_action : yaml_actions)
    {
      switch(decode_action_type(yaml_action["type"sv]))
      {
        case ActionType::Kalman:
          configure_kalman(yaml_action, actions);
          break;

        case ActionType::None:
          [[fallthrough]];
        default:
          break;
      }
    }
  }

  return actions::Store{};
}

} // namespace yafiyogi::mendel
