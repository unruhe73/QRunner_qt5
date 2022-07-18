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

#ifndef SCRIPTQUEUE_H
#define SCRIPTQUEUE_H

#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QQueue>

#include "scriptprocess.h"
#include "treewidgetitem.h"

/**
 * This class define an item for the scripts queue
 *
 * @author Giovanni Venturi
 */
class QueueItem
{
  public:
    /**
     * Create the item to insert into the script queue
     *
     * @param script is the script to insert in the scripts queue
     * @param widget is the related TreeWidgetItem of the script
     */
    QueueItem(ScriptProcess* script, TreeWidgetItem* widget)
      { m_scriptProcess = script; m_treeWidgetItem = widget; }

    /**
     * @returns the related Script Process reference
     */
    ScriptProcess* script() { return m_scriptProcess; }

    /**
     * @returns the related TreeWidgetItem reference
     */
    TreeWidgetItem* widget() { return m_treeWidgetItem; }

  private:
    /**
     * The Script Process reference
     */
    ScriptProcess* m_scriptProcess;

    /**
     * The TreeWidgetItem reference
     */
    TreeWidgetItem* m_treeWidgetItem;
};

/**
 * This class define a queue for the scripts, so when the running script process
 * starts this queue is accessed and the script inside the queue is enabled is executed
 *
 * @author Giovanni Venturi
 */
class ScriptQueue : public QObject
{
  Q_OBJECT

  public:
    /**
     * Create a Script Queue
     *
     * @param parent is the parent of the queue
     */
    ScriptQueue(QObject *parent= 0);

    /**
     * Add a list of scripts (a group)
     *
     * @param dir is the list of scripts to add to the queue
     * @param item is the group widget
     */
    void add(QList<QString> *dir, TreeWidgetItem* item);

    /**
     * Not yet used. It will be in future versions.
     *
     * @returns the number of process that are running in the queue
     */
    int countRunning() const;

    /**
     * @returns true is the queue is running the scripts processes
     */
    bool isRunning();

    /**
     * Assign the base directory path to save log files for ScriptProcess class
     */
    void assignBaseDir(const QString &basedir);

    /**
     * Remove all the scripts from the queue
     */
    void clear();

    /**
     * Start running the enabled process in the queue
     */
    void run();

    /**
     * Execute the last element added to the queue. Needed when you want to run just a script
     * and not all the queue
     */
    void runLast();

    /**
     * @returns the TreeWidgetItem reference if its related Script Process reference is in the queue else 0
     *
     * @param proc is the Script Process to look for into the queue
     */
    TreeWidgetItem *lookforScript(ScriptProcess* proc);

    /**
     * @returns the Script Process if its related TreeWidgetItem reference is in the queue else 0
     *
     * @param proc is the Script Process to look for into the queue
     */
    ScriptProcess *lookforWidget(TreeWidgetItem* widget);

    /**
     * @returns true is the queue is empty
     */
    bool isEmpty();

  private:
    /**
     * The generic Process script
     */
    ScriptProcess *m_script;

    /**
     * The queue of the scripts to execute
     */
    QList<QueueItem*> m_queue;

    /**
     * Assign the base directory path for the log files
     */
    QString m_basedir;

    /**
     * Total number of scripts that are running
     */
    int m_countRunning;

    /**
     * Running project condition
     */
    bool m_running;

  private slots:
    /**
     * Do some operations after the process has finished and it got
     * no error: change the script color in green
     *
     * @param proc the script process that has ended correctly
     */
    void executedOK(ScriptProcess* proc);

    /**
     * Do some operations after the process has finished and it got
     * some errors: change the script color in red
     *
     * @param proc the script process that has ended not correctly
     */
    void executedBad(ScriptProcess* proc);

    /**
     * Do some operations after the process has started: change
     * the script color in orange
     *
     * @param proc the script process that has started
     */
    void running(ScriptProcess* proc);

  signals:
    /**
     * Emitted when all scripts processes has been executed
     */
    void allScriptExecuted();
};

#endif
