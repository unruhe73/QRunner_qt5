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

#ifndef SCRIPTTREE_H
#define SCRIPTTREE_H

#include <QtCore/QProcess>

#include <QtWidgets/QTreeWidget>

class QString;
class QMouseEvent;
class QDomDocument;
class QDomElement;
class QDomNode;
class TreeWidgetItem;
class TextEdit;
class ScriptQueue;
class ScriptProcess;

/**
 * This class expand the QTreeWidget to have a specialized tree widget that
 * let to save a project, let to load a project, ...
 *
 * @author Giovanni Venturi
 */
class ScriptTree : public QTreeWidget
{
  Q_OBJECT

  public:
    /**
     * Create the Tree Scrip associating the TextEdit line widget to it
     *
     * @param outputBox is the text line widget that specify the tree directory where
     * to get the scripts to add to the project (to the tree script)
     * @param parent is the parent of the widget
     */
    ScriptTree(TextEdit *outputBox, QWidget *parent=0);

    /**
     * Delete all the scripts from the Scripts Queue
     */
    ~ScriptTree();

    /**
     * @returns true if you can execute the file
     *
     * @param exe is the file to check
     */
    bool isExecutable(const QString &exe);

    /**
     * Prepare the tree widget to have a new project removing all the scripts
     * from the Scripts Queue, cleaning the view (removing all its widgets),
     * assigning 0 as first element of the tree widget and assigning "not
     * modified" attribute to the widget
     */
    void clean();

    /**
     * @returns true if the existing tree has been modified, if you have no tree
     * yet you will get false
     */
    bool modified();

    /**
     * Reset the 'modified' attribute if there is at least a widget in the tree
     */
    void resetModified();

    /**
     * Check recursively if a project sub tree is correct (has all the requested
     * attributes, their names are correct, ...)
     *
     * @returns true if the sub tree is correct and compliant to the specifics
     *
     * @param node is the QDomNode reference where start visiting the sub tree
     */
    bool checkSubprojectFile(QDomNode *node) const;

    /**
     * Check recursively if a project tree is correct (has all the requested
     * attributes, their names are correct, ...)
     *
     * @returns true if the project tree is correct and compliant to the specifics
     *
     * @param node is the QDomNode reference where start visiting the tree
     */
    bool checkProjectFile(const QString& filename) const;

    /**
     * @returns true if the project was loaded correctly
     *
     * @param filename is the name of the file to load
     */
    bool loadProject(const QString& filename);

    /**
     * @returns true if the project was saved correctly
     *
     * @param filename is the name of the file to save
     */
    bool saveProjectTree(const QString& filename);

    /**
     * Hide the related console where a script put its output/error
     */
    void hideConsole();

    /**
     * Set the enabled/disabled condition
     */
    void enableDND(const bool& cond = true);

    /**
     * The reference of the dragging Tree Widget
     *
     * @returns the reference of the dragging Tree Widget
     */
    TreeWidgetItem *draggingItem();

  private:
    /**
     * Add a file script to the script tree
     *
     * @param item is the refernce where to add the new script file
     * @param filename is the filename of the file
     * @param checked is true if the script is checked: you can execute the script when start running
     * @param absoluteFilePath is the absolute file path for the filename
     * @param times is the number of times the script has to be executed (default is 1)
     * @param delay is the number of seconds the script has to be delayed before to run again (default is 0)
     * @param params is the input parameters line for the script
     *
     * @returns the reference to the new TreeWidgetItem added
     */
    TreeWidgetItem *addFile( QTreeWidgetItem *item, const QString& filename, bool checked, const QString& absoluteFilePath, int times = 1, int delay = 0, const QString& params = "" );

    /**
     * Create a new sub tree to host a group/sub group
     *
     * @param item is the QTreeWidgetItem reference: the point when to add the element in the tree
     * @param name is the name of the script
     * @param checked is the checked condition that let you know if a script is enabled or disabled to run
     */
    void createNewSubGroup( QTreeWidgetItem *item, const QString& name, bool checked );

    /**
     * Save the project tree (usually sub tree: it's used into \ref saveProjectTree(QString) )
     *
     * @param top the QTreeWidgetItem reference where the tree begins
     * @param root the QDomElement reference where the XML structure begins
     */
    void saveProjectTree(QTreeWidgetItem *top, QDomElement *root);

    /**
     * Set the script color into the tree
     *
     * @param color is the tree script color: default is black
     */
    void setScriptTreeColor(const QString& color = "#000000"); // start from the root to set color to the project tree

    /**
     * Set the script color into the sub tree
     *
     * @param color is the sub tree script color: default is black
     */
    void setScriptSubtreeColor(QTreeWidgetItem *top, const QString& color = "#000000");

    /**
     * Check each item of the sub tree and append. It's needed QDomNode reference and QTreeWidgetItem
     * reference because if it's compliant to the specifics than add the node.
     * It's used into \ref loadProject(QString)
     *
     * @param node is the QDomNode reference where to begin parsing
     * @param item is the QTreeWidgetItem reference where it begins adding the item
     * if it's compliant to the specifics
     */
    void parseSubgroup(QDomNode *node, QTreeWidgetItem *item);

    /**
     * Add the scripts (visiting the tree) to the Script Queue
     *
     * @param item is the starting point reference into the tree
     */
    void addProjectSubTree(QTreeWidgetItem *item);

    /**
     * Show the console for the related process
     *
     * @param process is the Script Process that need to show its output/errors into the console
     */
    void showConsole(ScriptProcess* process);

  private:
    /**
     * The textbox on which to write the script output
     */
    TextEdit *m_outputBox;

