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

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>

#include <QtCore/QDebug>

#include "treewidgetitem.h"
#include "scriptconf.h"

ScriptConf::ScriptConf(QWidget *parent)
  : QGroupBox(parent)
{
  // group box for advanced options on dropped files
  setTitle(tr("Script Options"));
  QVBoxLayout* confOptionLayout = new QVBoxLayout;

  QHBoxLayout* confHoriz1 = new QHBoxLayout;
  QLabel *confLabel1 = new QLabel(tr("repeat script:"));
  m_runTimes = new QSpinBox;

  // run the script at least one time
  m_runTimes->setMinimum( 1 );
  connect(m_runTimes, SIGNAL(valueChanged(int)), SLOT(assignRunTimes(int)));

  QLabel *confLabel2 = new QLabel(tr("times"));
  confHoriz1->addWidget(confLabel1);
  confHoriz1->addWidget(m_runTimes);
  confHoriz1->addWidget(confLabel2);

  confOptionLayout->addLayout(confHoriz1);

  QHBoxLayout* confDelayOptionHLayout = new QHBoxLayout;
  QLabel *confDelayLabel = new QLabel(tr("delay before repeat:"));
  m_delay = new QSpinBox;

  // delay before to re run again the script 0 seconds
  m_delay->setMinimum( 0 );
  connect(m_delay, SIGNAL(valueChanged(int)), SLOT(assignDelayTime(int)));

  QLabel *confSecondsLabel = new QLabel(tr("seconds"));
  confDelayOptionHLayout->addWidget(confDelayLabel);
  confDelayOptionHLayout->addWidget(m_delay);
  confDelayOptionHLayout->addWidget(confSecondsLabel);

  confOptionLayout->addLayout(confDelayOptionHLayout);

  QLabel *confLabel3 = new QLabel(tr("Here you can define the script environment variables:"));
  confOptionLayout->addWidget(confLabel3);

  QHBoxLayout* confHoriz2 = new QHBoxLayout;
  QPushButton *buttonAdd = new QPushButton(tr("Add variable"));
  QPushButton *buttonRemove = new QPushButton(tr("Remove variable"));
  connect( buttonAdd, SIGNAL(clicked()), this, SLOT(addEnvironmentVariable()));
  connect( buttonRemove, SIGNAL(clicked()), this, SLOT(removeEnvironmentVariable()));

  m_confEnv = new QTreeWidget(this);
  connect(m_confEnv, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(assignEnvironment(QTreeWidgetItem*,int)));
  QStringList headers;
  headers << tr("Variable Name");
  headers << tr("Value");
  m_confEnv->setHeaderLabels( headers );
  m_confEnv->setColumnCount( 2 );
  confHoriz2->addWidget( m_confEnv );

  QVBoxLayout* confVert = new QVBoxLayout;
  confVert->addWidget( buttonAdd );
  confVert->addWidget( buttonRemove );
  confVert->addStretch();

  confHoriz2->addLayout( confVert );
  confOptionLayout->addLayout( confHoriz2 );

  QHBoxLayout* confHoriz3 = new QHBoxLayout;
  m_paramsLine = new QLineEdit;
  connect(m_paramsLine, SIGNAL(editingFinished()), SLOT(assignParams()));
  m_paramsLine->setToolTip( tr("Specify here the input parameter line.") );
  QLabel* paramsLabel = new QLabel( tr("Command Line Parameters:") );
  confHoriz3->addWidget(paramsLabel);
  confHoriz3->addWidget(m_paramsLine);
  confOptionLayout->addLayout(confHoriz3);

  confOptionLayout->addStretch();

  setLayout(confOptionLayout);

  m_item = 0;
}


ScriptConf::~ScriptConf()
{
  delete m_runTimes;
  delete m_paramsLine;
}


void ScriptConf::setItem(TreeWidgetItem *item)
{
  if (item)
  {
    // I'm changing the item, so no sense to let enable the "modification project"
    // property. The real changes that has to be stored is when a script File
    // properties is going to change (you modify the delay value or the repeat times one, ...)
    m_recordModify = false;

    m_item = item;
    m_runTimes->setValue(m_item->times());
    m_delay->setValue(m_item->delay());

    setEnvironment(m_confEnv, m_item);

    m_paramsLine->setText(m_item->parameters());

    // are we sure that the 2 changes are done before the assignment?
    m_recordModify = true;
  }
}


void ScriptConf::setMonitorView(TreeWidgetItem *item, TextEditMonitor *box)
{
  if (item)
  {
    item->setTextEditMonitor( box );
  }
}


void ScriptConf::setEnvironment(QTreeWidget *confEnv, TreeWidgetItem *item)
{
  for (int i = 0; i < confEnv->topLevelItemCount(); i++)
    // hide all unwanted item: we changed the item
    confEnv->topLevelItem(i)->setHidden(true);

  QMapIterator<QTreeWidgetItem*, QPair<QString, QString> > iterator(item->environment());
  while (iterator.hasNext())
  {
    iterator.next();

    // show the related item for the environment
    iterator.key()->setHidden(false);
  }
}


// SLOTS

void ScriptConf::assignRunTimes(int value) // SLOT
{
  if (m_item)
  {
    qDebug() << "assignRunTimes(" << value << ")";
    m_item->setTimes( value );
    if (m_recordModify)
      emit modifiedProject();
  }
}


void ScriptConf::assignDelayTime(int value) // SLOT
{
  if (m_item)
  {
    qDebug() << "assignDelayTime(" << value << ")";
    m_item->setDelay( value );
    if (m_recordModify)
      emit modifiedProject();

    // enable again the changes storing: I have to enable it just in one method
    //  because for each QSpinBox value changed (for each widget value change, generally)
    //  I get a set value call (assignRunTimes(int) and assignDelayTime(int) in this case)
    //  infact I have to assign the delay time and the repeat times. Enabling on each
    //  setting value method I get a "project changed" event that is wrong.
    //m_recordModify = true;
  }
}


void ScriptConf::assignEnvironment( QTreeWidgetItem* item, int column ) // SLOT
{
  m_item->changeEnvironmentItem(item, column, item->text(column));
}


void ScriptConf::assignParams() // SLOT
{
  m_item->setParameters(m_paramsLine->text().trimmed());
  emit modifiedProject();
}


void ScriptConf::addEnvironmentVariable() // SLOT
{
  QStringList list;
  list << tr("name") << tr("value");
  QTreeWidgetItem* item = new QTreeWidgetItem(list);
  item->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  m_confEnv->addTopLevelItem(item);
  m_confEnv->editItem(item);

  m_item->addEnvironmentItem(item, item->text(0), item->text(1));

  // the project need to be saved again after this modify
  emit modifiedProject();
}


void ScriptConf::removeEnvironmentVariable() // SLOT
{
  // delete the selected environmente variable
  m_item->deleteEnvironmentItem( m_confEnv->selectedItems().at(0) );

  // delete item from the QTreeWidget
  delete m_confEnv->selectedItems().at(0);

  // the project need to be saved again after this modify
  emit modifiedProject();
}


void ScriptConf::StoreEnvironment(TreeWidgetItem* item) // SLOT
{
  QMapIterator<QTreeWidgetItem*, QPair<QString, QString> > iterator(item->environment());
  while (iterator.hasNext())
  {
    iterator.next();

    // prepare the environment
    m_confEnv->addTopLevelItem(iterator.key());
  }
}
