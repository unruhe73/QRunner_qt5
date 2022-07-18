/***************************************************************************
 *   Copyright (C) 2007-2022 by Giovanni Venturi                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

#include <QtGui/QFont>
#include <QtGui/QDropEvent>

#include <QtCore/QDebug>

#include "texteditmonitor.h"
#include "treewidgetitem.h"
#include "scripttree.h"

TextEditMonitor::TextEditMonitor(QWidget *parent)
  : QTextEdit(parent)
{
  setFont(QFont("Courier", 20));
}


void TextEditMonitor::dropEvent(QDropEvent* event)
{
  if (event->source() != NULL)
  {
    if (((ScriptTree*)event->source())->draggingItem()->isFile())
      emit droppedTreeWidgetItem(((ScriptTree*)event->source())->draggingItem(), this);
  }
}
