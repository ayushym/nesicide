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

#include <iterator> 
#include <string>
#include <sstream>
#include <cmath>

#include "stdafx.h"
#include "FamiTracker.h"
#include "FamiTrackerDoc.h"
#include "FamiTrackerView.h"
#include "InstrumentEditPanel.h"
#include "InstrumentEditorN163Wave.h"
#include "MainFrm.h"

using namespace std;

// CInstrumentEditorN163Wave dialog

IMPLEMENT_DYNAMIC(CInstrumentEditorN163Wave, CInstrumentEditPanel)

CInstrumentEditorN163Wave::CInstrumentEditorN163Wave(CWnd* pParent) : CInstrumentEditPanel(CInstrumentEditorN163Wave::IDD, pParent),
	m_pWaveEditor(NULL), 
	m_pInstrument(NULL)
{
//   IDD_INSTRUMENT_N163_WAVE DIALOGEX 0, 0, 372, 172
   CRect rect(CPoint(0,0),CSize(372,172));
   MapDialogRect(&rect);
   setFixedSize(rect.Width(),rect.Height());
   
// CP: Put all GROUPBOX items first so their child elements can be created.
//       GROUPBOX        "Wave editor",IDC_STATIC,7,7,228,116
   CGroupBox* mfc1 = new CGroupBox(this);
   CRect r1(CPoint(7,7),CSize(228,116));
   MapDialogRect(&r1);
   mfc1->Create(_T("Wave editor"),WS_VISIBLE,r1,this,IDC_STATIC);
   // IDC_STATIC do not get added to MFC-to-Qt map.
//       GROUPBOX        "Wave #",IDC_STATIC,244,7,121,35
   CGroupBox* mfc21 = new CGroupBox(this);
   CRect r21(CPoint(244,7),CSize(121,35));
   MapDialogRect(&r21);
   mfc21->Create(_T("Wave #"),WS_VISIBLE,r21,this,IDC_STATIC);
   // IDC_STATIC do not get added to MFC-to-Qt map.
//       GROUPBOX        "MML string",IDC_STATIC,7,132,358,33
   CGroupBox* mfc7 = new CGroupBox(this);
   CRect r7(CPoint(7,132),CSize(358,33));
   MapDialogRect(&r7);
   mfc7->Create(_T("MML string"),WS_VISIBLE,r7,this,IDC_STATIC);
   // IDC_STATIC do not get added to MFC-to-Qt map.
//       GROUPBOX        "Wave RAM settings",IDC_STATIC,244,45,121,52
   CGroupBox* mfc16 = new CGroupBox(this);
   CRect r16(CPoint(244,45),CSize(121,52));
   MapDialogRect(&r16);
   mfc16->Create(_T("Wave RAM settings"),WS_VISIBLE,r16,this,IDC_STATIC);
   // IDC_STATIC do not get added to MFC-to-Qt map.
//       PUSHBUTTON      "Sine",IDC_PRESET_SINE,14,101,41,12
   CButton* mfc2 = new CButton(this);
   CRect r2(CPoint(14,104),CSize(41,14));
   MapDialogRect(&r2);
   mfc2->Create(_T("Sine"),WS_VISIBLE,r2,this,IDC_PRESET_SINE);
   mfcToQtWidget.insert(IDC_PRESET_SINE,mfc2);
   QObject::connect(mfc2,SIGNAL(clicked()),this,SLOT(presetSine_clicked()));
//       PUSHBUTTON      "Triangle",IDC_PRESET_TRIANGLE,57,101,41,12
   CButton* mfc3 = new CButton(this);
   CRect r3(CPoint(57,104),CSize(41,14));
   MapDialogRect(&r3);
   mfc3->Create(_T("Triangle"),WS_VISIBLE,r3,this,IDC_PRESET_TRIANGLE);
   mfcToQtWidget.insert(IDC_PRESET_TRIANGLE,mfc3);
   QObject::connect(mfc3,SIGNAL(clicked()),this,SLOT(presetTriangle_clicked()));
//       PUSHBUTTON      "Sawtooth",IDC_PRESET_SAWTOOTH,100,101,41,12
   CButton* mfc4 = new CButton(this);
   CRect r4(CPoint(100,104),CSize(41,14));
   MapDialogRect(&r4);
   mfc4->Create(_T("Sawtooth"),WS_VISIBLE,r4,this,IDC_PRESET_SAWTOOTH);
   mfcToQtWidget.insert(IDC_PRESET_SAWTOOTH,mfc4);
   QObject::connect(mfc4,SIGNAL(clicked()),this,SLOT(presetSawtooth_clicked()));
//       PUSHBUTTON      "50% pulse",IDC_PRESET_PULSE_50,143,101,41,12
   CButton* mfc5 = new CButton(this);
   CRect r5(CPoint(143,104),CSize(41,14));
   MapDialogRect(&r5);
   mfc5->Create(_T("50% pulse"),WS_VISIBLE,r5,this,IDC_PRESET_PULSE_50);
   mfcToQtWidget.insert(IDC_PRESET_PULSE_50,mfc5);
   QObject::connect(mfc5,SIGNAL(clicked()),this,SLOT(presetPulse50_clicked()));
//       PUSHBUTTON      "25% pulse",IDC_PRESET_PULSE_25,186,101,41,12
   CButton* mfc6 = new CButton(this);
   CRect r6(CPoint(186,104),CSize(41,14));
   MapDialogRect(&r6);
   mfc6->Create(_T("25% pulse"),WS_VISIBLE,r6,this,IDC_PRESET_PULSE_25);
   mfcToQtWidget.insert(IDC_PRESET_PULSE_25,mfc6);
   QObject::connect(mfc6,SIGNAL(clicked()),this,SLOT(presetPulse25_clicked()));
//       EDITTEXT        IDC_MML,16,145,342,14,ES_AUTOHSCROLL
   CEdit* mfc8 = new CEdit(this);
   CRect r8(CPoint(16,145),CSize(342,14));
   MapDialogRect(&r8);
   mfc8->Create(ES_AUTOHSCROLL | WS_VISIBLE,r8,this,IDC_MML);
   mfcToQtWidget.insert(IDC_MML,mfc8);
//       PUSHBUTTON      "Copy wave",IDC_COPY_WAVE,240,90,52,14
   CButton* mfc9 = new CButton(this);
   CRect r9(CPoint(249,105),CSize(50,14));
   MapDialogRect(&r9);
   mfc9->Create(_T("Copy wave"),WS_VISIBLE,r9,this,IDC_COPY_WAVE);
   mfcToQtWidget.insert(IDC_COPY,mfc9);
   QObject::connect(mfc9,SIGNAL(clicked()),this,SLOT(copy_clicked()));
//       PUSHBUTTON      "Paste wave",IDC_PASTE_WAVE,240,104,52,14
   CButton* mfc10 = new CButton(this);
   CRect r10(CPoint(308,105),CSize(50,14));
   MapDialogRect(&r10);
   mfc10->Create(_T("Paste wave"),WS_VISIBLE,r10,this,IDC_PASTE_WAVE);
   mfcToQtWidget.insert(IDC_PASTE,mfc10);
   QObject::connect(mfc10,SIGNAL(clicked()),this,SLOT(paste_clicked()));
//       EDITTEXT        IDC_INDEX,253,20,39,12,ES_AUTOHSCROLL | ES_READONLY
//       CONTROL         "",IDC_INDEX_SPIN,"msctls_updown32",UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS,281,26,11,14
   // CP: Note, we fake a MFC "spin-box" separate control by placing it over it's "buddy" and connecting signals appropriately
   // to mimic the buddy relationship.
   CEdit* mfc11 = new CEdit(this);
   CSpinButtonCtrl* mfc12 = new CSpinButtonCtrl(this);
   CRect r11(CPoint(253,20),CSize(39,12));
   CRect r12(CPoint(r11.right-11,r11.top),CSize(11,12));
   MapDialogRect(&r11);
   MapDialogRect(&r12);
   mfc11->Create(ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE,r11,this,IDC_INDEX);
   mfc11->setBuddy(mfc12);
   mfcToQtWidget.insert(IDC_INDEX,mfc11);
   QObject::connect(mfc11,SIGNAL(textChanged(QString)),this,SLOT(index_textChanged(QString)));
   mfc12->Create(UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | WS_VISIBLE,r12,this,IDC_INDEX_SPIN);
   mfc12->setBuddy(mfc11);
   mfcToQtWidget.insert(IDC_INDEX_SPIN,mfc12);
   QObject::connect(mfc12,SIGNAL(valueChanged(int)),this,SLOT(indexSpin_valueChanged(int)));
//       LTEXT           "of",IDC_STATIC,300,22,8,8
   CStatic* mfc13 = new CStatic(this);
   CRect r13(CPoint(300,22),CSize(8,8));
   MapDialogRect(&r13);
   mfc13->Create(_T("of"),WS_VISIBLE,r13,this,IDC_STATIC);
   // IDC_STATIC do not get added to MFC-to-Qt map.
//       EDITTEXT        IDC_WAVES,316,20,37,12,ES_AUTOHSCROLL | ES_READONLY
//       CONTROL         "",IDC_WAVES_SPIN,"msctls_updown32",UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS,342,19,11,14
   // CP: Note, we fake a MFC "spin-box" separate control by placing it over it's "buddy" and connecting signals appropriately
   // to mimic the buddy relationship.
   CEdit* mfc14 = new CEdit(this);
   CSpinButtonCtrl* mfc15 = new CSpinButtonCtrl(this);
   CRect r14(CPoint(316,20),CSize(37,12));
   CRect r15(CPoint(r14.right-11,r14.top),CSize(11,12));
   MapDialogRect(&r14);
   MapDialogRect(&r15);
   mfc14->Create(ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE,r14,this,IDC_WAVES);
   mfc14->setBuddy(mfc15);
   mfcToQtWidget.insert(IDC_WAVES,mfc14);
   QObject::connect(mfc14,SIGNAL(textChanged(QString)),this,SLOT(waves_textChanged(QString)));
   mfc15->Create(UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | WS_VISIBLE,r15,this,IDC_WAVES_SPIN);
   mfc15->setBuddy(mfc14);
   mfcToQtWidget.insert(IDC_WAVES_SPIN,mfc15);
   QObject::connect(mfc15,SIGNAL(valueChanged(int)),this,SLOT(wavesSpin_valueChanged(int)));
//       LTEXT           "Wave size",IDC_STATIC,253,59,34,8
   CStatic* mfc17 = new CStatic(this);
   CRect r17(CPoint(253,59),CSize(34,8));
   MapDialogRect(&r17);
   mfc17->Create(_T("Wave size"),WS_VISIBLE,r17,this,IDC_STATIC);
   // IDC_STATIC do not get added to MFC-to-Qt map.
//       COMBOBOX        IDC_WAVE_SIZE,305,57,48,30,CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP
   CComboBox* mfc18 = new CComboBox(this);
   CRect r18(CPoint(305,57),CSize(48,30));
   MapDialogRect(&r18);
   mfc18->Create(CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE,r18,this,IDC_WAVE_SIZE);
   mfcToQtWidget.insert(IDC_WAVE_SIZE,mfc18);
   QObject::connect(mfc18,SIGNAL(currentIndexChanged(int)),this,SLOT(waveSize_currentIndexChanged(int)));
//       LTEXT           "Wave position",IDC_STATIC,253,78,46,8
   CStatic* mfc19 = new CStatic(this);
   CRect r19(CPoint(253,78),CSize(46,8));
   MapDialogRect(&r19);
   mfc19->Create(_T("Wave position"),WS_VISIBLE,r19,this,IDC_STATIC);
   // IDC_STATIC do not get added to MFC-to-Qt map.
//       COMBOBOX        IDC_WAVE_POS,305,75,48,30,CBS_DROPDOWN | WS_VSCROLL | WS_TABSTOP
   CComboBox* mfc20 = new CComboBox(this);
   CRect r20(CPoint(305,75),CSize(48,30));
   MapDialogRect(&r20);
   mfc20->Create(CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE,r20,this,IDC_WAVE_POS);
   mfcToQtWidget.insert(IDC_WAVE_POS,mfc20);
   QObject::connect(mfc20,SIGNAL(currentIndexChanged(int)),this,SLOT(wavePos_currentIndexChanged(int)));
}

