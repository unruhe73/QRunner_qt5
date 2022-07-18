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
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtWidgets/QMessageBox>

#include <QtGui/QMouseEvent>
#include <QtGui/QDesktopServices>
#include <QtGui/QDrag>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QFileInfoList>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QUrl>
#include <QtCore/QSettings>
#include <QtCore/QMimeData>

#include <QtCore/QDebug>

#ifdef Q_OS_LINUX
  #include <unistd.h>
#endif

#include "treewidgetitem.h"
#include "scripttree.h"
#include "scriptconf.h"
#include "scriptqueue.h"
#include "textedit.h"

ScriptTree::ScriptTree(TextEdit *outputBox, QWidget *parent)
  : QTreeWidget(parent), m_outputBox(outputBox)
{
  m_modified = false;
  m_localAction = false;
  enableDND();

  header()->hide();
  m_root = 0;
  setToolTip( tr("<p>Here you can <b>drop</b> files that will "
                 "be your executable script.</p><p>You can "
                 "<b>create/delete</b> script groups.</p><p> You can "
                 "<b>move</b> the script file from a group to "
                 "another.</p>") );
  QSettings settings("Testing Software", "QRunner");
  m_basedir = settings.value("basedir", QDir::homePath() + "/qrunner").toString();

  m_scriptQueue = new ScriptQueue(parent);
  m_scriptQueue->assignBaseDir(m_basedir);

  // connect signal
  connect(m_scriptQueue, SIGNAL(allScriptExecuted()), SIGNAL(readyToRun()));

  m_relatedProcess = NULL;
}


ScriptTree::~ScriptTree()
{
  m_scriptQueue->clear();
  delete m_scriptQueue;
}


bool ScriptTree::isExecutable(const QString &exe)
{
  if( exe.isEmpty() )
    return false;

#ifdef Q_OS_WIN
  return ( exe.endsWith( ".exe" ) || exe.endsWith( ".bat" ) );
#else
  return (access( QFile::encodeName( exe ), R_OK | X_OK ) == 0);
#endif
}


void ScriptTree::clean()
{
  if (m_root)
  {
    clear();
    m_root = 0;
    m_modified = false;

    // remove the allocated resource
    if (!m_XMLProjectDoc)
      delete m_XMLProjectDoc;

    // need to remove all queued item too
    m_scriptQueue->clear();
  }
}


bool ScriptTree::modified()
{
  if (m_root)
    return m_modified;
  else
    return false;
}


void ScriptTree::resetModified()
{
  if (m_root)
    m_modified = false;
}


bool ScriptTree::checkSubprojectFile(QDomNode *node) const
{
  QDomNode *subnode = new QDomNode( node->firstChild() );
  bool error = false;

  while(!subnode->isNull() && !error)
  {
    if(subnode->isElement())
    {
      // get the QDomElement item
      QDomElement elem = subnode->toElement();

      if (elem.tagName() == "subgroup")
      {
        if ((elem.hasAttribute("checked")) && (elem.hasAttribute("name")))
        {
          if ((elem.attribute("checked") != "true") && (elem.attribute("checked") != "false"))
            error = true;
          else
          { // ok the syntax is correct

            // check again recursively starting from the new subnode
            error = !checkSubprojectFile( subnode );
          }
        }
        else
          error = true;
      }
      else if (elem.tagName() == "file")
      {
        if ((elem.hasAttribute("path")) && (elem.hasAttribute("checked")) && (elem.hasAttribute("name")))
        {
          QDir file;
          file.setPath( elem.attribute("path") );

          // don't consider files that don't exist anymore
          if (file.exists())
          {
            if ((elem.attribute("checked") != "true") && (elem.attribute("checked") != "false"))
              error = true;
          }
        }
        else
          error = true;
      }
      else
        error = true;
    }
    *subnode = subnode->nextSibling();
  }
  delete subnode;

  return !error;
}


bool ScriptTree::checkProjectFile(const QString& filename) const
{
  // check if you got problem loading the XML Project file into QDomDocument
  QDomDocument doc("project");
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly))
    return false;
  if (!doc.setContent(&file))
  {
    file.close();
    return false;
  }
  file.close();

  // now check if the tags are recognized correctly and if the XML file structure respect QRunner specifics
  QDomElement docElem = doc.documentElement();
  QDomNode *node = new QDomNode( docElem.firstChild() );
  bool error = false;
  while(!node->isNull() && !error)
  {
    if(node->isElement())
    {
      QDomElement elem = node->toElement();  // gets the element
      if ((elem.tagName() != "group") || (!elem.hasAttribute("checked")) || (!elem.hasAttribute("name")))
        error = true;
      else
      {
        if ((elem.attribute("checked") != "true") && (elem.attribute("checked") != "false"))
          error = true;
        else
          // in the "group" element we can have "subgroup" ones
          error = !checkSubprojectFile(node);
      }
    }
    *node = node->nextSibling();
  }
  delete node;

  return !error;
}


