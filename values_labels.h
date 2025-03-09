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

#include <initializer_list>
#include <string>
#include <string_view>

#include "yy_cpp/yy_flat_map.h"

#include "yy_mqtt/yy_mqtt_types.h"

namespace yafiyogi::values {

class Labels final
{
  public:
    using LabelStore = yy_data::flat_map<std::string,
                                         std::string,
                                         yy_data::ClearAction::Keep,
                                         yy_data::ClearAction::Keep>;

    Labels(size_type capacity) noexcept;
    constexpr Labels() noexcept = default;
    constexpr Labels(const Labels &) noexcept = default;
    constexpr Labels(Labels &&) noexcept = default;

    constexpr Labels & operator=(const Labels &) noexcept = default;
    constexpr Labels & operator=(Labels &&) noexcept = default;

    void clear() noexcept;
    void clear(yy_data::ClearAction p_clear_action) noexcept;
    std::string & set_label(std::string_view p_label,
                            std::string_view p_value);

    [[nodiscard]]
    const std::string & get_label(const std::string_view p_label) const noexcept;

    template<typename Visitor>
    [[nodiscard]]
    bool get_label(Visitor && visitor,
                   const std::string_view p_label) const noexcept
    {
      return m_labels.find_value(std::forward<Visitor>(visitor), p_label).found;
    }

    void erase(const std::string_view p_label);

    [[nodiscard]]
    constexpr size_type size() const noexcept
    {
      return m_labels.size();
    }

    bool operator<(const Labels & other) const noexcept;
    bool operator==(const Labels & other) const noexcept;

    template<typename Visitor>
    void visit(Visitor && visitor) const
    {
      m_labels.visit(std::forward<Visitor>(visitor));
    }

  private:
    LabelStore m_labels{};
};

} // namespace yafiyogi::values
