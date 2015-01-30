//---------------------------------------------------------------------------
// Copyright (C) 2013-2014 Krzysztof Grochocki
//
// This file is part of NotifyMe
//
// NotifyMe is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// NotifyMe is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GNU Radio; see the file COPYING. If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street,
// Boston, MA 02110-1301, USA.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#ifndef SettingsFrmH
#define SettingsFrmH
#define WM_ALPHAWINDOWS (WM_USER + 666)
//---------------------------------------------------------------------------
#include "acPNG.hpp"
#include "acProgressBar.hpp"
#include "sBevel.hpp"
#include "sButton.hpp"
#include "sCheckBox.hpp"
#include "sEdit.hpp"
#include "sLabel.hpp"
#include "sListView.hpp"
#include "sPageControl.hpp"
#include "sPanel.hpp"
#include "sSkinManager.hpp"
#include "sSkinProvider.hpp"
#include "sSpinEdit.hpp"
#include <IdBaseComponent.hpp>
#include <IdThreadComponent.hpp>
#include <System.Actions.hpp>
#include <System.Classes.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class TSettingsForm : public TForm
{
__published:	// IDE-managed Components
	TsSkinManager *sSkinManager;
	TsSkinProvider *sSkinProvider;
	TActionList *ActionList;
	TAction *aExit;
	TAction *aLoadSettings;
	TAction *aSaveSettings;
	TsBevel *Bevel;
	TsButton *SaveButton;
	TsButton *CancelButton;
	TsButton *OKButton;
	TsPageControl *sPageControl;
	TsTabSheet *SettingsTabSheet;
	TsTabSheet *StatsTabSheet;
	TsLabel *NotificationLabel;
	TsCheckBox *OnVersionCheckBox;
	TsCheckBox *OnLastCheckBox;
	TsSpinEdit *CloudTimeOutSpinEdit;
	TsCheckBox *StatsCheckBox;
	TAction *aAllowSave;
	TsListView *sListView;
	TsButton *ReloadButton;
	TsButton *DeleteButton;
	TAction *aGetDataFromXML;
	TsCheckBox *ExInfoCheckBox;
	TAction *aShowExtInfo;
	TPopupMenu *PopupMenu;
	TMenuItem *aDeleteAll;
	TsCheckBox *FastStatsCheckBox;
	TIdThreadComponent *RebuildXMLThreadComponent;
	TsProgressBar *RebuildXMLProgressBar;
	TsLabel *RebuildXMLLabel;
	TImage *ProgressImage;
	TsPanel *ProgressPanel;
	TsCheckBox *FrmSendNotificationCheckBox;
	TsCheckBox *SaveInArchiveCheckBox;
	TAction *aDeleteItem;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall aExitExecute(TObject *Sender);
	void __fastcall aLoadSettingsExecute(TObject *Sender);
	void __fastcall aSaveSettingsExecute(TObject *Sender);
	void __fastcall SaveButtonClick(TObject *Sender);
	void __fastcall OKButtonClick(TObject *Sender);
	void __fastcall aAllowSaveExecute(TObject *Sender);
	void __fastcall aGetDataFromXMLExecute(TObject *Sender);
	void __fastcall sListViewCompare(TObject *Sender, TListItem *Item1, TListItem *Item2, int Data, int &Compare);
	void __fastcall sListViewColumnClick(TObject *Sender, TListColumn *Column);
	void __fastcall StatsTabSheetShow(TObject *Sender);
	void __fastcall aShowExtInfoExecute(TObject *Sender);
	void __fastcall ExInfoCheckBoxClick(TObject *Sender);
	void __fastcall aDeleteAllClick(TObject *Sender);
	void __fastcall RebuildXMLThreadComponentRun(TIdThreadComponent *Sender);
	void __fastcall sSkinManagerSysDlgInit(TacSysDlgData DlgData, bool &AllowSkinning);
	void __fastcall sPageControlPageChanging(TObject *Sender, TsTabSheet *NewPage, bool &AllowChange);
	void __fastcall aDeleteItemExecute(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TSettingsForm(TComponent* Owner);
	bool pFastStats;
	void __fastcall WMTransparency(TMessage &Message);
	BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(WM_ALPHAWINDOWS,TMessage,WMTransparency);
	END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TSettingsForm *SettingsForm;
//---------------------------------------------------------------------------
#endif
