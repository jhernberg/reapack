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

#include "path.hpp"

#include <algorithm>
#include <vector>

using namespace std;

#ifndef _WIN32
static const char SEPARATOR = '/';
#else
static const char SEPARATOR = '\\';
#endif

Path Path::s_root;

static vector<string> Split(const string &input)
{
  vector<string> list;

  size_t last = 0, size = input.size();

  while(last < size) {
    const size_t pos = input.find_first_of("\\/", max((size_t)1, last));

    if(pos == string::npos) {
      list.push_back(input.substr(last));
      break;
    }

    const string part = input.substr(last, pos - last);

    if(!part.empty())
      list.push_back(part);

    last = pos + 1;
  }

  return list;
}

Path::Path(const std::string &path)
{
  append(path);
}

void Path::prepend(const string &parts)
{
  if(parts.empty())
    return;

  for(const string &part : Split(parts))
    m_parts.push_front(part);
}

void Path::append(const string &parts)
{
  if(parts.empty())
    return;

  for(const string &part : Split(parts))
    m_parts.push_back(part);
}

void Path::clear()
{
  m_parts.clear();
}

void Path::removeLast()
{
  if(!empty())
    m_parts.pop_back();
}

string Path::basename() const
{
  if(empty())
    return {};

  return m_parts.back();
}

string Path::dirname() const
{
  if(empty())
    return {};

  Path dir(*this);
  dir.removeLast();

  return dir.join();
}

string Path::join(const char sep) const
{
  string path;

  for(auto it = m_parts.begin(); it != m_parts.end(); it++) {
    const string &part = *it;

    if(!path.empty())
      path.insert(path.end(), sep ? sep : SEPARATOR);

    path.append(part);
  }

  return path;
}

bool Path::operator==(const Path &o) const
{
  return m_parts == o.m_parts;
}

bool Path::operator!=(const Path &o) const
{
  return !(*this == o);
}

bool Path::operator<(const Path &o) const
{
  return m_parts < o.m_parts;
}

Path Path::operator+(const string &part) const
{
  Path path(*this);
  path.append(part);

  return path;
}

Path Path::operator+(const Path &o) const
{
  Path path(*this);
  path.m_parts.insert(path.m_parts.end(), o.m_parts.begin(), o.m_parts.end());

  return path;
}

const string &Path::at(const size_t index) const
{
  auto it = m_parts.begin();

  for(size_t i = 0; i < index; i++)
    it++;

  return *it;
}

string &Path::operator[](const size_t index)
{
  return const_cast<string &>(at(index));
}

const string &Path::operator[](const size_t index) const
{
  return at(index);
}

UseRootPath::UseRootPath(const std::string &path)
  : m_backup(move(Path::s_root))
{
  Path::s_root = path;
}

UseRootPath::~UseRootPath()
{
  Path::s_root = move(m_backup);
}
