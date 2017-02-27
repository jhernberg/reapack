/* ReaPack: Package manager for REAPER
 * Copyright (C) 2015-2017  Christian Fillion
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

#ifndef REAPACK_DOWNLOAD_HPP
#define REAPACK_DOWNLOAD_HPP

#include "config.hpp"
#include "path.hpp"
#include "thread.hpp"

#include <fstream>
#include <sstream>

#include <curl/curl.h>

struct DownloadContext {
  static void GlobalInit();
  static void GlobalCleanup();

  DownloadContext();
  ~DownloadContext();

  CURL *m_curl;
};

class Download : public ThreadTask {
public:
  enum Flag {
    NoCacheFlag = 1<<0,
  };

  Download(const std::string &url, const NetworkOpts &, int flags = 0);
  const std::string &url() const { return m_url; }

  void start();
  void run(DownloadContext *) override;

private:
  virtual std::ostream *openStream() = 0;
  virtual void closeStream() {}

private:
  bool has(Flag f) const { return (m_flags & f) != 0; }
  static size_t WriteData(char *, size_t, size_t, void *);
  static int UpdateProgress(void *, double, double, double, double);

  std::string m_url;
  NetworkOpts m_opts;
  int m_flags;
};

class MemoryDownload : public Download {
public:
  MemoryDownload(const std::string &name, const std::string &url,
    const NetworkOpts &, int flags = 0);

  std::string contents() const { return m_stream.str(); }

protected:
  std::ostream *openStream() override { return &m_stream; }

private:
  std::string summary() const override;

  std::string m_name;
  std::stringstream m_stream;
};

class FileDownload : public Download {
public:
  FileDownload(const Path &target, const std::string &url,
    const NetworkOpts &, int flags = 0);

  const TempPath &path() const { return m_path; }

protected:
  std::ostream *openStream() override;
  void closeStream() override;

private:
  std::string summary() const override;

  TempPath m_path;
  std::ofstream m_stream;
};

#endif
