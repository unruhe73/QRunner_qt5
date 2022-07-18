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

#include <QtWidgets/QDialog>

#include <QtCore/QList>
#include <QtCore/QPair>

#include "texteditmonitor.h"

class ScriptProcess;

class MonitorView : public QDialog
{
  Q_OBJECT
  public:
    /**
     * The Monitor View use a QDialog to contain more script output logs
     */
    MonitorView(QWidget * parent = 0);

    /**
     * Assign a script to the monitor view if a box is still available
     *
     * @param script is the reference to the script
     *
     * @returns -1 if no more boxes available
     */
    //int assignScriptProcess(ScriptProcess* script);

  private:
    /**
     * Declare (4) QTextEdit box to let assign to 4 running scripts
     */
    QList< QPair< ScriptProcess*, TextEditMonitor* > > m_box;

  signals:
    void droppedTreeWidgetItem(TreeWidgetItem *, TextEditMonitor *);
};
