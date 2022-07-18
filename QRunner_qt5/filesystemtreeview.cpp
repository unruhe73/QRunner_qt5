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

#include <QtWidgets/QApplication>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QFileSystemModel>
#include <QtWidgets/QHeaderView>

#include <QtGui/QMouseEvent>
#include <QtGui/QDrag>
#include <QtGui/QDragEnterEvent>

#include <QtCore/QEvent>
#include <QtCore/QMimeData>

#include <QtCore/QDebug>

#include "filesystemtreeview.h"

FileSystemTreeView::FileSystemTreeView(const QString & directory, QWidget *parent)
   : QTreeView(parent)
{
  enableDND();

  m_directory = directory;
  m_dirModel = new QFileSystemModel;
  setModel(m_dirModel);
  //setRootIndex(m_dirModel->index(m_directory));
  m_dirModel->setRootPath(m_directory);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setAnimated(true);
  hideColumn(1);
  hideColumn(2);
  hideColumn(3);
  setColumnWidth(0, 300);
  header()->hide();

  setToolTip(tr("<p>You can <b>drag</b> files form here and drop them "
                 "to the left tree panel.</p>"
                 "<p>If you want, you can <b>change</b> the current "
                 "directory modifying the edit box text on the "
                 "top (<i>selected directory</i>).</p>"));

  connect(&m_path, SIGNAL(directoryChanged(QString)), m_dirModel, SLOT(refresh()));
}


void FileSystemTreeView::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    m_dragStartPosition = event->pos();
    QModelIndex index = indexAt(m_dragStartPosition);

    // just if an item (file/directory) has been selected you can get info on it
    if (index.isValid())
      m_data = m_dirModel->fileInfo(index).absoluteFilePath();
  }
  QTreeView::mousePressEvent(event);
}


void FileSystemTreeView::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_DND)
    return;
  if (!(event->buttons() & Qt::LeftButton))
    return;
  if ((event->pos() - m_dragStartPosition).manhattanLength()
    < QApplication::startDragDistance())
      return;

  emit startDraging();

  QDrag *drag = new QDrag(this);
  QMimeData *mimeData = new QMimeData;

  mimeData->setText(m_data);
  drag->setMimeData(mimeData);
  drag->exec(Qt::CopyAction);
}


void FileSystemTreeView::changeDirectory(const QString & directory)
{
  setRootIndex(m_dirModel->index(directory));

  if (!m_path.directories().isEmpty())
    m_path.removePaths(m_path.directories());
  m_path.addPath(directory);
}


void FileSystemTreeView::enableDND(const bool& cond)
{
  m_DND = cond;
  //setAcceptDrops(true);
  setDropIndicatorShown(cond);
  setDragEnabled(cond);
}
