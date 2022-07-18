/***************************************************************************
 *   Copyright (C) 2007-2022 by Giovanni Venturi                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QtWidgets/QTextEdit>

class ScriptProcess;

/**
 * This class define Text Area that show the temporary standard output
 * and standard error channel of a scipt process execution
 *
 * @author Giovanni Venturi
 */
class TextEdit : public QTextEdit
{
  Q_OBJECT
  public:
    /**
     * Create the Text Area that refers to the Scrip Process @p proc output
     *
     * @param proc is the reference to the Scrip Proccess that need to show its output/error messages
     * @param parent is the parent widget
     */
    TextEdit(ScriptProcess* proc = 0, QWidget *parent = 0);

    /**
     * Assign the Scrip Process to the Text Area
     *
     * @param script is the script that need to show its error/output messages
     */
    void assignScriptProcess(ScriptProcess* script);

    /**
     * @returns the Script Process reference that need to show the process' output/error messages
     */
    ScriptProcess *process() const;

  protected:
    /**
     * Capture the mouse events of the widget
     */
    virtual void mousePressEvent( QMouseEvent* event );

  private:
    /**
     * The Process Script reference
     */
    ScriptProcess* m_proc;
};

#endif
