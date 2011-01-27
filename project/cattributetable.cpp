#include "cattributetable.h"
#include "cnesicideproject.h"

#include "main.h"

CAttributeTable::CAttributeTable(IProjectTreeViewItem* parent)
{
   // Add node to tree
   InitTreeItem(parent);
   
   // Allocate attributes
   m_editor = (AttributeTableEditorForm*)NULL;
}

CAttributeTable::~CAttributeTable()
{
   if (m_editor)
   {
      delete m_editor;
   }
}

bool CAttributeTable::serialize(QDomDocument& doc, QDomNode& node)
{
   QDomElement element = addElement( doc, node, "attributetable" );
   element.setAttribute("name", m_name);
   element.setAttribute("uuid", uuid());
   return true;
}

bool CAttributeTable::deserialize(QDomDocument&, QDomNode& node)
{
   QDomElement element = node.toElement();

   if (element.isNull())
   {
      return false;
   }

   if (!element.hasAttribute("name"))
   {
      return false;
   }

   if (!element.hasAttribute("uuid"))
   {
      return false;
   }

   m_name = element.attribute("name");

   setUuid(element.attribute("uuid"));

   return true;
}

void CAttributeTable::contextMenuEvent(QContextMenuEvent* event, QTreeView* parent)
{
   QMenu menu(parent);
   menu.addAction("&Delete");

   QAction* ret = menu.exec(event->globalPos());

   if (ret)
   {
      if (ret->text() == "&Delete")
      {
         if (QMessageBox::question(parent, "Delete Attribute Table", "Are you sure you want to delete " + name(),
                                   QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
         {
            return;
         }

         if (m_editor)
         {
            QTabWidget* tabWidget = (QTabWidget*)m_editor->parentWidget()->parentWidget();
            tabWidget->removeTab(tabWidget->indexOf(m_editor));
         }

         // TODO: Fix this logic so the memory doesn't get lost.
//         nesicideProject->getProject()->getSources()->removeChild(this);
//         nesicideProject->getProject()->getSources()->getSourceItems().removeAll(this);
         ((CProjectTreeViewModel*)parent->model())->layoutChangedEvent();
      }
   }
}

void CAttributeTable::openItemEvent(QTabWidget* tabWidget)
{
   if (m_editor)
   {
      if (m_editor->isVisible())
      {
         tabWidget->setCurrentWidget(m_editor);
      }
      else
      {
         tabWidget->addTab(m_editor, this->caption());
         tabWidget->setCurrentWidget(m_editor);
      }
   }
   else
   {
      m_editor = new AttributeTableEditorForm();
      tabWidget->addTab(m_editor, this->caption());
      tabWidget->setCurrentWidget(m_editor);
   }
}

bool CAttributeTable::onCloseQuery()
{
   return true;
}

void CAttributeTable::onClose()
{
   if (m_editor)
   {
      delete m_editor;
      m_editor = (AttributeTableEditorForm*)NULL;
   }
}

bool CAttributeTable::isDocumentSaveable()
{
   return true;
}

void CAttributeTable::onSaveDocument()
{
}

bool CAttributeTable::canChangeName()
{
   return true;
}

bool CAttributeTable::onNameChanged(QString newName)
{
   if (m_name != newName)
   {
      m_name = newName;

      if ( m_editor )
      {
         QTabWidget* tabWidget = (QTabWidget*)m_editor->parentWidget()->parentWidget();
         tabWidget->setTabText(tabWidget->indexOf(m_editor), newName);
      }
   }

   return true;
}