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
#include <QtGui/QMouseEvent>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMessageBox>

#include "textedit.h"
#include "scriptprocess.h"

TextEdit::TextEdit(ScriptProcess* proc, QWidget *parent)
  : QTextEdit(parent), m_proc(proc)
{
  setFont( QFont("Courier", 10) ); //Tahoma
}


ScriptProcess *TextEdit::process() const
{
  return m_proc;
}


void TextEdit::mousePressEvent( QMouseEvent* event )
{
  QTextEdit::mousePressEvent(event);
  if ((event->button() == Qt::LeftButton) && (m_proc->isRunning()))
  {
    bool ok;
    QString text = QInputDialog::getText(this, tr("Send Text"),
      tr("Type the text you want to send to the script"), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty() && m_proc)
    {
      append( text );
      text += "\n";
      qint64 err = m_proc->write(text.toLocal8Bit());
      if (err == -1)
        QMessageBox::critical(0, tr("Writing Error"),
          tr("An error occurred writing to %1 script stdout").arg(m_proc->name()));
    }
  }
}


void TextEdit::assignScriptProcess(ScriptProcess* script)
{
  m_proc = script;
}
