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

#include <memory>
#include <stop_token>

#include "yy_cpp/yy_ring_buffer.h"

#include "actions_handler_fwd.hpp"
#include "actions_store.hpp"
#include "values_store.hpp"

namespace yafiyogi::mendel {

class ActionsHandler
{
  public:
    using MessageQueue = yy_data::ring_buffer<values::MetricDataVector, 32>;

    ActionsHandler(actions::StorePtr m_actions_store,
                   values::StorePtr p_values_store);
    void Run(std::stop_token p_stop_token);
    bool QWrite(values::MetricDataVector & p_data);

  private:
    actions::StorePtr m_actions_store{};
    values::StorePtr m_values_store{};
    MessageQueue m_queue{};
};

using ActionsHandlerPtr = std::shared_ptr<ActionsHandler>;

} // namespace yafiyogi::mendel
