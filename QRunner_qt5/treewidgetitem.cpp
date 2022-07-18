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

#include <QtCore/QFileInfo>
#include <QtCore/QDebug>

#include <QtWidgets/QFileIconProvider>

#include "treewidgetitem.h"

// name is the absolute path filename if the item will be declared as a file
TreeWidgetItem::TreeWidgetItem(const QString& name, bool checked)
{
  setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

  // the item is default checked, but you can specify a different default value (false)
  if (checked)
    setCheckState(0, Qt::Checked);
  m_checked = checked;

  // item not yet declared
  m_type = None;

  m_assignedName = name;
  setText(0, m_assignedName);

  // never executed, just created
  m_executed = false;
  m_running = false;

  // no monitor view enabled
  m_textEditMonitor = 0;
}


void TreeWidgetItem::setGroup()
{
  m_type = Group;
  setText(0, m_assignedName);
  setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable |
    Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
  QFileIconProvider pix;
  setIcon(0, QIcon(pix.icon(QFileIconProvider::Folder)));

  // it's a group than you cannot run the script
  m_times = 0;
  m_delay = 0;
}


void TreeWidgetItem::setFile(int times, int delay)
{
  m_type = File;
  QFileInfo file(m_assignedName);
  m_name = file.fileName();
  m_path = file.path();
  setForeground(0, QBrush("#000000"));
  setText(0, m_name);
  setToolTip(0, m_assignedName);
  setIcon(0, QIcon(":/images/new.png"));

  // Assign the default values (for the spinbox)
  m_times = times;
  m_delay = delay;
}


bool TreeWidgetItem::isGroup()
{
  return (m_type == Group);
}


bool TreeWidgetItem::isFile()
{
  return (m_type == File);
}


void TreeWidgetItem::setFileName(const QString& name)
{
  m_name = name;
}


QString TreeWidgetItem::fileName() const
{
  return m_name;
}


QString TreeWidgetItem::assignedName() const
{
  return m_assignedName;
}


QString TreeWidgetItem::filePath() const
{
  return m_path;
}


QString TreeWidgetItem::name() const
{
  return text(0);
}


void TreeWidgetItem::setParameters(const QString& params)
{
  m_parameters = params;
}


QString TreeWidgetItem::parameters() const
{
  return m_parameters;
}


void TreeWidgetItem::setChecked(bool checked)
{
  if (checked)
    setCheckState(0, Qt::Checked);
  else
    setCheckState(0, Qt::Unchecked);
  m_checked = checked;
}


bool TreeWidgetItem::checked()
{
  return m_checked;
}


void TreeWidgetItem::setTimes(int times)
{
  m_times = times;
}


void TreeWidgetItem::setDelay(int time)
{
  m_delay = time;
}


void TreeWidgetItem::setTextEditMonitor(TextEditMonitor *box)
{
  m_textEditMonitor = box;
}


int TreeWidgetItem::times() const
{
  return m_times;
}


int TreeWidgetItem::delay() const
{
  return m_delay;
}


TextEditMonitor *TreeWidgetItem::textEditMonitor() const
{
  return m_textEditMonitor;
}


void TreeWidgetItem::setRunning(const bool& cond)
{
  m_running = cond;
}


bool TreeWidgetItem::running() const
{
  return m_running;
}


void TreeWidgetItem::setExecuted(const bool& cond)
{
  m_executed = cond;
}


bool TreeWidgetItem::executed() const
{
  return m_executed;
}


void TreeWidgetItem::createEnvironmentItem(const QString& name, const QString& value)
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText(0, name);
  item->setText(1, value);
  item->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  m_environment[item] = qMakePair(name, value);
}


void TreeWidgetItem::addEnvironmentItem(QTreeWidgetItem* item, const QString& name, const QString& value)
{
  bool find = false;
  QMapIterator<QTreeWidgetItem*, QPair<QString, QString> > i(m_environment);

  while (i.hasNext() && !find)
  {
    i.next();
    find = (item->text(0) == i.value().first);
  }

  if (!find)
    m_environment[item] = qMakePair(name, value);
  else
    // script already present
    delete item;
}


void TreeWidgetItem::changeEnvironmentItem(QTreeWidgetItem* item, const int& column, const QString& text)
{
  bool find = false;

  if (column == 0)
  {
    QMapIterator<QTreeWidgetItem*, QPair<QString, QString> > i(m_environment);
    while (i.hasNext() && !find)
    {
      i.next();
      find = (text == i.value().first);
    }
  }

  if (!find)
  {
    QString name = item->text(0);
    QString value = item->text(1);
    m_environment.remove( item );
    if (column == 0)
      name = text;
    else
      value = text;
    m_environment[item] = qMakePair(name, value);
    item->setText(0, name);
    item->setText(1, value);
  }
  else
  {
    m_environment.remove( item );
    delete item;
  }
}


void TreeWidgetItem::deleteEnvironmentItem(QTreeWidgetItem* item)
{
  m_environment.remove( item );
}


QMap<QTreeWidgetItem*, QPair<QString, QString>> TreeWidgetItem::environment()
{
  return m_environment;
}
