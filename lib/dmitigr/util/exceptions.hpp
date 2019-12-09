// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#ifndef DMITIGR_UTIL_EXCEPTIONS_HPP
#define DMITIGR_UTIL_EXCEPTIONS_HPP

#include "dmitigr/util/dll.hpp"

#include <string>
#include <system_error>

namespace dmitigr {

/**
 * @brief An exception thrown on system error.
 */
class Sys_exception final : public std::system_error {
public:
  /**
   * @brief The constructor.
   */
  DMITIGR_UTIL_API explicit Sys_exception(const std::string& func);

  /**
   * @brief Prints the last system error to the standard error.
   */
  static DMITIGR_UTIL_API void report(const char* const func) noexcept;

  /**
   * @returns The last system error code.
   */
  static DMITIGR_UTIL_API int last_error() noexcept;
};

#ifdef _WIN32
/**
 * @brief A category of Windows Socket Application (WSA) errors.
 */
class Wsa_error_category final : public std::error_category {
public:
  /**
   * @returns The literal `dmitigr_wsa_error`.
   */
  const char* name() const noexcept override;

  /**
   * @returns The string that describes the error condition denoted by `ev`.
   *
   * @remarks The caller should not rely on the
   * return value since it is a subject to change.
   */
  std::string message(int ev) const override;
};

/**
 * @returns The reference to instance of type Wsa_error_category.
 */
DMITIGR_UTIL_API const Wsa_error_category& wsa_error_category() noexcept;

/**
 * @brief An exception thrown on Windows Socket Application (WSA) error.
 */
class Wsa_exception final : public std::system_error {
public:
  /**
   * @brief The constructor.
   */
  DMITIGR_UTIL_API explicit Wsa_exception(const std::string& func);

  /**
   * @brief Prints the last WSA error to the standard error.
   */
  static DMITIGR_UTIL_API void report(const char* const func) noexcept;

  /**
   * @returns The last WSA error code.
   */
  static DMITIGR_UTIL_API int last_error() noexcept;
};
#endif

/**
 * Convenient macro for cross-platform network programming.
 *
 * This macro can be useful because on Windows some network functions
 * reports errors via GetLastError(), while others - via WSAGetLastError().
 */
#ifdef _WIN32
#define DMITIGR_NET_EXCEPTION dmitigr::Wsa_exception
#else
#define DMITIGR_NET_EXCEPTION dmitigr::Sys_exception
#endif

} // namespace dmitigr

#ifdef DMITIGR_UTIL_HEADER_ONLY
#include "dmitigr/util/exceptions.cpp"
#endif

#endif  // DMITIGR_UTIL_EXCEPTIONS_HPP
