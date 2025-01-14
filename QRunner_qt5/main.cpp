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

#include <QtWidgets/QApplication>
#include <QtGui/QIcon>
#include <QtCore/QTranslator>
#include <QtCore/QLocale>

#include <QtCore/QDebug>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QString locale = QLocale::system().name();
  QTranslator translator;
  bool translated = translator.load(QString("qrunner_") + locale);
  if (translated)
  {
    app.installTranslator(&translator);
  }

  MainWindow window;
  window.show();
  return app.exec();
}
