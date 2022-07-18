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
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtGui/QCloseEvent>
#include <QtCore/QFileInfo>
#include <QtCore/QMutableListIterator>
#include <QtCore/QSettings>

#include <QtCore/QDebug>

#include "mainwindow.h"
#include "monitorview.h"
#include "projectview.h"
#include "settings.h"
#include "version.h"

MainWindow::MainWindow()
{
  QWidget *w = new QWidget;
  setCentralWidget(w);
  m_projectView = new ProjectView;
  w->setLayout(m_projectView);

  connect(m_projectView, SIGNAL(showStatusMessage(QString)), SLOT(showStatusBarMessage(QString)));

  createActions();
  createMenus();
  //createToolBars();

  readSettings();

  // set as empty the filename
  setCurrentFile("");

  showStatusBarMessage(tr("Ready."));

  // disable Save/Save As/Run action
  modify(false);
}


MainWindow::~MainWindow()
{
   qDebug() << "Bye!!!";
   delete m_projectMenu;
   delete m_settingsMenu;
   delete m_helpMenu;
   delete m_newAct;
   delete m_openAct;
   delete m_saveAct;
   delete m_saveAsAct;
   delete m_runProjectAct;
   delete m_exitAct;
   delete m_changeDirAct;
   delete m_aboutAct;
   delete m_aboutQtAct;
}


bool MainWindow::canContinue()
{
  if (isWindowModified())
  {
    int ret = QMessageBox::warning(this, "QRunner",
                tr("The project has been modified.\n"
                   "Do you want to save your changes?"),
                QMessageBox::Save | QMessageBox::Default,
                QMessageBox::Discard,
                QMessageBox::Cancel | QMessageBox::Escape);
    if (ret == QMessageBox::Save)
    {
      return save();
    }
    else if (ret == QMessageBox::Cancel)
      return false;
  }
  return true;
}


void MainWindow::newProject() // SLOT
{
  if (canContinue())
  {
    m_projectView->clearView();
    setCurrentFile("");

    // no project to run and save
    modify(false);
  }
}


void MainWindow::open() // SLOT
{
  if (canContinue())
  {
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Open Project"), QDir::homePath(), tr("QRunner files (*.qrprj)"));
    if (!filename.isEmpty())
      loadFile(filename);
  }
}


bool MainWindow::saveAs() // SLOT
{
  QString filename = QFileDialog::getSaveFileName(this,
    tr("Save Project"), QDir::homePath(), tr("QRunner Project (*.qrprj)"));
  if (filename.isEmpty())
    return false;

  return saveFile(filename);
}


bool MainWindow::save() // SLOT
{
  if (m_projectFilename.isEmpty())
    return saveAs();
  else
    return saveFile(m_projectFilename);
}


void MainWindow::about() // SLOT
{
  QMessageBox::about(this, tr("About QRunner"),
          tr("<h2>QRunner %1</h2>"
             "<p>Copyright &copy; 2007-2022 Giovanni Venturi</p>"
             "<p>QRunner let you execute/manage scripts and "
             "executable applications.</p>"
             "<p>Releasead under GPL license.</p>").arg(qrunnerVersion));
}


void MainWindow::openRecentFile() // SLOT
{
  if (canContinue())
  {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
      loadFile(action->data().toString());
  }
}


void MainWindow::activeRunning() // SLOT
{
  // no script is running
  m_runProjectAct->setDisabled(false);
  m_newAct->setDisabled(false);
  m_openAct->setDisabled(false);
  m_saveAct->setDisabled(false);
  m_saveAsAct->setDisabled(false);
  for (int i = 0; i < MaxRecentFiles; ++i)
    m_recentFileActions[i]->setDisabled(false);

  // executed all the scripts: activated "Running stuff"
  showStatusBarMessage(tr("Ready."));
}


void MainWindow::deactiveRunning() // SLOT
{
  m_runProjectAct->setDisabled(true);
  m_newAct->setDisabled(true);
  m_openAct->setDisabled(true);
  m_saveAct->setDisabled(true);
  m_saveAsAct->setDisabled(true);
  for ( int i = 0; i < MaxRecentFiles; ++i )
    m_recentFileActions[i]->setDisabled(true);

  // script are going to start: deactivate "Running stuff"
  showStatusBarMessage(tr("Executing scripts..."));
}


