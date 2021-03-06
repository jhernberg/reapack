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

#ifndef REAPACK_REPORT_HPP
#define REAPACK_REPORT_HPP

#include "dialog.hpp"

#include "ostream.hpp"
#include "receipt.hpp"
#include "registry.hpp"

class Package;
class Version;

class Report : public Dialog {
public:
  Report(const Receipt &);

protected:
  void onInit() override;

  virtual void fillReport();

  void printHeader(const char *);

  void printInstalls();
  void printUpdates();
  void printErrors();
  void printRemovals();

private:
  Receipt m_receipt;
  OutputStream m_stream;
};

#endif
