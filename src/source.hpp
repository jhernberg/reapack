/* ReaPack: Package manager for REAPER
 * Copyright (C) 2015-2016  Christian Fillion
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef REAPACK_SOURCE_HPP
#define REAPACK_SOURCE_HPP

#include <string>

#include "package.hpp"

class Package;
class Path;
class Version;

class Source {
public:
  enum Platform {
    UnknownPlatform,
    GenericPlatform,

    // windows
    WindowsPlatform,
    Win32Platform,
    Win64Platform,

    // os x
    DarwinPlatform,
    Darwin32Platform,
    Darwin64Platform,
  };

  static Platform getPlatform(const char *);

  Source(const std::string &file, const std::string &url,
    const Version * = nullptr);

  void setPlatform(Platform p) { m_platform = p; }
  Platform platform() const { return m_platform; }

  void setTypeOverride(Package::Type t) { m_type = t; }
  Package::Type typeOverride() const { return m_type; }

  bool isMain() const { return m_file.empty(); }
  const std::string &file() const;
  const std::string &url() const { return m_url; }

  const Version *version() const { return m_version; }
  const Package *package() const;

  std::string fullName() const;
  Path targetPath() const;

private:
  Platform m_platform;
  Package::Type m_type;
  std::string m_file;
  std::string m_url;
  const Version *m_version;
};

#endif
