/*
** FamiTracker - NES/Famicom sound tracker
** Copyright (C) 2005-2012  Jonathan Liss
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

//
// This is the base class for all classes that takes care of 
// translating notes to channel register writes.
//

#include "stdafx.h"
#include "FamiTracker.h"
#include "FamiTrackerDoc.h"
#include "SoundGen.h"
#include "ChannelHandler.h"

// Range for the pitch wheel command in notes
int CChannelHandler::PITCH_WHEEL_RANGE = 6;


CChannelHandler::CChannelHandler() : 
	m_iChannelID(0), 
	m_bEnabled(false), 
	m_iInstrument(0), 
	m_iLastInstrument(MAX_INSTRUMENTS),
	m_pNoteLookupTable(NULL),
	m_pVibratoTable(NULL),
	m_pDocument(NULL),
	m_pAPU(NULL),
	m_iPitch(0),
	m_iNote(0),
	m_iDefaultDuty(0),
	m_iDutyPeriod(0),
	m_iMaxPeriod(0x7FF),		// Default for 2A03 regs
	m_bGate(false)
{ 
	m_iSeqVolume = 0;
}

CChannelHandler::~CChannelHandler()
{
}

void CChannelHandler::InitChannel(CAPU *pAPU, int *pVibTable, CFamiTrackerDoc *pDoc)
{
	// Called from main thread

	m_pAPU = pAPU;
	m_pVibratoTable = pVibTable;
	m_pDocument = pDoc;

	m_bDelayEnabled = false;

	m_iEffect = 0;

	m_iVibratoStyle = VIBRATO_NEW;

	ResetChannel();
}

int CChannelHandler::LimitPeriod(int Period) const
{
	if (Period > m_iMaxPeriod)
		Period = m_iMaxPeriod;

	if (Period < 0)
		Period = 0;

	return Period;
}

int CChannelHandler::LimitVolume(int Volume) const
{
	if (Volume > 15)
		Volume = 15;

	if (Volume < 0)
		Volume = 0;

	return Volume;
}

void CChannelHandler::SetMaxPeriod(int Period)
{
	m_iMaxPeriod = Period;
}

void CChannelHandler::SetPitch(int Pitch)
{
	// Pitch ranges from -511 to +512
	m_iPitch = Pitch;
	if (m_iPitch == 512)
		m_iPitch = 511;
}

int CChannelHandler::GetPitch() const 
{ 
	if (m_iPitch != 0 && m_iNote != 0 && m_pNoteLookupTable != NULL) {
		// Interpolate pitch
		int LowNote = m_iNote - PITCH_WHEEL_RANGE;
		int HighNote = m_iNote + PITCH_WHEEL_RANGE;

		if (LowNote < 0)
			LowNote = 0;
		if (HighNote > 95)
			HighNote = 95;

		int Freq = m_pNoteLookupTable[m_iNote];
		int Lower = m_pNoteLookupTable[LowNote];
		int Higher = m_pNoteLookupTable[HighNote];
		int Pitch;

		if (m_iPitch < 0)
			Pitch = (Freq - Lower);
		else
			Pitch = (Higher - Freq);

		return (Pitch * m_iPitch) / 511;
	}

	return 0;
}

void CChannelHandler::SetVibratoStyle(int Style)
{
	m_iVibratoStyle = Style;
}

void CChannelHandler::Arpeggiate(unsigned int Note)
{
	m_iPeriod = TriggerNote(Note);
}

void CChannelHandler::ResetChannel()
{
	// Resets the channel states (volume, instrument & duty)
	// Clears channel registers

	// Instrument 
	m_iInstrument		= MAX_INSTRUMENTS;
	m_iLastInstrument	= MAX_INSTRUMENTS;

	for (int i = 0; i < SEQ_COUNT; ++i)
		m_iSeqEnabled[i] = 0;

	// Volume 
	m_iVolume			= MAX_VOL;

	m_iDefaultDuty		= 0;
	m_iSeqVolume		= 0;

	// Period
	m_iPeriod			= 0;
	m_iLastPeriod		= 0xFFFF;
	m_iPeriodPart		= 0;

	// Effect states
	m_iPortaSpeed		= 0;
	m_iPortaTo			= 0;
	m_iArpeggio			= 0;
	m_iArpState			= 0;
	m_iVibratoSpeed		= 0;
	m_iVibratoPhase		= (m_iVibratoStyle == VIBRATO_OLD) ? 48 : 0;
	m_iTremoloSpeed		= 0;
	m_iTremoloPhase		= 0;
	m_iFinePitch		= 0x80;
	m_iPeriod			= 0;
	m_iVolSlide			= 0;
	m_bDelayEnabled		= false;
	m_iNoteCut			= 0;
	m_iVibratoDepth		= 0;
	m_iTremoloDepth		= 0;

	// States
	m_bRelease			= false;
	m_bGate				= false;
	m_bEnabled			= false;

	RegisterKeyState(m_iChannelID, -1);

	// Clear channel registers
	ClearRegisters();
}

// Handle common things before letting the channels play the notes
void CChannelHandler::PlayNote(stChanNote *pNoteData, int EffColumns)
{
	ASSERT (pNoteData != NULL);

	// Handle delay commands
	if (HandleDelay(pNoteData, EffColumns))
		return;

	// Handle global effects
	theApp.GetSoundGenerator()->EvaluateGlobalEffects(pNoteData, EffColumns);

	// Let the channel play
	HandleNoteData(pNoteData, EffColumns);
}

void CChannelHandler::HandleCustomEffects(int EffNum, int EffParam)
{
}

bool CChannelHandler::HandleInstrument(int Instrument, bool Trigger, bool NewInstrument)
{
	return true;
}

void CChannelHandler::HandleEmptyNote()
{
}

void CChannelHandler::HandleHalt()
{
}

void CChannelHandler::HandleRelease()
{
}

void CChannelHandler::HandleNote(int Note, int Octave)
{
}

void CChannelHandler::HandleNoteData(stChanNote *pNoteData, int EffColumns)
{
	int LastInstrument = m_iInstrument;
	int Instrument = pNoteData->Instrument;
	int Note = pNoteData->Note;

	// Clear the note cut effect
	if (pNoteData->Note != NONE) {
		m_iNoteCut = 0;
	}

	// Effects
	for (int n = 0; n < EffColumns; n++) {
		unsigned char EffNum   = pNoteData->EffNumber[n];
		unsigned char EffParam = pNoteData->EffParam[n];
		HandleCustomEffects(EffNum, EffParam);
	}

	// Instrument
	if (Note == HALT || Note == RELEASE) 
		Instrument = MAX_INSTRUMENTS;	// Ignore instrument for release and halt commands

	if (Instrument != MAX_INSTRUMENTS)
		m_iInstrument = Instrument;

	bool Trigger = (Note != NONE) && (Note != HALT) && (Note != RELEASE);
	bool NewInstrument = (m_iInstrument != LastInstrument) || (m_iInstrument == MAX_INSTRUMENTS);

	if (m_iInstrument == MAX_INSTRUMENTS) {
		// No instrument selected, default to 0
		m_iInstrument = 0;
	}

	if (NewInstrument || Trigger) {
		if (!HandleInstrument(m_iInstrument, Trigger, NewInstrument))
			return;
	}

	// Volume
	if (pNoteData->Vol < 0x10) {
		m_iVolume = pNoteData->Vol << VOL_SHIFT;
	}

	// Clear release flag
	if (pNoteData->Note != RELEASE && pNoteData->Note != NONE) {
		m_bRelease = false;
	}

	// Note
	switch (pNoteData->Note) {
		case NONE:
			HandleEmptyNote();
			break;
		case HALT:
			HandleHalt();
			break;
		case RELEASE:
			HandleRelease();
			break;
		default:
			HandleNote(pNoteData->Note, pNoteData->Octave);
			break;
	}
}

void CChannelHandler::SetNoteTable(unsigned int *pNoteLookupTable)
{
	// Installs the note lookup table
	m_pNoteLookupTable = pNoteLookupTable;
}

unsigned int CChannelHandler::TriggerNote(int Note)
{
	if (Note >= NOTE_COUNT)
		Note = NOTE_COUNT - 1;
	if (Note < 0)
		Note = 0;

	// Trigger a note, return note period
	RegisterKeyState(m_iChannelID, Note);

	if (!m_pNoteLookupTable)
		return Note;

	return m_pNoteLookupTable[Note];
}

void CChannelHandler::CutNote()
{
	// Cut currently playing note

	RegisterKeyState(m_iChannelID, -1);

	m_bGate = false;
}

void CChannelHandler::ReleaseNote()
{
	// Release currently playing note

	if (!m_bEnabled)
		return;

	RegisterKeyState(m_iChannelID, -1);

	m_bRelease = true;
}

int CChannelHandler::RunNote(int Octave, int Note)
{
	// Run the note and handle portamento
	int NewNote = MIDI_NOTE(Octave, Note);
	int NesFreq = TriggerNote(NewNote);

	if (m_iPortaSpeed > 0 && m_iEffect == EF_PORTAMENTO) {
		if (m_iPeriod == 0)
			m_iPeriod = NesFreq;
		m_iPortaTo = NesFreq;
	}
	else
		m_iPeriod = NesFreq;

	m_bGate = true;

	return NewNote;
}

void CChannelHandler::SetupSlide(int Type, int EffParam)
{
	#define GET_SLIDE_SPEED(x) (((x & 0xF0) >> 3) + 1)

	m_iPortaSpeed = GET_SLIDE_SPEED(EffParam);
	m_iEffect = Type;

	if (Type == EF_SLIDE_UP)
		m_iNote = m_iNote + (EffParam & 0xF);
	else
		m_iNote = m_iNote - (EffParam & 0xF);

	m_iPortaTo = TriggerNote(m_iNote);
}

bool CChannelHandler::CheckCommonEffects(unsigned char EffCmd, unsigned char EffParam)
{
	// Handle common effects for all channels

	switch (EffCmd) {
		case EF_PORTAMENTO:
			m_iPortaSpeed = EffParam;
			m_iEffect = EF_PORTAMENTO;
			if (!EffParam)
				m_iPortaTo = 0;
			break;
		case EF_VIBRATO:
			m_iVibratoDepth = (EffParam & 0x0F) << 4;
			m_iVibratoSpeed = EffParam >> 4;
			if (!EffParam)
				m_iVibratoPhase = (m_iVibratoStyle == VIBRATO_OLD) ? 48 : 0;
			break;
		case EF_TREMOLO:
			m_iTremoloDepth = (EffParam & 0x0F) << 4;
			m_iTremoloSpeed = EffParam >> 4;
			if (!EffParam)
				m_iTremoloPhase = 0;
			break;
		case EF_ARPEGGIO:
			m_iArpeggio = EffParam;
			m_iEffect = EF_ARPEGGIO;
			break;
		case EF_PITCH:
			m_iFinePitch = EffParam;
			break;
		case EF_PORTA_DOWN:
			m_iPortaSpeed = EffParam;
			m_iEffect = EF_PORTA_DOWN;
			break;
		case EF_PORTA_UP:
			m_iPortaSpeed = EffParam;
			m_iEffect = EF_PORTA_UP;
			break;
		case EF_VOLUME_SLIDE:
			m_iVolSlide = EffParam;
			break;
		case EF_NOTE_CUT:
			m_iNoteCut = EffParam + 1;
			break;
//		case EF_TARGET_VOLUME_SLIDE:
			// TODO implement
//			break;
		default:
			return false;
	}
	
	return true;
}

bool CChannelHandler::HandleDelay(stChanNote *pNoteData, int EffColumns)
{
	// Handle note delay, Gxx

	if (m_bDelayEnabled) {
		m_bDelayEnabled = false;
		HandleNoteData(&m_cnDelayed, m_iDelayEffColumns);
	}
	
	// Check delay
	for (int i = 0; i < EffColumns; ++i) {
		if (pNoteData->EffNumber[i] == EF_DELAY && pNoteData->EffParam[i] > 0) {
			m_bDelayEnabled = true;
			m_cDelayCounter = pNoteData->EffParam[i];
			m_iDelayEffColumns = EffColumns;
			memcpy(&m_cnDelayed, pNoteData, sizeof(stChanNote));

			// Only one delay/row is allowed. Remove global effects
			for (int j = 0; j < EffColumns; ++j) {
				switch (m_cnDelayed.EffNumber[j]) {
					case EF_DELAY:
						m_cnDelayed.EffNumber[j] = EF_NONE;
						m_cnDelayed.EffParam[j] = 0;
						break;
					case EF_JUMP:
						theApp.GetSoundGenerator()->SetJumpPattern(m_cnDelayed.EffParam[j]);
						m_cnDelayed.EffNumber[j] = EF_NONE;
						m_cnDelayed.EffParam[j] = 0;
						break;
					case EF_SKIP:
						theApp.GetSoundGenerator()->SetSkipRow(m_cnDelayed.EffParam[j]);
						m_cnDelayed.EffNumber[j] = EF_NONE;
						m_cnDelayed.EffParam[j] = 0;
						break;
				}
			}
			return true;
		}
	}

	return false;
}

void CChannelHandler::UpdateNoteCut()
{
	// Note cut ()
	if (m_iNoteCut > 0) {
		m_iNoteCut--;
		if (m_iNoteCut == 0) {
			CutNote();
		}
	}
}

void CChannelHandler::UpdateDelay()
{
	// Delay (Gxx)
	if (m_bDelayEnabled) {
		if (!m_cDelayCounter) {
			m_bDelayEnabled = false;
			PlayNote(&m_cnDelayed, m_iDelayEffColumns);
		}
		else
			m_cDelayCounter--;
	}
}

void CChannelHandler::UpdateVolumeSlide()
{
	// Volume slide (Axx)
	m_iVolume -= (m_iVolSlide & 0x0F);
	if (m_iVolume < 0)
		m_iVolume = 0;

	m_iVolume += (m_iVolSlide & 0xF0) >> 4;
	if (m_iVolume < 0)
		m_iVolume = MAX_VOL;
}

void CChannelHandler::UpdateTargetVolumeSlide()
{
	// TODO implement
}

void CChannelHandler::UpdateVibratoTremolo()
{
	// Vibrato and tremolo
	m_iVibratoPhase = (m_iVibratoPhase + m_iVibratoSpeed) & 63;
	m_iTremoloPhase = (m_iTremoloPhase + m_iTremoloSpeed) & 63;
}

void CChannelHandler::LinearAdd(int Step)
{
	m_iPeriod = (m_iPeriod << 5) | m_iPeriodPart;
	int value = (m_iPeriod * Step) / 512;
	if (value == 0)
		value = 1;
	m_iPeriod += value;
	m_iPeriodPart = m_iPeriod & 0x1F;
	m_iPeriod >>= 5;
}

void CChannelHandler::LinearRemove(int Step)
{
	m_iPeriod = (m_iPeriod << 5) | m_iPeriodPart;
	int value = (m_iPeriod * Step) / 512;
	if (value == 0)
		value = 1;
	m_iPeriod -= value;
	m_iPeriodPart = m_iPeriod & 0x1F;
	m_iPeriod >>= 5;
}

void CChannelHandler::PeriodAdd(int Step)
{
	if (m_pDocument->GetLinearPitch())
		LinearAdd(Step);
	else
		m_iPeriod += Step;
}

void CChannelHandler::PeriodRemove(int Step)
{
	if (m_pDocument->GetLinearPitch())
		LinearRemove(Step);
	else
		m_iPeriod -= Step;
}

void CChannelHandler::UpdateEffects()
{
	// Handle other effects
	switch (m_iEffect) {
		case EF_ARPEGGIO:
			if (m_iArpeggio != 0) {
				switch (m_iArpState) {
					case 0:
						m_iPeriod = TriggerNote(m_iNote);
						break;
					case 1:
						m_iPeriod = TriggerNote(m_iNote + (m_iArpeggio >> 4));
						if ((m_iArpeggio & 0x0F) == 0)
							++m_iArpState;
						break;
					case 2:
						m_iPeriod = TriggerNote(m_iNote + (m_iArpeggio & 0x0F));
						break;
				}
				m_iArpState = (m_iArpState + 1) % 3;
			}
			break;
		case EF_PORTAMENTO:
		case EF_SLIDE_UP:
		case EF_SLIDE_DOWN:
			// Automatic portamento
			if (m_iPortaSpeed > 0 && m_iPortaTo > 0) {
				if (m_iPeriod > m_iPortaTo) {
					PeriodRemove(m_iPortaSpeed);
					// TODO: check this
//					if (m_iPeriod > 0x1000)	// it was negative
//						m_iPeriod = 0x00;
					if (m_iPeriod < m_iPortaTo)
						m_iPeriod = m_iPortaTo;
				}
				else if (m_iPeriod < m_iPortaTo) {
					PeriodAdd(m_iPortaSpeed);
					if (m_iPeriod > m_iPortaTo)
						m_iPeriod = m_iPortaTo;
				}
			}
			break;
		case EF_PORTA_DOWN:
			PeriodAdd(m_iPortaSpeed);
			m_iPeriod = LimitPeriod(m_iPeriod);
			break;
		case EF_PORTA_UP:
			PeriodRemove(m_iPortaSpeed);
			m_iPeriod = LimitPeriod(m_iPeriod);
			break;
	}
}

void CChannelHandler::ProcessChannel()
{
	// Run all default and common channel processing
	// This gets called each frame
	//

	UpdateDelay();
	UpdateNoteCut();

	if (!m_bEnabled)
		return;

	UpdateVolumeSlide();
	UpdateVibratoTremolo();
	UpdateEffects();
}

int CChannelHandler::GetVibrato() const
{
	// Vibrato offset (4xx)
	int VibFreq;

	if ((m_iVibratoPhase & 0xF0) == 0x00)
		VibFreq = m_pVibratoTable[m_iVibratoDepth + m_iVibratoPhase];
	else if ((m_iVibratoPhase & 0xF0) == 0x10)
		VibFreq = m_pVibratoTable[m_iVibratoDepth + 15 - (m_iVibratoPhase - 16)];
	else if ((m_iVibratoPhase & 0xF0) == 0x20)
		VibFreq = -m_pVibratoTable[m_iVibratoDepth + (m_iVibratoPhase - 32)];
	else if ((m_iVibratoPhase & 0xF0) == 0x30)
		VibFreq = -m_pVibratoTable[m_iVibratoDepth + 15 - (m_iVibratoPhase - 48)];

	if (m_pDocument->GetVibratoStyle() == VIBRATO_OLD) {
		VibFreq += m_pVibratoTable[m_iVibratoDepth + 15] + 1;
		VibFreq >>= 1;
	}

	if (m_pDocument->GetLinearPitch())
		VibFreq = (m_iPeriod * VibFreq) / 128;

	return VibFreq;
}

int CChannelHandler::GetTremolo() const
{
	// Tremolo offset (7xx)
	int TremVol;
	int Phase = m_iTremoloPhase >> 1;

	if ((Phase & 0xF0) == 0x00)
		TremVol = m_pVibratoTable[m_iTremoloDepth + Phase];
	else if ((Phase & 0xF0) == 0x10)
		TremVol = m_pVibratoTable[m_iTremoloDepth + 15 - (Phase - 16)];

	return (TremVol >> 1);
}

int CChannelHandler::GetFinePitch() const
{
	// Fine pitch setting (Pxx)
	return (0x80 - m_iFinePitch);
}

// Sequence routines

void CChannelHandler::RunSequence(int Index, CSequence *pSequence)
{
	if (m_iSeqEnabled[Index] == 1 && pSequence->GetItemCount() > 0) {

		int Value = pSequence->GetItem(m_iSeqPointer[Index]);

		switch (Index) {
			// Volume modifier
			case SEQ_VOLUME:
				m_iSeqVolume = Value;
				break;
			// Arpeggiator
			case SEQ_ARPEGGIO:
				switch (pSequence->GetSetting()) {
					case ARP_SETTING_ABSOLUTE:
						m_iPeriod = TriggerNote(m_iNote + Value);
						break;
					case ARP_SETTING_FIXED:
						m_iPeriod = TriggerNote(Value);
						break;
					case ARP_SETTING_RELATIVE:
						m_iNote += Value;
						if (m_iNote > 95)
							m_iNote = 95;
						if (m_iNote < 0)
							m_iNote = 0;
						m_iPeriod = TriggerNote(m_iNote);
						break;
				}
				break;
			// Pitch
			case SEQ_PITCH:
				m_iPeriod += Value;
				m_iPeriod = LimitPeriod(m_iPeriod);
				break;
			// Hi-pitch
			case SEQ_HIPITCH:
				m_iPeriod += Value << 4;
				m_iPeriod = LimitPeriod(m_iPeriod);
				break;
			// Duty cycling
			case SEQ_DUTYCYCLE:
				m_iDutyPeriod = Value;
				break;
		}

		m_iSeqPointer[Index]++;

		int Release = pSequence->GetReleasePoint();
		int Items = pSequence->GetItemCount();
		int Loop = pSequence->GetLoopPoint();

		if (m_iSeqPointer[Index] == (Release + 1) || m_iSeqPointer[Index] == Items) {
			// End point reached
			if (Loop != -1 && !(m_bRelease && Release != -1)) {
				m_iSeqPointer[Index] = Loop;
			}
			else {
				if (m_iSeqPointer[Index] == Items) {
					// End of sequence 
					m_iSeqEnabled[Index] = 2;
				}
				else if (!m_bRelease)
					// Waiting for release
					m_iSeqPointer[Index]--;
			}
		}

		pSequence->SetPlayPos(m_iSeqPointer[Index]);
	}
	else if (m_iSeqEnabled[Index] == 2) {
		
		///////////////// temporary /////////////////////

		switch (Index) {
			case SEQ_ARPEGGIO:
				if (pSequence->GetSetting() == ARP_SETTING_FIXED) {
					m_iPeriod = TriggerNote(m_iNote);
				}
				break;
		}

		m_iSeqEnabled[Index] = 0;

		///////////////// temporary /////////////////////

		pSequence->SetPlayPos(-1);
	}
}

CSequence *CChannelHandler::GetSequence(int Index, int Type)
{
	// Return a sequence, must be overloaded
	return NULL;
}

void CChannelHandler::ReleaseSequences(int Chip)
{
	if (!m_bEnabled)
		return;

	for (int i = 0; i < SEQ_COUNT; ++i) {
		if (m_iSeqEnabled[i] == 1) {
			CSequence *pSeq = m_pDocument->GetSequence(Chip, m_iSeqIndex[i], i);
			ReleaseSequence(i, pSeq);
		}
	}

//	m_bSequencesReleased = true;
}

void CChannelHandler::ReleaseSequence(int Index, CSequence *pSeq)
{
	int ReleasePoint = pSeq->GetReleasePoint();

	if (ReleasePoint != -1) {
		m_iSeqPointer[Index] = ReleasePoint;
	}
}

int CChannelHandler::CalculatePeriod(bool InvertPitch) const 
{
	if (InvertPitch)
		return LimitPeriod(m_iPeriod + GetVibrato() - GetFinePitch() - GetPitch());

	return LimitPeriod(m_iPeriod - GetVibrato() + GetFinePitch() + GetPitch());
}

int CChannelHandler::CalculateVolume(int Limit) const
{
	// Volume calculation
	int Volume;

	Volume = m_iVolume >> VOL_SHIFT;
	Volume = (m_iSeqVolume * Volume) / 15 - GetTremolo();

	if (Volume < 0)
		Volume = 0;
	if (Volume > Limit)
		Volume = Limit;

	if (m_iSeqVolume > 0 && m_iVolume > 0 && Volume == 0)
		Volume = 1;

	if (!m_bGate)
		Volume = 0;

	return Volume;
}

void CChannelHandler::AddCycles(int count)
{
	theApp.GetSoundGenerator()->AddCycles(count);
}

void CChannelHandler::WriteRegister(uint16 Reg, uint8 Value)
{
	m_pAPU->Write(Reg, Value);
	theApp.GetSoundGenerator()->WriteRegister(Reg, Value);
}

void CChannelHandler::WriteExternalRegister(uint16 Reg, uint8 Value)
{
	m_pAPU->ExternalWrite(Reg, Value);
}

void CChannelHandler::RegisterKeyState(int Channel, int Note)
{
	theApp.GetSoundGenerator()->RegisterKeyState(Channel, Note);
}