bool ScriptTree::loadProject(const QString& filename)
{
  if (filename.isEmpty())
    // no project file selected
    return false;

  // check if you got problem with the XML Project file
  if (!checkProjectFile(filename))
  {
    QMessageBox::critical(0, tr("Parse Error"),
      tr("<p>You cannot load this project.<br>The file doesn't fit the QRunner Project format!"));
    return false;
  }

  // remove Tree and set variable to "no modify"
  clean();

  QDomDocument doc("project");
  QFile file(filename);
  doc.setContent(&file);
  file.close();
  QDomElement docElem = doc.documentElement();
  QDomNode *node = new QDomNode( docElem.firstChild() );
  int index = 0;
  while(!node->isNull())
  {
    if(node->isElement())
    {
      QDomElement elem = node->toElement();  // gets the element
      if (elem.attribute("checked") == "true")
        createNewGroup( elem.attribute("name"), true );
      else
        createNewGroup( elem.attribute("name"), false );

      // in the "group" element we can have "subgroup" ones
      parseSubgroup(node, topLevelItem(index++));
    }
    *node = node->nextSibling();
  }
  delete node;

  // the project was just loaded, than nothing was modifyed
  m_modified = false;
  emit modifiedProject(false);

  return true;
}


bool ScriptTree::saveProjectTree(const QString& filename)
{
  bool result;
  m_XMLProjectDoc = new QDomDocument;
  QDomElement root = m_XMLProjectDoc->createElement( "project" );

  // add the QRunner project XML file version
  root.setAttribute( "version", "1.0" );

  m_XMLProjectDoc->appendChild( root );

  for (int i = 0; i < topLevelItemCount(); i++)
  {
    QDomElement *subroot = new QDomElement (m_XMLProjectDoc->createElement( "group" ));
    QDomAttr *attr = new QDomAttr( m_XMLProjectDoc->createAttribute( "name" ) );
    attr->setValue( topLevelItem(i)->text(0) );
    subroot->setAttributeNode( *attr );
    delete attr;

    attr = new QDomAttr( m_XMLProjectDoc->createAttribute( "checked" ));
    if (((TreeWidgetItem *)topLevelItem(i))->checked())
      attr->setValue( "true" );
    else
      attr->setValue( "false" );
    subroot->setAttributeNode( *attr );
    delete attr;

    root.appendChild( *subroot );
    if (topLevelItem(i)->child(0))
      // append the new tree we found
      saveProjectTree(topLevelItem(i), subroot);
    delete subroot;
  }
  QString xml = "<?xml version=\"1.0\" ?>\n";
  xml.append( m_XMLProjectDoc->toString() );
  QFile file( filename );
  if ( file.open( QIODevice::WriteOnly ) )
  {
    QTextStream stream( &file );
    stream << xml;
    file.close();
    result = true;
  }
  else
  {
    QMessageBox::critical(0, tr("Writing Error"),
      tr("Could not open temporary file '%1' for writing").arg(filename));
    result = false;
  }

  return result;
}


void ScriptTree::hideConsole()
{
  m_outputBox->hide();
  m_outputBox->assignScriptProcess(0);
}


void ScriptTree::enableDND(const bool& cond)
{
  m_DND = cond;
  setAcceptDrops(cond);
  setDropIndicatorShown(cond);
  //setDragEnabled(cond);
}


TreeWidgetItem *ScriptTree::draggingItem()
{
  return m_draggingItem;
}


// Private members

TreeWidgetItem *ScriptTree::addFile( QTreeWidgetItem *item, const QString& filename, bool checked, const QString& absoluteFilePath, int times, int delay, const QString& params)
{
  // adding script reading a project file (no by drag and drop)
  TreeWidgetItem *fileItem = new TreeWidgetItem(absoluteFilePath);
  fileItem->setFile(times, delay);
  fileItem->setChecked(checked);
  fileItem->setTimes(times);
  fileItem->setDelay(delay);
  fileItem->setParameters(params);
  fileItem->setForeground(0, QColor(0, 0, 0));
  fileItem->setFileName( filename );
  item->addChild(fileItem);
  expandItem(item);
  fileItem->setText(0, filename);
  fileItem->setToolTip( 0, absoluteFilePath );
  m_modified = true;

  // give the reference to the added script File
  return fileItem;
}