    /**
     * The root of the scripts tree
     */
    TreeWidgetItem *m_root;

    /**
     * Contains the check state before changing it
     */
    Qt::CheckState m_statebefore;

    /**
     * Contains the midified condition. If it's added/removed items into/from the tree than it's changed to true.
     */
    bool m_modified;

    /**
     * Specify the position of the item to remove or to execute from/into the view
     */
    QPoint m_pointerPosition;

    /**
     * Specify the position of a starting drag event
     */
    QPoint m_dragStartPosition;

    /**
     * The QContextMenuEvent reference. It's needed from other members to execute RMB actions
     */
    QContextMenuEvent *m_event;

    /**
     * QDomDocument reference. It's needed in \ref saveProjectTree(QTreeWidgetItem,QDomElement)
     */
    QDomDocument *m_XMLProjectDoc;

    /**
     * Drag and drop condition. True if the drag and drop is enabled
     */
    bool m_DND;

    /**
     * true if the drop is happening local to the Script Tree widget and don't come from the File System Tree View one
     */
    bool m_localAction;

    /**
     * The data exchanged during drag and drop action
     */
    QString m_data;

    /**
     * The scripts queue reference
     */
    ScriptQueue *m_scriptQueue;

    /**
     * The related QProcess instance of the script/widget
     */
    ScriptProcess *m_relatedProcess;

    /**
     * The base directory where to save the log files
     */
    QString m_basedir;

    /**
     * The QProcess to see the the log file
     */
    QProcess *m_procShowLog;

    /**
     * The reference to the dragging Tree Widget
     */
    TreeWidgetItem *m_draggingItem;

  protected:
    /**
     * Reimplement the dragEnterEvent. It needs to set the drag and drop object to text/plain
     *
     * @param event is the QDragEnterEvent reference
     */
    void dragEnterEvent(QDragEnterEvent *event);

    /**
     * Reimplement the dragMoveEvent. It needs to understand if you could do the drop action.
     *
     * @param event is the QDragMoveEvent reference
     */
    void dragMoveEvent(QDragMoveEvent *event);

    /**
     * Reimplement the dropEvent. It needs to add/move the tree widget
     *
     * @param event is the QDropEvent reference
     */
    void dropEvent(QDropEvent *event);


    /**
     * Reimplement the mousePressEvent. It needs to store the starting drag position,
     * show/hide console if you clicke on a file/group.
     *
     * @param event is the QMouseEvent reference
     */
    void mousePressEvent(QMouseEvent *event);

    /**
     * Reimplement the mouseMoveEvent. It needs to understand if you have to start the drag action.
     *
     * @param event is the QDragEnterEvent reference
     */
    void mouseMoveEvent(QMouseEvent *event);

    /**
     * Reimplement the mouseReleaseEvent. It needs to understand if it has check or uncheck the item.
     *
     * @param event is the QMouseEvent reference
     */
    void mouseReleaseEvent(QMouseEvent *event);

    /**
     * Reimplement the contextMenuEvent. It needs to understand what context menu
     * has to show according to mouse pointer position.
     *
     * @param event is the QContextMenuEvent reference
     */
    void contextMenuEvent(QContextMenuEvent *event);

  private slots:
    /**
     * Show the file log related to the script according to mouse pointer position. \ref m_pointerPosition
     */
    void showLogFile();

    /**
     * Create a new Group in the tree project and then the user has to edit it assigning a name
     *
     * @param name is group name
     * @param checked is true if has to check the new item: deafult is true
     */
    void createNewGroup( const QString& name = "", bool checked = true );

    /**
     * Create a new Sub Group in the tree project and then the user has to edit it assigning a name
     *
     * @param name is sub group name
     * @param checked is true if has to check the new item: deafult is true
     */
    void createNewSubGroup( const QString& name = "", bool checked = true );

    /**
     * Remove the selected item form the tree
     */
    void deleteItem();

    /**
     * Execute the selected script
     */
    void runScript();

    /**
     * Stop the selected script
     */
    void stopScript();

    /**
     * Run again the script. The number of time a script has to be executed is greater that one
     */
    void runScriptAgain();

    /**
     * Deallocate the allocated QProcess
     */
    void finishedShowLog( int exitCode, QProcess::ExitStatus exitStatus );

  public slots:
    /**
     * Execute the whole scripts tree of the project
     */
    void runProjectTree();

    /**
     * set the not local drag and drop
     */
    void setExternalDND();

  signals:
    /**
     * Emitted when a new project has to be created
     */
    void newProject();

    /**
     * Emitted when a project has to be loaded
     */
    void loadProject();

    /**
     * Emitted when a project has to be saved
     */
    void saveProject();

    /**
     * Emitted when a project has changed/unchanged
     *
     * @param value is the modified condition
     */
    void modifiedProject(bool value = true);

    /**
     * Emitted when an item has been deleted
     */
    void deletedItem();

    /**
     * Emitted when all project scripts has been executed
     */
    void readyToRun();

    /**
     * Emitted when a script is going to start
     */
    void runningScript();

    /**
     * Emitted when a file has been selected
     *
     * @param item is the TreeWidgetItem reference has been selected
     */
    void fileSelected(TreeWidgetItem *item);

    /**
     * Emitted when a group has been selected
     */
    void groupSelected();

    /**
     * Emitted when no file and no group has been selected
     */
    void nothingSelected();

    /**
     * Emitted to display status messages in the main view
     */
    void showStatusMessage(QString);

    /**
     * Emitted when has been added an environment loading a project
     */
    void newEnvironmentItemAdded(TreeWidgetItem*);
};

#endif
