#include "mapperinformationdockwidget.h"
#include "ui_mapperinformationdockwidget.h"

#include "emulator_core.h"

#include "dbg_cnesmappers.h"

#include "main.h"

MapperInformationDockWidget::MapperInformationDockWidget(QWidget *parent) :
    CDebuggerBase(parent),
    ui(new Ui::MapperInformationDockWidget)
{
   ui->setupUi(this);

   QObject::connect ( emulator, SIGNAL(cartridgeLoaded()), this, SLOT(cartridgeLoaded()) );
   QObject::connect ( emulator, SIGNAL(emulatorReset()), this, SLOT(updateInformation()) );
   QObject::connect ( emulator, SIGNAL(emulatorPaused(bool)), this, SLOT(updateInformation()) );
   QObject::connect ( breakpointWatcher, SIGNAL(breakpointHit()), this, SLOT(updateInformation()) );

   // Force UI update so it doesn't look uninitialized completely.
   cartridgeLoaded();
}

MapperInformationDockWidget::~MapperInformationDockWidget()
{
    delete ui;
}

void MapperInformationDockWidget::changeEvent(QEvent* e)
{
   CDebuggerBase::changeEvent(e);

   switch (e->type())
   {
      case QEvent::LanguageChange:
         ui->retranslateUi(this);
         break;
      default:
         break;
   }
}

void MapperInformationDockWidget::showEvent(QShowEvent* e)
{
   QObject::connect ( emulator, SIGNAL(updateDebuggers()), this, SLOT(updateInformation()) );
   updateInformation();
}

void MapperInformationDockWidget::hideEvent(QHideEvent* e)
{
   QObject::disconnect ( emulator, SIGNAL(updateDebuggers()), this, SLOT(updateInformation()) );
}

void MapperInformationDockWidget::cartridgeLoaded()
{
   char buffer [ 128 ];
   sprintf ( buffer, "Mapper %d: %s", nesGetMapper(), mapperNameFromID(nesGetMapper()) );
   ui->info->setText ( buffer );
   ui->tabWidget->setTabEnabled(0, nesMapperRemapsPRGROM());
   ui->tabWidget->setTabEnabled(1, nesMapperRemapsCHRMEM());
   ui->internalInfo->setCurrentIndex(nesGetMapper());
}