void ScriptTree::createNewSubGroup( const QString& name, bool checked )
{
  TreeWidgetItem *subgroup = new TreeWidgetItem;
  subgroup->setGroup();
  subgroup->setChecked(checked);
  itemAt(m_event->pos())->addChild(subgroup);
  expandItem(itemAt(m_event->pos()));
  if (name.isEmpty())
    editItem(subgroup);
  else
    subgroup->setText(0, name);
  m_modified = true;
  emit modifiedProject();
}


void ScriptTree::setScriptTreeColor(const QString& color)
{
  for (int i = 0; i < topLevelItemCount(); i++)
  {
    topLevelItem(i)->setForeground(0, QBrush(color, Qt::SolidPattern));
    if (topLevelItem(i)->child(0))
      setScriptSubtreeColor(topLevelItem(i));
  }
}


void ScriptTree::setScriptSubtreeColor(QTreeWidgetItem *top, const QString& color)
{
  for (int i = 0; i < top->childCount(); i++)
  {
    top->child(i)->setForeground(0, QBrush(color, Qt::SolidPattern));
    if (top->child(i)->child(0))
      setScriptSubtreeColor(top->child(i));
  }
}


void ScriptTree::saveProjectTree(QTreeWidgetItem *top, QDomElement *root)
{
  if (!root)
    // cannot append to NULL
    return;

  QDomElement subroot;

  // append all the childs to the tree root element
  for (int i = 0; i < top->childCount(); i++)
  {
    if (((TreeWidgetItem *)top->child(i))->isGroup())
    {
      subroot = m_XMLProjectDoc->createElement( "subgroup" );
      QDomAttr *attr = new QDomAttr( m_XMLProjectDoc->createAttribute( "name" ) );
      attr->setValue( top->child(i)->text(0) );
      subroot.setAttributeNode( *attr );
      delete attr;

      attr = new QDomAttr( m_XMLProjectDoc->createAttribute( "checked" ) );
      if (((TreeWidgetItem *)top->child(i))->checked())
        attr->setValue( "true" );
      else
        attr->setValue( "false" );
      subroot.setAttributeNode( *attr );
      delete attr;
    }
    else
    {
      subroot = m_XMLProjectDoc->createElement( "file" );

      QDomAttr *attr = new QDomAttr( m_XMLProjectDoc->createAttribute( "checked" ));
      if (((TreeWidgetItem *)top->child(i))->checked())
        attr->setValue( "true" );
      else
        attr->setValue( "false" );
      subroot.setAttributeNode( *attr );
      delete attr;

      attr = new QDomAttr( m_XMLProjectDoc->createAttribute( "path" ) );
      attr->setValue( ((TreeWidgetItem *)top->child(i))->filePath() );
      subroot.setAttributeNode( *attr );
      delete attr;

      attr = new QDomAttr( m_XMLProjectDoc->createAttribute( "name" ) );
      attr->setValue( ((TreeWidgetItem *)top->child(i))->fileName() );
      subroot.setAttributeNode( *attr );
      delete attr;

      if (((TreeWidgetItem *)top->child(i))->times() > 1)
      {
        // don't need to save this attribute value if it is equal to 1
        attr = new QDomAttr( m_XMLProjectDoc->createAttribute( "times" ) );
        attr->setValue( QString::number(((TreeWidgetItem *)top->child(i))->times()) );
        subroot.setAttributeNode( *attr );
        delete attr;
      }

      if (((TreeWidgetItem *)top->child(i))->delay() > 0)
      {
        // don't need to save this attribute value if it is equal to 0
        attr = new QDomAttr( m_XMLProjectDoc->createAttribute( "delay" ) );
        attr->setValue( QString::number(((TreeWidgetItem *)top->child(i))->delay()) );
        subroot.setAttributeNode( *attr );
        delete attr;
      }

      if (!((TreeWidgetItem *)top->child(i))->parameters().isEmpty())
      {
        // don't need to save this attribute value if it is equal to 1
        attr = new QDomAttr( m_XMLProjectDoc->createAttribute( "parameters" ) );
        attr->setValue( ((TreeWidgetItem *)top->child(i))->parameters() );
        subroot.setAttributeNode( *attr );
        delete attr;
      }

      // save the Environment data
      if (!((TreeWidgetItem *)top->child(i))->environment().isEmpty())
      {
        QDomElement tag = m_XMLProjectDoc->createElement( "environment" );
        subroot.appendChild( tag );

        QMapIterator<QTreeWidgetItem*, QPair<QString, QString> > iterator(((TreeWidgetItem *)top->child(i))->environment());
        while (iterator.hasNext())
        {
          iterator.next();

          // how many "env" as the number of environment variables we have
          QDomElement subtag = m_XMLProjectDoc->createElement( "env" );
          tag.appendChild( subtag );

          // variable name
          attr = new QDomAttr( m_XMLProjectDoc->createAttribute( "name" ) );
          attr->setValue( iterator.value().first );
          subtag.setAttributeNode( *attr );
          delete attr;

          // variable value
          attr = new QDomAttr( m_XMLProjectDoc->createAttribute( "value" ) );
          attr->setValue( iterator.value().second );
          subtag.setAttributeNode( *attr );
          delete attr;
        }
      }
    }
    root->appendChild( subroot );

    if (top->child(i)->child(0))
      saveProjectTree(top->child(i), &subroot);
  }
}