CInstrumentEditorN163Wave::~CInstrumentEditorN163Wave()
{
	SAFE_RELEASE(m_pWaveEditor);
}

void CInstrumentEditorN163Wave::DoDataExchange(CDataExchange* pDX)
{
	CInstrumentEditPanel::DoDataExchange(pDX);
}

void CInstrumentEditorN163Wave::SelectInstrument(int Instrument)
{
	m_pInstrument = (CInstrumentN163*)GetDocument()->GetInstrument(Instrument);

	CComboBox *pSizeBox = (CComboBox*)GetDlgItem(IDC_WAVE_SIZE);
	CComboBox *pPosBox = (CComboBox*)GetDlgItem(IDC_WAVE_POS);

	CString SizeStr;
	SizeStr.Format(_T("%i"), m_pInstrument->GetWaveSize());
	pSizeBox->SelectString(0, SizeStr);

	FillPosBox(m_pInstrument->GetWaveSize());

	CString PosStr;
	PosStr.Format(_T("%i"), m_pInstrument->GetWavePos());
	pPosBox->SetWindowText(PosStr);

	/*
	if (m_pInstrument->GetAutoWavePos()) {
		CheckDlgButton(IDC_POSITION, 1);
		GetDlgItem(IDC_WAVE_POS)->EnableWindow(FALSE);
	}
	else {
		CheckDlgButton(IDC_POSITION, 0);
		GetDlgItem(IDC_WAVE_POS)->EnableWindow(TRUE);
	}
	*/

	if (m_pWaveEditor) {
		m_pWaveEditor->SetInstrument(m_pInstrument);
		m_pWaveEditor->SetLength(m_pInstrument->GetWaveSize());
	}

	CSpinButtonCtrl *pIndexSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_INDEX_SPIN);
	CSpinButtonCtrl *pWavesSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_WAVES_SPIN);

	int WaveCount = m_pInstrument->GetWaveCount();

	pIndexSpin->SetRange(0, WaveCount - 1);
	pIndexSpin->SetPos(0);
	pWavesSpin->SetPos(WaveCount - 1);

	m_iWaveIndex = 0;
}

