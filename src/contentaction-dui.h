/*
 * Copyright (C) 2010 Nokia Corporation.
 *
 * Contact: Marius Vollmer <marius.vollmer@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef CONTENTACTION_DUI_H
#define CONTENTACTION_DUI_H

/*!
  \namespace ContentAction::Dui

  \brief DUI integration

  The text-highlighter part of libcontentaction can be used to implement
  DuiLabelHighlighter functionality.
*/

class DuiLabel;

namespace ContentAction {
namespace Dui {

void highlightLabel(DuiLabel *label);
void highlightLabel(DuiLabel *label, QStringList typesToHighlight);
void dehighlightLabel(DuiLabel *label);

} // end namespace
} // end namespace

#endif
