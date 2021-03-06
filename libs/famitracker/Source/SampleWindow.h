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

#pragma once

#include "cqtmfc.h"

class CSampleWindowThread;

class CSampleWinState
{
public:
	virtual ~CSampleWinState() {};

	virtual void Activate() = 0;
	virtual void Deactivate() = 0;
	virtual void SetSampleRate(int SampleRate) = 0;
	virtual void SetSampleData(int *iSamples, unsigned int iCount) = 0;
	virtual void Draw(CDC *pDC, bool bMessage) = 0;

protected:
	static const int WIN_WIDTH = 145;
	static const int WIN_HEIGHT = 36;
};

// CSampleWindow

class CSampleWindow : public CWnd
{
   Q_OBJECT
   // Qt stuff
   QTimer* pTimer;
protected:
   QSize sizeHint() const { return QSize(141,36); } // Not sure why the size is protected...but...
public slots:
   void onIdleSlot();
public: // For some reason MOC doesn't like the protection specification inside DECLARE_DYNAMIC
   
	DECLARE_DYNAMIC(CSampleWindow)

public:
	CSampleWindow();
	virtual ~CSampleWindow();

	void SetSampleRate(int SampleRate);
	void FlushSamples(int *Samples, int Count);

	UINT ThreadProc();

	static const int WIN_WIDTH = 145;
	static const int WIN_HEIGHT = 40;
 	
private:
	static const int STATE_COUNT = 4;

private:
	void NextState();

private:
	CSampleWinState *m_pStates[STATE_COUNT];
	unsigned int	m_iCurrentState;

	int				m_iBufferSize;
	int				*m_pBuffer1;
	int				*m_pBuffer2;
	int				*m_pFillBuffer;

	HANDLE			m_hNewSamples;

	bool			m_bThreadRunning;

	CCriticalSection m_csBufferSelect;
	CCriticalSection m_csBuffer;

public:
	virtual BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
public:
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
};
