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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtCore/QDir>

class QAction;
class ProjectView;

/**
 * This class is the main of the application it creates the status bar, the menu bar
 * the tree views, ... and manage their event
 *
 * @author Giovanni Venturi
 */
class MainWindow : public QMainWindow
{
  Q_OBJECT

  public:
    /**
     * Create the status bar, the menu bar, the tree views, ...
     */
    MainWindow();

    /**
     * Deallocate the memory
     */
    ~MainWindow();

    /**
     * Ask if save (and save then) or not the Project in case it's changed
     */
    bool canContinue();

  private slots:
    /**
     * Called when you want to create a new project
     */
    void newProject();

    /**
     * Called when you want to load a project
     */
    void open();

    /**
     * Called when you want to save a project
     */
    bool save();

    /**
     * Called when you want to save a project with a different name (o give it a name for the first time)
     */
    bool saveAs();

    /**
     * Called when you want to know about the QRunner author, ...
     */
    void about();


    /**
     * Called when you want to load a recent loaded project
     */
    void openRecentFile();

    /**
     * Enable some actions because QRunner has ended to execute scripts
     */
    void activeRunning();

    /**
     * Disable some actions because QRunner is going to execute scripts
     */
    void deactiveRunning();

    /**
     * The project has been modified: enable some kind of actions (save, save as)
     */
    void modify(const bool& cond);

    /**
     * Called to set some general options for the project
     */
    void editSettings();

  private:
    /**
     * Save the project as @p filename
     *
     * @param filename is the file name of the project you want to save
     */
    bool saveFile(const QString &filename);

    /**
     * Set the project name that will be saved at @p filename
     *
     * @param filename is the name of the current project (usually setted the first time you save it)
     */
    void setCurrentFile(const QString &filename);

    /**
     * Update the recent entries of the opened project in the menu
     */
    void updateRecentFileActions();

    /**
     * @returns the stripped name of @p fullFilename removing the file path
     *
     * @param fullFilename is a file name included the complete file path
     */
    QString strippedName(const QString &fullFilename);

    /**
     * @returns true if the project @p filename has been correctly loaded
     *
     * @param filename is the filename to load
     */
    bool loadFile(const QString &filename);

    /**
     * Create the actions (for the menu and for the context menu with
     * the right mouse button click)
     */
    void createActions();

    /**
     * Create the menus
     */
    void createMenus();

    /**
     * Create the tool bars
     */
    void createToolBars();

    /**
     * Read the QRunner general settings
     */
    void readSettings();

    /**
     * Write the QRunner general settings
     */
    void writeSettings();

  private:
    /**
     * The project view reference
     */
    ProjectView *m_projectView;

    /**
     * The project filename
     */
    QString m_projectFilename;

    /**
     * The maximum number of the file that you can load and consider as recent
     */
    enum { MaxRecentFiles = 5 };

    /**
     * The action for recent files
     */
    QAction *m_recentFileActions[MaxRecentFiles];

    /**
     * The separator action for recent files. It's needed
     * because you have to hide or show recent files as needed
     */
    QAction *m_separatorAction;

    /**
     * The list of recent loaded files
     */
    QStringList m_recentFiles;


    /**
     * The Project menu reference
     */
    QMenu *m_projectMenu;

    /**
     * The menu settings reference
     */
    QMenu *m_settingsMenu;

    /**
     * The menu help reference
     */
    QMenu *m_helpMenu;

    /**
     * The 'New Project' action
     */
    QAction *m_newAct;

    /**
     * The 'Load' action
     */
    QAction *m_openAct;

    /**
     * The 'Save' action
     */
    QAction *m_saveAct;

    /**
     * The 'Save as...' action
     */
    QAction *m_saveAsAct;

    /**
     * The 'Run all scripts folders' action
     */
    QAction *m_runProjectAct;

    /**
     * The 'Exit' action
     */
    QAction *m_exitAct;

    /**
     * The 'Change directory' action
     */
    QAction *m_changeDirAct;

    /**
     * The 'Settings...' action
     */
    QAction *m_configAct;

    /**
     * The 'About' action
     */
    QAction *m_aboutAct;

    /**
     * The 'About Qt' action
     */
    QAction *m_aboutQtAct;

    /**
     * The toolbar reference
     */
    QToolBar *m_projectToolBar;

  protected:
    /**
     * Reimplement closeEvent to be sure to save project if modified and to save the general settings
     */
    virtual void closeEvent(QCloseEvent *event);

  signals:
    /**
     * It's emitted when the project run
     */
    void runProject();

  public slots:
    /**
     * Send a messagge to the status bar
     *
     * @param msg is the message to send to the status bar
     */
    void showStatusBarMessage(const QString& msg);
};

#endif
