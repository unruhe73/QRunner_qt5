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
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QDialogButtonBox>

#include <QtCore/QDir>
#include <QtCore/QCoreApplication>

#include "settings.h"

Settings::Settings() : QDialog()
{
  setWindowTitle(tr("General Options"));
  QVBoxLayout* confOptionLayout = new QVBoxLayout;

  QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
  QCoreApplication::setApplicationName(APPLICATION_NAME);

  QHBoxLayout* basedirHoriz = new QHBoxLayout;
  QLabel *basedirLabel = new QLabel(tr("Log Files Directory:"));
  m_basedir = new QLineEdit;
  QString dir = m_settings.value("basedir", QDir::homePath() + "/qrunner").toString();
  m_basedir->setToolTip(dir);

  QDir directory(dir);
  if (!directory.exists())
    // if the directory doesn't exist than reassign the default value
    dir = QDir::homePath() + "/qrunner";

  m_basedir->setText(dir);
  m_basedir->setReadOnly(true);

  QPushButton* dirButton = new QPushButton(tr("Change"));
  dirButton->setToolTip(tr("Click here to change the Log Files Home Directory"));
  basedirHoriz->addWidget(basedirLabel);
  basedirHoriz->addWidget(m_basedir);
  basedirHoriz->addWidget(dirButton);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(dirButton, SIGNAL(clicked()), this, SLOT(changeLogDirectory()));

  // add the widget and the layout in the vertical layout
  confOptionLayout->addLayout(basedirHoriz);
  confOptionLayout->addStretch();
  confOptionLayout->addWidget(buttonBox);
  confOptionLayout->addStretch();
  setLayout(confOptionLayout);
}


Settings::~Settings()
{
  delete m_basedir;
}


// SLOTS
void Settings::accepted() // SLOT
{
  m_settings.setValue("basedir", m_basedir->text());
  accept();
}


void Settings::changeLogDirectory() // SLOT
{
  QFileDialog *dialog = new QFileDialog;
  dialog->setOption( QFileDialog::ShowDirsOnly, true );
  dialog->setDirectory( m_basedir->text() );
  dialog->setWindowTitle( tr("Choose Directory") );
  if (dialog->exec())
    m_basedir->setText(dialog->selectedFiles().at(0));
  delete dialog;
}
