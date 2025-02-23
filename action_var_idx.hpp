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

#include <tuple>

#include "yy_cpp/yy_type_traits.h"

namespace yafiyogi::mendel::actions {

class var_idx final
{
  public:
    constexpr var_idx(const std::string & p_var,
                      size_type p_idx) noexcept:
      m_var(p_var),
      m_idx(p_idx)
    {
    }

    constexpr var_idx(std::string && p_var,
                      size_type p_idx) noexcept:
      m_var(std::move(p_var)),
      m_idx(p_idx)
    {
    }

    constexpr var_idx() noexcept = default;
    constexpr var_idx(const var_idx &) noexcept = default;
    constexpr var_idx(var_idx &&) noexcept = default;
    constexpr ~var_idx() noexcept = default;

    constexpr var_idx & operator=(const var_idx &) noexcept = default;
    constexpr var_idx & operator=(var_idx &&) noexcept = default;

    constexpr bool operator<(const std::string & var) const noexcept
    {
      return m_var < var;
    }

    constexpr bool operator==(const std::string & var) const noexcept
    {
      return m_var == var;
    }

    constexpr const std::string & var() const noexcept
    {
      return m_var;
    }

    constexpr size_type idx() const noexcept
    {
      return m_idx;
    }

  private:
    std::string m_var;
    size_type m_idx;
};

template<std::size_t Idx,
         typename T,
         std::enable_if_t<std::is_base_of_v<var_idx,
                                            yy_traits::remove_cvr_t<T>>, bool> = true>
inline constexpr auto get(T && p_var_idx) noexcept
{
  static_assert(Idx < 2, "var_idx index out of range");

  if constexpr (Idx == 0)
  {
    return p_var_idx.var();
  }

  if constexpr (Idx == 1)
  {
    return p_var_idx.idx();
  }
}

using var_idx_types = std::tuple<std::string, size_type>;

} // namespace yafiyogi::mendel::actions

namespace std {

// Specialize tuple_size<> for yafiyogi::mendel::actionsKalmanAction::var_idx.
template<>
struct tuple_size<yafiyogi::mendel::actions::var_idx>:
      tuple_size<yafiyogi::mendel::actions::var_idx_types>
{
};

// Specialize tuple_element<> for yafiyogi::mendel::actionsKalmanAction::var_idx.
template<std::size_t Idx>
struct tuple_element<Idx, yafiyogi::mendel::actions::var_idx>:
      tuple_element<Idx, yafiyogi::mendel::actions::var_idx_types>
{
};

} // namespace std
