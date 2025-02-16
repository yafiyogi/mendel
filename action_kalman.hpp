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

#pragma once

#include <string>

#include "yy_cpp/yy_flat_map.h"
#include "yy_cpp/yy_vector.h"
#include "yy_maths/yy_ekf.hpp"

#include "action.hpp"
#include "action_var_idx.hpp"
#include "values_store.hpp"

namespace yafiyogi::mendel::actions {

using KalmanOptions = yy_data::flat_map<std::string, std::string>;

class KalmanAction final:
      public Action
{
  public:
    KalmanAction(const KalmanOptions & p_options);
    void run(const values::Store & store) noexcept override;

  private:
    using ekf = yy_maths::ekf;
    using matrix = ekf::matrix;
    using vector = ekf::vector;
    using size_type = ekf::size_type;

    ekf m_ekf{};
    vector m_observations{};
    vector m_hx{};

    std::string m_output_topic;
    using VarIdxVector = yy_quad::simple_vector<var_idx>;
    VarIdxVector m_outputs{};
    VarIdxVector m_inputs{};
};

} // namespace yafiyogi::mendel::actions