//BEGIN_MESSAGE_MAP(CInstrumentEditorN163Wave, CInstrumentEditPanel)
//	ON_COMMAND(IDC_PRESET_SINE, OnPresetSine)
//	ON_COMMAND(IDC_PRESET_TRIANGLE, OnPresetTriangle)
//	ON_COMMAND(IDC_PRESET_SAWTOOTH, OnPresetSawtooth)
//	ON_COMMAND(IDC_PRESET_PULSE_50, OnPresetPulse50)
//	ON_COMMAND(IDC_PRESET_PULSE_25, OnPresetPulse25)
//	ON_MESSAGE(WM_USER_WAVE_CHANGED, OnWaveChanged)
//	ON_BN_CLICKED(IDC_COPY, OnBnClickedCopy)
//	ON_BN_CLICKED(IDC_PASTE, OnBnClickedPaste)
//	ON_CBN_SELCHANGE(IDC_WAVE_SIZE, OnWaveSizeChange)
//	ON_CBN_EDITCHANGE(IDC_WAVE_POS, OnWavePosChange)
//	ON_CBN_SELCHANGE(IDC_WAVE_POS, OnWavePosSelChange)
////	ON_BN_CLICKED(IDC_POSITION, OnPositionClicked)
//	ON_EN_CHANGE(IDC_WAVES, OnWavesChange)
//	ON_EN_CHANGE(IDC_INDEX, OnIndexChange)
//END_MESSAGE_MAP()

