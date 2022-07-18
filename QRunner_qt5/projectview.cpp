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

#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QFileDialog>
#include <QtCore/QEvent>

#include <QtCore/QDebug>

#include "projectview.h"
#include "scripttree.h"
#include "scriptconf.h"
#include "filesystemtreeview.h"
#include "lineedit.h"
#include "textedit.h"
#include "monitorview.h"


ProjectView::ProjectView(QWidget *parent)
  : QVBoxLayout(parent)
{
  m_modified = false;

  QHBoxLayout* hlayout1 = new QHBoxLayout;
  QLabel *dirlab = new QLabel(tr("selected directory:"));
  m_lineEdit = new LineEdit(this);
  m_lineEdit->setText(m_currentDirectory.absolutePath());
  m_lineEdit->setReadOnly(true);
  m_lineEdit->setToolTip(tr("<p>To change this text click here or press <b>CTRL + H</b> "
                             "key, or use the <i>Edit</i> menu."));
  m_upButton = new QPushButton(tr("&Up"));
  m_upButton->setToolTip(tr("<p>You can change the selected directory with the parent one.</p>"));
  m_backButton = new QPushButton( tr("&Back") );
  m_backButton->setToolTip(tr("<p>You can change the selected directory with the previous one.</p>"));
  hlayout1->addWidget(dirlab);
  hlayout1->addWidget(m_lineEdit);
  hlayout1->addWidget(m_upButton);
  hlayout1->addWidget(m_backButton);

  connect(m_upButton, SIGNAL(clicked()), SLOT(directoryUp()));
  connect(m_backButton, SIGNAL(clicked()), SLOT(directoryBack()));

  QHBoxLayout* hlayout2 = new QHBoxLayout;

  // console script output
  m_outputBox = new TextEdit;
  m_outputBox->setToolTip(tr("<p>In this area you will see the scripts output"));

  // left side view
  m_scriptTree = new ScriptTree(m_outputBox);

  // connect context menu item with some signals so to manage the call in the MainWindow
  connect(m_scriptTree, SIGNAL(newProject()), SIGNAL(createNewProject()));
  connect(m_scriptTree, SIGNAL(saveProject()), SIGNAL(saveProject()));
  connect(m_scriptTree, SIGNAL(loadProject()), SIGNAL(openProject()));

  connect(m_scriptTree, SIGNAL(modifiedProject(bool)), SIGNAL(modifiedView(bool)));
  connect(m_scriptTree, SIGNAL(deletedItem()), SLOT(showFileSystem()));

  // right side view
  m_treeDir = new FileSystemTreeView(m_currentDirectory.absolutePath());

  // start setting the external Drag & Drop
  connect(m_treeDir, SIGNAL(startDraging()), m_scriptTree, SLOT(setExternalDND()));
  m_scriptConf = new ScriptConf;

  hlayout2->addWidget(m_scriptTree);

  // now we stack the two autoexclusive widget
  m_stack = new QStackedWidget;
  m_stack->addWidget(m_treeDir);
  m_stack->addWidget(m_scriptConf);

  hlayout2->addWidget(m_stack);

  connect(m_scriptConf, SIGNAL(modifiedProject(bool)), SIGNAL(modifiedView(bool)));

  // main layout
  addLayout(hlayout1);
  addLayout(hlayout2);

  // console script output
  addWidget(m_outputBox);
  m_outputBox->hide();

  connect(m_scriptTree, SIGNAL(fileSelected(TreeWidgetItem*)), SLOT(showItemConfig(TreeWidgetItem*)));
  connect(m_scriptTree, SIGNAL(groupSelected()), SLOT(showFileSystem()));
  connect(m_scriptTree, SIGNAL(nothingSelected()), SLOT(showFileSystem()));
  connect(m_scriptTree, SIGNAL(readyToRun()), SLOT(executedScript()));

  // manage incoming signals
  connect(this, SIGNAL(modifyDirectory()), SLOT(modifyingDirectory()));
  connect(m_scriptTree, SIGNAL(runningScript()), SLOT(execScript()));
  connect(m_scriptTree, SIGNAL(showStatusMessage(QString)), SIGNAL(showStatusMessage(QString)));
  connect(m_scriptTree, SIGNAL(newEnvironmentItemAdded(TreeWidgetItem*)), m_scriptConf, SLOT(StoreEnvironment(TreeWidgetItem*)));

  m_mv = new MonitorView();
  m_mv->show();
  connect(m_mv, SIGNAL(droppedTreeWidgetItem(TreeWidgetItem*,TextEditMonitor*)), SLOT(assignMonitorView(TreeWidgetItem*,TextEditMonitor*)));
}


