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

#ifndef SCRIPTCONF_H
#define SCRIPTCONF_H

#include <QtWidgets/QGroupBox>

class QSpinBox;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;
class TreeWidgetItem;
class TextEditMonitor;

/**
 * This class is the QGroupBox that contains the widgets that is needed to configure the single items:
 *   - the number of times a script has to be executed
 *   - the environment (variable name + its value) inside the script has to be executed
 *   - the input parameters line the script will use
 *
 * This widget is shown when you click on a file in the Script Tree
 *
 * @author Giovanni Venturi
 */
class ScriptConf : public QGroupBox
{
  Q_OBJECT

  public:
    /**
     * Create the ScriptConf widget
     *
     * @param parent is the parent widget
     */
    ScriptConf(QWidget *parent = 0);

    /**
     * Deallocate the memory
     */
    ~ScriptConf();

    /**
     * Set the related item to configure
     *
     * @param item is the TreeWidgetItem reference
     */
    void setItem(TreeWidgetItem *item);

    /**
     * Set the related tree widget item to the monitor view into configuration
     *
     * @param item is the TreeWidgetItem reference
     * @param box is the TextEditMonitor reference
     */
    void setMonitorView(TreeWidgetItem *item, TextEditMonitor *box);

  private:
    /**
     * Set the Environment for the script. The QtreeWidgetItem lines of the "Environment" has to exixts.
     * This member just hide all the lines and show the ones related to @p item
     *
     * @param confEnv is the QTreeWidget containing:
     *   - the variable name
     *   - the variable value
     * @param item is the related TreeWidgetItem reference
     */
    void setEnvironment(QTreeWidget *confEnv, TreeWidgetItem *item);

  private:
    /**
     * The TreeWidgetItem on which you can operate the configuration
     */
    TreeWidgetItem *m_item;

    /**
     * Contains the number of time a script has to be executed
     */
    QSpinBox *m_runTimes;

    /**
     * Contains the number of seconds to delay before to run again the script
     */
    QSpinBox *m_delay;

    /**
     * Contains the environment (name + value)
     */
    QTreeWidget *m_confEnv;

    /**
     * Contains the input parameters line
     */
    QLineEdit* m_paramsLine;

    /**
     * Record each modification to the script properties/environment
     */
    bool m_recordModify;

  signals:
    /**
     * Emitted to say to the project view is some configuration value is changed.
     * The standard value has always to be TRUE: I need to specify a parameter
     * cause of compatibilty with the related mainwindow slot than needs anyway
     * a parameter that could be also FALSE
     */
    void modifiedProject(bool value = true);

  private slots:
    /**
     * Assign the number of times a script has to be executed
     *
     * @param value is the number of time the script has to be executed
     */
    void assignRunTimes(int value);
    
    /**
     * Assign the number of seconds a script has to delay before to be executed again
     *
     * @param value is the number of seconds the script has to be delayed before to run again
     */
    void assignDelayTime(int value);

    /**
     * Assign the environment:
     *  - name if @p column is 0
     *  - value if @p column is 1
     *
     * @param item is the QTreeWidgetItem reference in the QTreeWidget
     * @param column is the column selected: 0 for name, 1 for value
     */
    void assignEnvironment( QTreeWidgetItem* item, int column );

    /**
     * Assign the input line parameters
     */
    void assignParams();

    /**
     * Add the Environment line that user has to modify
     */
    void addEnvironmentVariable();

    /**
     * Remove the selected Environment line
     */
    void removeEnvironmentVariable();

    /**
     * Create the environment got from the @p item environment
     *
     * @param item the TreeWidgetItem reference containing the enviroment to add to the ScriptConf widget
     */
    void StoreEnvironment(TreeWidgetItem* item);
};

#endif
