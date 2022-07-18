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

#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QtWidgets/QLineEdit>

class ProjectView;

/**
 * This class let define the Line Edit Path for the File System Tree
 *
 * @author Giovanni Venturi
 */
class LineEdit : public QLineEdit
{
  Q_OBJECT
  public:
    /**
     * Create the Line Edit related to the Project View
     *
     * @param pv is the Project View reference
     * @param parent is the parent of the widget
     */
    LineEdit(ProjectView* pv, QWidget *parent = 0);

  protected:
    /**
     * Capture the mouse events of the widget
     */
    virtual void mousePressEvent(QMouseEvent* event);

  private:
    /**
     * The reference to the Project View
     */
    ProjectView* m_projectView;
};

#endif
