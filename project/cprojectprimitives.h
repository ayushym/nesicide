#ifndef CPROJECTPRIMITIVES_H
#define CPROJECTPRIMITIVES_H

#include "cprojectbase.h"
#include "cattributetables.h"

#include <QString>

class CProjectPrimitives : public CProjectBase
{
public:
   CProjectPrimitives(IProjectTreeViewItem* parent);
   virtual ~CProjectPrimitives();

   // Helper functions
   void initializeProject();
   void terminateProject();

   // IXMLSerializable Interface Implementation
   virtual bool serialize(QDomDocument& doc, QDomNode& node);
   virtual bool deserialize(QDomDocument& doc, QDomNode& node, QString& errors);

   // IProjectTreeViewItem Interface Implmentation
   QString caption() const;
   virtual void contextMenuEvent(QContextMenuEvent* event, QTreeView* parent);
   virtual void openItemEvent(CProjectTabWidget*) {}
   virtual bool onCloseQuery()
   {
      return true;
   }
   virtual void onClose() {}
   virtual void saveItemEvent() {}
   virtual bool canChangeName()
   {
      return false;
   }
   virtual bool onNameChanged(QString)
   {
      return true;
   }

private:
   // Contained children
   CAttributeTables* m_pAttributeTables;
};

#endif // CPROJECTPRIMITIVES_H
