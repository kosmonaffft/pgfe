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

#include "notice.hpp"

#include <cassert>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Notice::~Notice()
{
  pq_result_.release(); // freed in libpq/fe-protocol3.c:pqGetErrorNotice3()
}

DMITIGR_PGFE_INLINE Notice::Notice(const ::PGresult* const result) noexcept
  : Problem{detail::pq::Result{const_cast< ::PGresult*>(result)}}
{
  /*
   * In fact result is not const. So it's okay to const_cast.
   * (Allocated in libpq/fe-protocol3.c:pqGetErrorNotice3().)
   */
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE bool Notice::is_valid() const noexcept
{
  return static_cast<bool>(pq_result_);
}

DMITIGR_PGFE_INLINE bool Notice::is_invariant_ok() const noexcept
{
  const auto sev = severity();
  return ((static_cast<int>(sev) == -1) ||
    (sev == Problem_severity::log) ||
    (sev == Problem_severity::info) ||
    (sev == Problem_severity::debug) ||
    (sev == Problem_severity::notice) ||
    (sev == Problem_severity::warning)) && Problem::is_invariant_ok();
}

} // namespace dmitigr::pgfe