void CInstrumentEditorN163Wave::presetSine_clicked()
{
   OnPresetSine();
}

void CInstrumentEditorN163Wave::presetTriangle_clicked()
{
   OnPresetTriangle();
}

void CInstrumentEditorN163Wave::presetSawtooth_clicked()
{
   OnPresetSawtooth();
}

void CInstrumentEditorN163Wave::presetPulse50_clicked()
{
   OnPresetPulse50();
}

void CInstrumentEditorN163Wave::presetPulse25_clicked()
{
   OnPresetPulse25();
}

void CInstrumentEditorN163Wave::copy_clicked()
{
   OnBnClickedCopy();
}

void CInstrumentEditorN163Wave::paste_clicked()
{
   OnBnClickedPaste();
}

void CInstrumentEditorN163Wave::index_textChanged(QString text)
{
   OnIndexChange();
}

void CInstrumentEditorN163Wave::indexSpin_valueChanged(int val)
{
   // Update the "buddy"
   SetDlgItemInt(IDC_INDEX,val);
   OnIndexChange();
}

void CInstrumentEditorN163Wave::waves_textChanged(QString text)
{
   OnWavesChange();
}

void CInstrumentEditorN163Wave::wavesSpin_valueChanged(int val)
{
   // Update the "buddy"
   SetDlgItemInt(IDC_WAVES,val);
   OnWavesChange();
}

