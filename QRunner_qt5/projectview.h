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

#ifndef PROJECTVIEW_H
#define PROJECTVIEW_H

#include <QtCore/QDir>
#include <QtWidgets/QVBoxLayout>

class LineEdit;
class QPushButton;
class FileSystemTreeView;
class ScriptTree;
class TreeWidgetItem;
class ScriptConf;
class QStackedWidget;
class TextEdit;
class MonitorView;
class TextEditMonitor;

/**
 * This class define the Project View that contains the main GUI widgets excluded status bar, menu bar
 *
 * @author Giovanni Venturi
 */
class ProjectView : public QVBoxLayout
{
  Q_OBJECT

  public:
    /**
     * Create the Project View creating the Tree File System View,
     * the Tree Scrip View and the Monitor View
     */
    ProjectView(QWidget *parent = 0);

    /**
     * Clear the View
     */
    void clearView();

    /**
     * Save the Project
     *
     * @param filename the file name of the project
     *
     * @returns true if it's been saved
     */
    bool writeFile(const QString &filename);

    /**
     * Load the Project
     *
     * @param filename the name of the Project to load
     *
     * @returns true if the project was loaded
     */
    bool readFile(const QString &filename);

    /**
     * @returns the file system path where you can drag and drop file script
     */
    QString fileSystemPath() const;

    /**
     * Set the file system path where you can drag and drop file script
     *
     * @param fs the file system path to set
     */
    void setFileSystemPath(const QString &fs);

    /**
     * It returns the associated MonitorView class
     */
    MonitorView *associatedMonitorView();

  private:
    /**
     * The widget where is showed the File System Path
     */
    LineEdit *m_lineEdit;

    /**
     * The button to get the upper directory in the File System Path
     */
    QPushButton *m_upButton;

    /**
     * The button to get back to the previous selected directory in the File System Path
     */
    QPushButton *m_backButton;

    /**
     * The widget that shows the File System Tree
     */
    FileSystemTreeView *m_treeDir;

    /**
     * The widget that shows the Scripts Tree
     */
    ScriptTree *m_scriptTree;

    /**
     * The configuration widget
     */
    ScriptConf *m_scriptConf;

    /**
     * The stack for TreeViewFileSystem and ScriptConf
     */
    QStackedWidget *m_stack;

    /**
     * Reference to the widget for the console script output
     */
    TextEdit *m_outputBox;

    /**
     * Store the previous directory path for the Tree File System
     */
    QDir m_oldDirectory;

    /**
     * Store the current directory path for the Tree File System
     */
    QDir m_currentDirectory;

    /**
     * Store modified condition of the Project
     */
    bool m_modified;

    /**
     * The Monitor View: you can see more script logs at the same time in a 2x2 grid
     */
    MonitorView *m_mv;

  private slots:
    /**
     * Called when you clik on the QButton Up near the Line Edit
     */
    void directoryUp();

    /**
     * Called when you clik on the QButton Back near the Line Edit
     */
    void directoryBack();

    /**
     * Execute all the scripts of the Project
     */
    void runScripts();

    /**
     * Execute the current script (under the mouse pointer) of the Project
     */
    void execScript();

    /**
     * Called when all the scripts of the Project has been executed
     */
    void executedScript();

    /**
     * Called to show the item configuration
     *
     * @param item is the widget to which the options parameters has to be shown
     */
    void showItemConfig(TreeWidgetItem *item);

    /**
     * Called to assign a monitor view with the related tree widget item script
     *
     * @param item is the widget related to the script
     * @param box is the monitor view text edit widget
     *
     */
    void assignMonitorView(TreeWidgetItem *item, TextEditMonitor *box);

  public slots:
    /**
     * Called when has to be shown the File System in the Project View
     */
    void showFileSystem();

    /**
     * Called when the directory path of the Tree File System ha sto be changed
     */
    void modifyingDirectory();

  signals:
    /**
     * Emitted when the Project has been changed
     */
    void modifiedView(bool);

    /**
     * Emitted when the directory path of the Tree File System has to be changed
     */
    void modifyDirectory();

    /**
     * Emitted when a script it's running
     */
    void runningScript();

    /**
     * Emitted when a script ended its execution
     */
    void endedExecution();

    // context menu signals
    /**
     * Emitted when you choose to create a new Project
     */
    void createNewProject();

    /**
     * Emitted when you choose to save the Project
     */
    void saveProject();

    /**
     * Emitted when you choose to load a Project
     */
    void openProject();

    /**
     * Emitted when has to be shown a new status message
     *
     * @param string is the string to be shown in the status message bar
     */
    void showStatusMessage(QString);
};

#endif