void MainWindow::modify(const bool& cond)
{
  setWindowModified(cond);
  m_saveAsAct->setDisabled(!cond);
  m_saveAct->setDisabled(!cond);
  m_runProjectAct->setDisabled(!cond);
}


void MainWindow::editSettings()
{
  Settings *settings = new Settings;
  settings->exec();
  delete settings;
}


bool MainWindow::saveFile(const QString &filename)
{
  QString saveName = filename;
  if (saveName.right(6) != ".qrprj")
    saveName += ".qrprj";
  if (!m_projectView->writeFile(saveName))
  {
    statusBar()->showMessage(tr("Saving canceled"), 2000);
    return false;
  }
  setCurrentFile(saveName);
  statusBar()->showMessage(tr("File saved"), 2000);
  return true;
}


void MainWindow::setCurrentFile(const QString &filename)
{
  m_projectFilename = filename;
  setWindowModified(false);

  QString showName = tr("Untitled");
  if (!m_projectFilename.isEmpty())
  {
    showName = strippedName( m_projectFilename );
    m_recentFiles.removeAll( m_projectFilename );
    m_recentFiles.prepend( m_projectFilename );
    updateRecentFileActions();
  }

  setWindowTitle(QString("%1[*] - %2").arg(showName, "QRunner"));
}


void MainWindow::updateRecentFileActions()
{
  QMutableStringListIterator i(m_recentFiles);
  while (i.hasNext())
  {
    if (!QFile::exists(i.next()))
      i.remove();
  }

  for (int j = 0; j < MaxRecentFiles; ++j)
  {
    if (j < m_recentFiles.count())
    {
      QString text = QString("&%1 %2")
        .arg(j + 1)
        .arg(strippedName(m_recentFiles[j]));
      m_recentFileActions[j]->setText(text);
      m_recentFileActions[j]->setData(m_recentFiles[j]);
      m_recentFileActions[j]->setVisible(true);
    }
    else
      m_recentFileActions[j]->setVisible(false);
  }
  m_separatorAction->setVisible(!m_recentFiles.isEmpty());
}


QString MainWindow::strippedName(const QString &fullFilename)
{
  return QFileInfo(fullFilename).fileName();
}


bool MainWindow::loadFile(const QString &filename)
{
  if (!m_projectView->readFile(filename))
  {
    statusBar()->showMessage(tr("Loading canceled"), 2000);
    return false;
  }
  setCurrentFile(filename);
  statusBar()->showMessage(tr("File loaded"), 2000);

  // now you can run a project
  m_runProjectAct->setDisabled(false);

  return true;
}