void ProjectView::clearView()
{
  m_modified = false;
  m_scriptTree->resetModified();
  m_scriptTree->clean();
  m_scriptTree->hideConsole();
  showFileSystem();
}


bool ProjectView::writeFile(const QString &filename)
{
  return m_scriptTree->saveProjectTree(filename);
}


bool ProjectView::readFile(const QString &filename)
{
  clearView();
  return m_scriptTree->loadProject(filename);
}


QString ProjectView::fileSystemPath() const
{
  return m_lineEdit->text();
}


void ProjectView::setFileSystemPath(const QString &fs)
{
  m_lineEdit->setText(fs);
  m_currentDirectory.setPath(fs);
  m_treeDir->changeDirectory(fs);
}


MonitorView *ProjectView::associatedMonitorView()
{
  return m_mv;
}


// SLOTS
void ProjectView::directoryUp() // SLOT
{
  m_oldDirectory.setPath(m_currentDirectory.absolutePath());
  m_currentDirectory.cdUp();
  m_lineEdit->setText(m_currentDirectory.absolutePath());
  m_treeDir->changeDirectory(m_currentDirectory.absolutePath());
  showFileSystem();
}


void ProjectView::directoryBack() // SLOT
{
  if (m_currentDirectory.absolutePath() == m_oldDirectory.absolutePath())
    return;
  m_currentDirectory.setPath(m_oldDirectory.absolutePath());
  m_lineEdit->setText(m_currentDirectory.absolutePath());
  m_treeDir->changeDirectory(m_currentDirectory.absolutePath());
  showFileSystem();
}


void ProjectView::runScripts() // SLOT
{
  m_scriptTree->runProjectTree();
}


void ProjectView::execScript() // SLOT
{
  // disable the DND for the trees
  m_scriptTree->enableDND(false);
  m_treeDir->enableDND(false);

  emit runningScript();
}


void ProjectView::executedScript() // SLOT
{
  // enable the DND for the trees
  m_scriptTree->enableDND();
  m_treeDir->enableDND();

  emit endedExecution();
}


void ProjectView::showItemConfig(TreeWidgetItem *item) // SLOT
{
  // it shows the configuration and hides the file system tree
  m_scriptConf->setItem(item);
  m_stack->setCurrentIndex(1);
  //statusBar()->showMessage(tr("Now you can configure the script options."));
}


void ProjectView::assignMonitorView(TreeWidgetItem *item, TextEditMonitor *box) // SLOT
{
  m_scriptConf->setMonitorView(item, box);
  //m_mv->assignScriptProcess(item, box);
}


void ProjectView::modifyingDirectory() // SLOT
{
  QFileDialog *dialog = new QFileDialog;

  //dialog->setOption(QFileDialog::ShowDirsOnly, true);
  dialog->setFileMode(QFileDialog::Directory);
  dialog->setDirectory(m_currentDirectory.absolutePath());
  dialog->setWindowTitle(tr("Choose Directory"));
  if (dialog->exec())
  {
    QString directory = dialog->selectedFiles().at(0);
    if (m_currentDirectory.absolutePath() != directory)
    {
      m_lineEdit->setText(directory);
      m_treeDir->changeDirectory(directory);
      m_oldDirectory.setPath(m_currentDirectory.absolutePath());
      m_currentDirectory.setPath(directory);
      //statusBar()->showMessage(tr("Directory changed."));
    }
/*    else
    statusBar()->showMessage(tr("Directory not changed anymore."));*/
  }
/*  else
  statusBar()->showMessage(tr("Directory not changed anymore."));*/
  delete dialog;
}


void ProjectView::showFileSystem()  // SLOT
{
  // shows the file system tree and hides the configuration
  m_stack->setCurrentIndex( 0 );
  emit showStatusMessage(tr("Ready."));
}