void ScriptTree::parseSubgroup(QDomNode *node, QTreeWidgetItem *item)
{
  QDomNode *subnode = new QDomNode( node->firstChild() );

  while(!subnode->isNull())
  {
    if(subnode->isElement())
    {
      QDomElement elem = subnode->toElement();  // gets the element
      if (elem.tagName() == "subgroup")
      {
        createNewSubGroup( item, elem.attribute("name"),
          (elem.attribute("checked") == "true") );

        // jump into the children widgets to get the current sub group widget just created with createNewSubGroup member.
        // Use the recursion to parse the deeper level of the sub group into the XML file.
        // This procedure creates the widget tree items while it's parsing the XML file with the QDomElement class.
        // XML file is complete. The tree widget is created at run time.
        int i = 0;
        while (item->child(++i) != NULL)
          ;
        parseSubgroup( subnode, item->child(--i) );
      }
      else if (elem.tagName() == "file")
      {
        QDir file;
        file.setPath( elem.attribute("path") );

        // don't add to the tree file that doesn't exist anymore
        if (file.exists())
        {
          TreeWidgetItem *newScript = 0;
          int times = 1;
          if (!elem.attribute("times").isEmpty())
            times = elem.attribute("times").toInt();

          int delay = 0;
          if (!elem.attribute("delay").isEmpty())
            delay = elem.attribute("delay").toInt();

          newScript = addFile( item, elem.attribute("name"),
            (elem.attribute("checked") == "true"),
            file.absoluteFilePath(elem.attribute("name")), times, delay,
            elem.attribute("parameters") );

          // parse the environment part if found
          if (subnode->hasChildNodes())
          {
            // "file" tag has children
            if (subnode->firstChild().isElement())
            {
              // is the first child "environment"?
              if (subnode->firstChild().toElement().tagName() == "environment")
              {
                // ok now get all the variables + values
                QDomNode *envNode = new QDomNode( subnode->firstChild().firstChild() );
                while (!envNode->isNull())
                {
                  if(envNode->isElement())
                  {
                    QDomElement envElem = envNode->toElement();  // gets the element
                    if (envElem.tagName() == "env")
                    {
                      // add the new enviroment in the related item
                      newScript->createEnvironmentItem(envElem.attribute("name"), envElem.attribute("value"));
                    }
                  }
                  emit newEnvironmentItemAdded(newScript);
                  *envNode = envNode->nextSibling();
                }
                delete envNode;
              }
            }
          }
        }
      }
    }
    *subnode = subnode->nextSibling();
  }
  delete subnode;
}


void ScriptTree::addProjectSubTree(QTreeWidgetItem *item)
{
  if (((TreeWidgetItem *)item)->isFile())
  {
    if (((TreeWidgetItem *)item)->checked())
    {
      // now look for the parents so to understand the dir/subdirs the file is in
      QTreeWidgetItem *item0;
      QList<QString> lifo;
      item0 = item;
      while (item0->parent() != NULL)
      {
        lifo.push_back(item0->parent()->text(0));
        item0 = item0->parent();
      }

      // now add the script to the queue
      m_scriptQueue->add(&lifo, (TreeWidgetItem *)item);
    }
  }
  else
    for (int i = 0; i < item->childCount(); i++)
    {
      if (((TreeWidgetItem *)item->child(i))->checked())
      {
        // you can run the tree
        if (((TreeWidgetItem *)item->child(i))->isGroup())
        {
          if (item->child(i)->child(0))
            addProjectSubTree(item->child(i));
        }
        else
        {
          // now look for the parents so to understand the dir/subdirs the file is in
          QTreeWidgetItem *item0;
          QList<QString> lifo;
          item0 = item;
          lifo.push_back(item0->text(0));
          while (item0->parent() != NULL)
          {
            lifo.push_back(item0->parent()->text(0));
            item0 = item0->parent();
          }

          // now add the script to the queue
          m_scriptQueue->add(&lifo, (TreeWidgetItem *)item->child(i));
        }
      }
    }
}


