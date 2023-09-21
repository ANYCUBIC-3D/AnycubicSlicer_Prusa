#include "ACCloudMachinePrivate.hpp"
#include "ACCloudManger.hpp"

#include "../GUI.hpp"
#include "../GUI_App.hpp"
#include "../GUI_Utils.hpp"
#include "../BitmapCache.hpp"
#include "../ACLabel.hpp"
#include "libslic3r/Utils.hpp"

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/font.h>
#include <wx/graphics.h>

#include <string>
#include <stdio.h>

//#define GetEditHwnd() ((HWND)(GetEditHWND()))

namespace Slic3r {

namespace GUI {

ACQuickResponseDialog::ACQuickResponseDialog(wxWindow* parent, const wxSize& size) :
	wxPopupTransientWindow(parent, wxBORDER_SIMPLE),
	buttonAndriod	{ new ACButton(this, "", "icon-andriod", "icon-andriod", "icon-andriod", wxBORDER_NONE, wxSize(120, 120)) },
	buttonIOS		{ new ACButton(this, "", "icon-ios", "icon-ios", "icon-ios", wxBORDER_NONE, wxSize(120, 120)) },
	buttonLogout	{ new ACButton(this, _L("Sign Out"), "", "", "") },
	title			{ new wxStaticText(this, wxID_ANY, "121") },
	textAndriod		{ new wxStaticText(this, wxID_ANY, _L("Andriod"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER) },
	textIOS			{ new wxStaticText(this, wxID_ANY, "IOS", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER) },
	sizer			{ new wxBoxSizer(wxVERTICAL) }
{
	Init();
	Connect();
}
ACQuickResponseDialog::~ACQuickResponseDialog()
{
	wxDELETE(buttonAndriod);
	wxDELETE(buttonIOS);
	wxDELETE(buttonLogout);
	wxDELETE(title);
	wxDELETE(textAndriod);
	wxDELETE(textIOS);
}
void ACQuickResponseDialog::Init()
{
	SetMinSize(FromDIP(wxSize(440, 312)));
	title->SetMinSize(FromDIP(wxSize(306, 36)));
	title->SetFont(ACLabel::sysFont(13, false));
	title->SetLabel(_L("Printing starts. You can view the printing progress or pause printing in Anycubic Cloud APP"));

	wxFont font = ACLabel::sysFont(14, true);
	textAndriod->SetFont(font);
	textIOS->SetFont(font);

	wxCursor cursor(wxCURSOR_HAND);
	buttonLogout->SetCursor(cursor);

	buttonAndriod->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_ICON);
	buttonAndriod->SetCornerRadius(0);
	buttonAndriod->SetPaddingSize(wxSize(0, 0));
	buttonAndriod->SetMinSize(wxSize(120, 120));//不需要使用FromDIP

	buttonIOS->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_ICON);
	buttonIOS->SetCornerRadius(0);
	buttonIOS->SetPaddingSize(wxSize(0, 0));
	buttonIOS->SetMinSize(wxSize(120, 120));//不需要使用FromDIP

	buttonLogout->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV3);
	buttonLogout->SetCornerRadius(0);
	buttonLogout->SetPaddingSize(wxSize(0, 0));
	buttonLogout->SetBackgroundColor(ACStateColor(wxColour(244, 244, 244)));
	buttonLogout->SetMinSize(wxSize(120, 40));//不需要使用FromDIP


	wxBoxSizer* vBoxAndriod = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* vBoxIOS = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* hBox = new wxBoxSizer(wxHORIZONTAL);
	vBoxAndriod->Add(buttonAndriod, 0, wxEXPAND | wxALL, 0);
	vBoxAndriod->AddSpacer(FromDIP(6));
	vBoxAndriod->Add(textAndriod,   0, wxEXPAND | wxALL, 0);
	vBoxIOS->Add(buttonIOS, 0, wxEXPAND | wxALL, 0);
	vBoxIOS->AddSpacer(FromDIP(6));
	vBoxIOS->Add(textIOS,   0, wxEXPAND | wxALL, 0);
	hBox->Add(vBoxAndriod);
	hBox->AddStretchSpacer(1);
	hBox->Add(vBoxIOS);
	
	sizer->AddSpacer(FromDIP(30));
	sizer->Add(title, 0, wxLEFT | wxRIGHT, FromDIP(70));
	sizer->AddSpacer(FromDIP(21));
	sizer->Add(hBox, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(55));
	sizer->AddStretchSpacer(1);
	sizer->Add(buttonLogout, 0, wxEXPAND | wxALL, 0);

	SetSize(FromDIP(wxSize(440, 312)));
	SetSizerAndFit(sizer);
	Layout();
}
void ACQuickResponseDialog::Connect()
{
	buttonAndriod->Bind(wxEVT_BUTTON, &ACQuickResponseDialog::OnButtonAndriod, this);
	buttonIOS    ->Bind(wxEVT_BUTTON, &ACQuickResponseDialog::OnButtonIOS,     this);
	buttonLogout ->Bind(wxEVT_BUTTON, &ACQuickResponseDialog::OnButtonLogout,  this);
}
void ACQuickResponseDialog::OnButtonAndriod(wxCommandEvent& event)
{
	wxLogMessage("andriod url");
}
void ACQuickResponseDialog::OnButtonIOS(wxCommandEvent& event)
{
	wxLogMessage("ios url");
}
void ACQuickResponseDialog::OnButtonLogout(wxCommandEvent& event)
{
	if (Cloud->cloudClient)//调用登出接口
		Cloud->cloudClient->logout();
}

} // GUI
} // Slic3r
