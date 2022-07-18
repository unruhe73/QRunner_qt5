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

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QWidget>

#include <QtCore/QDebug>

#include "monitorview.h"

MonitorView::MonitorView(QWidget* parent)
  : QDialog(parent)
{
  QGridLayout *layout = new QGridLayout;
  for (int i = 0; i < 4; i++)
  {
    TextEditMonitor *item = new TextEditMonitor;
    item->setText(QString("monitor %1").arg(i));
    QPair<ScriptProcess*, TextEditMonitor*> elem = qMakePair((ScriptProcess*) NULL, item);
    m_box.append(elem);
    layout->addWidget( m_box.at(i).second, i/2, i%2 );
    connect(item, SIGNAL(droppedTreeWidgetItem(TreeWidgetItem*,TextEditMonitor*)), this, SIGNAL(droppedTreeWidgetItem(TreeWidgetItem*,TextEditMonitor*)));
  }
  setLayout( layout );
}


/*int MonitorView::assignScriptProcess(ScriptProcess* script)
{
  return 0;
}*/
