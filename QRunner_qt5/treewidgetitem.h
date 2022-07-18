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

#ifndef TREEWIDGETITEM_H
#define TREEWIDGETITEM_H

#include <QtCore/QMap>
#include <QtCore/QPair>

#include <QtWidgets/QTreeWidgetItem>

class TextEditMonitor;

enum ItemType {None, Group, File};

/**
 * This class is derived from QTreeWidgetItem because it adds
 * some special features to the item. You can define file and
 * group item that will be presented as document and folder
 * respectively and you add to the item its features as
 * number of time to repeat the script, the environment, the
 * input parameters line, ...
 *
 * @author Giovanni Venturi
 */
class TreeWidgetItem : public QTreeWidgetItem
{
  public:
    /**
     * Create the item assigning a name e deciding if enable it or not
     *
     * @param name The name of the item in the tree
     * @param checked True if the group/file can be executed
     */
    TreeWidgetItem(const QString& name = "", bool checked = true);

    /**
     * set the item as a Group: it will be displayed with a folder icon
     */
    void setGroup();

    /**
     * set the item as a File: it will be displayed with a document icon.
     *  The default values are used when the file is created with a drag and drop
     *  from the file system tree view to the project folder view
     *
     * @param time is the number of time the script File has to be executed (the default is 1 time)
     * @param delay is the number of seconds the script File has to delay before run again
     *   (the default is 0 seconds)
     */
    void setFile(int times = 1, int delay = 0);

    /**
     * @returns true if the item is a Group
     */
    bool isGroup();

    /**
     * @returns true if the item is a File
     */
    bool isFile();

    /**
     * @returns The assigned complete name:
     *   - Group Name if the item is a Group
     *   - Absolute File Path if the item is a File
     */
    QString assignedName() const;

    /**
     * Assign the file name
     *
     * @param name is the file name
     */
    void setFileName(const QString& name);

    /**
     * @returns file name without the path if the item is a File
     */
    QString fileName() const;

    /**
     * @returns path of the file if the item is a File
     */
    QString filePath() const;

    /**
     * @returns the item name in the tree
     */
    QString name() const;

    /**
     * Assign the input parameters line for the script.
     * The item is a File in this case.
     */
    void setParameters(const QString& params);

    /**
     * @returns the input parameters line
     */
    QString parameters() const;

    /**
     * Check or remove check on the item
     *
     * @param checked the default value is true: the File/Group will be executed
     */
    void setChecked(bool checked = true);

    /**
     * @returns true if the item is checked: the File/Group will be executed
     */
    bool checked();

    /**
     * Set the number of times the script has to be executed if it can be executed
     *
     * @param times the number of times to exec a script File
     */
    void setTimes(int times);

    /**
     * Set the number of seconds the script has to be delayed before to run again
     *
     * @param times the number of seconds to delay before to exec a script File again
     */
    void setDelay(int time);

    /**
     * Associate the Monitor View with the script related to this widget
     *
     * @param box is the TextEditMonitor reference
     */
    void setTextEditMonitor(TextEditMonitor *box);

    /**
     * @returns times the number of times to exec a script File
     */
    int times() const;

    /**
     * @returns seconds of time to delay before exec a script File again
     */
    int delay() const;

    /**
     * @return the reference to the TextEditMonitor if available (!= 0)
     */
    TextEditMonitor *textEditMonitor() const;

    /**
     * Assign the running state for the related script of the widget
     *
     * @param cond is true is the script is running
     */
    void setRunning(const bool& cond = true);

    /**
     * @returns true if the script is running
     */
    bool running() const;

    /**
     * Assign the executed script condition.
     *
     * @param cond is true if the script has been executed
     */
    void setExecuted(const bool& cond = true);

    /**
     * @returns true if the script was executed
     */
    bool executed() const;

    /**
     * Create a new environment item (name + value) in the Environment widget ScriptConf
     *
     * @param name is the environment variable name
     * @param value is the environment variable value
     */
    void createEnvironmentItem(const QString& name, const QString& value);

    /**
     * add the environment item in the Environment widget
     *
     * @param item is the QTreeWidgetItem that's referred to the ScripConf widget line of the environment (name + value)
     * @param name is the environment variable name
     * @param value is the environment variable value
     */
    void addEnvironmentItem(QTreeWidgetItem* item, const QString& name, const QString& value);

    /**
     * change the environment item in the Environment widget
     *
     * @param item is the QTreeWidgetItem that represents the environment (name + value)
     * @param column if 0 specifies the environment variable name, if 1 specifies the environment variable value
     * @param text is the new environment value
     */
    void changeEnvironmentItem(QTreeWidgetItem* item, const int& column, const QString& text);

    /**
     * delete the environment item in the Environment widget
     *
     * @param item is the QTreeWidgetItem that represents the environment (name + value)
     */
    void deleteEnvironmentItem(QTreeWidgetItem* item);

    /**
     * @returns the assigned environment to the script File. Each QTreeWidgetItem
     *   has a couple of value:
     *     1. the environment variable name
     *     1. the environment variable value
     */
    QMap<QTreeWidgetItem*, QPair<QString, QString> > environment();

  private:
    /**
     * The type of the item: not assigned, group or file
     */
    int m_type;

    /**
     * this is the complete name of the item:
     *   - the Group Name if it's Group
     *   - the Absolute File Path if it's a File
     *
     * The assigment of this parameter is done in the
     * constructor and for a file has to be its
     * complete absolute file path (file path + file name)
     */
    QString m_assignedName;

    /**
     * If the item is a script File, this is the name without the path
     */
    QString m_name;

    /**
     * If the item is a script File this is just the path
     */
    QString m_path;

    /**
     * The input parameters line
     */
    QString m_parameters;

    /**
     * It's true if the item is checked
     */
    bool m_checked;

    /**
     * The number of times the script has to be executed
     */
    int m_times;

    /**
     * The number of seconds the script has to be belayed before to run again
     */
    int m_delay;

    /**
     * It's true if the related script File is running
     */
    bool m_running;

    /**
     * It's false if the related script File was never executed
     */
    bool m_executed;

    /**
     * It's the environment assigned to the script File when created. Each QTreeWidgetItem
     *   has a couple of value:
     *     1. the environment variable name
     *     1. the environment variable value
     */
    QMap<QTreeWidgetItem*, QPair<QString, QString> > m_environment;

    /**
     * Reference to the Monitor View TextEditMonitor
     */
    TextEditMonitor *m_textEditMonitor;
};

#endif
