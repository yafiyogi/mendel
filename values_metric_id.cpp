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

#include "yy_cpp/yy_tokenizer.h"

#include "values_metric_id.h"

namespace yafiyogi::values {

using tokenizer_type = yy_util::tokenizer<std::string_view::value_type, ':'>;
using token_type = tokenizer_type::token_type;

MetricId::MetricId(std::string_view p_metric_id) noexcept
{
  tokenizer_type tokenizer{yy_quad::make_const_span(p_metric_id)};

  auto id{tokenizer.scan()};
  m_id = std::string{id.begin(), id.end()};

  auto location{tokenizer.source()};
  m_location = std::string{location.begin(), location.end()};

}

MetricId::MetricId(std::string_view p_id,
                   std::string_view p_location) noexcept:
  m_id(p_id),
  m_location(p_location)
{
}

} // namespace yafiyogi::values
