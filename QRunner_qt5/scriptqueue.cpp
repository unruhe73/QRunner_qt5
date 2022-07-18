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

#include "scriptqueue.h"

#include <QtCore/QDebug>

ScriptQueue::ScriptQueue(QObject *parent)
   : QObject(parent)
{
  m_countRunning = 0;
  m_running = false;
}


void ScriptQueue::add(QList<QString> *list, TreeWidgetItem* item)
{
  ScriptProcess *script = new ScriptProcess(list, item, m_basedir);
  QueueItem *elem = new QueueItem(script, item);
  m_queue.push_back(elem);

  // connect the ScriptProcess...
  connect(script, SIGNAL(finishedOK(ScriptProcess*)), SLOT(executedOK(ScriptProcess*)));
  connect(script, SIGNAL(finishedBad(ScriptProcess*)), SLOT(executedBad(ScriptProcess*)));
  connect(script, SIGNAL(running(ScriptProcess*)), SLOT(running(ScriptProcess*)));
}


void ScriptQueue::assignBaseDir(const QString &basedir)
{
  m_basedir = basedir;
}


int ScriptQueue::countRunning() const
{
  return m_countRunning;
}


bool ScriptQueue::isRunning()
{
  return m_running;
}


void ScriptQueue::clear()
{
  int index = 0;
  while (index < m_queue.size())
  {
    // we don't want to remove the element from the queue, but just to access to it
    delete m_queue.at(index)->script();

    delete m_queue.at(index++);
  }
  m_queue.clear();
}


void ScriptQueue::run()
{
  int index = m_countRunning = 0;

  m_running = true;
  if (m_queue.isEmpty())
  {
    m_running = false;
    emit allScriptExecuted();
  }

  while (index < m_queue.size())
  {
    // we don't want to remove the element from the queue, but just to access to it
    m_script = m_queue.at(index)->script();
    m_queue.at(index++)->widget()->setRunning();

    m_countRunning++;
    m_script->run();
  }
}


void ScriptQueue::runLast()
{
  m_running = true;
  if (m_queue.isEmpty())
  {
    m_running = false;
    emit allScriptExecuted();
  }

  // add the item to the queue and start running it
  m_script = m_queue.last()->script();
  m_queue.last()->widget()->setRunning();

  m_script->run();
}


TreeWidgetItem *ScriptQueue::lookforScript(ScriptProcess* proc)
{
  int index = 0;
  bool found = false;
  TreeWidgetItem *elem = 0;

  while ((index < m_queue.size()) && (!found))
  {
    if (m_queue.at(index)->script() == proc)
    {
      found = true;
      elem = m_queue.at(index)->widget();
    }
    index++;
  }

  return elem;
}


ScriptProcess *ScriptQueue::lookforWidget(TreeWidgetItem* widget)
{
  int index = 0;
  bool found = false;
  ScriptProcess *elem = 0;

  while ((index < m_queue.size()) && (!found))
  {
    if (m_queue.at(index)->widget() == widget)
    {
      found = true;
      elem = m_queue.at(index)->script();
    }
    index++;
  }

  return elem;
}


bool ScriptQueue::isEmpty()
{
  return m_queue.isEmpty();
}


// SLOTS
void ScriptQueue::executedOK(ScriptProcess* proc)
{
  TreeWidgetItem *item;

  m_countRunning--;
  if ((item = lookforScript(proc)))
  {
    // the script finished the execution correctly
    item->setForeground(0, QBrush("#008000"));

    // now you can open the log file with the editor: script ended its execution
    item->setRunning(false);
    item->setExecuted();
  }

  if (m_countRunning == 0)
  {
    m_running = false;
    emit allScriptExecuted();
  }
}


void ScriptQueue::executedBad(ScriptProcess* proc)
{
  TreeWidgetItem *item;

  m_countRunning--;

  // something gone wrong during the execution
  if ((item = lookforScript(proc)))
  {
    // the script finished the execution badly
    item->setForeground(0, QBrush("#FF0000"));

    // now you can open the log file with the editor: script ended its execution
    item->setRunning(false);
    item->setExecuted();
  }

  if (m_countRunning == 0)
  {
    m_running = false;
    emit allScriptExecuted();
  }
}


void ScriptQueue::running(ScriptProcess* proc)
{
  TreeWidgetItem* item;

  if ((item = lookforScript(proc)))
    // the script started running
    item->setForeground(0, QBrush("#DC8600"));
}