void CInstrumentEditorN163Wave::waveSize_currentIndexChanged(int index)
{
   OnWaveSizeChange();
}

void CInstrumentEditorN163Wave::wavePos_editTextChanged(QString text)
{
   OnWavePosChange();
}

void CInstrumentEditorN163Wave::wavePos_currentIndexChanged(int index)
{
   OnWavePosSelChange();
}

// CInstrumentEditorN163Wave message handlers

BOOL CInstrumentEditorN163Wave::OnInitDialog()
{
	CInstrumentEditPanel::OnInitDialog();

	// Create wave editor
	CRect rect(SX(20), SY(30), 0, 0);
	m_pWaveEditor = new CWaveEditorN163(10, 8, 32, 16);
	m_pWaveEditor->CreateEx(WS_EX_CLIENTEDGE, NULL, _T(""), WS_CHILD | WS_VISIBLE, rect, this);
	m_pWaveEditor->ShowWindow(SW_SHOW);
	m_pWaveEditor->UpdateWindow();

	CComboBox *pSize = (CComboBox*)GetDlgItem(IDC_WAVE_SIZE);

	CSpinButtonCtrl *pIndexSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_INDEX_SPIN);
	CSpinButtonCtrl *pWavesSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_WAVES_SPIN);

	pIndexSpin->SetRange(0, CInstrumentN163::MAX_WAVE_COUNT - 1);
	pWavesSpin->SetRange(0, CInstrumentN163::MAX_WAVE_COUNT - 1);
	
	CComboBox *pWaveSize = (CComboBox*)GetDlgItem(IDC_WAVE_SIZE);

	for (int i = 0; i < CInstrumentN163::MAX_WAVE_SIZE; i += 4) {
		CString txt;
		txt.Format(_T("%i"), i + 4);
		pWaveSize->AddString(txt);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CInstrumentEditorN163Wave::OnPresetSine()
{
	int size = m_pInstrument->GetWaveSize();

	for (int i = 0; i < size; ++i) {
		float angle = (float(i) * 3.141592f * 2.0f) / float(size) + 0.049087375f;
		int sample = int((sinf(angle) + 1.0f) * 7.5f + 0.5f);
		m_pInstrument->SetSample(m_iWaveIndex, i, sample);
	}

	m_pWaveEditor->WaveChanged();
	theApp.GetSoundGenerator()->WaveChanged();
}

void CInstrumentEditorN163Wave::OnPresetTriangle()
{
	int size = m_pInstrument->GetWaveSize();

	for (int i = 0; i < size; ++i) {
		int sample = ((i < (size / 2) ? i : ((size - 1) - i))) * (16 / (size / 2));
		m_pInstrument->SetSample(m_iWaveIndex, i, sample);
	}

	m_pWaveEditor->WaveChanged();
	theApp.GetSoundGenerator()->WaveChanged();
}

void CInstrumentEditorN163Wave::OnPresetPulse50()
{
	int size = m_pInstrument->GetWaveSize();

	for (int i = 0; i < size; ++i) {
		int sample = (i < (size / 2) ? 0 : 15);
		m_pInstrument->SetSample(m_iWaveIndex, i, sample);
	}

	m_pWaveEditor->WaveChanged();
	theApp.GetSoundGenerator()->WaveChanged();
}

void CInstrumentEditorN163Wave::OnPresetPulse25()
{
	int size = m_pInstrument->GetWaveSize();

	for (int i = 0; i < size; ++i) {
		int sample = (i < (size / 4) ? 0 : 15);
		m_pInstrument->SetSample(m_iWaveIndex, i, sample);
	}

	m_pWaveEditor->WaveChanged();
	theApp.GetSoundGenerator()->WaveChanged();
}

void CInstrumentEditorN163Wave::OnPresetSawtooth()
{
	int size = m_pInstrument->GetWaveSize();

	for (int i = 0; i < size; ++i) {
		int sample = (i * 16) / size;
		m_pInstrument->SetSample(m_iWaveIndex, i, sample);
	}

	m_pWaveEditor->WaveChanged();
	theApp.GetSoundGenerator()->WaveChanged();
}

void CInstrumentEditorN163Wave::OnBnClickedCopy()
{
	CString Str;

	int len = m_pInstrument->GetWaveSize();

	//Str.Format(_T("%i, "), m_pInstrument->GetSamplePos());

	// Assemble a MML string
	for (int i = 0; i < len; ++i)
		Str.AppendFormat(_T("%i "), m_pInstrument->GetSample(m_iWaveIndex,i));

	if (!OpenClipboard())
		return;

	EmptyClipboard();

	int size = Str.GetLength() + 1;
	HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE, size);
	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hMem);  
	strcpy_s(lptstrCopy, size, Str.GetBuffer());
	GlobalUnlock(hMem);
	SetClipboardData(CF_TEXT, hMem);

	CloseClipboard();	
}

