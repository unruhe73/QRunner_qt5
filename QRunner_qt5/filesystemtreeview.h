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

#ifndef FILESYSTEMTREEVIEW_H
#define FILESYSTEMTREEVIEW_H

#include <QtWidgets/QTreeView>
#include <QtCore/QFileSystemWatcher>

class QFileSystemModel;
class QPoint;

/**
 * This class provide the widget for the File System Tree
 *
 * @author Giovanni Venturi
 */
class FileSystemTreeView : public QTreeView
{
  Q_OBJECT

  public:
    /**
     * Create the widget assigning the path to show
     *
     * @param directory is the path of the directory to show in the widget
     * @param parent is the widget parent
     */
    FileSystemTreeView(const QString & directory, QWidget *parent=0);

  private:
    /**
     * The directory model for the view
     */
    QFileSystemModel *m_dirModel;

    /**
     * The starting position of the mouse when start the drag & drop
     */
    QPoint m_dragStartPosition;

    /**
     * The directory showed by the widget in the tree
     */
    QString m_directory;

    /**
     * The data used in the drag and drop: it contains the
     * full path of the selected file/directory
     */
    QString m_data;

    /**
     * True if the drag and drop is enables
     */
    bool m_DND;

    /**
     * Let keep under control the showed path if some file/directory
     * is added or removed so to update the tree view
     */
    QFileSystemWatcher m_path;

  public:
    /**
     * Assign a directory to show in the tree widget
     *
     * @param directory is the directory to show in the tree widget
     */
    void changeDirectory(const QString & directory);

    /**
     * Enable/disable the drag and drop
     *
     * @param cond enable/disable drag and drop for the widget if cond is true/false
     */
    void enableDND(const bool& cond = true);

  protected:
    /**
     * Reimplement the mousePressEvent action
     *
     * @param event is the QMouseEvent data
     */
    void mousePressEvent(QMouseEvent *event);

    /**
     * Reimplement mouseMoveEvent
     *
     * @param event is the QMouseEvent data
     */
    void mouseMoveEvent(QMouseEvent *event);

  signals:
    /**
     * Emitted when you start dragging
     */
    void startDraging();
};

#endif