void ScriptTree::showConsole(ScriptProcess* process)
{
  // read the temporary file and put it into the m_outputBox
  QString line;
  QFile file(process->tmpFile());

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  QTextStream in(&file);
  while (!in.atEnd())
    line += in.readLine() + "\n";

  m_outputBox->show();
  m_outputBox->setText(line);
  m_outputBox->assignScriptProcess(process);
  process->assignConsole(m_outputBox);
}



// Protected members

void ScriptTree::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("text/plain"))
    event->acceptProposedAction();
}


void ScriptTree::dragMoveEvent(QDragMoveEvent *event)
{
  if (itemAt(event->pos()) == NULL)
  {
    // you cannot drop here
    event->setDropAction(Qt::IgnoreAction);
    event->ignore();
  }
  else
  {
    if (((TreeWidgetItem*)itemAt(event->pos()))->isGroup())
    {
      // if the file is not yet in the group you can add it here
      int i = 0;
      bool found = false;
      while ((i < itemAt(event->pos())->childCount()) && !found)
      {
        if (m_localAction)
          found = ((TreeWidgetItem*)itemAt(event->pos())->child(i++))->name() == event->mimeData()->text();
        else
          found = ((TreeWidgetItem*)itemAt(event->pos())->child(i++))->assignedName() == event->mimeData()->text();
      }

      if (!found)
      {
        //emit showStatusMessage(tr("If you want, you can drop the script now."));
        // if the drop is a local file than is ok, if it's not local than we need the script is executable
        // this has to change if the file system tree is filtered with scripts file directly
        if (isExecutable(event->mimeData()->text()) || m_localAction)
        {
          event->setDropAction(Qt::MoveAction);
          event->acceptProposedAction();
        }
        else
        {
          // you can't add a not executable file to the list: it will not be executed
          event->setDropAction(Qt::IgnoreAction);
          event->ignore();
        }
      }
      else
      {
        // you cannot add the same file in the same group...
        event->setDropAction(Qt::IgnoreAction);
        event->ignore();
        //emit showStatusMessage(tr("You cannot add the same script twice in this place!"));
      }
    }
    else
    {
      // you can't drop on a file
      event->setDropAction(Qt::IgnoreAction);
      event->ignore();
    }
  }
}


void ScriptTree::dropEvent(QDropEvent *event)
{
  QTreeWidgetItem *selectedItem = itemAt(event->pos());
  if (event->mimeData()->hasText())
  {
    if (itemAt(event->pos()) != NULL)
    {
      // you can add a file script just to a group
      if (((TreeWidgetItem*)selectedItem)->isGroup())
      {
        if (m_localAction)
        {
          TreeWidgetItem* itemA = static_cast<TreeWidgetItem*>(itemAt(m_dragStartPosition));
          TreeWidgetItem* itemB = static_cast<TreeWidgetItem*>(selectedItem);
          if (itemA->isFile())
          {
            itemA->parent()->takeChild(itemA->parent()->indexOfChild(itemA));
            itemB->addChild(itemA);
            expandItem(itemB);
            emit groupSelected();
//            m_localAction = false;  // the drop comes from File System Tree
          }
          else if (itemA->isGroup())
          {
            TreeWidgetItem* newItem;
            if (m_localAction && (itemA != itemB) )
            {
              // just if action is local and not referred to the same folder as source and destination
              newItem = new TreeWidgetItem( itemA->text(0) );
              newItem->setGroup();
              newItem->addChildren( itemA->takeChildren() );
              itemB->addChild( newItem );
              delete itemA;

              expandItem( newItem );
              expandItem( itemB );
              emit groupSelected();
//              m_localAction = false;  // the drop comes from File System Tree
            }
          }
        }
        else
        {
          if (QFileInfo(event->mimeData()->text()).isDir())
          {
            // we need to add all the file of this directory
            QDir dir(event->mimeData()->text());
            dir.setFilter(QDir::Files | QDir::NoSymLinks);
            //dir.setSorting(QDir::Size | QDir::Reversed);

            QFileInfoList list = dir.entryInfoList();
            for (int i = 0; i < list.size(); ++i)
            {
              QFileInfo fileInfo = list.at(i);
              TreeWidgetItem *item = new TreeWidgetItem(fileInfo.absoluteFilePath());
              item->setFile();
              selectedItem->addChild(item);
              expandItem(selectedItem);
            }
          }
          else
          {
            // it's a simple file not a directory: added
            TreeWidgetItem *item = new TreeWidgetItem(event->mimeData()->text());
            item->setFile();
            selectedItem->addChild(item);
            expandItem(selectedItem);
          }
        }
        m_modified = true;
        emit modifiedProject();
        event->acceptProposedAction();
      }
    }
  }
}


