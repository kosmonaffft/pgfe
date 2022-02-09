// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#include "row_info.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Row_info::Row_info(detail::pq::Result&& pq_result)
  : pq_result_(std::move(pq_result))
{}

DMITIGR_PGFE_INLINE std::size_t Row_info::field_count() const noexcept
{
  return static_cast<std::size_t>(pq_result_.field_count());
}

DMITIGR_PGFE_INLINE bool Row_info::is_empty() const noexcept
{
  return !field_count();
}

DMITIGR_PGFE_INLINE std::string_view
Row_info::field_name(const std::size_t index) const noexcept
{
  assert(index < field_count());
  return pq_result_.field_name(static_cast<int>(index));
}

DMITIGR_PGFE_INLINE std::size_t
Row_info::field_index(const std::string_view name, const std::size_t offset) const noexcept
{
  const std::size_t fc{field_count()};
  assert(offset < fc);
  for (std::size_t i{offset}; i < fc; ++i) {
    const std::string_view nm{pq_result_.field_name(i)};
    if (nm == name)
      return i;
  }
  return fc;
}

DMITIGR_PGFE_INLINE std::uint_fast32_t
Row_info::table_oid(const std::size_t index) const noexcept
{
  assert(index < field_count());
  return pq_result_.field_table_oid(static_cast<int>(index));
}

DMITIGR_PGFE_INLINE std::int_fast32_t Row_info::table_column_number(const std::size_t index) const noexcept
{
  assert(index < field_count());
  return pq_result_.field_table_column(int(index));
}

DMITIGR_PGFE_INLINE std::uint_fast32_t Row_info::type_oid(const std::size_t index) const noexcept
{
  assert(index < field_count());
  return pq_result_.field_type_oid(int(index));
}

DMITIGR_PGFE_INLINE std::int_fast32_t Row_info::type_size(const std::size_t index) const noexcept
{
  assert(index < field_count());
  return pq_result_.field_type_size(int(index));
}

DMITIGR_PGFE_INLINE std::int_fast32_t Row_info::type_modifier(const std::size_t index) const noexcept
{
  assert(index < field_count());
  return pq_result_.field_type_modifier(int(index));
}

DMITIGR_PGFE_INLINE Data_format Row_info::data_format(const std::size_t index) const noexcept
{
  assert(index < field_count());
  return pq_result_.field_format(int(index));
}

} // namespace dmitigr::pgfe
