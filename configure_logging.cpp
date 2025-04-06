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

#include "yy_cpp/yy_string_case.h"
#include "yy_cpp/yy_string_util.h"
#include "yy_cpp/yy_make_lookup.h"
#include "yy_cpp/yy_yaml_util.h"

#include "configure_logging.h"
#include "logger.h"

namespace yafiyogi::mendel {
namespace {

using namespace std::string_view_literals;
using namespace fmt::literals;

constexpr auto log_levels =
  yy_data::make_lookup<std::string_view, spdlog::level::level_enum>({{"trace", spdlog::level::level_enum::trace},
                                                                     {"debug", spdlog::level::level_enum::debug},
                                                                     {"info", spdlog::level::level_enum::info},
                                                                     {"warn", spdlog::level::level_enum::warn},
                                                                     {"warning", spdlog::level::level_enum::warn},
                                                                     {"error", spdlog::level::level_enum::err},
                                                                     {"err", spdlog::level::level_enum::err},
                                                                     {"critical", spdlog::level::level_enum::critical},
                                                                     {"off", spdlog::level::level_enum::off}});

}

logger_config configure_logging(const YAML::Node & yaml_logging,
                                logger_config log_config)
{
  if(yaml_logging)
  {
    if(auto log = yy_util::yaml_get_optional_value<std::string_view>(yaml_logging["filename"sv]);
       log.has_value())
    {
      log_config.filename = log.value();
    }

    if(auto level = yy_util::yaml_get_optional_value<std::string_view>(yaml_logging["level"sv]);
       level.has_value())
    {
      log_config.level = log_levels.lookup(yy_util::to_lower(yy_util::trim(level.value())), spdlog::level::info);
    }

    if(auto flush = yy_util::yaml_get_optional_value<std::string_view>(yaml_logging["flush"sv]);
       flush.has_value())
    {
      log_config.flush = log_levels.lookup(yy_util::to_lower(yy_util::trim(flush.value())), spdlog::level::info);
    }
  }

  return log_config;
}

} // namespace yafiyogi::mendel
