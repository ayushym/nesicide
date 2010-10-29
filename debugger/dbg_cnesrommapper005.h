#if !defined ( ROM_MAPPER005_H )
#define ROM_MAPPER005_H

#include "cnesrom.h"

class CROMMapper005 : public CROM
{
public:
   CROMMapper005();
   virtual ~CROMMapper005();

   static void RESET ();
   static uint32_t MAPPER ( uint32_t addr );
   static void MAPPER ( uint32_t addr, uint8_t data );
   static uint32_t LMAPPER ( uint32_t addr );
   static void LMAPPER ( uint32_t addr, uint8_t data );
   static void SYNCH ( uint32_t ppuCycle, uint32_t ppuAddr );
   static void SETCPU ( void );
   static void SETPPU ( void );
   static void LOAD ( MapperState* data );
   static void SAVE ( MapperState* data );

protected:
   // MMC5
   static uint8_t m_prgMode;
   static uint8_t m_chrMode;
   static uint8_t m_irqScanline;
   static uint8_t m_irqEnabled;
   static uint8_t m_irqStatus;
   static bool          m_prgRAM [ 4 ];
   static bool          m_wp;
   static uint8_t m_wp1;
   static uint8_t m_wp2;
   static uint8_t  m_mult1;
   static uint8_t  m_mult2;
   static uint16_t m_prod;
   static uint8_t  m_fillTile;
   static uint8_t  m_fillAttr;
};

#endif