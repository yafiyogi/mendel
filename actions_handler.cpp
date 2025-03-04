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

#include "fmt/format.h"

#include "actions_handler.hpp"

namespace yafiyogi::mendel {

ActionsHandler::ActionsHandler(values::StorePtr p_values_store):
  m_values_store_ptr(std::move(p_values_store))
{
  m_values_store = m_values_store_ptr.get();
}

void ActionsHandler::Run(std::stop_token p_stop_token)
{
  actions_type l_actions;

  values::Store & l_values_store = *m_values_store;

  while(!p_stop_token.stop_requested())
  {
    while(m_queue.swap_out(l_actions))
    {
      for(auto & action : l_actions)
      {
        action->Run(l_values_store);
      }

      l_actions.clear(yy_data::ClearAction::Keep);
    }

    m_queue.wait(p_stop_token, [this] { return !m_queue.empty();});
  }
}

bool ActionsHandler::Write(actions_type & data)
{
  return m_queue.swap_in(data);
}

} // namespace yafiyogi::mendel
