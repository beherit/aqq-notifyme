//---------------------------------------------------------------------------
// Copyright (C) 2013-2016 Krzysztof Grochocki
//
// This file is part of NotifyMe
//
// NotifyMe is free software: you can redistribute it and/or modify
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
// along with GNU Radio. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------

#include <vcl.h>
#include <windows.h>
#include "SettingsFrm.h"
#include <inifiles.hpp>
#include <XMLDoc.hpp>
#include <IdHashMessageDigest.hpp>
#include <PluginAPI.h>
#include <LangAPI.hpp>
#pragma hdrstop

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------

//Uchwyt-do-formy-ustawien---------------------------------------------------
TSettingsForm *hSettingsForm;
//Struktury-glowne-----------------------------------------------------------
TPluginLink PluginLink;
TPluginInfo PluginInfo;
//Uchwyt-do-okna-timera------------------------------------------------------
HWND hTimerFrm;
//Lista-JID-oraz-ID-timerow--------------------------------------------------
TMemIniFile* AntySpamList = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
//Lista-JID-wraz-z-nickami---------------------------------------------------
TMemIniFile* ContactsNickList = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
//ID-wywolania-enumeracji-listy-kontaktow------------------------------------
int ReplyListID = 0;
//Sciezka-do-pliku-ze-statystykami-------------------------------------------
UnicodeString StatsFilePath;
//IKONY-W-INTERFEJSIE--------------------------------------------------------
int FASTACCESS;
//SETTINGS-------------------------------------------------------------------
bool OnVersionChk;
bool OnLastChk;
int CloudTimeOut;
bool FrmSendNotificationChk;
bool SaveInArchiveChk;
bool StatsChk;
bool FastStatsChk;
//FORWARD-AQQ-HOOKS----------------------------------------------------------
INT_PTR __stdcall OnColorChange(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnContactsUpdate(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnLangCodeChanged(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnListReady(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnReplyList(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnThemeChanged(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnXMLIDDebug(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall ServiceNotifyMeFastStatsItem(WPARAM wParam, LPARAM lParam);
//FORWARD-TIMER--------------------------------------------------------------
LRESULT CALLBACK TimerFrmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//FORWARD-OTHER-FUNCTION-----------------------------------------------------
void RebuildNotifyMeFastStats();
//---------------------------------------------------------------------------

//Pobieranie sciezki katalogu prywatnego wtyczek
UnicodeString GetPluginUserDir()
{
	return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETPLUGINUSERDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll);
}
//---------------------------------------------------------------------------

//Pobieranie sciezki kompozycji
UnicodeString GetThemeDir()
{
	return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETTHEMEDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll);
}
//---------------------------------------------------------------------------
UnicodeString GetDefaultThemeDir()
{
	return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETAPPPATH,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll) + "\\\\System\\\\Shared\\\\Themes\\\\Standard";
}
//---------------------------------------------------------------------------

//Pobieranie sciezki do skorki kompozycji
UnicodeString GetThemeSkinDir()
{
	return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETTHEMEDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll) + "\\\\Skin";
}
//---------------------------------------------------------------------------

//Sprawdzanie czy wlaczona jest zaawansowana stylizacja okien
bool ChkSkinEnabled()
{
	TStrings* IniList = new TStringList();
	IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
	TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
	Settings->SetStrings(IniList);
	delete IniList;
	UnicodeString SkinsEnabled = Settings->ReadString("Settings","UseSkin","1");
	delete Settings;
	return StrToBool(SkinsEnabled);
}
//---------------------------------------------------------------------------

//Sprawdzanie ustawien animacji AlphaControls
bool ChkThemeAnimateWindows()
{
	TStrings* IniList = new TStringList();
	IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
	TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
	Settings->SetStrings(IniList);
	delete IniList;
	UnicodeString AnimateWindowsEnabled = Settings->ReadString("Theme","ThemeAnimateWindows","1");
	delete Settings;
	return StrToBool(AnimateWindowsEnabled);
}
//---------------------------------------------------------------------------
bool ChkThemeGlowing()
{
	TStrings* IniList = new TStringList();
	IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
	TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
	Settings->SetStrings(IniList);
	delete IniList;
	UnicodeString GlowingEnabled = Settings->ReadString("Theme","ThemeGlowing","1");
	delete Settings;
	return StrToBool(GlowingEnabled);
}
//---------------------------------------------------------------------------

//Pobieranie ustawien koloru AlphaControls
int GetHUE()
{
	return (int)PluginLink.CallService(AQQ_SYSTEM_COLORGETHUE,0,0);
}
//---------------------------------------------------------------------------
int GetSaturation()
{
	return (int)PluginLink.CallService(AQQ_SYSTEM_COLORGETSATURATION,0,0);
}
//---------------------------------------------------------------------------
int GetBrightness()
{
	return (int)PluginLink.CallService(AQQ_SYSTEM_COLORGETBRIGHTNESS,0,0);
}
//---------------------------------------------------------------------------

//Pobieranie pseudonimu kontaktu podajac jego JID
UnicodeString GetContactNick(UnicodeString JID)
{
	//Odczyt pseudonimu z pliku INI
	UnicodeString Nick = ContactsNickList->ReadString("Nick",JID,"");
	//Pseudonim nie zostal pobrany
	if(Nick.IsEmpty())
	{
		//Usuwanie indeksu konta z JID
		if(JID.Pos(":")) JID.Delete(JID.Pos(":"),JID.Length());
		return JID;
	}
	return Nick;
}
//---------------------------------------------------------------------------

//Pobieranie nazwy konta przez podanie jego indeksu
UnicodeString GetAccountJID(int UserIdx)
{
	TPluginStateChange PluginStateChange;
	PluginLink.CallService(AQQ_FUNCTION_GETNETWORKSTATE,(WPARAM)&PluginStateChange,UserIdx);
	return (wchar_t*)PluginStateChange.JID;
}
//---------------------------------------------------------------------------
UnicodeString GetAccountJIDW(int UserIdx)
{
	TPluginStateChange PluginStateChange;
	PluginLink.CallService(AQQ_FUNCTION_GETNETWORKSTATE,(WPARAM)&PluginStateChange,UserIdx);
	return (wchar_t*)PluginStateChange.Server;
}
//---------------------------------------------------------------------------

//Pokazanie chmurki informacyjnej
void ShowNotification(UnicodeString Text)
{
	TPluginShowInfo PluginShowInfo;
	ZeroMemory(&PluginShowInfo, sizeof(TPluginShowInfo));
	PluginShowInfo.cbSize = sizeof(TPluginShowInfo);
	PluginShowInfo.Event = tmeInfo;
	PluginShowInfo.Text = Text.w_str();
	PluginShowInfo.ImagePath = (wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETPNG_FILEPATH,21,0);
	PluginShowInfo.TimeOut = 1000 * CloudTimeOut;
	PluginLink.CallService(AQQ_FUNCTION_SHOWINFO,0,(LPARAM)(&PluginShowInfo));
}
//---------------------------------------------------------------------------

//Pokazanie informacji w oknie rozmowy
void ShowFrmSendNotification(UnicodeString JID, UnicodeString Res, int UserIdx, UnicodeString Text)
{
	//Struktura kontatku
	TPluginMicroMsg PluginMicroMsg;
	ZeroMemory(&PluginMicroMsg, sizeof(TPluginMicroMsg));
	PluginMicroMsg.cbSize = sizeof(TPluginMicroMsg);
	PluginMicroMsg.Msg = Text.w_str();
	PluginMicroMsg.SaveToArchive = SaveInArchiveChk;
	//Struktura wiadomosci
	TPluginContact PluginContact;
	ZeroMemory(&PluginContact, sizeof(TPluginContact));
	PluginContact.cbSize = sizeof(TPluginContact);
	PluginContact.JID = JID.w_str();
	PluginContact.Resource = Res.w_str();
	PluginContact.UserIdx = UserIdx;
	//Pokazanie informacji
	PluginLink.CallService(AQQ_CONTACTS_ADDLINEINFO,(WPARAM)(&PluginContact),(LPARAM)(&PluginMicroMsg));
}
//---------------------------------------------------------------------------

//Zapisywanie informacji do pliku ze statystykami
void SaveInfoToStatsFile(UnicodeString JID, UnicodeString Nick, int Type, UnicodeString Target)
{
	//Pobieranie aktualnej daty oraz czasu
	TDateTime CurrTime = TDateTime::CurrentDateTime();
	UnicodeString CurrTimeStr = CurrTime.FormatString("yyyy-mm-dd hh:nn:ss");
	//Tworzenie nowego dokumentu XML
	_di_IXMLDocument XMLDoc = NewXMLDocument();
	_di_IXMLNode MainNode;
	//Wczytanie utworzonego juz pliku XML
	if(FileExists(StatsFilePath))
	{
		XMLDoc->LoadFromFile(StatsFilePath);
		MainNode = XMLDoc->DocumentElement;
	}
	//Tworzenie pliku XML
	else MainNode = XMLDoc->AddChild("items");
	//Dodanie nowego elementu
	_di_IXMLNode ChildNode = MainNode->AddChild("item");
	ChildNode->Attributes["jid"] = JID;
	ChildNode->Attributes["nick"] = Nick;
	ChildNode->Attributes["type"] = Type;
	ChildNode->Attributes["target"] = Target;
	ChildNode->Attributes["time"] = CurrTimeStr;
	//Zapisywanie zmian w pliku
	XMLDoc->SaveToFile(StatsFilePath);
}
//---------------------------------------------------------------------------
void SaveInfoToStatsFileEx(UnicodeString JID, UnicodeString Nick, UnicodeString Type, UnicodeString Target, UnicodeString Time)
{
	//Tworzenie nowego dokumentu XML
	_di_IXMLDocument XMLDoc = NewXMLDocument();
	_di_IXMLNode MainNode;
	//Wczytanie utworzonego juz pliku XML
	if(FileExists(StatsFilePath))
	{
		XMLDoc->LoadFromFile(StatsFilePath);
		MainNode = XMLDoc->DocumentElement;
	}
	//Tworzenie pliku XML
	else MainNode = XMLDoc->AddChild("items");
	//Dodanie nowego elementu
	_di_IXMLNode ChildNode = MainNode->AddChild("item");
	ChildNode->Attributes["jid"] = JID;
	ChildNode->Attributes["nick"] = Nick;
	ChildNode->Attributes["type"] = Type;
	ChildNode->Attributes["target"] = Target;
	ChildNode->Attributes["time"] = Time;
	//Zapisywanie zmian w pliku
	XMLDoc->SaveToFile(StatsFilePath);
}
//---------------------------------------------------------------------------

//Otwieranie okna ustawien wtyczki
void OpenPluginSettings(bool FastStats)
{
	//Przypisanie uchwytu do formy ustawien
	if(!hSettingsForm)
	{
		Application->Handle = (HWND)SettingsForm;
		hSettingsForm = new TSettingsForm(Application);
	}
	//Przekazanie zmiennej typu otwarcia okna
	hSettingsForm->pFastStats = FastStats;
	//Pokaznie okna ustawien
	hSettingsForm->Show();
}
//---------------------------------------------------------------------------

//Procka okna timera
LRESULT CALLBACK TimerFrmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//Notfikacja timera
	if(uMsg==WM_TIMER)
	{
		//Zatrzymanie timera
		 KillTimer(hTimerFrm,wParam);
		//Pobranie ID timera
		int TimerID = wParam;
		//Timer sprawdzania wersji oprogramowania
		if(AntySpamList->ValueExists("Version",TimerID))
		{
			//Pobieranie identyfikatora kontaktu
			UnicodeString From = AntySpamList->ReadString("Version",TimerID,"");
			//Wylaczanie zabezpieczenia
			AntySpamList->DeleteKey("Version",TimerID);
			AntySpamList->DeleteKey("Version",From);
		}
		//Timer sprawdzania ostatniej aktywnosci
		else if(AntySpamList->ValueExists("Last",TimerID))
		{
			//Pobieranie identyfikatora kontaktu
			UnicodeString From = AntySpamList->ReadString("Last",TimerID,"");
			//Wylaczanie zabezpieczenia
			AntySpamList->DeleteKey("Last",TimerID);
			AntySpamList->DeleteKey("Last",From);
		}

		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
//---------------------------------------------------------------------------

//Hook na zmiane kolorystyki AlphaControls
INT_PTR __stdcall OnColorChange(WPARAM wParam, LPARAM lParam)
{
	//Okno ustawien zostalo juz stworzone
	if(hSettingsForm)
	{
		//Wlaczona zaawansowana stylizacja okien
		if(ChkSkinEnabled())
		{
			TPluginColorChange ColorChange = *(PPluginColorChange)wParam;
			hSettingsForm->sSkinManager->HueOffset = ColorChange.Hue;
			hSettingsForm->sSkinManager->Saturation = ColorChange.Saturation;
			hSettingsForm->sSkinManager->Brightness = ColorChange.Brightness;
		}
	}

	return 0;
}
//---------------------------------------------------------------------------

//Hook na zmianê stanu kontaktu
INT_PTR __stdcall OnContactsUpdate(WPARAM wParam, LPARAM lParam)
{
	//Pobieranie danych nt. kontaktu
	TPluginContact ContactsUpdateContact = *(PPluginContact)wParam;
	//Kontakt nie pochodzi z wtyczki
	if(!ContactsUpdateContact.FromPlugin)
	{
		//Pobieranie identyfikatora kontatku
		UnicodeString JID = (wchar_t*)ContactsUpdateContact.JID;
		//Pobieranie indeksu konta
		int UserIdx = ContactsUpdateContact.UserIdx;
		//Pobranie pseudonimu kontatku
		UnicodeString Nick = (wchar_t*)ContactsUpdateContact.Nick;
		//Zapisywanie pseudonimu kontatku do pamieci
		ContactsNickList->WriteString("Nick",JID+":"+IntToStr(UserIdx),Nick);
	}

	return 0;
}
//---------------------------------------------------------------------------

//Hook na zmiane lokalizacji
INT_PTR __stdcall OnLangCodeChanged(WPARAM wParam, LPARAM lParam)
{
	//Czyszczenie cache lokalizacji
	ClearLngCache();
	//Pobranie sciezki do katalogu prywatnego uzytkownika
	UnicodeString PluginUserDir = GetPluginUserDir();
	//Ustawienie sciezki lokalizacji wtyczki
	UnicodeString LangCode = (wchar_t*)lParam;
	LangPath = PluginUserDir + "\\\\Languages\\\\NotifyMe\\\\" + LangCode + "\\\\";
	if(!DirectoryExists(LangPath))
	{
		LangCode = (wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETDEFLANGCODE,0,0);
		LangPath = PluginUserDir + "\\\\Languages\\\\NotifyMe\\\\" + LangCode + "\\\\";
	}
	//Aktualizacja lokalizacji form wtyczki
	for(int i=0;i<Screen->FormCount;i++)
		LangForm(Screen->Forms[i]);
	//Aktualizacja lokalizacji elementu szybkiego dostepu do ustawien wtyczki (statystyki)
	RebuildNotifyMeFastStats();
	//Aktualizacja stastyk w oknie ustawien
	if((hSettingsForm)&&(StatsChk))
		hSettingsForm->aGetDataFromXML->Execute();

	return 0;
}
//---------------------------------------------------------------------------

//Hook na zakonczenie ladowania listy kontaktow przy starcie AQQ
INT_PTR __stdcall OnListReady(WPARAM wParam, LPARAM lParam)
{
	//Pobranie ID dla enumeracji kontaktow
	ReplyListID = GetTickCount();
	//Wywolanie enumeracji kontaktow
	PluginLink.CallService(AQQ_CONTACTS_REQUESTLIST,(WPARAM)ReplyListID,0);

	return 0;
}
//---------------------------------------------------------------------------

//Hook na enumeracje listy kontatkow
INT_PTR __stdcall OnReplyList(WPARAM wParam, LPARAM lParam)
{
	//Sprawdzanie ID wywolania enumeracji
	if((int)wParam==ReplyListID)
	{
		//Pobieranie danych nt. kontaktu
		TPluginContact ReplyListContact = *(PPluginContact)lParam;
		//Kontakt nie pochodzi z wtyczki
		if(!ReplyListContact.FromPlugin)
		{
			//Pobieranie identyfikatora kontatku
			UnicodeString JID = (wchar_t*)ReplyListContact.JID;
			//Pobieranie indeksu konta
			int UserIdx = ReplyListContact.UserIdx;
			//Pobranie pseudonimu kontatku
			UnicodeString Nick = (wchar_t*)ReplyListContact.Nick;
			//Zapisywanie pseudonimu kontatku do pamieci
			ContactsNickList->WriteString("Nick",JID+":"+IntToStr(UserIdx),Nick);
		}
	}

	return 0;
}
//---------------------------------------------------------------------------

//Hook na zmiane kompozycji
INT_PTR __stdcall OnThemeChanged(WPARAM wParam, LPARAM lParam)
{
	//Okno ustawien zostalo juz stworzone
	if(hSettingsForm)
	{
		//Wlaczona zaawansowana stylizacja okien
		if(ChkSkinEnabled())
		{
			//Pobieranie sciezki nowej aktywnej kompozycji
			UnicodeString ThemeSkinDir = StringReplace((wchar_t*)lParam, "\\", "\\\\", TReplaceFlags() << rfReplaceAll) + "\\\\Skin";
			//Plik zaawansowanej stylizacji okien istnieje
			if(FileExists(ThemeSkinDir + "\\\\Skin.asz"))
			{
				//Dane pliku zaawansowanej stylizacji okien
				ThemeSkinDir = StringReplace(ThemeSkinDir, "\\\\", "\\", TReplaceFlags() << rfReplaceAll);
				hSettingsForm->sSkinManager->SkinDirectory = ThemeSkinDir;
				hSettingsForm->sSkinManager->SkinName = "Skin.asz";
				//Ustawianie animacji AlphaControls
				if(ChkThemeAnimateWindows()) hSettingsForm->sSkinManager->AnimEffects->FormShow->Time = 200;
				else hSettingsForm->sSkinManager->AnimEffects->FormShow->Time = 0;
				hSettingsForm->sSkinManager->Effects->AllowGlowing = ChkThemeGlowing();
				//Zmiana kolorystyki AlphaControls
				hSettingsForm->sSkinManager->HueOffset = GetHUE();
				hSettingsForm->sSkinManager->Saturation = GetSaturation();
				hSettingsForm->sSkinManager->Brightness = GetBrightness();
				//Aktywacja skorkowania AlphaControls
				hSettingsForm->sSkinManager->Active = true;
			}
			//Brak pliku zaawansowanej stylizacji okien
			else hSettingsForm->sSkinManager->Active = false;
		}
		//Zaawansowana stylizacja okien wylaczona
		else hSettingsForm->sSkinManager->Active = false;
	}

	return 0;
}
//---------------------------------------------------------------------------

//Hook na odbieranie pakietow XML zawierajace ID
INT_PTR __stdcall OnXMLIDDebug(WPARAM wParam, LPARAM lParam)
{
	//Pobranie pakietu XML
	UnicodeString XML = (wchar_t*)wParam;
	//Wczytanie pakietu XML do dokumentu XML
	_di_IXMLDocument XMLDoc = LoadXMLData(XML);
	_di_IXMLNode MainNode = XMLDoc->DocumentElement;
	//Sprawdzanie nazwy glownej galezi
	if(MainNode->NodeName=="iq")
	{
		//Sprawdzanie typu pakietu
		UnicodeString type = MainNode->Attributes["type"];
		if(type=="get")
		{
			//Enumeracja podgalezi
			int NodesCount = MainNode->ChildNodes->GetCount();
			for(int Count=0;Count<NodesCount;Count++)
			{
				//Pobieranie podgalezi
				_di_IXMLNode ChildNodes = MainNode->ChildNodes->GetNode(Count);
				//Sprawdzanie nazwy podgalezi
				if(ChildNodes->NodeName=="query")
				{
					//Pobieranie wartosci xmlns
					UnicodeString xmlns = ChildNodes->Attributes["xmlns"];
					//Sprawdzanie wersji oprogramowania
					if((xmlns=="jabber:iq:version")&&(OnVersionChk))
					{
						//Pobieranie nadawcy pakietu XML
						TPluginXMLChunk XMLChunk = *(PPluginXMLChunk)lParam;
						UnicodeString From = (wchar_t*)XMLChunk.From;
						UnicodeString Res = "";
						if(From.Pos("/"))
						{
							Res = From;
							Res = Res.Delete(1,Res.Pos("/"));
							From = From.Delete(From.Pos("/"),From.Length());
						}
						int UserIdx = XMLChunk.UserIdx;
						//Anty self-check & server-check
						if((From!=GetAccountJID(XMLChunk.UserIdx))&&(From!=GetAccountJIDW(XMLChunk.UserIdx)))
						{
							//Zabezpieczenie przed spamem
							if(!AntySpamList->ValueExists("Version",From+"/"+Res+":"+UserIdx))
							{
								//Wygenerowanie losowego ID timera
								int TimerID = PluginLink.CallService(AQQ_FUNCTION_GETNUMID,0,0);
								//Zapisanie informacji o wywolaniu
								AntySpamList->WriteInteger("Version",From+"/"+Res+":"+UserIdx,TimerID);
								AntySpamList->WriteString("Version",TimerID,From+"/"+Res+":"+UserIdx);
								//Wlaczenie timera
								SetTimer(hTimerFrm,TimerID,5000,(TIMERPROC)TimerFrmProc);
								//Ustawianie tekstu notyfikacji
								UnicodeString Text = GetContactNick(From+":"+IntToStr(UserIdx)) + " " + GetLangStr("ChecksVersion");
								//Pokazanie chmurki informacyjnej
								ShowNotification(Text);
								//Pokazanie informacji w oknie rozmowy
								if(FrmSendNotificationChk) ShowFrmSendNotification(From,Res,UserIdx,Text);
							}
							//Zapisywanie informacji do pliku ze statystykami
							if(StatsChk) SaveInfoToStatsFile(From,GetContactNick(From+":"+IntToStr(UserIdx)),1,GetAccountJID(XMLChunk.UserIdx));
						}
					}
					//Sprawdzanie ostatniej aktywnosci
					else if((xmlns=="jabber:iq:last")&&(OnLastChk))
					{
						//Pobieranie nadawcy pakietu XML
						TPluginXMLChunk XMLChunk = *(PPluginXMLChunk)lParam;
						UnicodeString From = (wchar_t*)XMLChunk.From;
						UnicodeString Res = "";
						if(From.Pos("/"))
						{
							Res = From;
							Res = Res.Delete(1,Res.Pos("/"));
							From = From.Delete(From.Pos("/"),From.Length());
						}
						int UserIdx = XMLChunk.UserIdx;
						//Anty self-check & server-check
						if((From!=GetAccountJID(XMLChunk.UserIdx))&&(From!=GetAccountJIDW(XMLChunk.UserIdx)))
						{
							//Zabezpieczenie przed spamem
							if(!AntySpamList->ValueExists("Last",From+"/"+Res+":"+UserIdx))
							{
								//Wygenerowanie losowego ID timera
								int TimerID = PluginLink.CallService(AQQ_FUNCTION_GETNUMID,0,0);
								//Zapisanie informacji o wywolaniu
								AntySpamList->WriteInteger("Last",From+"/"+Res+":"+UserIdx,TimerID);
								AntySpamList->WriteString("Last",TimerID,From+"/"+Res+":"+UserIdx);
								//Wlaczenie timera
								SetTimer(hTimerFrm,TimerID,5000,(TIMERPROC)TimerFrmProc);
								//Ustawianie tekstu notyfikacji
								UnicodeString Text = GetContactNick(From+":"+IntToStr(UserIdx)) + " " + GetLangStr("ChecksActivity");
								//Pokazanie chmurki informacyjnej
								ShowNotification(Text);
								//Pokazanie informacji w oknie rozmowy
								if(FrmSendNotificationChk) ShowFrmSendNotification(From,Res,UserIdx,Text);
							}
							//Zapisywanie informacji do pliku ze statystykami
							if(StatsChk) SaveInfoToStatsFile(From,GetContactNick(From+":"+IntToStr(UserIdx)),2,GetAccountJID(XMLChunk.UserIdx));
						}
					}
				}
			}
		}
	}

	return 0;
}
//---------------------------------------------------------------------------

//Serwis szybkiego dostepu do ustawien wtyczki (statystyki)
INT_PTR __stdcall ServiceNotifyMeFastStatsItem(WPARAM wParam, LPARAM lParam)
{
	//Otwieranie okna ustawien wtyczki (statystyki)
	OpenPluginSettings(true);

	return 0;
}
//---------------------------------------------------------------------------

//Usuwanie elementu szybkiego dostepu do ustawien wtyczki (statystyki)
void DestroyNotifyMeFastStats()
{
	TPluginAction DestroyNotifyMeFastStatsItem;
	ZeroMemory(&DestroyNotifyMeFastStatsItem,sizeof(TPluginAction));
	DestroyNotifyMeFastStatsItem.cbSize = sizeof(TPluginAction);
	DestroyNotifyMeFastStatsItem.pszName = L"NotifyMeFastStatsItemButton";
	PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM,0,(LPARAM)(&DestroyNotifyMeFastStatsItem));
}
//---------------------------------------------------------------------------

//Tworzenie elementu szybkiego dostepu do ustawien wtyczki (statystyki)
void BuildNotifyMeFastStats()
{
	TPluginAction BuildNotifyMeFastStatsItem;
	ZeroMemory(&BuildNotifyMeFastStatsItem,sizeof(TPluginAction));
	BuildNotifyMeFastStatsItem.cbSize = sizeof(TPluginAction);
	BuildNotifyMeFastStatsItem.pszName = L"NotifyMeFastStatsItemButton";
	BuildNotifyMeFastStatsItem.pszCaption = ("NotifyMe ("+GetLangStr("Statistics")+")").w_str();
	BuildNotifyMeFastStatsItem.IconIndex = FASTACCESS;
	BuildNotifyMeFastStatsItem.pszService = L"sNotifyMeFastStatsItem";
	BuildNotifyMeFastStatsItem.pszPopupName = L"popPlugins";
	PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&BuildNotifyMeFastStatsItem));
}
//---------------------------------------------------------------------------

//Aktualizacja elementu szybkiego dostepu do ustawien wtyczki (statystyki)
void RebuildNotifyMeFastStats()
{
	TPluginActionEdit RebuildNotifyMeFastStatsItem;
	ZeroMemory(&RebuildNotifyMeFastStatsItem,sizeof(TPluginActionEdit));
	RebuildNotifyMeFastStatsItem.cbSize = sizeof(TPluginActionEdit);
	RebuildNotifyMeFastStatsItem.pszName = L"NotifyMeFastStatsItemButton";
	RebuildNotifyMeFastStatsItem.Caption = ("NotifyMe ("+GetLangStr("Statistics")+")").w_str();
	RebuildNotifyMeFastStatsItem.IconIndex = FASTACCESS;
	RebuildNotifyMeFastStatsItem.Enabled = true;
	RebuildNotifyMeFastStatsItem.Visible = true;
	RebuildNotifyMeFastStatsItem.Checked = false;
	PluginLink.CallService(AQQ_CONTROLS_EDITPOPUPMENUITEM,0,(LPARAM)(&RebuildNotifyMeFastStatsItem));
}
//---------------------------------------------------------------------------

//Odczyt ustawien
void LoadSettings()
{
	TIniFile *Ini = new TIniFile(GetPluginUserDir()+"\\\\NotifyMe\\\\Settings.ini");
	OnVersionChk = Ini->ReadBool("Settings","OnVersion",true);
	OnLastChk = Ini->ReadBool("Settings","OnLast",true);
	CloudTimeOut = Ini->ReadInteger("Settings","CloudTimeOut",6);
	FrmSendNotificationChk = Ini->ReadBool("Settings","FrmSendNotification",true);
	SaveInArchiveChk = Ini->ReadBool("Settings","SaveInArchive",false);
	StatsChk = Ini->ReadBool("Settings","Stats",false);
	FastStatsChk = Ini->ReadBool("Settings","FastStats",false);
	DestroyNotifyMeFastStats();
	if((StatsChk)&&(FastStatsChk)) BuildNotifyMeFastStats();
	delete Ini;
}
//---------------------------------------------------------------------------

//Zapisywanie zasobów
void ExtractRes(wchar_t* FileName, wchar_t* ResName, wchar_t* ResType)
{
	TPluginTwoFlagParams PluginTwoFlagParams;
	PluginTwoFlagParams.cbSize = sizeof(TPluginTwoFlagParams);
	PluginTwoFlagParams.Param1 = ResName;
	PluginTwoFlagParams.Param2 = ResType;
	PluginTwoFlagParams.Flag1 = (int)HInstance;
	PluginLink.CallService(AQQ_FUNCTION_SAVERESOURCE,(WPARAM)&PluginTwoFlagParams,(LPARAM)FileName);
}
//---------------------------------------------------------------------------

//Obliczanie sumy kontrolnej pliku
UnicodeString MD5File(UnicodeString FileName)
{
	if(FileExists(FileName))
	{
		UnicodeString Result;
		TFileStream *fs;
		fs = new TFileStream(FileName, fmOpenRead | fmShareDenyWrite);
		try
		{
			TIdHashMessageDigest5 *idmd5= new TIdHashMessageDigest5();
			try
			{
				Result = idmd5->HashStreamAsHex(fs);
			}
			__finally
			{
				delete idmd5;
			}
		}
		__finally
		{
			delete fs;
		}
		return Result;
	}
	else return 0;
}
//---------------------------------------------------------------------------

extern "C" INT_PTR __declspec(dllexport) __stdcall Load(PPluginLink Link)
{
	//Linkowanie wtyczki z komunikatorem
	PluginLink = *Link;
	//Sciezka folderu prywatnego wtyczek
	UnicodeString PluginUserDir = GetPluginUserDir();
	//Tworzenie katalogow lokalizacji
	if(!DirectoryExists(PluginUserDir+"\\\\Languages"))
		CreateDir(PluginUserDir+"\\\\Languages");
	if(!DirectoryExists(PluginUserDir+"\\\\Languages\\\\NotifyMe"))
		CreateDir(PluginUserDir+"\\\\Languages\\\\NotifyMe");
	if(!DirectoryExists(PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\EN"))
		CreateDir(PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\EN");
	if(!DirectoryExists(PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\PL"))
		CreateDir(PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\PL");
	//Wypakowanie plikow lokalizacji
	//FAB7435FEF90C1CC68D7F2AC92815F36
	if(!FileExists(PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\EN\\\\Const.lng"))
		ExtractRes((PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\EN\\\\Const.lng").w_str(),L"EN_CONST",L"DATA");
	else if(MD5File(PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\EN\\\\Const.lng")!="FAB7435FEF90C1CC68D7F2AC92815F36")
		ExtractRes((PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\EN\\\\Const.lng").w_str(),L"EN_CONST",L"DATA");
	//148EC06F41518E3A8ABA2FFB184CE7A4
	if(!FileExists(PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\EN\\\\TSettingsForm.lng"))
		ExtractRes((PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\EN\\\\TSettingsForm.lng").w_str(),L"EN_SETTINGSFRM",L"DATA");
	else if(MD5File(PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\EN\\\\TSettingsForm.lng")!="148EC06F41518E3A8ABA2FFB184CE7A4")
		ExtractRes((PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\EN\\\\TSettingsForm.lng").w_str(),L"EN_SETTINGSFRM",L"DATA");
	//399F5F05929FA99B6A2F6C624D0FCECD
	if(!FileExists(PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\PL\\\\Const.lng"))
		ExtractRes((PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\PL\\\\Const.lng").w_str(),L"PL_CONST",L"DATA");
	else if(MD5File(PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\PL\\\\Const.lng")!="399F5F05929FA99B6A2F6C624D0FCECD")
		ExtractRes((PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\PL\\\\Const.lng").w_str(),L"PL_CONST",L"DATA");
	//4EFD8428E00EFD63B4D0C213DCA1005E
	if(!FileExists(PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\PL\\\\TSettingsForm.lng"))
		ExtractRes((PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\PL\\\\TSettingsForm.lng").w_str(),L"PL_SETTINGSFRM",L"DATA");
	else if(MD5File(PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\PL\\\\TSettingsForm.lng")!="4EFD8428E00EFD63B4D0C213DCA1005E")
		ExtractRes((PluginUserDir+"\\\\Languages\\\\NotifyMe\\\\PL\\\\TSettingsForm.lng").w_str(),L"PL_SETTINGSFRM",L"DATA");
	//Ustawienie sciezki lokalizacji wtyczki
	UnicodeString LangCode = (wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETLANGCODE,0,0);
	LangPath = PluginUserDir + "\\\\Languages\\\\NotifyMe\\\\" + LangCode + "\\\\";
	if(!DirectoryExists(LangPath))
	{
		LangCode = (wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETDEFLANGCODE,0,0);
		LangPath = PluginUserDir + "\\\\Languages\\\\NotifyMe\\\\" + LangCode + "\\\\";
	}
	//Tworzeniu katalogu z ustawieniami wtyczki
	if(!DirectoryExists(PluginUserDir+"\\\\NotifyMe"))
		CreateDir(PluginUserDir+"\\\\NotifyMe");
	//Sciezka do pliku ze statystykami
	StatsFilePath = PluginUserDir+"\\\\NotifyMe\\\\Stats.xml";
	//Wypakiwanie ikonki NotifyMe.dll.png
	//E9C441E4118D0DDAA67361FE0423B39A
	if(!DirectoryExists(PluginUserDir + "\\\\Shared"))
		CreateDir(PluginUserDir + "\\\\Shared");
	if(!FileExists(PluginUserDir + "\\\\Shared\\\\NotifyMe.dll.png"))
		ExtractRes((PluginUserDir + "\\\\Shared\\\\NotifyMe.dll.png").w_str(),L"SHARED",L"DATA");
	else if(MD5File(PluginUserDir + "\\\\Shared\\\\NotifyMe.dll.png")!="E9C441E4118D0DDAA67361FE0423B39A")
		ExtractRes((PluginUserDir + "\\\\Shared\\\\NotifyMe.dll.png").w_str(),L"SHARED",L"DATA");
	//Wypakiwanie ikonki FastAccess.png
	//C65DEEB1DB0588F07CBE175D9172FAB6
	if(!FileExists(PluginUserDir + "\\\\NotifyMe\\\\FastAccess.png"))
		ExtractRes((PluginUserDir + "\\\\NotifyMe\\\\FastAccess.png").w_str(),L"FASTACCESS",L"DATA");
	else if(MD5File(PluginUserDir + "\\\\NotifyMe\\\\FastAccess.png")!="C65DEEB1DB0588F07CBE175D9172FAB6")
		ExtractRes((PluginUserDir + "\\\\NotifyMe\\\\FastAccess.png").w_str(),L"FASTACCESS",L"DATA");
	//Przypisanie ikonki FASTACCESS do interfejsu AQQ
	FASTACCESS = PluginLink.CallService(AQQ_ICONS_LOADPNGICON,0, (LPARAM)(PluginUserDir + "\\\\NotifyMe\\\\FastAccess.png").w_str());
	//Tworzenie serwisu szybkiego dostep do ustawien wtyczki (statystyki)
	PluginLink.CreateServiceFunction(L"sNotifyMeFastStatsItem",ServiceNotifyMeFastStatsItem);
	//Odczyt ustawien
	LoadSettings();
	//Hook na zmiane kolorystyki AlphaControls
	PluginLink.HookEvent(AQQ_SYSTEM_COLORCHANGEV2,OnColorChange);
	//Hook na zmianê stanu kontaktu
	PluginLink.HookEvent(AQQ_CONTACTS_UPDATE,OnContactsUpdate);
	//Hook na zmiane lokalizacji
	PluginLink.HookEvent(AQQ_SYSTEM_LANGCODE_CHANGED,OnLangCodeChanged);
	//Hook na zakonczenie ladowania listy kontaktow przy starcie AQQ
	PluginLink.HookEvent(AQQ_CONTACTS_LISTREADY,OnListReady);
	//Hook na enumeracje listy kontatkow
	PluginLink.HookEvent(AQQ_CONTACTS_REPLYLIST,OnReplyList);
	//Hook na zmiane kompozycji
	PluginLink.HookEvent(AQQ_SYSTEM_THEMECHANGED,OnThemeChanged);
	//Hook na odbieranie pakietow XML zawierajace ID
	PluginLink.HookEvent(AQQ_SYSTEM_XMLIDDEBUG,OnXMLIDDebug);
	//Wszystkie moduly zostaly zaladowane
	if(PluginLink.CallService(AQQ_SYSTEM_MODULESLOADED,0,0))
	{
		//Pobranie ID dla enumeracji kontaktów
		ReplyListID = GetTickCount();
		//Wywolanie enumeracji kontaktow
		PluginLink.CallService(AQQ_CONTACTS_REQUESTLIST,(WPARAM)ReplyListID,0);
	}
	//Rejestowanie klasy okna timera
	WNDCLASSEX wincl;
	wincl.cbSize = sizeof (WNDCLASSEX);
	wincl.style = 0;
	wincl.lpfnWndProc = TimerFrmProc;
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;
	wincl.hInstance = HInstance;
	wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	wincl.lpszMenuName = NULL;
	wincl.lpszClassName = L"TNotifyMeTimer";
	wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wincl);
	//Tworzenie okna timera
	hTimerFrm = CreateWindowEx(0, L"TNotifyMeTimer", L"",	0, 0, 0, 0, 0, NULL, NULL, HInstance, NULL);

	return 0;
}
//---------------------------------------------------------------------------

extern "C" INT_PTR __declspec(dllexport) __stdcall Unload()
{
	//Usuwanie okna timera
	DestroyWindow(hTimerFrm);
	//Wyrejestowanie klasy okna timera
	UnregisterClass(L"TNotifyMeTimer",HInstance);
	//Szybki dostepu do ustawien wtyczki (statystyki)
	//Usuwanie interwejsu
	DestroyNotifyMeFastStats();
	//Usuwanie serwisu
	PluginLink.DestroyServiceFunction(ServiceNotifyMeFastStatsItem);
	//Wyladowanie wszystkich hookow
	PluginLink.UnhookEvent(OnColorChange);
	PluginLink.UnhookEvent(OnContactsUpdate);
	PluginLink.UnhookEvent(OnLangCodeChanged);
	PluginLink.UnhookEvent(OnListReady);
	PluginLink.UnhookEvent(OnReplyList);
	PluginLink.UnhookEvent(OnThemeChanged);
	PluginLink.UnhookEvent(OnXMLIDDebug);

	return 0;
}
//---------------------------------------------------------------------------

//Ustawienia wtyczki
extern "C" INT_PTR __declspec(dllexport)__stdcall Settings()
{
	//Otwieranie okna ustawien wtyczki
	OpenPluginSettings(false);

	return 0;
}
//---------------------------------------------------------------------------

//Informacje o wtyczce
extern "C" PPluginInfo __declspec(dllexport) __stdcall AQQPluginInfo(DWORD AQQVersion)
{
	PluginInfo.cbSize = sizeof(TPluginInfo);
	PluginInfo.ShortName = L"NotifyMe";
	PluginInfo.Version = PLUGIN_MAKE_VERSION(1,2,0,2);
	PluginInfo.Description = L"Powiadamia o sprawdzaniu naszej wersji oprogramowania oraz ostatniej aktywnoœci przez innego u¿ytkownika.";
	PluginInfo.Author = L"Krzysztof Grochocki";
	PluginInfo.AuthorMail = L"contact@beherit.pl";
	PluginInfo.Copyright = L"Krzysztof Grochocki";
	PluginInfo.Homepage = L"beherit.pl";
	PluginInfo.Flag = 0;
	PluginInfo.ReplaceDefaultModule = 0;

	return &PluginInfo;
}
//---------------------------------------------------------------------------
