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

#include "spdlog/spdlog.h"

#include "yy_cpp/yy_constants.hpp"
#include "yy_cpp/yy_find_iter_util.hpp"

#include "action_store.hpp"

namespace yafiyogi::mendel::actions {

Store::Store(store_type && p_store,
             actions_type && p_actions):
  m_store(std::move(p_store)),
  m_actions(std::move(p_actions))
{
}

void StoreBuilder::Add(ActionPtr p_action,
                       Inputs & p_inputs)
{
  auto [iter, found] = yy_data::find_iter(m_actions, p_action);
  if(!found)
  {
    iter = m_actions.emplace(iter, std::move(p_action)).iter;
  }

  Action * l_action = iter->get();
  for(auto & input_id : p_inputs)
  {
    if(auto [pointers, p_found] = m_store_builder.add(input_id, action_pointers{});
       nullptr != pointers)
    {
      if(auto [action_pos, action_found] = yy_data::find_iter(*pointers, l_action);
         !action_found)
      {
        pointers->emplace(action_pos, l_action);
      }
    }
  }
}

Store StoreBuilder::Create()
{
  return Store{m_store_builder.create_automaton(), std::move(m_actions)};
}

} // namespace yafiyogi::mendel::actions
