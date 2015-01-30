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
#include <vcl.h>
#include <XMLDoc.hpp>
#include <LangAPI.hpp>
#pragma hdrstop
#include "SettingsFrm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "acPNG"
#pragma link "acProgressBar"
#pragma link "sBevel"
#pragma link "sButton"
#pragma link "sCheckBox"
#pragma link "sEdit"
#pragma link "sLabel"
#pragma link "sListView"
#pragma link "sPageControl"
#pragma link "sPanel"
#pragma link "sSkinManager"
#pragma link "sSkinProvider"
#pragma link "sSpinEdit"
#pragma resource "*.dfm"
TSettingsForm *SettingsForm;
//---------------------------------------------------------------------------
__declspec(dllimport)UnicodeString GetPluginUserDir();
__declspec(dllimport)UnicodeString GetThemeSkinDir();
__declspec(dllimport)UnicodeString GetThemeDir();
__declspec(dllimport)UnicodeString GetDefaultThemeDir();
__declspec(dllimport)bool ChkSkinEnabled();
__declspec(dllimport)bool ChkThemeAnimateWindows();
__declspec(dllimport)bool ChkThemeGlowing();
__declspec(dllimport)int GetHUE();
__declspec(dllimport)int GetSaturation();
__declspec(dllimport)int GetBrightness();
__declspec(dllimport)void LoadSettings();
__declspec(dllimport)void SaveInfoToStatsFileEx(UnicodeString JID, UnicodeString Nick, UnicodeString Type, UnicodeString Target, UnicodeString Time);
//---------------------------------------------------------------------------
__fastcall TSettingsForm::TSettingsForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::WMTransparency(TMessage &Message)
{
  Application->ProcessMessages();
  if(sSkinManager->Active) sSkinProvider->BorderForm->UpdateExBordersPos(true,(int)Message.LParam);
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::FormCreate(TObject *Sender)
{
  //Lokalizowanie formy
  LangForm(this);
  //Wlaczona zaawansowana stylizacja okien
  if(ChkSkinEnabled())
  {
	UnicodeString ThemeSkinDir = GetThemeSkinDir();
	//Plik zaawansowanej stylizacji okien istnieje
	if(FileExists(ThemeSkinDir + "\\\\Skin.asz"))
	{
	  //Dane pliku zaawansowanej stylizacji okien
	  ThemeSkinDir = StringReplace(ThemeSkinDir, "\\\\", "\\", TReplaceFlags() << rfReplaceAll);
	  sSkinManager->SkinDirectory = ThemeSkinDir;
	  sSkinManager->SkinName = "Skin.asz";
	  //Ustawianie animacji AlphaControls
	  if(ChkThemeAnimateWindows()) sSkinManager->AnimEffects->FormShow->Time = 200;
	  else sSkinManager->AnimEffects->FormShow->Time = 0;
	  sSkinManager->Effects->AllowGlowing = ChkThemeGlowing();
	  //Zmiana kolorystyki AlphaControls
	  sSkinManager->HueOffset = GetHUE();
	  sSkinManager->Saturation = GetSaturation();
	  sSkinManager->Brightness = GetBrightness();
	  //Aktywacja skorkowania AlphaControls
	  sSkinManager->Active = true;
	}
	//Brak pliku zaawansowanej stylizacji okien
	else sSkinManager->Active = false;
  }
  //Zaawansowana stylizacja okien wylaczona
  else sSkinManager->Active = false;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::FormShow(TObject *Sender)
{
  //Odczyt ustawien
  aLoadSettings->Execute();
  //Wylaczenie przycisku "Zastosuj"
  SaveButton->Enabled = false;
  //Pokazanie karty statystyk
  if(pFastStats) sPageControl->ActivePage = StatsTabSheet;
  //Pokazanie karty ustawien
  else sPageControl->ActivePage = SettingsTabSheet;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aExitExecute(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aLoadSettingsExecute(TObject *Sender)
{
  TIniFile *Ini = new TIniFile(GetPluginUserDir() + "\\\\NotifyMe\\\\Settings.ini");
  OnVersionCheckBox->Checked = Ini->ReadBool("Settings","OnVersion",true);
  OnLastCheckBox->Checked = Ini->ReadBool("Settings","OnLast",true);
  CloudTimeOutSpinEdit->Value = Ini->ReadInteger("Settings","CloudTimeOut",6);
  FrmSendNotificationCheckBox->Checked = Ini->ReadBool("Settings","FrmSendNotification",true);
  SaveInArchiveCheckBox->Checked = Ini->ReadBool("Settings","SaveInArchive",false);
  StatsCheckBox->Checked = Ini->ReadBool("Settings","Stats",false);
  StatsTabSheet->Enabled = StatsCheckBox->Checked;
  FastStatsCheckBox->Enabled = StatsCheckBox->Checked;
  FastStatsCheckBox->Checked = Ini->ReadBool("Settings","FastStats",false);
  ExInfoCheckBox->Checked = Ini->ReadBool("Settings","ExInfo",false);
  delete Ini;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aSaveSettingsExecute(TObject *Sender)
{
  TIniFile *Ini = new TIniFile(GetPluginUserDir()+"\\\\NotifyMe\\\\Settings.ini");
  Ini->WriteBool("Settings","OnVersion",OnVersionCheckBox->Checked);
  Ini->WriteBool("Settings","OnLast",OnLastCheckBox->Checked);
  Ini->WriteInteger("Settings","CloudTimeOut",CloudTimeOutSpinEdit->Value);
  Ini->WriteBool("Settings","FrmSendNotification",FrmSendNotificationCheckBox->Checked);
  Ini->WriteBool("Settings","SaveInArchive",SaveInArchiveCheckBox->Checked);
  Ini->WriteBool("Settings","Stats",StatsCheckBox->Checked);
  Ini->WriteBool("Settings","FastStats",FastStatsCheckBox->Checked);
  Ini->WriteBool("Settings","ExInfo",ExInfoCheckBox->Checked);
  delete Ini;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::SaveButtonClick(TObject *Sender)
{
  //Zapisanie ustawien
  aSaveSettings->Execute();
  //Odczyt ustawien w rdzeniu
  LoadSettings();
  //Wylaczanie przycisku
  SaveButton->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::OKButtonClick(TObject *Sender)
{
  //Zapisanie ustawien
  aSaveSettings->Execute();
  //Odczyt ustawien w rdzeniu
  LoadSettings();
  //Zamkniecie formy
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aAllowSaveExecute(TObject *Sender)
{
  SaveButton->Enabled = true;
  SaveInArchiveCheckBox->Enabled = FrmSendNotificationCheckBox->Checked;
  StatsTabSheet->Enabled = StatsCheckBox->Checked;
  FastStatsCheckBox->Enabled = StatsCheckBox->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aGetDataFromXMLExecute(TObject *Sender)
{
  sListView->Clear();
  if(FileExists(GetPluginUserDir()+"\\\\NotifyMe\\\\Stats.xml"))
  {
	_di_IXMLDocument XMLDoc = NewXMLDocument();
	XMLDoc->LoadFromFile(GetPluginUserDir()+"\\\\NotifyMe\\\\Stats.xml");
	_di_IXMLNode MainNode = XMLDoc->DocumentElement;
	int NodesCount = MainNode->ChildNodes->GetCount();
	for(int Count=0;Count<NodesCount;Count++)
	{
	  _di_IXMLNode ChildNodes = MainNode->ChildNodes->GetNode(Count);
	  sListView->Items->Add();
	  sListView->Items->Item[sListView->Items->Count-1]->Caption = ChildNodes->Attributes["nick"];
	  sListView->Items->Item[sListView->Items->Count-1]->SubItems->Add(ChildNodes->Attributes["jid"]);
	  if(ChildNodes->Attributes["type"]=="1") sListView->Items->Item[sListView->Items->Count-1]->SubItems->Add(GetLangStr("Version"));
	  else sListView->Items->Item[sListView->Items->Count-1]->SubItems->Add(GetLangStr("Activity"));
	  sListView->Items->Item[sListView->Items->Count-1]->SubItems->Add(ChildNodes->Attributes["time"]);
	  sListView->Items->Item[sListView->Items->Count-1]->SubItems->Add(ChildNodes->Attributes["target"]);
	}
  }
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::DeleteButtonClick(TObject *Sender)
{
  //Zaznaczono na liscie jakis element
  if(sListView->ItemIndex!=-1)
  {
	//Wylaczenie komponentow
	SettingsTabSheet->Enabled = false;
	ExInfoCheckBox->Enabled = false;
	DeleteButton->Enabled = false;
	ReloadButton->Enabled = false;
	//Usuwanie wybranego elementu z listy
	sListView->Items->Item[sListView->ItemIndex]->Delete();
	//Wczytanie grafiki postepu z aktywnej kompozycji
	if(FileExists(GetThemeDir()+"////Graphics////ShortInfoURL.png"))
	 ProgressImage->Picture->LoadFromFile(GetThemeDir()+"////Graphics////ShortInfoURL.png");
	else if(FileExists(GetDefaultThemeDir()+"////Graphics////ShortInfoURL.png"))
	 ProgressImage->Picture->LoadFromFile(GetDefaultThemeDir()+"////Graphics////ShortInfoURL.png");
	//Ustawienie paska postepu
	RebuildXMLProgressBar->Position = 0;
	RebuildXMLProgressBar->Max = sListView->Items->Count;
	if(ExInfoCheckBox->Checked)	ProgressPanel->Margins->Top = 52;
	else ProgressPanel->Margins->Top = 12;
	//Ukrycie listy
	sListView->Visible = false;
	//Wlaczenie watku przebudowy pliku XML
	RebuildXMLThreadComponent->Start();
  }
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::sListViewColumnClick(TObject *Sender, TListColumn *Column)
{
  static bool SortParam = false;
  ((TsListView *)Sender)->Tag = Column->Index;
  ((TsListView *)Sender)->CustomSort(NULL, (long)SortParam);
  SortParam = !SortParam;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::sListViewCompare(TObject *Sender, TListItem *Item1, TListItem *Item2, int Data, int &Compare)
{
  if(((TsListView *)Sender)->Tag==0)
  {
	Compare = CompareText(Item1->Caption, Item2->Caption);
  }
  else
  {
	int Tag = ((TsListView *)Sender)->Tag - 1;
	Compare = CompareText(Item1->SubItems->Strings[Tag], Item2->SubItems->Strings[Tag]);
  }
  if(Data==1) Compare = -Compare;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::StatsTabSheetShow(TObject *Sender)
{
  //Pobranie danych z pliku XML
  aGetDataFromXML->Execute();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aShowExtInfoExecute(TObject *Sender)
{
  //Szczegolowe informacje
  if(ExInfoCheckBox->Checked)
  {
	//Margines panelu postepu
	ProgressPanel->Margins->Top = 52;
	//Szerokosc formy
	Width = 512;
	//Wysokosc formy
	Height = 412;
	//Szerokosc kolumn
	sListView->Column[1]->Width = 90;
	sListView->Column[4]->Width = 90;
  }
  //Brak szczegolowych informacji
  else
  {
	//Margines panelu postepu
	ProgressPanel->Margins->Top = 12;
	//Szerokosc formy
	Width = 332;
	//Wysokosc formy
	Height = 327;
	//Szerokosc kolumn
	sListView->Column[1]->Width = 0;
	sListView->Column[4]->Width = 0;
  }
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::ExInfoCheckBoxClick(TObject *Sender)
{
  //Pokazywanie dodatkowych danych
  aShowExtInfo->Execute();
  //Zezwolenie na zapisanie ustawien
  aAllowSave->Execute();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aDeleteAllClick(TObject *Sender)
{
  if(FileExists(GetPluginUserDir()+"\\\\NotifyMe\\\\Stats.xml"))
  {
	DeleteFile(GetPluginUserDir()+"\\\\NotifyMe\\\\Stats.xml");
	sListView->Clear();
  }
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::RebuildXMLThreadComponentRun(TIdThreadComponent *Sender)
{
  //Usuwanie pliku XML
  if(FileExists(GetPluginUserDir()+"\\\\NotifyMe\\\\Stats.xml"))
   DeleteFile(GetPluginUserDir()+"\\\\NotifyMe\\\\Stats.xml");
  //Zapisywanie ponownie wszystkich danych do pliku XML
  for(int Count=0;Count<sListView->Items->Count;Count++)
  {
	//Pobieranie danych z listy i parsowanie
	UnicodeString Nick = sListView->Items->Item[Count]->Caption;
	UnicodeString JID = sListView->Items->Item[Count]->SubItems->Strings[0];
	UnicodeString Type = sListView->Items->Item[Count]->SubItems->Strings[1];
	if(Type==GetLangStr("Version")) Type = "1";
	else Type = "2";
	UnicodeString Time = sListView->Items->Item[Count]->SubItems->Strings[2];
	UnicodeString Target = sListView->Items->Item[Count]->SubItems->Strings[3];
	//Zapisywanie danych
	SaveInfoToStatsFileEx(JID,Nick,Type,Target,Time);
	//Dodanie pozycji do paska postepu
	RebuildXMLProgressBar->Position++;
  }
  //Pokazanie listy
  sListView->Visible = true;
  //Wlaczenie komponentow
  SettingsTabSheet->Enabled = true;
  ExInfoCheckBox->Enabled = true;
  DeleteButton->Enabled = true;
  ReloadButton->Enabled = true;
  //Zatrzymanie watku
  RebuildXMLThreadComponent->Terminate();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::sSkinManagerSysDlgInit(TacSysDlgData DlgData, bool &AllowSkinning)
{
  AllowSkinning = false;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::sPageControlPageChanging(TObject *Sender, TsTabSheet *NewPage,
          bool &AllowChange)
{
  if(NewPage->Name=="SettingsTabSheet")
  {
	//Szerokosc formy
	Width = 332;
	//Wysokosc formy
	Height = 327;
  }
  else aShowExtInfo->Execute();
}
//---------------------------------------------------------------------------

