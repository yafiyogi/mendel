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

    template<std::size_t Idx>
    auto get() const
    {
      static_assert(Idx < 2, "var_idx index out of range");

      if constexpr (Idx == 0)
      {
        return var();
      }

      if constexpr (Idx == 1)
      {
        return idx();
      }
    }

    using types = std::tuple<std::string, size_type>;

  private:
    std::string m_var;
    size_type m_idx;
};

} // namespace yafiyogi::mendel::actions

namespace std {

// Specialize tuple_size<> for yafiyogi::mendel::actionsKalmanAction::var_idx.
template<>
struct tuple_size<yafiyogi::mendel::actions::var_idx>:
      tuple_size<yafiyogi::mendel::actions::var_idx::types>
{
};

// Specialize tuple_element<> for yafiyogi::mendel::actionsKalmanAction::var_idx.
template<std::size_t Idx>
struct tuple_element<Idx, yafiyogi::mendel::actions::var_idx>:
      tuple_element<Idx, yafiyogi::mendel::actions::var_idx::types>
{
};

} // namespace std