void MapperInformationDockWidget::updateInformation()
{
   CBreakpointInfo* pBreakpoints = nesGetBreakpointDatabase();
   int idx;
   char buffer [ 16 ];
   nesMapper001Info mapper001Info;
   nesMapper004Info mapper004Info;

   // Show PRG-ROM absolute addresses...
   sprintf ( buffer, "%02X(%05X)", nesGetPRGROMAbsoluteAddress(0x8000)>>13, nesGetPRGROMAbsoluteAddress(0x8000) );
   ui->prg0->setText ( buffer );
   sprintf ( buffer, "%02X(%05X)", nesGetPRGROMAbsoluteAddress(0xA000)>>13, nesGetPRGROMAbsoluteAddress(0xA000) );
   ui->prg1->setText ( buffer );
   sprintf ( buffer, "%02X(%05X)", nesGetPRGROMAbsoluteAddress(0xC000)>>13, nesGetPRGROMAbsoluteAddress(0xC000) );
   ui->prg2->setText ( buffer );
   sprintf ( buffer, "%02X(%05X)", nesGetPRGROMAbsoluteAddress(0xE000)>>13, nesGetPRGROMAbsoluteAddress(0xE000) );
   ui->prg3->setText ( buffer );

   // Show CHR memory absolute addresses...
   sprintf ( buffer, "%02X:%05X(%05X)", nesGetCHRMEMAbsoluteAddress(0x0000)>>13, nesGetCHRMEMAbsoluteAddress(0x0000)&MASK_8KB, nesGetCHRMEMAbsoluteAddress(0x0000) );
   ui->chr0->setText ( buffer );
   sprintf ( buffer, "%02X:%05X(%05X)", nesGetCHRMEMAbsoluteAddress(0x0400)>>13, nesGetCHRMEMAbsoluteAddress(0x0400)&MASK_8KB, nesGetCHRMEMAbsoluteAddress(0x0400) );
   ui->chr1->setText ( buffer );
   sprintf ( buffer, "%02X:%05X(%05X)", nesGetCHRMEMAbsoluteAddress(0x0800)>>13, nesGetCHRMEMAbsoluteAddress(0x0800)&MASK_8KB, nesGetCHRMEMAbsoluteAddress(0x0800) );
   ui->chr2->setText ( buffer );
   sprintf ( buffer, "%02X:%05X(%05X)", nesGetCHRMEMAbsoluteAddress(0x0C00)>>13, nesGetCHRMEMAbsoluteAddress(0x0C00)&MASK_8KB, nesGetCHRMEMAbsoluteAddress(0x0C00) );
   ui->chr3->setText ( buffer );
   sprintf ( buffer, "%02X:%05X(%05X)", nesGetCHRMEMAbsoluteAddress(0x1000)>>13, nesGetCHRMEMAbsoluteAddress(0x1000)&MASK_8KB, nesGetCHRMEMAbsoluteAddress(0x1000) );
   ui->chr4->setText ( buffer );
   sprintf ( buffer, "%02X:%05X(%05X)", nesGetCHRMEMAbsoluteAddress(0x1400)>>13, nesGetCHRMEMAbsoluteAddress(0x1400)&MASK_8KB, nesGetCHRMEMAbsoluteAddress(0x1400) );
   ui->chr5->setText ( buffer );
   sprintf ( buffer, "%02X:%05X(%05X)", nesGetCHRMEMAbsoluteAddress(0x1800)>>13, nesGetCHRMEMAbsoluteAddress(0x1800)&MASK_8KB, nesGetCHRMEMAbsoluteAddress(0x1800) );
   ui->chr6->setText ( buffer );
   sprintf ( buffer, "%02X:%05X(%05X)", nesGetCHRMEMAbsoluteAddress(0x1C00)>>13, nesGetCHRMEMAbsoluteAddress(0x1C00)&MASK_8KB, nesGetCHRMEMAbsoluteAddress(0x1C00) );
   ui->chr7->setText ( buffer );

   switch ( nesGetMapper() )
   {
      case 1:
         nesMapper001GetInformation(&mapper001Info);
         sprintf ( buffer, "%02X", mapper001Info.shiftRegister );
         ui->shiftRegister->setText ( buffer );
         sprintf ( buffer, "%d", mapper001Info.shiftRegisterBit );
         ui->shiftRegisterBit->setText ( buffer );
         break;

      case 4:
         nesMapper004GetInformation(&mapper004Info);
         ui->irqEnabled->setChecked ( mapper004Info.irqEnabled );
         ui->irqAsserted->setChecked ( mapper004Info.irqAsserted );
         ui->ppuAddrA12->setChecked ( mapper004Info.ppuAddrA12 );
         sprintf ( buffer, "%02X", mapper004Info.irqReload );
         ui->irqReload->setText ( buffer );
         sprintf ( buffer, "%02X", mapper004Info.irqCounter );
         ui->irqCounter->setText ( buffer );
         sprintf ( buffer, "%d", mapper004Info.ppuCycle );
         ui->lastA12Cycle->setText ( buffer );
         break;
   }

   // Check breakpoints for hits and highlight if necessary...
   for ( idx = 0; idx < pBreakpoints->GetNumBreakpoints(); idx++ )
   {
      BreakpointInfo* pBreakpoint = pBreakpoints->GetBreakpoint(idx);

      if ( pBreakpoint->hit )
      {
         if ( pBreakpoint->type == eBreakOnMapperEvent )
         {
            // Update display...
            show();
         }
      }
   }
}