void ScriptTree::mousePressEvent(QMouseEvent *event)
{
  ScriptProcess* process;
  if ((event->button() == Qt::LeftButton) && (itemAt(event->pos()) != NULL))
  {
    m_statebefore = itemAt(event->pos())->checkState(0);
    if (((TreeWidgetItem *)itemAt(event->pos()))->isFile())
    {
      if ((process = m_scriptQueue->lookforWidget((TreeWidgetItem *)itemAt(event->pos()))))
      {
        // virtually remove the connections for the current process output to the m_outputBox
        if (m_outputBox->process())
          m_outputBox->process()->assignConsole(0);

        // if the file is a script that's running or it's been stopped than show its console
        showConsole(process);

        if (!process->isRunning())
        {
          // clicked on file while the script is not running: show configuration options
          emit fileSelected((TreeWidgetItem *)itemAt(event->pos()));
        }
        else
        {
          // clicked on file while the script running: show file system tree
          emit groupSelected();
        }
      }
      else
      {
        // no process related: no data to show
        m_outputBox->clear();
        m_outputBox->hide();

        // clicked on file: show configuration options
        emit fileSelected((TreeWidgetItem *)itemAt(event->pos()));
      }
    }
    else
    {
      // clicked on group: show file system tree and hide the console
      m_outputBox->hide();

      emit groupSelected();
    }

    if (event->button() == Qt::LeftButton)
    {
      // annotate the position and the name of the item where you left clicked
      m_dragStartPosition = event->pos();
      m_draggingItem = ((TreeWidgetItem *)itemAt(event->pos()));
      m_data = m_draggingItem->text(0);
    }
  }
  else
  {
    hideConsole();
    emit nothingSelected();
  }
  QTreeWidget::mousePressEvent(event);
}


void ScriptTree::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_DND)
    return;
  if (!(event->buttons() & Qt::LeftButton))
    return;
  if ((event->pos() - m_dragStartPosition).manhattanLength()
    < QApplication::startDragDistance())
      return;

  m_localAction = true;
  QDrag *drag = new QDrag(this);
  QMimeData *mimeData = new QMimeData;

  mimeData->setText(m_data);
  drag->setMimeData(mimeData);
  drag->exec(Qt::MoveAction);
}


void ScriptTree::mouseReleaseEvent(QMouseEvent *event)
{
  QTreeWidget::mouseReleaseEvent(event);
  if ((event->button() == Qt::LeftButton) && (itemAt(event->pos()) != NULL))
  {
    Qt::CheckState stateafter = itemAt(event->pos())->checkState(0);
    if ((m_statebefore == Qt::Unchecked) && (stateafter == Qt::Checked))
      ((TreeWidgetItem *)itemAt(event->pos()))->setChecked();

    else if ((m_statebefore == Qt::Checked) && (stateafter == Qt::Unchecked))
      ((TreeWidgetItem *)itemAt(event->pos()))->setChecked(false);

    if (stateafter != m_statebefore)
    {
      // the check was changed
      m_modified = true;
      emit modifiedProject();
    }
  }
}


