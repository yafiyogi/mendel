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

#include <chrono>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "logger.h"

namespace yafiyogi::mendel {
namespace {

using namespace std::string_view_literals;
using namespace fmt::literals;

constexpr std::string_view access_log_format{"Web Access from: [{}:{}] to [{}:{}]"sv};

constexpr std::string_view g_std_err{"stderr"sv};
constexpr std::string_view g_log_name{"activity"sv};

static std::mutex g_logger_mtx{};
static std::string g_std_err_str{g_std_err};
static std::string g_logger_name{g_std_err};
static logger_ptr g_logger = spdlog::stderr_color_mt(g_logger_name);

void set_g_logger(logger_ptr log)
{
  std::unique_lock lck{g_logger_mtx};

  auto old_logger = g_logger;

  g_logger = log;
  spdlog::flush_every(std::chrono::seconds(5));
  spdlog::set_default_logger(g_logger);

  if(old_logger)
  {
    spdlog::drop(old_logger->name());
  }
}

} // anonymous namespace

void set_logger(std::string_view file_path)
{
  set_g_logger(spdlog::daily_logger_mt(g_log_name.data(), file_path.data(), 0, 0));
}

void set_logger()
{
  stop_log();
  set_logger(g_default_file_path);
}

void set_console_logger()
{
  if(!spdlog::get(g_std_err_str))
  {
    set_g_logger(spdlog::stderr_color_mt(g_std_err_str));
  }
}

logger_ptr get_log()
{
  std::unique_lock lck{g_logger_mtx};

  return g_logger;
}

void stop_log()
{
  std::unique_lock lck{g_logger_mtx};

  auto old_logger = g_logger;

  g_logger = spdlog::stderr_color_mt(g_std_err_str);
  spdlog::set_default_logger(g_logger);

  if(old_logger)
  {
    spdlog::drop(old_logger->name());
  }
}

void stop_log(std::string_view logger_name)
{
  spdlog::drop(logger_name.data());
}

void stop_all_logs()
{
  std::unique_lock lck{g_logger_mtx};
  g_logger = logger_ptr{};

  spdlog::drop_all();
}

} // namespace yafiyogi::mendel
