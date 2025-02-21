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

// Why mendel? See: https://en.wikipedia.org/wiki/Tinker_Tailor_Soldier_Spy#Characters

#include <unistd.h>
#include <csignal>

#include <exception>
#include <memory>

#include "boost/program_options.hpp"
#include "fmt/ostream.h"
#include "mosquittopp.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"

#include "yy_cpp/yy_locale.h"
#include "yy_cpp/yy_yaml_util.h"

#include "configure_mqtt.h"
#include "configure_logging.h"
#include "configure_values.h"
#include "logger.h"
#include "mqtt_client.h"
#include "mqtt_handler.h"

namespace {

static std::atomic<bool> exit_program{false};

void signal_handler(int /* signal */)
{
  std::signal(SIGINT, SIG_DFL);
  std::signal(SIGTERM, SIG_DFL);
  exit_program = true;
}

}

namespace bpo = boost::program_options;

int main(int argc, char* argv[])
{
  using namespace yafiyogi;
  using namespace std::string_view_literals;

  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  mendel::logger_config log_config{std::string{mendel::g_default_file_path}, spdlog::level::debug};
  spdlog::set_level(log_config.level);
  mendel::set_console_logger();
  yy_locale::set_locale();

  std::string config_file{"mendel.yaml"};
  bool no_run = false;

  bpo::options_description desc("Usage");

  desc.add_options()
    ("help,h", "print usage")
    ("conf,f", bpo::value(&config_file), "config file")
    ("log,l", bpo::value(&log_config.filename), "log file")
    ("no-run,n", bpo::bool_switch(&no_run), "confgure only, don't run");

  bpo::variables_map vm;
  bpo::store(bpo::command_line_parser(argc, argv).options(desc).run(), vm);
  bpo::notify(vm);

  if(vm.count("help"))
  {
    spdlog::info("{}"sv, fmt::streamed(desc));

    mendel::stop_all_logs();

    return 0;
  }

  const YAML::Node yaml_config = YAML::LoadFile(config_file);

  if(0 == vm.count("log"))
  {
    if(auto yaml_mendel = yaml_config["mendel"sv];
       yaml_mendel)
    {
      log_config = mendel::configure_logging(yaml_mendel["logging"sv],
                                             log_config);
    }
  }

  mendel::set_logger(log_config.filename);
  spdlog::set_level(log_config.level);

  const auto yaml_values = yaml_config["values"sv];
  if(!yaml_values)
  {
    spdlog::error("Not found values config"sv);

    return 1;
  }

  auto values_config{values::configure_values(yaml_values)};

  const auto yaml_mqtt = yaml_config["mqtt"sv];
  if(!yaml_mqtt)
  {
    spdlog::error("Not found mqtt config"sv);

    return 1;
  }

  auto mqtt_config{mendel::configure_mqtt(yaml_mqtt,
                                          values_config)};

  if(!no_run)
  {
    mosqpp::lib_init();

    auto client = std::make_unique<mendel::mqtt_client>(mqtt_config);

    client->connect();
    try
    {
      while(!exit_program)
      {
        if(auto rc = client->loop();
           rc)
        {
          std::this_thread::sleep_for(std::chrono::seconds{15});
          spdlog::info("reconnect [{}]"sv, rc);
          client->reconnect();
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

    client->disconnect();

    while(client->is_connected())
    {
      sleep(1);
    }

    mosqpp::lib_cleanup();
  }

  spdlog::info("Ende"sv);
  mendel::stop_all_logs();

  return 0;
}