void CInstrumentEditorN163Wave::OnBnClickedPaste()
{
	// Copy from clipboard
	if (!OpenClipboard())
		return;

	HANDLE hMem = GetClipboardData(CF_TEXT);
	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hMem);
	ParseString(lptstrCopy);
	GlobalUnlock(hMem);
	CloseClipboard();
}

void CInstrumentEditorN163Wave::ParseString(LPTSTR pString)
{
   qDebug("********************************************** STRING FUNCTIONS!!!!!!");
//	string str(pString);

//	// Convert to register values
//	istringstream values(str);
//	istream_iterator<int> begin(values);
//	istream_iterator<int> end;
//	int i;

//	for (i = 0; (i < CInstrumentN163::MAX_WAVE_SIZE) && (begin != end); ++i) {
//		int value = *begin++;
//		if (value >= 0 && value <= 15)
//			m_pInstrument->SetSample(m_iWaveIndex, i, value);
//	}

//	int size = i & 0xFC;
//	if (size < 4)
//		size = 4;
//	m_pInstrument->SetWaveSize(size);

//	CString SizeStr;
//	SizeStr.Format(_T("%i"), size);
//	((CComboBox*)GetDlgItem(IDC_WAVE_SIZE))->SelectString(0, SizeStr);

//	FillPosBox(size);

//	m_pWaveEditor->SetLength(size);
//	m_pWaveEditor->WaveChanged();
}

