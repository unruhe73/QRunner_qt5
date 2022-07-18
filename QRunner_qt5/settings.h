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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtWidgets/QDialog>

#include <QtCore/QSettings>

class QLineEdit;

/**
 * declare Organization and Application Name for this application
 */
const QString ORGANIZATION_NAME = "GiovanniVenturiDeveloper";
const QString APPLICATION_NAME = "QRunner";

/**
 * This class manage the QRunner general options
 *
 * @author Giovanni Venturi
 */
class Settings : public QDialog
{
  Q_OBJECT

  public:
    /**
     * Create the dialog
     */
    Settings();

    /**
     * Deallocate the line edit
     */
    ~Settings();

  private:
    /**
     * The general options
     */
    QSettings m_settings;
    /**
     * The Line Edit to choose the base directory for the log files
     */
    QLineEdit *m_basedir;

  private slots:
    /**
     * Called when you choose ok button
     */
    void accepted();

    /**
     * Change the base directory where to save the log files
     */
    void changeLogDirectory();
};

#endif
