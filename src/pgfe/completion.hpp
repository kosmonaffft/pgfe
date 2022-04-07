// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DMITIGR_PGFE_COMPLETION_HPP
#define DMITIGR_PGFE_COMPLETION_HPP

#include "dll.hpp"
#include "response.hpp"

#include <optional>
#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A successful operation completion.
 */
class Completion final : public Response {
public:
  /// Default-constructible. (Constructs an invalid instance.)
  DMITIGR_PGFE_API Completion();

  /// Not copy-constructible.
  Completion(const Completion&) = delete;

  /// Not copy-assignable.
  Completion& operator=(const Completion&) = delete;

  /// Move-constructible.
  DMITIGR_PGFE_API Completion(Completion&& rhs) noexcept;

  /// Move-assignable.
  DMITIGR_PGFE_API Completion& operator=(Completion&& rhs) noexcept;

  /// Swaps this instance with `rhs`.
  DMITIGR_PGFE_API void swap(Completion& rhs) noexcept;

  /// @see Message::is_valid().
  DMITIGR_PGFE_API bool is_valid() const noexcept override;

  /**
   * @returns The operation name which may be:
   *   -# an empty string that denotes a response to an empty query request;
   *   -# the string "invalid response" that denotes an ununderstood response;
   *   -# a word in uppercase that identifies the completed SQL command;
   *   -# a word in lowercase that identifies the completed operation.
   *
   * @remarks The operation name is not always matches a SQL command name. For
   * example, the operation name for `END` command is "COMMIT", the
   * operation name for `CREATE TABLE AS` command is "SELECT" etc.
   */
  DMITIGR_PGFE_API const std::string& operation_name() const noexcept;

  /**
   * @returns The number of rows affected by a completed SQL command.
   *
   * @remarks SQL commands for which this information is available are:
   * `INSERT`, `DELETE`, `UPDATE`, `SELECT` or `CREATE TABLE AS`, `MOVE`,
   * `FETCH`, `COPY`.
   */
  DMITIGR_PGFE_API std::optional<long> affected_row_count() const noexcept;

private:
  friend Connection;

  long affected_row_count_{-2}; // -1 - no value, -2 - invalid instance
  std::string operation_name_;

  /**
   * The constructor.
   *
   * @par Requires
   * `tag.data()`.
   */
  explicit Completion(const std::string_view tag);

  bool is_invariant_ok() const noexcept;
};

/// Completion is swappable.
inline void swap(Completion& lhs, Completion& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "completion.cpp"
#endif

#endif  // DMITIGR_PGFE_COMPLETION_HPP