void ScriptTree::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu menu(this);
  TreeWidgetItem* item = (TreeWidgetItem*)itemAt(event->pos());
  if (m_scriptQueue->isRunning())
  {
    if (item->isGroup())
    {
    }
    else
    {
      // clicked on a script
      if (item->running())
      {
        m_relatedProcess = m_scriptQueue->lookforWidget(item);
        QAction *stopScript = new QAction(tr("&Stop this script"), this);
        stopScript->setStatusTip(tr("Stop running this script"));
        connect(stopScript, SIGNAL(triggered()), this, SLOT(stopScript()));

        menu.addAction(stopScript);
        menu.exec(event->globalPos());
      }
      else
      {
        if (item->executed())
        {
          QAction *showLogFile = new QAction(tr("&Show the Log file"), this);
          showLogFile->setStatusTip(tr("Show the related Log file with a text editor"));
          connect(showLogFile, SIGNAL(triggered()), this, SLOT(showLogFile()));
          menu.addAction(showLogFile);

          if (item->checked())
          {
            QAction* runScript = new QAction(tr("&Run this script again"), this);
            runScript->setStatusTip(tr("Run this script again"));
            connect(runScript, SIGNAL(triggered()), this, SLOT(runScriptAgain()));
            menu.addAction(runScript);
          }
          // assign the position to let start the script related to this widget position when needed
          m_pointerPosition = event->pos();

          // open the context menu
          menu.exec(event->globalPos());
        }
      }
    }
  }
  else
  {
    QAction *runScript;

    if (item != NULL)
    {
      if (item->isGroup())
      {
        m_event = event;
        QAction *delGroup = new QAction(tr("&Delete the group"), this);
        delGroup->setStatusTip(tr("Remove the group from the project"));
        connect(delGroup, SIGNAL(triggered()), this, SLOT(deleteItem()));

        // you can create a new subgroup when you click on a group: not on a file
        if ( (item->child(0) == NULL) || ((item->child(0) != NULL) && (((TreeWidgetItem*)item)->isGroup())) )
        {
          QAction *newSubGroup = new QAction(tr("&Create a new sub group"), this);
          newSubGroup->setStatusTip(tr("Create a new script sub group to add to the project"));
          connect(newSubGroup, SIGNAL(triggered()), this, SLOT(createNewSubGroup()));

          menu.addAction(newSubGroup);
        }

        menu.addAction(delGroup);
        if (item->checked())
        {
          runScript = new QAction(tr("&Run this script folder"), this);
          runScript->setStatusTip(tr("Run this script folder with all subfolders"));
          connect(runScript, SIGNAL(triggered()), this, SLOT(runScript()));
          menu.addAction(runScript);
        }
      }
      else
      {
        if (item->executed())
        {
          QAction *showLogFile = new QAction(tr("&Show the Log file"), this);
          showLogFile->setStatusTip(tr("Show the related Log file with a text editor"));
          connect(showLogFile, SIGNAL(triggered()), this, SLOT(showLogFile()));
          menu.addAction(showLogFile);
        }

        QAction *delFile = new QAction(tr("&Delete the script"), this);
        delFile->setStatusTip(tr("Remove the script from the project"));
        connect(delFile, SIGNAL(triggered()), this, SLOT(deleteItem()));
        menu.addAction(delFile);

        if (item->checked())
        {
          runScript = new QAction(tr("&Run this script"), this);
          runScript->setStatusTip(tr("Run this script"));
          connect(runScript, SIGNAL(triggered()), this, SLOT(runScript()));
          menu.addAction(runScript);
        }
      }

      // assign the position to let remove the item and then add the entry to the menu
      m_pointerPosition = event->pos();
    }
    else
    {
      // there is at least one group
      if (topLevelItemCount())
      {
        QAction *runProject = new QAction(tr("Run all folders scripts"), this);
        runProject->setStatusTip(tr("Run all the project folders scripts"));
        connect(runProject, SIGNAL(triggered()), this, SLOT(runProjectTree()));

        // adding menu
        menu.addAction(runProject);
      }

      QAction *newGroup = new QAction(tr("&Create a new group"), this);
      newGroup->setStatusTip(tr("Create a new script group to add to the project"));
      connect(newGroup, SIGNAL(triggered()), this, SLOT(createNewGroup()));

      // adding menu
      menu.addAction(newGroup);
    }
    QAction *newProject = new QAction(tr("C&reate a new project"), this);
    newProject->setStatusTip(tr("Create a new script project"));
    newProject->setIcon(QIcon(":/images/new.png"));
    connect(newProject, SIGNAL(triggered()), this, SIGNAL(newProject()));

    menu.addAction(newProject);

    QAction *loadProject = new QAction(tr("&Open an existing project"), this);
    loadProject->setStatusTip(tr("Open an existing project"));
    loadProject->setIcon(QIcon(":/images/open.png"));
    connect(loadProject, SIGNAL(triggered()), this, SIGNAL(loadProject()));

    menu.addAction(loadProject);

    // if no modifies in the Project you don't need to save it
    if (m_modified)
    {
      QAction *saveProject = new QAction(tr("&Save the project"), this);
      saveProject->setStatusTip(tr("Save the script project"));
      saveProject->setIcon(QIcon(":/images/save.png"));
      connect(saveProject, SIGNAL(triggered()), this, SIGNAL(saveProject()));

      menu.addAction(saveProject);
    }
    menu.exec(event->globalPos());
  }
}



// SLOTS

// Slot
void ScriptTree::createNewGroup( const QString& name, bool checked )
{
  if (m_root == 0)
  {
    m_root = new TreeWidgetItem;
    m_root->setGroup();
    addTopLevelItem( m_root );
    m_root->setChecked(checked);
    if (name.isEmpty())
      editItem(m_root);
    else
      m_root->setText(0, name);
  }
  else
  {
    TreeWidgetItem *group = new TreeWidgetItem;
    group->setGroup();
    addTopLevelItem( group );
    group->setChecked(checked);
    if (name.isEmpty())
      editItem(group);
    else
      group->setText(0, name);
  }
  m_modified = true;
  emit modifiedProject();
}