void MainWindow::createActions()
{
  m_newAct = new QAction(tr("&New"), this);
  m_newAct->setIcon(QIcon(":/images/new.png"));
  m_newAct->setShortcut(tr("Ctrl+N"));
  m_newAct->setStatusTip(tr("Create a new project"));
  connect(m_newAct, SIGNAL(triggered()), SLOT(newProject()));
  connect(m_projectView, SIGNAL(createNewProject()), SLOT(newProject()));

  m_openAct = new QAction(tr("&Open..."), this);
  m_openAct->setIcon(QIcon(":/images/open.png"));
  m_openAct->setShortcut(tr("Ctrl+O"));
  m_openAct->setStatusTip(tr("Open an existing project"));
  connect(m_openAct, SIGNAL(triggered()), SLOT(open()));
  connect(m_projectView, SIGNAL(openProject()), SLOT(open()));

  m_saveAct = new QAction(tr("&Save"), this);
  m_saveAct->setIcon(QIcon(":/images/save.png"));
  m_saveAct->setShortcut(tr("Ctrl+S"));
  m_saveAct->setStatusTip(tr("Save the project to disk"));
  connect(m_saveAct, SIGNAL(triggered()), SLOT(save()));
  connect(m_projectView, SIGNAL(saveProject()), SLOT(save()));

  m_saveAsAct = new QAction(tr("Sa&ve As..."), this);
  m_saveAsAct->setShortcut(tr("Ctrl+V"));
  m_saveAsAct->setStatusTip(tr("Save the project to disk as..."));
  connect(m_saveAsAct, SIGNAL(triggered()), SLOT(saveAs()));

  for (int i = 0; i < MaxRecentFiles; ++i)
  {
    m_recentFileActions[i] = new QAction(this);
    m_recentFileActions[i]->setVisible(false);
    connect(m_recentFileActions[i], SIGNAL(triggered()),
      SLOT(openRecentFile()));
  }

  m_runProjectAct = new QAction(tr("&Run all scripts folders"), this);
  m_runProjectAct->setShortcut(tr("Ctrl+R"));
  m_runProjectAct->setStatusTip(tr("Run all the project scripts folders"));

  // at startup there is no project that can be executed
  m_runProjectAct->setDisabled(true);

  connect(m_runProjectAct, SIGNAL(triggered()), m_projectView, SLOT(runScripts()));
  connect(m_projectView, SIGNAL(runningScript()), SLOT(deactiveRunning()));
  connect(m_projectView, SIGNAL(endedExecution()), SLOT(activeRunning()));

  m_exitAct = new QAction(tr("E&xit"), this);
  m_exitAct->setShortcut(tr("Ctrl+Q"));
  m_exitAct->setStatusTip(tr("Exit the application"));
  connect(m_exitAct, SIGNAL(triggered()), SLOT(close()));

  m_configAct = new QAction(tr("O&ptions..."), this);
  m_configAct->setShortcut(tr("Ctrl+P"));
  m_configAct->setStatusTip(tr("Change the QRunner settings"));
  connect(m_configAct, SIGNAL(triggered()), this, SLOT(editSettings()));

  m_changeDirAct = new QAction(tr("C&hange directory"), this);
  m_changeDirAct->setShortcut(tr("Ctrl+H"));
  m_changeDirAct->setStatusTip(tr("Change the selected directory to select script files"));
  connect(m_changeDirAct, SIGNAL(triggered()), m_projectView, SIGNAL(modifyDirectory()));

  m_aboutAct = new QAction(tr("&About"), this);
  m_aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  m_aboutQtAct = new QAction(tr("About &Qt"), this);
  m_aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
  connect(m_aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  // connect signal coming from the View
  connect(m_projectView, SIGNAL(modifiedView(bool)), SLOT(modify(bool)));
}


void MainWindow::createMenus()
{
  m_projectMenu = menuBar()->addMenu(tr("&Project"));
  m_projectMenu->addAction(m_newAct);
  m_projectMenu->addAction(m_openAct);
  m_projectMenu->addAction(m_saveAct);
  m_projectMenu->addAction(m_saveAsAct);

  // adding the recent opened project files
  m_separatorAction = m_projectMenu->addSeparator();
  for (int i = 0; i < MaxRecentFiles; ++i )
    m_projectMenu->addAction(m_recentFileActions[i]);

  m_projectMenu->addSeparator();
  m_projectMenu->addAction(m_runProjectAct);
  m_projectMenu->addSeparator();
  m_projectMenu->addAction(m_exitAct);

  m_settingsMenu = menuBar()->addMenu(tr("&Settings"));
  m_settingsMenu->addAction(m_changeDirAct);
  m_settingsMenu->addAction(m_configAct);

  m_helpMenu = menuBar()->addMenu(tr("&Help"));
  m_helpMenu->addAction(m_aboutAct);
  m_helpMenu->addAction(m_aboutQtAct);
}


void MainWindow::createToolBars()
{
  m_projectToolBar = addToolBar( tr("&Project") );
  m_projectToolBar->addAction(m_newAct);
  m_projectToolBar->addAction(m_openAct);
  m_projectToolBar->addAction(m_saveAct);
}


void MainWindow::readSettings()
{
  QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);

  QRect rect = settings.value("geometry", QRect(100, 100, 500, 400)).toRect();
  move(rect.topLeft());
  resize(rect.size());

  m_recentFiles = settings.value("recentFiles").toStringList();
  updateRecentFileActions();

  // assign the file system to the current directory
  m_projectView->setFileSystemPath(settings.value("fileSystemPath", QDir::currentPath()).toString());
}


void MainWindow::writeSettings()
{
  QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);

  settings.setValue("geometry", geometry());
  settings.setValue("recentFiles", m_recentFiles);
  settings.setValue("fileSystemPath", m_projectView->fileSystemPath());
}


void MainWindow::closeEvent(QCloseEvent *event)
{
  if (canContinue())
  {
    writeSettings();
    m_projectView->associatedMonitorView()->close();
    event->accept();
  }
  else
    event->ignore();
}


void MainWindow::showStatusBarMessage(const QString& msg) // SLOT
{
  statusBar()->showMessage(msg);
}