LRESULT CInstrumentEditorN163Wave::OnWaveChanged(WPARAM wParam, LPARAM lParam)
{
	CString str;
	int Size = m_pInstrument->GetWaveSize();
	for (int i = 0; i < Size; ++i) {
		str.AppendFormat(_T("%i "), m_pInstrument->GetSample(m_iWaveIndex, i));
	}
	SetDlgItemText(IDC_MML, str);
	return 0;
}

void CInstrumentEditorN163Wave::OnWaveSizeChange()
{
	BOOL trans;
	int size = GetDlgItemInt(IDC_WAVE_SIZE, &trans, FALSE);
	size = size & 0xFC;//(size / 4) * 4;
	
	if (size > CInstrumentN163::MAX_WAVE_SIZE)
		size = CInstrumentN163::MAX_WAVE_SIZE;
	if (size < 4)
		size = 4;

	m_pInstrument->SetWaveSize(size);

	FillPosBox(size);

	m_pWaveEditor->SetLength(size);
	m_pWaveEditor->WaveChanged();
}

void CInstrumentEditorN163Wave::OnWavePosChange()
{
	BOOL trans;
	int pos = GetDlgItemInt(IDC_WAVE_POS, &trans, FALSE);
	
	if (pos > 255)
		pos = 255;
	if (pos < 0)
		pos = 0;

	m_pInstrument->SetWavePos(pos);
}

void CInstrumentEditorN163Wave::OnWavePosSelChange()
{
	CString str;
	CComboBox *pPosBox = (CComboBox*)GetDlgItem(IDC_WAVE_POS);
	pPosBox->GetLBText(pPosBox->GetCurSel(), str);

   int pos = _ttoi(str);

	if (pos > 255)
		pos = 255;
	if (pos < 0)
		pos = 0;

	m_pInstrument->SetWavePos(pos);
}

void CInstrumentEditorN163Wave::FillPosBox(int size)
{
	CComboBox *pPosBox = (CComboBox*)GetDlgItem(IDC_WAVE_POS);
	pPosBox->ResetContent();

	CString str;
	for (int i = 0; i < 128; i += size) {
		str.Format(_T("%i"), i);
		pPosBox->AddString(str);
	}
}
/*
void CInstrumentEditorN163Wave::OnPositionClicked()
{
	if (IsDlgButtonChecked(IDC_POSITION)) {
		GetDlgItem(IDC_WAVE_POS)->EnableWindow(FALSE);
		m_pInstrument->SetAutoWavePos(true);
	}
	else {
		GetDlgItem(IDC_WAVE_POS)->EnableWindow(TRUE);
		m_pInstrument->SetAutoWavePos(false);
	}
}
*/
void CInstrumentEditorN163Wave::OnWavesChange()
{
	CSpinButtonCtrl *pIndexSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_INDEX_SPIN);

	int count = GetDlgItemInt(IDC_WAVES) + 1;
	
	if (m_pInstrument != NULL)
		m_pInstrument->SetWaveCount(count);

	pIndexSpin->SetRange(0, count - 1);
	pIndexSpin->RedrawWindow();

	if (pIndexSpin->GetPos() > (count - 1))
		pIndexSpin->SetPos(count - 1);

	if (m_pWaveEditor != NULL) {
		m_pWaveEditor->SetWave(m_iWaveIndex);
		m_pWaveEditor->WaveChanged();
	}
}

void CInstrumentEditorN163Wave::OnIndexChange()
{
	m_iWaveIndex = GetDlgItemInt(IDC_INDEX);

	if (m_pWaveEditor != NULL) {
		m_pWaveEditor->SetWave(m_iWaveIndex);
		m_pWaveEditor->WaveChanged();
	}
}

void CInstrumentEditorN163Wave::OnKeyReturn()
{
	// Parse MML string
	TCHAR text[256];
	GetDlgItemText(IDC_MML, text, 256);
	ParseString(text);
}
