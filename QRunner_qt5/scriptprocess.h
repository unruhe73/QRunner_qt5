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

#ifndef SCRIPTPROCESS_H
#define SCRIPTPROCESS_H

#include <QtCore/QProcess>
#include <QtCore/QList>
#include <QtCore/QFile>
#include <QtCore/QTemporaryFile>
#include <QtCore/QTextStream>

class TextEdit;
class TreeWidgetItem;
class QTreeWidgetItem;
class TextEditMonitor;

#include <QtCore/QPair>
#include <QtCore/QMap>

/**
 * This class let define and start scripts
 *
 * @author Giovanni Venturi
 */
class ScriptProcess : public QProcess
{
Q_OBJECT
  public:
    /**
     * You construct a Process Script releted to an item and using the
     * base directory as starting path for its log file
     *
     * @param tree is needed to navigate the project tree and to know the path to give to the log file
     * @param item is the item related to the script with all its properties
     * @param basedir is the base directory path where to write the log file
     */
    ScriptProcess(QList<QString> *tree, TreeWidgetItem *item, const QString &basedir);

    /**
     * start running the related process
     */
    void run();

    /**
     * @returns the temporary file used by Qt to store the temporary log
     *
     * You need it instead of using log file directly because you can run the
     * same project more times and in this case the temporary file report just
     * the log of the current running while the log file append the new log
     * messages to the previous log messages
     */
    QString tmpFile() const;

    /**
     * @returns the name of the script
     */
    QString name() const;

    /**
     * @returns the number of times the script has to be executed
     */
    int times() const;

    /**
     * Assign the TextEdit box to this script to show its temporary log messages
     */
    void assignConsole(TextEdit* outbox);

    /**
     * @returns true is the scipt is running
     */
    bool isRunning();

  private:

    /**
     * The script name
     */
    QString m_name;

    /**
     * The input line parameters
     */
    QString m_params;

    /**
     * The log file
     */
    QFile m_logfile;

    /**
     * The Qt temporary file handler
     */
    QTemporaryFile m_tmp;

    /**
     * The Qt temporary file stream
     */
    QTextStream m_tmpFile;

    /**
     * The log file stream
     */
    QTextStream m_outLog;

    /**
     * The return code of the executed script
     */
    int m_code;

    /**
     * The number of times the script has to be executed
     */
    int m_times;

    /**
     * The number of seconds the script has to delay before start again
     */
    int m_delay;

    /**
     * The number of times the script has been executed
     */
    int m_executedTimes;

    /**
     * The running script condition
     */
    bool m_running;

    /**
     * The reference for the TextEdit widget
     */
    TextEdit *m_outputBox;

    /**
     * The reference for the TextEditMonitor widget
     */
    TextEditMonitor *m_mvOut;

    /**
     * The QProcess status on exit
     */
    QProcess::ExitStatus m_status;

    /**
     * The QProcess status on error
     */
    QProcess::ProcessError m_err;

    /**
     * The PID of the process related to the executed script will running
     */
    Q_PID m_pid;

    /**
     * The enviroment variables related to the TreeWidgetItem that corresponds to the script
     */
    QMap<QTreeWidgetItem*, QPair<QString, QString> > m_environment;

  private slots:

    /**
     * Says what to do when the standard output channel gets data
     */
    void sentOutputText();

    /**
     * Says what to do when the standard error channel gets data
     */
    void sentErrorText();

    /**
     * Says what to do when the script has been executed
     */
    void scriptEnded(int code, QProcess::ExitStatus status);

    /**
     * Says what to do when the script returned some errors
     */
    void gotError(QProcess::ProcessError err);

    /**
     * Assign the started condition and the PID
     */
    void startedProcess();

    /**
     * Run again the same script when the number of times to run it is greater than 1
     */
    void runAgain();

  signals:

    /**
     * Emitted when script process ended correctly
     */
    void finishedOK(ScriptProcess*);

    /**
     * Emitted when script process ended not correctly
     */
    void finishedBad(ScriptProcess*);

    /**
     *  Emitted when script process start running
     */
    void running(ScriptProcess*);
};

#endif
