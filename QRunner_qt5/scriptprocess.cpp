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

#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtCore/QDebug>

#include <QtWidgets/QMessageBox>

#include "scriptprocess.h"
#include "treewidgetitem.h"
#include "textedit.h"
#include "texteditmonitor.h"

ScriptProcess::ScriptProcess(QList<QString> *tree, TreeWidgetItem *item, const QString &basedir)
 : QProcess()
{
  m_times = item->times();
  m_delay = item->delay();
  m_name = item->assignedName();
  m_params = item->parameters();
  m_environment = item->environment();

  qDebug() << "execution of: '" << m_name << "'";

  connect(this, SIGNAL(readyReadStandardOutput()), SLOT(sentOutputText()));
  connect(this, SIGNAL(readyReadStandardError()), SLOT(sentErrorText()));
  connect(this, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(scriptEnded(int,QProcess::ExitStatus)));
  connect(this, SIGNAL(error(QProcess::ProcessError)), SLOT(gotError(QProcess::ProcessError)));
  connect(this, SIGNAL(started()), SLOT(startedProcess()));

  m_mvOut = item->textEditMonitor();

  QDir file(m_name);
  QString str(basedir + "/");
  while (!tree->isEmpty())
  {
    str += tree->takeLast() + "/";
  }
  if (!file.mkpath(str))
    qDebug() << "cannot create" << str;
  m_logfile.setFileName(str + file.dirName() + ".log");
  m_running = false;
  if (!m_tmp.open())
    qDebug() << "cannot open in writing the temporary file:" << m_tmp.fileName();
  m_tmp.setAutoRemove( true );
  m_tmpFile.setDevice(&m_tmp);
  m_outputBox = 0;
}


void ScriptProcess::run()
{
  emit running(this);

  // check if log file's writeble
  if (!m_logfile.open(QIODevice::Append | QIODevice::Text))
  {
    qDebug() << "cannot create file in writing: " << m_logfile.fileName();
    emit finishedBad(this);
    return;
  }

  m_executedTimes = 1;
  m_outLog.setDevice(&m_logfile);
  qDebug() << "executing #" << m_executedTimes << " of " << m_times << " " << m_name;
  if (m_times > 1)
  {
    m_outLog << "executing #1 of " << m_times << "\n\n";
    m_outLog.flush();
    m_tmpFile << "executing #1 of " << m_times << "\n\n";
    m_tmpFile.flush();
  }

  QStringList env;
  QMapIterator<QTreeWidgetItem*, QPair<QString, QString> > iterator(m_environment);
  while (iterator.hasNext())
  {
    iterator.next();

    // prepare the environment
    env << iterator.value().first + "=" + iterator.value().second;
  }
  setEnvironment(env);

#ifdef Q_OS_WIN
  start("cmd /C \"" + m_name + "\" " + m_params);
#else
  QStringList params = m_params.split(' ');
  start(m_name, params);
#endif
}


QString ScriptProcess::tmpFile() const
{
  return m_tmp.fileName();
}


QString ScriptProcess::name() const
{
  return m_name;
}


int ScriptProcess::times() const
{
  return m_times;
}


void ScriptProcess::assignConsole(TextEdit* outbox)
{
  m_outputBox = outbox;
}


bool ScriptProcess::isRunning()
{
  return m_running;
}


void ScriptProcess::runAgain()
{
  // run again a script: m_times > 1
  m_executedTimes++;
  qDebug() << "executing #" << m_executedTimes << " of " << m_times << " " << m_name;
  m_outLog << "\n\nexecuting #" << m_executedTimes << " of " << m_times << "\n\n";
  m_outLog.flush();
  m_tmpFile << "\n\nexecuting #" << m_executedTimes << " of " << m_times << "\n\n";
  m_tmpFile.flush();

  QStringList env;
  QMapIterator<QTreeWidgetItem*, QPair<QString, QString> > iterator(m_environment);
  while (iterator.hasNext())
  {
    iterator.next();

    // prepare the environment
    env << iterator.value().first + "=" + iterator.value().second;
  }
  setEnvironment(env);

#ifdef Q_OS_WIN
  start("cmd /C \"" + m_name + "\" " + m_params);
#else
  QStringList params = m_params.split(' ');
  start(m_name, params);
#endif
}


void ScriptProcess::sentOutputText() // SLOT
{
  QByteArray newData = readAllStandardOutput();
#ifdef Q_OS_WIN
  QString textToWrite = QString::fromLatin1(newData);
#else
  QString textToWrite = QString::fromLocal8Bit(newData);
#endif

  m_outLog << textToWrite;
  m_outLog.flush();
  m_tmpFile << textToWrite;
  m_tmpFile.flush();

  if (m_outputBox)
    m_outputBox->append(textToWrite);

  if (m_mvOut != 0)
    m_mvOut->append(textToWrite);
}


void ScriptProcess::sentErrorText() // SLOT
{
  QByteArray newData = readAllStandardError();
#ifdef Q_OS_WIN
  QString textToWrite = QString::fromLatin1(newData);
#else
  QString textToWrite = QString::fromLocal8Bit(newData);
#endif

  m_outLog << textToWrite;
  m_outLog.flush();
  m_tmpFile << textToWrite;
  m_tmpFile.flush();

  if (textToWrite.at(textToWrite.length() - 1) == '\n')
    textToWrite.resize(textToWrite.length() - 1);
  if (m_outputBox)
    m_outputBox->append(textToWrite);

  if (m_mvOut != 0)
    m_mvOut->append(textToWrite);
}


void ScriptProcess::scriptEnded(int code, QProcess::ExitStatus status) // SLOT
{
  m_status = status;
  m_code = code;
  if (m_times > m_executedTimes)
  {
    // delay if requested and more then one run
    if (m_delay > 0)
      // you should delay m_times seconds, but you cannot freeze the GUI,
      //  so demand it to the timer timeout
      QTimer::singleShot( 1000 * m_delay, this, SLOT(runAgain()) );
    else
      runAgain();
  }
  else
  {
    m_running = false;
    m_logfile.close();
    switch (status)
    {
      case QProcess::NormalExit:
        // the script finished normally
        emit finishedOK(this);
        break;
      case QProcess::CrashExit:
        // the script finished with a crash
        emit finishedBad(this);
        break;
    }
  }

  // reset its pid
  m_pid = Q_PID(NULL);
}


void ScriptProcess::gotError(QProcess::ProcessError err) //SLOT
{
/*
  switch (err)
  {
    case QProcess::FailedToStart:
      //emit availableText( "<font color=\"red\">" + m_process->name() + " failed to start</font>");
      break;
    case QProcess::Crashed:
      //emit availableText( "<font color=\"red\">" + m_process->name() + " crashed</font>");
      break;
    default:
      //emit availableText( "<font color=\"red\">" + m_process->name() + " had error</font>");
      break;
  }
*/
  m_err = err;
}


void ScriptProcess::startedProcess()  // SLOT
{
  if (state() == QProcess::Running)
  {
    m_running = true;

    // the script is running with PID: m_pid
    m_pid = processId();
  }
  else
    // not running: reset PID
    m_pid = Q_PID(NULL);
}