// SLOT
void ScriptTree::showLogFile()
{
  TreeWidgetItem* item = ((TreeWidgetItem*)itemAt(m_pointerPosition));
  TreeWidgetItem* parent = ((TreeWidgetItem*)item->parent());
  QStringList list;
  list << item->fileName();
  while (parent != NULL)
  {
    list << parent->name();
    parent = (TreeWidgetItem*)parent->parent();
  }
  QString fullpath = m_basedir;
  int i = list.size();
  while (i > 0)
    fullpath += "/" + list[--i];
  fullpath += ".log";

#ifdef Q_OS_WIN
  m_procShowLog = new QProcess;
  m_procShowLog->start("notepad.exe", QStringList() << fullpath);
  connect(m_procShowLog, SIGNAL(finished( int, QProcess::ExitStatus )), SLOT(finishedShowLog( int, QProcess::ExitStatus)));
#else
  QDesktopServices::openUrl((QUrl)fullpath);
#endif
}


// Slot
void ScriptTree::createNewSubGroup( QTreeWidgetItem *item, const QString& subgroupname, bool checked )
{
  TreeWidgetItem *subgroup = new TreeWidgetItem;
  subgroup->setGroup();
  subgroup->setChecked(checked);
  item->addChild(subgroup);
  expandItem(item);
  if (subgroupname.isEmpty())
    editItem(subgroup);
  else
    subgroup->setText(0, subgroupname);
  m_modified = true;
  emit modifiedProject();
}


// Slot
void ScriptTree::deleteItem()
{
  delete itemAt(m_pointerPosition);
  if (topLevelItemCount())
  {
    // there are more elements available
    m_modified = true;
    emit modifiedProject();
  }
  else
  {
    // there are no more elements available:
    //  project was totally empty, than
    //  we do not need to say: "modifiedProject"
    m_modified = false;
    emit modifiedProject(false);
  }
  emit deletedItem();
}


// Slot
void ScriptTree::runScript()
{
  m_outputBox->hide();
  m_outputBox->clear();

  // queue the scripts
  m_scriptQueue->clear();
  addProjectSubTree(itemAt(m_pointerPosition));

  // now we can execute the queued scripts
  if (m_scriptQueue->isEmpty())
  {
    QMessageBox::critical(0, tr("Running Error"),
      tr("<p>You cannot execute scripts if you don't add at least one script!</p>"));
    emit readyToRun();
  }
  else
  {
    // set the default color for the tree widgets
    setScriptTreeColor();

    // advise that the script is going to start
    emit runningScript();

    m_scriptQueue->run();
  }
}


// Slot
void ScriptTree::stopScript()
{
  m_relatedProcess->kill();
}


// Slot
void ScriptTree::runScriptAgain()
{
  addProjectSubTree(itemAt(m_pointerPosition));

  // now we can execute the queued scripts
  if (m_scriptQueue->isEmpty())
  {
    QMessageBox::critical(0, tr("Running Error"),
      tr("<p>You cannot execute scripts if you don't add at least one script!</p>"));
    emit readyToRun();
  }
  else
  {
    // advise that the script is going to start
    emit runningScript();

    m_scriptQueue->runLast();
  }
}


// Slot
void ScriptTree::finishedShowLog( int exitCode, QProcess::ExitStatus exitStatus )
{
  Q_UNUSED(exitCode);
  Q_UNUSED(exitStatus);
  delete m_procShowLog;
}


// Slot
void ScriptTree::runProjectTree()
{
  // clear and hide the TextEdit
  m_outputBox->hide();
  m_outputBox->clear();

  // remove all the scripts from the queue
  m_scriptQueue->clear();

  for (int i = 0; i < topLevelItemCount(); i++)
  {
    if (((TreeWidgetItem *)topLevelItem(i))->checked())
      // you can run the tree
      if (topLevelItem(i)->child(0))
        // queue the scripts
        addProjectSubTree(topLevelItem(i));
  }

  // execute the scripts
  if (m_scriptQueue->isEmpty())
  {
    QMessageBox::critical(0, tr("Running Error"),
      tr("<p>You cannot execute scripts if you don't add at least one script!</p>"));
    emit readyToRun();
  }
  else
  {
    // advise that the script is going to start
    emit runningScript();

    m_scriptQueue->run();
  }
}


void ScriptTree::setExternalDND()
{
  // the drop comes from File System Tree
  m_localAction = false;
}
