/**
 * @file
 * Declares a utility for retrieving the library version.
 */
#pragma once

#include <string>
#include "export.h"

#include <libral/ral.hpp>
#include <libral/type.hpp>

namespace libral {

  /**
   * Query the library version.
   * @return A version string with \<major>.\<minor>.\<patch>
   */
  std::string LIBRAL_EXPORT version();

  ral open(const std::string& data_dir);
}  // namespace libral
