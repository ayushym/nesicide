//    NESICIDE - an IDE for the 8-bit NES.
//    Copyright (C) 2009  Christopher S. Pow

//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "cnesrommapper034.h"

#include "cregisterdata.h"

// Mapper 034 Registers
static CBitfieldData* tbl8000_FFFFBitfields [] =
{
   new CBitfieldData("PRG Bank", 0, 8, "%X", 0)
};

static CBitfieldData* tbl7FFDBitfields [] =
{
   new CBitfieldData("PRG Bank", 0, 8, "%X", 0)
};

static CBitfieldData* tbl7FFEBitfields [] =
{
   new CBitfieldData("CHR Bank", 0, 8, "%X", 0)
};

static CBitfieldData* tbl7FFFBitfields [] =
{
   new CBitfieldData("CHR Bank", 0, 8, "%X", 0)
};

static CRegisterData* tblRegisters [] =
{
   new CRegisterData(0x7ffd, "NINA-001 PRG Mapping", nesMapperLowRead, nesMapperLowWrite, 1, tbl7FFDBitfields),
   new CRegisterData(0x7ffe, "NINA-001 CHR Mapping", nesMapperLowRead, nesMapperLowWrite, 1, tbl7FFEBitfields),
   new CRegisterData(0x7fff, "NINA-001 CHR Mapping", nesMapperLowRead, nesMapperLowWrite, 1, tbl7FFFBitfields),
   new CRegisterData(0x8000, "BxROM PRG Mapping", nesMapperHighRead, nesMapperHighWrite, 1, tbl8000_FFFFBitfields)
};

static const char* rowHeadings [] =
{
   ""
};

static const char* columnHeadings [] =
{
   "7FFD",
   "7FFE",
   "7FFF",
   "8000"
};

static CRegisterDatabase* dbRegisters = new CRegisterDatabase(eMemory_cartMapper,1,4,4,tblRegisters,rowHeadings,columnHeadings);

uint8_t  CROMMapper034::m_reg[] = { 0x00, };

CROMMapper034::CROMMapper034()
{
}

CROMMapper034::~CROMMapper034()
{
}

void CROMMapper034::RESET ( bool soft )
{
   m_mapper = 34;

   m_dbRegisters = dbRegisters;

   CROM::RESET ( m_mapper, soft );

   m_pPRGROMmemory [ 0 ] = m_PRGROMmemory [ 0 ];
   m_pPRGROMmemory [ 1 ] = m_PRGROMmemory [ 1 ];
   m_pPRGROMmemory [ 2 ] = m_PRGROMmemory [ 2 ];
   m_pPRGROMmemory [ 3 ] = m_PRGROMmemory [ 3 ];

   // CHR ROM/RAM already set up in CROM::RESET()...
}

uint32_t CROMMapper034::LMAPPER ( uint32_t addr )
{
   if ( (addr >= 0x7ffd) && (addr < 0x8000) )
   {
      return m_reg[addr-0x7ffd];
   }
   else if ( addr >= 0x6000 )
   {
      return CROM::SRAMVIRT(addr);
   }
}

void CROMMapper034::LMAPPER ( uint32_t addr, uint8_t data )
{
   uint8_t bank;

   if ( (addr >= 0x7ffd) && (addr < 0x8000) )
   {
      m_reg[addr-0x7ffd] = data;

      switch ( addr )
      {
      case 0x7ffd:
         bank = (m_reg[0]&(m_numPrgBanks-1))<<2;

         m_pPRGROMmemory [ 0 ] = m_PRGROMmemory [ bank+0 ];
         m_pPRGROMmemory [ 1 ] = m_PRGROMmemory [ bank+1 ];
         m_pPRGROMmemory [ 2 ] = m_PRGROMmemory [ bank+2 ];
         m_pPRGROMmemory [ 3 ] = m_PRGROMmemory [ bank+3 ];
         break;
      case 0x7ffe:
         bank = (m_reg[1]&((m_numChrBanks<<1)-1));

         m_pCHRmemory [ 0 ] = m_CHRmemory [ (bank<<2)+0 ];
         m_pCHRmemory [ 1 ] = m_CHRmemory [ (bank<<2)+1 ];
         m_pCHRmemory [ 2 ] = m_CHRmemory [ (bank<<2)+2 ];
         m_pCHRmemory [ 3 ] = m_CHRmemory [ (bank<<2)+3 ];
         break;
      case 0x7fff:
         bank = (m_reg[2]&((m_numChrBanks<<1)-1));

         m_pCHRmemory [ 4 ] = m_CHRmemory [ (bank<<2)+0 ];
         m_pCHRmemory [ 5 ] = m_CHRmemory [ (bank<<2)+1 ];
         m_pCHRmemory [ 6 ] = m_CHRmemory [ (bank<<2)+2 ];
         m_pCHRmemory [ 7 ] = m_CHRmemory [ (bank<<2)+3 ];
         break;
      }

      if ( nesIsDebuggable() )
      {
         // Check mapper state breakpoints...
         CNES::CHECKBREAKPOINT(eBreakInMapper,eBreakOnMapperState,0);
      }
   }
   else if ( addr >= 0x6000 )
   {
      CROM::SRAMVIRT(addr,data);
   }
}

uint32_t CROMMapper034::DEBUGINFO ( uint32_t addr )
{
   return m_reg[3];
}

void CROMMapper034::HMAPPER ( uint32_t addr, uint8_t data )
{
   uint8_t bank;

   m_reg[3] = data%m_numPrgBanks;

   bank = m_reg[3]<<2;

   m_pPRGROMmemory [ 0 ] = m_PRGROMmemory [ bank ];
   m_pPRGROMmemory [ 1 ] = m_PRGROMmemory [ bank+1 ];
   m_pPRGROMmemory [ 2 ] = m_PRGROMmemory [ bank+2 ];
   m_pPRGROMmemory [ 3 ] = m_PRGROMmemory [ bank+3 ];

   if ( nesIsDebuggable() )
   {
      // Check mapper state breakpoints...
      CNES::CHECKBREAKPOINT(eBreakInMapper,eBreakOnMapperState,0);
   }
}
