#include "ACCloudLogin.hpp"
#include "ACCloudManger.hpp"

#include "../ACLabel.hpp"
#include "../GUI.hpp"
#include "../MainFrame.hpp"
#include "../GUI_App.hpp"
#include "../GUI_Utils.hpp"
#include "../MsgDialog.hpp"
#include "libslic3r/Utils.hpp"

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/font.h>
#include <wx/timer.h>


#include <string>


//ConvertUtf8  wxString默认utf8编码
//#define CUTF8 .utf8_string().c_str()
//#define CUTF8 .mb_str(wxConvUTF8);

#define CUTF8 .utf8_str().data()

#define ERRORDLG(s1, s2) { textLog->SetLabel(s1); WarningDialog dlg(this, s1, s2); dlg.ShowModal(); }

namespace Slic3r {

namespace GUI {

wxDEFINE_EVENT(EVT_ACCLOUD_LOGIN_SUCCESS, wxCommandEvent);

ACCloudLoginDialog::ACCloudLoginDialog(wxWindow* parent)
    : DPIDialog(parent, wxID_ANY, wxString(""), wxDefaultPosition, wxSize(742, 562), /*wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER*/ wxNO_BORDER, ""),
	panel0				{ new ACDrawBackgroundPrivate(this, std::string("background_login"), 300, 560) },
	panel				{ new wxPanel(this) },
	textLogin			{ new wxStaticText(panel, wxID_ANY, _L("LOGIN")) },
	buttonClose			{ new ACButton(panel, "", "icon-close_40-nor_black", "software_close-hover", "icon-close_40-dis_black", wxBORDER_NONE, wxSize(50, 50)) },
	textGuide			{ new wxStaticText(panel, wxID_ANY, _L("Already have account?")) },
	buttonGuide			{ new ACButton(panel, _L("Login Now"), "icon-return-blue", "icon-return-black", "", wxBORDER_NONE, wxSize(16, 12)) },
	//textServer			{ new wxStaticText(panel, wxID_ANY, _L("Server")) },
	//line				{ new ACStaticDashLine(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE) },
	buttonMobile		{ new ACButtonUnderline(panel, _L("Mobile Login"), "", "", "", wxBORDER_NONE) },
	buttonEmail			{ new ACButtonUnderline(panel, _L("Email Login"), "", "", "", wxBORDER_NONE) },
	textName			{ new wxStaticText(panel, wxID_ANY, _L("Name"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL) },
	name				{ new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER) },
	//textAccountType		{ new wxStaticText(panel, wxID_ANY, _L("AccountType")) },
	textAccount			{ new wxStaticText(panel, wxID_ANY, _L("Account")) },
	account				{ new ACTextCtrlAccount(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER)},
	textVerication		{ new wxStaticText(panel, wxID_ANY, _L("Verication")) },
	verication			{ new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER) },
	buttonGetCode		{ new ACButton(panel, _L("Get Code"), "", "", "") },
	textPassword		{ new wxStaticText(panel, wxID_ANY, _L("Password")) },
	password			{ new ACTextCtrlPassword(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_PASSWORD) },
	textConfirm			{ new wxStaticText(panel, wxID_ANY, _L("Confirm")) },
	confirm				{ new ACTextCtrlPassword(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_PASSWORD) },
	//checkBox			{ new wxCheckBox(panel, wxID_ANY, _L("Remember me"), wxDefaultPosition, wxDefaultSize) },
	checkBox			{ new ACCheckBox(panel) },
	buttonForgetPassword{ new wxButton(panel, wxID_ANY, _L("Forget Password?"), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE) },
	checkBoxAgree		{ new ACCheckBox(panel) },
	textAgree0			{ new wxStaticText(panel, wxID_ANY, _L("I agree to the")) },
	textAgree1			{ new wxStaticText(panel, wxID_ANY, _L("and")) },
	buttonTerms			{ new wxButton(panel, wxID_ANY,  _L("Terms of Service"), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE) },
	buttonPrivacy		{ new wxButton(panel, wxID_ANY,  _L("The Privacy Policy"), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE) },
	button				{ new ACButton(panel, _L("Log In"), "", "", "", wxBORDER_NONE, wxSize(180, 36)) },
	textNoAccount		{ new wxStaticText(panel, wxID_ANY, _L("No Account?"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT) },
	buttonSignUpNow		{ new wxButton(panel, wxID_ANY, _L("Sign Up Now!"), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE) },
	textLog				{ new wxStaticText(panel, wxID_ANY, "Log output some info", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER)},
	sizer				{ new wxBoxSizer(wxHORIZONTAL) },
	hBoxTop				{ new wxBoxSizer(wxHORIZONTAL) },
	hBoxGuide			{ new wxBoxSizer(wxHORIZONTAL) },
	hBoxServer			{ new wxBoxSizer(wxHORIZONTAL) },
	hBoxRegister		{ new wxBoxSizer(wxHORIZONTAL) },
	hBoxVerication		{ new wxBoxSizer(wxHORIZONTAL) },
	hBoxAgree			{ new wxBoxSizer(wxHORIZONTAL) },
	vBox				{ new wxBoxSizer(wxVERTICAL) },
	hBoxPassword1		{ new wxBoxSizer(wxHORIZONTAL) },
	hBoxTips			{ new wxBoxSizer(wxHORIZONTAL) }
{
	Create();
	Init();
	Connect();
	isRegionCN = GUI::wxGetApp().is_region_CN();//区域设置，是否为中国区域
	//isVersionCN = false;//国际版
	ReLayout(isRegionCN ? LayoutMode::L_LOGIN_CN : LayoutMode::L_LOGIN_INT);
	if (!Cloud->isInit)//只进行一次初始化
		Cloud->InitLater();
	Cloud->PreInit();
	Cloud->SetPtr(this);//传递ui指针，Cloud的ui指针是变动的
}
ACCloudLoginDialog::~ACCloudLoginDialog()
{
	Cloud->SetPtr(nullptr);
	wxDELETE(textLogin);
	wxDELETE(buttonClose);

	wxDELETE(textGuide);
	wxDELETE(buttonGuide);

	//wxDELETE(textServer);
	//wxDELETE(comboServer);

	//wxDELETE(line);

	wxDELETE(buttonMobile);
	wxDELETE(buttonEmail);

	wxDELETE(textName);
	wxDELETE(name);

	//wxDELETE(textAccountType);
	//wxDELETE(comboAccountType);

	wxDELETE(textAccount);
	wxDELETE(account);

	wxDELETE(textVerication);
	wxDELETE(verication);
	wxDELETE(buttonGetCode);

	wxDELETE(textPassword);
	wxDELETE(password);

	wxDELETE(textConfirm);
	wxDELETE(confirm);

	wxDELETE(checkBox);
	wxDELETE(buttonForgetPassword);

	wxDELETE(checkBoxAgree);
	wxDELETE(textAgree0);
	wxDELETE(textAgree1);
	wxDELETE(buttonTerms);
	wxDELETE(buttonPrivacy);

	wxDELETE(button);

	wxDELETE(textNoAccount);
	wxDELETE(buttonSignUpNow);

	wxDELETE(textLog);

	//wxDELETE(panel);
	wxDELETE(panel0);
}
void ACCloudLoginDialog::Create()
{
	//wxArrayString choices;
	//choices.Add(_L("International"));
	//choices.Add(_L("China"));
	//comboServer = new ACComboBoxIcon(panel, wxID_ANY, std::string("icon_cloud_server"), wxEmptyString, wxDefaultPosition, wxDefaultSize, choices, wxCB_READONLY);
	//comboServer = new wxBitmapComboBox(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, choices, wxCB_READONLY);
	//comboServer = new ACComboBoxDrawText(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, choices, wxCB_READONLY);
	//choices.clear();
	//choices.Add(_L("Email"));
	//choices.Add(_L("Phone Number"));
	//comboAccountType = new ACComboBoxIcon(panel, wxID_ANY, std::string("icon_cloud_account_type"), wxEmptyString, wxDefaultPosition,
	//wxDefaultSize, choices, wxCB_READONLY);

}
void ACCloudLoginDialog::Init()
{
	AddWindowDrakEdg(this);

	const int margin = 32;
	const int textWidth = 20;
	//SetBackgroundColour(AC_COLOR_WHITE);
	panel->SetBackgroundColour(wxColour(255, 255, 255));
	wxSize size = FromDIP(wxSize(742, 562));
	SetMinSize(size);
	SetSize(size);

	wxCursor cursor(wxCURSOR_HAND);
	buttonClose         ->SetCursor(cursor);
	buttonGuide         ->SetCursor(cursor);
	buttonMobile        ->SetCursor(cursor);
	buttonEmail         ->SetCursor(cursor);
	buttonGetCode       ->SetCursor(cursor);
	buttonForgetPassword->SetCursor(cursor);
	buttonTerms         ->SetCursor(cursor);
	buttonPrivacy       ->SetCursor(cursor);
	button              ->SetCursor(cursor);
	buttonSignUpNow     ->SetCursor(cursor);
	checkBox            ->SetCursor(cursor);
	checkBoxAgree       ->SetCursor(cursor);


	wxFont font11 = ACLabel::sysFont(11, false);
	textName      ->SetFont(font11);
	textAccount   ->SetFont(font11);
	textPassword  ->SetFont(font11);
	textVerication->SetFont(font11);
	textConfirm   ->SetFont(font11);

	wxFont font12 = ACLabel::sysFont(12, false);
	textGuide    ->SetFont(font12);
	buttonGuide  ->SetFont(font12);
	textAgree0   ->SetFont(font12);
	textAgree1   ->SetFont(font12);
	buttonTerms  ->SetFont(font12);
	buttonPrivacy->SetFont(font12);
	textLog      ->SetFont(font12);

	wxFont fontNormal = ACLabel::sysFont(13, false);
	name         ->SetFont(fontNormal);
	account      ->SetFont(fontNormal);
	verication   ->SetFont(fontNormal);
	password     ->SetFont(fontNormal);
	confirm      ->SetFont(fontNormal);
	checkBox     ->SetFont(fontNormal);
	textNoAccount->SetFont(fontNormal);
	buttonGetCode->SetFont(fontNormal);

	buttonSignUpNow->SetFont(fontNormal);
	buttonForgetPassword->SetFont(fontNormal);

	wxFont font14 = ACLabel::sysFont(14, false);
	buttonMobile->SetFont(font14);
	buttonEmail ->SetFont(font14);

	wxFont font15 = ACLabel::sysFont(15, false);
	button->SetFont(font15);


	wxFont font26 = ACLabel::sysFont(26, true);
	textLogin->SetFont(font26);

	textLogin->SetForegroundColour(wxColour(51, 51, 51));
	textLogin->SetMinSize(FromDIP(wxSize(240, 35)));

	buttonClose->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_ICON);
	buttonClose->SetCornerRadius(0);
	buttonClose->SetPaddingSize(wxSize(0, 0));
	buttonClose->SetBackgroundColor(ACStateColor(wxColour(255, 255, 255)));
	buttonClose->SetMinSize(wxSize(50, 50));//不需要使用FromDIP

	buttonGuide->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV2);
	buttonGuide->SetCornerRadius(0);
	buttonGuide->SetPaddingSize(wxSize(0, 0));
	buttonGuide->SetSpacing(FromDIP(5));
	buttonGuide->SetBackgroundColor(ACStateColor(wxColour(255, 255, 255, 230)));
	buttonGuide->SetMinSize(wxSize(60, 16));//不需要使用FromDIP
	buttonGuide->SetTextColorNormal(wxColour(57, 134, 255));//blue font color

	//textServer->SetMinSize(FromDIP(wxSize(70, 20)));
	//comboServer->SetMinSize(FromDIP(wxSize(300, 28)));
	//comboServer->SetSelection(0);
	//comboServer->SetBackgroundColour(wxColour(255, 255, 255));

	//line->SetBackgroundColour(wxColour(200, 200, 200));
	//line->SetMinSize(FromDIP(wxSize(310 - 2, 2)));

	buttonMobile->SetMinSize(wxSize(100, 58));//派生自ACButton 不需要使用FromDIP
	buttonMobile->SetSelect(true);
	buttonEmail->SetMinSize(wxSize(100, 58));//派生自ACButton 不需要使用FromDIP
	buttonEmail->SetSelect(false);

	textName->SetMinSize(FromDIP(wxSize(70, textWidth)));
	name->SetMinSize(FromDIP(wxSize(300, 28)));
	name->SetHint(_L("Please enter nickname"));

	textAccount->SetMinSize(FromDIP(wxSize(70, textWidth)));
	account->SetMinSize(FromDIP(wxSize(300, 28)));
	account->SetModelType(ACTextCtrlAccount::ModelType::AC_PHONE);
	//account->SetInitialSize(account->GetBestSize());
	
	//textLogin->SetLayoutDirection(wxLayoutDirection::wxLayout_RightToLeft);
	//textAccountType->Show(false);
	//comboAccountType->SetMinSize(FromDIP(wxSize(100, 28)));
	//comboAccountType->SetSelection(1);

	//account->SetMinSize(wxSize(150, 28));
	//account->SetHint(_L("Please enter email"));
	//account->SetMargins(wxPoint(margin, -1));
	//account->SetInsertionPointEnd();
	//account->SetHint(_L("Please enter phone number"));
	
	//account->SetModelType(ACTextCtrlAccount::ModelType::AC_EMAIL);
	//account->SetModelType(account->GetModelType());

	textVerication->SetMinSize(FromDIP(wxSize(70, textWidth)));
	verication->SetMinSize(FromDIP(wxSize(204, 28)));
	verication->SetHint(_L("Please enter verication code"));
	buttonGetCode->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV0);
	buttonGetCode->SetCornerRadius(6);
	buttonGetCode->SetPaddingSize(wxSize(0, 0));
	//buttonGetCode->SetBackgroundColor(ACStateColor(wxColour(255, 255, 255)));
	buttonGetCode->SetMinSize(wxSize(86, 28));//不需要使用FromDIP
	ACStateColor fgColor;
	fgColor.append(wxColour(168, 168, 168),   ACStateColor::Disabled);
	fgColor.append(AC_COLOR_BG_WHITE_PRESSED, ACStateColor::Pressed);
	fgColor.append(AC_COLOR_BG_WHITE_HOVER,   ACStateColor::Hovered);
	fgColor.append(AC_COLOR_BG_WHITE,         ACStateColor::Normal);
	buttonGetCode->SetTextColor(fgColor);

	textPassword->SetMinSize(FromDIP(wxSize(70, textWidth)));
	password->SetMinSize(FromDIP(wxSize(300, 28)));
	//password->SetMargins(wxPoint(margin, -1));
	password->SetHint(_L("Please enter your password"));

	textConfirm->SetMinSize(FromDIP(wxSize(70, textWidth)));
	confirm->SetMinSize(FromDIP(wxSize(300, 28)));
	confirm->SetHint(_L("Please confirm password"));

	checkBox->SetValue(true);
	checkBox->SetBitmapPosition(wxLEFT);
	//checkBox->SetLabel(_L("Remember me"));
	checkBox->SetLabel(_L("Auto Login"));
	checkBox->SetMinSize(FromDIP(wxSize(100, 24)));
	checkBox->SetBackgroundColour(*wxWHITE);

	buttonForgetPassword->SetMinSize(wxSize(120, 28));
	buttonForgetPassword->SetBackgroundColour(*wxWHITE);
	buttonForgetPassword->SetForegroundColour(wxColour(57, 134, 255));//blue font color
	hBoxPassword1->Add(checkBox, 0, wxALIGN_CENTER | wxALL, 0);
	hBoxPassword1->AddStretchSpacer(1);
	hBoxPassword1->Add(buttonForgetPassword, 0, wxALIGN_CENTER | wxALL, 0);

	checkBoxAgree->SetValue(true);
	checkBoxAgree->SetBackgroundColour(*wxWHITE);
	buttonTerms->SetBackgroundColour(*wxWHITE);
	buttonTerms->SetForegroundColour(wxColour(57, 134, 255));//blue font color
	buttonPrivacy->SetBackgroundColour(*wxWHITE);
	buttonPrivacy->SetForegroundColour(wxColour(57, 134, 255));//blue font color
	

	button->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV0);
	button->SetCornerRadius(6);
	button->SetPaddingSize(wxSize(0, 0));
	button->SetSpacing(0);
	button->SetMinSize(/*FromDIP(*/wxSize(310, 36)/*)*/);//不需要进行DIP设置
	//button->SetBackgroundColor(*wxWHITE);
	
	textNoAccount->SetMinSize(FromDIP(wxSize(90, 24)));
	buttonSignUpNow->SetMinSize(FromDIP(wxSize(90, 24)));
	buttonSignUpNow->SetBackgroundColour(*wxWHITE);
	buttonSignUpNow->SetForegroundColour(wxColour(57, 134, 255));//blue font color
	hBoxTips->AddStretchSpacer(1);
	hBoxTips->Add(textNoAccount, 0, wxALIGN_CENTER | wxTOP, FromDIP(4));
	hBoxTips->AddSpacer(FromDIP(4));
	hBoxTips->Add(buttonSignUpNow, 0, wxALIGN_CENTER | wxALL, 0);
	hBoxTips->AddStretchSpacer(1);

	//
	textLog->SetForegroundColour(wxColour(229, 46, 46));
	
	//Layout Panel
	InitLayoutPanel();

	sizer->Add(panel0, 30, wxALIGN_CENTER | wxEXPAND | wxALL, 1);
	sizer->Add(panel,  44, wxALIGN_CENTER | wxEXPAND | wxALL, 1);

	SetSizerAndFit(sizer);
	Layout();
	Refresh();
}
void ACCloudLoginDialog::Connect()
{
	//comboServer->Bind(wxEVT_COMBOBOX, &ACCloudLoginDialog::OnComboBoxServer, this);
	timer.Bind(wxEVT_TIMER, &ACCloudLoginDialog::OnTimer, this);
	//Connect(timer->GetId(), wxEVT_TIMER, wxTimerEventHandler(ACCloudLoginDialog::OnTimer), nullptr, this);
	//Bind(EVT_ACLOUD_PRINTER, &ACCloudLoginDialog::OnReceiveEvent, this);
	//Bind(EVT_ACCLOUD_LOGIN_SUCCESS, [this](wxCommandEvent&) { Close(); });

	buttonClose  ->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { Close(); });

	buttonGuide  ->Bind(wxEVT_BUTTON, &ACCloudLoginDialog::OnButtonGuide,   this);
	buttonMobile ->Bind(wxEVT_BUTTON, &ACCloudLoginDialog::OnButtonMobile,  this);
	buttonEmail  ->Bind(wxEVT_BUTTON, &ACCloudLoginDialog::OnButtonEmail,   this);
	buttonGetCode->Bind(wxEVT_BUTTON, &ACCloudLoginDialog::OnButtonGetCode, this);

	buttonForgetPassword->Bind(wxEVT_BUTTON, &ACCloudLoginDialog::OnButtonForgetPassword, this);
	buttonSignUpNow     ->Bind(wxEVT_BUTTON, &ACCloudLoginDialog::OnButtonSignUpNow, this);
	button              ->Bind(wxEVT_BUTTON, &ACCloudLoginDialog::OnButtonClicked, this);

	checkBox     ->Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) { OnCheckBox(); });
	checkBoxAgree->Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) { OnCheckBoxAgree(); });
}
void ACCloudLoginDialog::InitLayoutPanel()
{
	const int margin = FromDIP(65);
	const int spaceColumn = FromDIP(10);
	int flagFull = wxALIGN_CENTER | wxEXPAND | wxLEFT | wxRIGHT;
	int flagLeft = wxALIGN_LEFT | wxALIGN_BOTTOM | wxEXPAND | wxLEFT | wxRIGHT;

	// (LOG IN/SIGN UP/RESET PASSWORD text) && Close button
	hBoxTop->Add(textLogin, 0, wxALIGN_LEFT | wxTOP, FromDIP(50));
	hBoxTop->AddStretchSpacer(1);
	hBoxTop->Add(buttonClose, 0, wxALIGN_RIGHT | wxALL, 0);
	vBox->Add(hBoxTop, 0, wxALIGN_CENTER | wxEXPAND | wxLEFT, margin);//0
	vBox->AddSpacer(FromDIP(10));//1


	//Guide text and Guide button
	hBoxGuide->Add(textGuide, 0, wxALIGN_CENTER | wxALL, 0);
	hBoxGuide->AddSpacer(FromDIP(4));
	hBoxGuide->Add(buttonGuide, 0, wxALIGN_CENTER | wxALL, 0);
	vBox->Add(hBoxGuide, 0, flagFull, margin);//2
	vBox->AddSpacer(FromDIP(24));//3

	//Server
	//hBoxServer->Add(textServer, 0, wxALIGN_BOTTOM | wxEXPAND | wxALL, 0);
	//hBoxServer->Add(comboServer, 1, wxALIGN_CENTER | wxEXPAND | wxALL, 0);
	//vBox->Add(hBoxServer, 0, flagFull, margin);//4
	//vBox->AddSpacer(FromDIP(18));//5

	//line
	//vBox->Add(line, 0, flagFull, margin);
	//vBox->AddSpacer(FromDIP(16));

	//Register button
	hBoxRegister->Add(buttonMobile, 1, wxALIGN_CENTER | wxEXPAND | wxALL, 0);
	hBoxRegister->AddSpacer(FromDIP(10));
	hBoxRegister->Add(buttonEmail, 1, wxALIGN_CENTER | wxEXPAND | wxALL, 0);
	vBox->Add(hBoxRegister, 0, flagFull, margin);//4
	vBox->AddSpacer(FromDIP(30));//5

	//Name
	vBox->Add(textName, 0, flagFull, margin);
	vBox->Add(name, 0, flagFull, margin);
	vBox->AddSpacer(spaceColumn);//8

	//vBox->Add(textAccountType,  0, flagLeft, margin);
	//vBox->Add(comboAccountType, 0, flagFull, margin);
	//vBox->AddSpacer(spaceColumn);

	//Account
	vBox->Add(textAccount, 0, flagFull, margin);
	vBox->Add(account, 0, flagFull, margin);
	vBox->AddSpacer(spaceColumn);//11

	//Verication
	vBox->Add(textVerication, 0, flagFull, margin);
	hBoxVerication->Add(verication, 1, wxALIGN_CENTER | wxEXPAND | wxALL, 0);
	hBoxVerication->AddSpacer(FromDIP(10));
	hBoxVerication->Add(buttonGetCode, 0, wxALIGN_CENTER | wxEXPAND | wxALL, 0);
	vBox->Add(hBoxVerication, 0, flagFull, margin);
	vBox->AddSpacer(spaceColumn);//14

	//Password
	vBox->Add(textPassword, 0, flagFull, margin);
	vBox->Add(password, 0, flagFull, margin);
	vBox->AddSpacer(spaceColumn);//17

	//Confirm
	vBox->Add(textConfirm, 0, flagFull, margin);
	vBox->Add(confirm, 0, flagFull, margin);
	vBox->AddSpacer(spaceColumn);//20

	//Remember me
	vBox->Add(hBoxPassword1, 0, flagFull, margin);
	vBox->AddSpacer(FromDIP(10));//22

	//Agress terms
	hBoxAgree->Add(checkBoxAgree, 0, wxALIGN_CENTER /*| wxEXPAND*/ | wxALL, 0);
	hBoxAgree->AddSpacer(FromDIP(3));
	hBoxAgree->Add(textAgree0, 0, wxALIGN_CENTER | wxEXPAND | wxTOP, FromDIP(4));
	hBoxAgree->Add(buttonTerms, 0, wxALIGN_CENTER | wxEXPAND | wxALL, 0);
	hBoxAgree->Add(textAgree1, 0, wxALIGN_CENTER | wxEXPAND | wxTOP, FromDIP(4));
	hBoxAgree->AddSpacer(FromDIP(6));
	hBoxAgree->Add(buttonPrivacy, 0, wxALIGN_CENTER | wxEXPAND | wxALL, 0);
	hBoxAgree->AddStretchSpacer(1);
	vBox->Add(hBoxAgree, 0, flagFull, margin);
	vBox->AddSpacer(FromDIP(10));//24


	vBox->Add(button, 0, flagFull, margin);
	vBox->AddSpacer(FromDIP(10));//26
	vBox->Add(hBoxTips, 0, flagFull, margin);

	vBox->AddStretchSpacer(1);

	vBox->Add(textLog, 0, flagLeft, margin);
	vBox->AddSpacer(12);



	panel->SetMinSize(FromDIP(wxSize(440, 560)));
	panel->SetSizer/*AndFit*/(vBox, true);
}

void ACCloudLoginDialog::ReLayout(LayoutMode mode)
{
	layoutMode = mode;
	ChangeText(mode);
	switch (mode)
	{
	case LayoutMode::L_LOGIN_CN:
	{
		//Guide text and Guide button
		textGuide->Hide();
		hBoxGuide->GetItem(1)->Show(false);
		buttonGuide->Hide();

		//Register
		ResetMobile();
		buttonMobile->Hide();
		buttonEmail->Hide();

		//Name
		textName->Hide();
		name->Hide();
		vBox->GetItem(8)->Show(false);

		//Verication
		textVerication->Hide();
		verication->Hide();
		buttonGetCode->Hide();
		vBox->GetItem(14)->Show(false);

		//Confirm
		textConfirm->Hide();
		confirm->Hide();
		vBox->GetItem(20)->Show(false);

		//Remember me
		checkBox->Show();
		buttonForgetPassword->Show();
		vBox->GetItem(22)->Show(true);

		//Agress terms
		checkBoxAgree->Hide();
		textAgree0->Hide();
		buttonTerms->Hide();
		textAgree1->Hide();
		buttonPrivacy->Hide();
		vBox->GetItem(24)->Show(false);

		//Tips
		textNoAccount->Show();
		buttonSignUpNow->Show();
		vBox->GetItem(26)->Show(true);
	}
		break;
	case LayoutMode::L_LOGIN_INT:
	{
		//Guide text and Guide button
		textGuide->Hide();
		hBoxGuide->GetItem(1)->Show(false);
		buttonGuide->Hide();

		//Register
		buttonMobile->Show();
		buttonEmail->Show();

		//Name
		textName->Hide();
		name->Hide();
		vBox->GetItem(8)->Show(false);

		//Verication
		textVerication->Hide();
		verication->Hide();
		buttonGetCode->Hide();
		vBox->GetItem(14)->Show(false);

		//Confirm
		textConfirm->Hide();
		confirm->Hide();
		vBox->GetItem(20)->Show(false);

		//Remember me
		checkBox->Show();
		buttonForgetPassword->Show();
		vBox->GetItem(22)->Show(true);

		//Agress terms
		checkBoxAgree->Hide();
		textAgree0->Hide();
		buttonTerms->Hide();
		textAgree1->Hide();
		buttonPrivacy->Hide();
		vBox->GetItem(24)->Show(false);

		//Tips
		textNoAccount->Show();
		buttonSignUpNow->Show();
		vBox->GetItem(26)->Show(true);
	}
		break;
	case LayoutMode::L_SIGNUP_CN:
	{
		//Guide text and Guide button
		textGuide->Show();
		hBoxGuide->GetItem(1)->Show(true);
		buttonGuide->Show();

		//Register
		ResetMobile();
		buttonMobile->Hide();
		buttonEmail->Hide();

		//Name
		textName->Show();
		name->Show();
		vBox->GetItem(8)->Show(true);

		//Verication
		textVerication->Show();
		verication->Show();
		buttonGetCode->Show();
		vBox->GetItem(14)->Show(true);

		//Confirm
		textConfirm->Hide();
		confirm->Hide();
		vBox->GetItem(20)->Show(false);

		//Remember me
		checkBox->Hide();
		buttonForgetPassword->Hide();
		vBox->GetItem(22)->Show(false);

		//Agress terms
		checkBoxAgree->Show();
		textAgree0->Show();
		buttonTerms->Show();
		textAgree1->Show();
		buttonPrivacy->Show();
		vBox->GetItem(24)->Show(true);

		//Tips
		textNoAccount->Hide();
		buttonSignUpNow->Hide();
		vBox->GetItem(26)->Show(false);
	}
		break;
	case LayoutMode::L_SIGNUP_INT:
	{
		//Guide text and Guide button
		textGuide->Show();
		hBoxGuide->GetItem(1)->Show(true);
		buttonGuide->Show();

		//Register
		buttonMobile->Show();
		buttonEmail->Show();

		//Name
		textName->Show();
		name->Show();
		vBox->GetItem(8)->Show(true);

		//Verication
		textVerication->Show();
		verication->Show();
		buttonGetCode->Show();
		vBox->GetItem(14)->Show(true);

		//Confirm
		textConfirm->Hide();
		confirm->Hide();
		vBox->GetItem(20)->Show(false);

		//Remember me
		checkBox->Hide();
		buttonForgetPassword->Hide();
		vBox->GetItem(22)->Show(false);

		//Agress terms
		checkBoxAgree->Show();
		textAgree0->Show();
		buttonTerms->Show();
		textAgree1->Show();
		buttonPrivacy->Show();
		vBox->GetItem(24)->Show(true);

		//Tips
		textNoAccount->Hide();
		buttonSignUpNow->Hide();
		vBox->GetItem(26)->Show(false);
	}
		break;
	case LayoutMode::L_RESET_CN:
	{
		//Guide text and Guide button
		textGuide->Hide();
		hBoxGuide->GetItem(1)->Show(false);
		buttonGuide->Show();

		//Register
		ResetMobile();
		buttonMobile->Hide();
		buttonEmail->Hide();

		//Name
		textName->Hide();
		name->Hide();
		vBox->GetItem(8)->Show(false);

		//Verication
		textVerication->Show();
		verication->Show();
		buttonGetCode->Show();
		vBox->GetItem(14)->Show(true);

		//Confirm
		textConfirm->Show();
		confirm->Show();
		vBox->GetItem(20)->Show(true);

		//Remember me
		checkBox->Hide();
		buttonForgetPassword->Hide();
		vBox->GetItem(22)->Show(false);

		//Agress terms
		checkBoxAgree->Hide();
		textAgree0->Hide();
		buttonTerms->Hide();
		textAgree1->Hide();
		buttonPrivacy->Hide();
		vBox->GetItem(24)->Show(false);

		//Tips
		textNoAccount->Hide();
		buttonSignUpNow->Hide();
		vBox->GetItem(26)->Show(false);
	}
		break;
	case LayoutMode::L_RESET_INT:
	{
		//Guide text and Guide button
		textGuide->Hide();
		hBoxGuide->GetItem(1)->Show(false);
		buttonGuide->Show();

		//Register
		buttonMobile->Show();
		buttonEmail->Show();

		//Name
		textName->Hide();
		name->Hide();
		vBox->GetItem(8)->Show(false);

		//Verication
		textVerication->Show();
		verication->Show();
		buttonGetCode->Show();
		vBox->GetItem(14)->Show(true);

		//Confirm
		textConfirm->Show();
		confirm->Show();
		vBox->GetItem(20)->Show(true);

		//Remember me
		checkBox->Hide();
		buttonForgetPassword->Hide();
		vBox->GetItem(22)->Show(false);

		//Agress terms
		checkBoxAgree->Hide();
		textAgree0->Hide();
		buttonTerms->Hide();
		textAgree1->Hide();
		buttonPrivacy->Hide();
		vBox->GetItem(24)->Show(false);

		//Tips
		textNoAccount->Hide();
		buttonSignUpNow->Hide();
		vBox->GetItem(26)->Show(false);
	}
		break;
	default:
		break;
	}
	Layout();
	Refresh();
}
void ACCloudLoginDialog::ChangeText(LayoutMode mode)
{
	switch (mode)
	{
	case LayoutMode::L_LOGIN_CN:
	case LayoutMode::L_LOGIN_INT:
	{
		textLogin    ->SetLabel(_L("Log In"));
		buttonGuide  ->SetLabel(_L("Login Now"));
		buttonMobile ->SetLabel(_L("Mobile Login"));
		buttonEmail  ->SetLabel(_L("Email Login"));
		button       ->SetLabel(_L("Log In"));
		button->SetEnable(true);
		button->Layout();
	}
		break;
	case LayoutMode::L_SIGNUP_CN:
	case LayoutMode::L_SIGNUP_INT:
	{
		textLogin    ->SetLabel(_L("Sign Up"));
		buttonGuide  ->SetLabel(_L("Login Now"));
		buttonMobile ->SetLabel(_L("Register by Mobile"));
		buttonEmail  ->SetLabel(_L("Register by Email"));
		button       ->SetLabel(_L("Sign Up"));
		button->SetEnable(checkBoxAgree->isChecked());
		button->Layout();
	}
		break;
	case LayoutMode::L_RESET_CN:
	case LayoutMode::L_RESET_INT:
	{
		textLogin    ->SetLabel(_L("Reset Password"));
		buttonGuide  ->SetLabel(_L("Back To Login"));
		buttonMobile ->SetLabel(_L("Reset Via Phone"));
		buttonEmail  ->SetLabel(_L("Reset Via Email"));
		button       ->SetLabel(_L("Reset Password"));
		button->SetEnable(true);
		button->Layout();
	}
		break;
	default:
		break;
	}
}
void ACCloudLoginDialog::ResetMobile()
{
	/***********OnButtonMobile**********/
	buttonMobile->SetSelect(true);
	buttonEmail ->SetSelect(false);
	account->SetModelType(ACTextCtrlAccount::ModelType::AC_PHONE);
	/***********OnButtonMobile**********/
}
void ACCloudLoginDialog::OnButtonGuide(wxCommandEvent& event)
{
	switch (layoutMode)
	{
	case LayoutMode::L_LOGIN_CN:
		break;
	case LayoutMode::L_LOGIN_INT:
		break;
	case LayoutMode::L_SIGNUP_CN:
	case LayoutMode::L_RESET_CN:
		ReLayout(L_LOGIN_CN);
		break;
	case LayoutMode::L_SIGNUP_INT:
	case LayoutMode::L_RESET_INT:
		ReLayout(L_LOGIN_INT);
		break;
	default:
		break;
	}
}
void ACCloudLoginDialog::OnButtonMobile(wxCommandEvent& event)
{
	buttonMobile->SetSelect(true);
	buttonEmail->SetSelect(false);
	account->SetModelType(ACTextCtrlAccount::ModelType::AC_PHONE);

}
void ACCloudLoginDialog::OnButtonEmail(wxCommandEvent& event)
{
	buttonMobile->SetSelect(false);
	buttonEmail->SetSelect(true);
	account->SetModelType(ACTextCtrlAccount::ModelType::AC_EMAIL);
}
void ACCloudLoginDialog::OnButtonForgetPassword(wxCommandEvent& event)
{
	if (!isRegionCN)
	{
		ReLayout(LayoutMode::L_RESET_INT);
	}
	else
	{
		ReLayout(LayoutMode::L_RESET_CN);
	}
}
void ACCloudLoginDialog::OnButtonSignUpNow(wxCommandEvent& event)
{
	if (!isRegionCN)
	{
		ReLayout(LayoutMode::L_SIGNUP_INT);
	}
	else
	{
		ReLayout(LayoutMode::L_SIGNUP_CN);
	}
}
void ACCloudLoginDialog::OnButtonGetCode(wxCommandEvent& event)
{
    if(!wxGetApp().mainframe->showWebDialog()){
        /*WarningDialog dialog(wxGetApp().mainframe, _L("Human machine authentication failed, please re authenticate."), _L("Warning"), wxYES);
        dialog.ShowModal();*/
        return ;
	}

	seconds = 59;
	buttonGetCode->SetEnable(false);
	timer.Start(1000);
	buttonGetCode->SetMinSize(wxSize(100, 24));
	Layout();
	Refresh();
}
void ACCloudLoginDialog::OnButtonClicked(wxCommandEvent& event)
{
	button->SetCursor(wxCursor(wxCURSOR_WAIT));//鼠标等待
	if (Cloud->cloudClient)
	{
		auto s1 = account->GetValue()CUTF8;
		auto s2 = password->GetValue()CUTF8;

		//const char* name_;//昵称
		//const char* account_;//账号
		//const char* password_;//密码
		//const char* verication_;//验证码
		//const char* confirm_; //密码 二次输入

		bool go{ true };
		//switch (layoutMode)
		//{
		//case L_LOGIN_CN://登录
		//{
		//	go = (CrcAccountPhone() && CrcPassword());
		//}
		//break;
		//case L_LOGIN_INT://登录
		//{
		//	go = (buttonEmail && (buttonEmail->isSelect ? CrcAccountEmail() : CrcAccountPhone()) && CrcPassword());
		//}
		//break;
		//case L_SIGNUP_CN://注册
		//{
		//	go = (CrcName() && CrcAccountPhone() && CrcVerification() && CrcPassword());
		//}
		//break;
		//case L_SIGNUP_INT://注册
		//{
		//	go = (CrcName() && buttonEmail && (buttonEmail->isSelect ? CrcAccountEmail() : CrcAccountPhone()) \
		//		&& CrcVerification() && CrcPassword());
		//}
		//break;
		//case L_RESET_CN://重置密码
		//{
		//	go = (CrcAccountPhone() && CrcVerification() && CrcPassword() && CrcConfirm());
		//}
		//break;
		//case L_RESET_INT://重置密码
		//{
		//	go = (buttonEmail && (buttonEmail->isSelect ? CrcAccountEmail() : CrcAccountPhone())\
		//		&& CrcVerification() && CrcPassword() && CrcConfirm());
		//}
		//break;
		//default:
		//	break;
		//}
		if (go)
			switch (layoutMode)
			{
			case L_LOGIN_CN:
			case L_LOGIN_INT: //登录
			{
				//Cloud->cloudClient->login("18824288536", "zhang1zhen23", isRegionCN ? 1 : 2);//国外固定为2，非2的为国内
				EResultCode code = Cloud->Login("15110000001", "jiang123", isRegionCN ? 1 : 2);//国外固定为2，非2的为国内
				switch (code)
				{
				case E_FALSE://执行失败
					textLog->SetLabel(Cloud->logMessage);
					break;
				case E_TRUE://执行成功
				{
					textLog->SetLabel(_L("successed"));
					Cloud->PrinterList();//获取打印机
					wxCommandEvent* evt = new wxCommandEvent(EVT_ACCLOUD_LOGIN_SUCCESS);
					wxPostEvent(GUI::wxGetApp().mainframe, *evt);//异步执行打印机

					//wxQueueEvent(this, evt); //通过异步来执行关闭
					Close();//关闭界面
				}
				break;
				case E_TIMEOUT://超时
					textLog->SetLabel(_L("Failed time out"));
					break;
				default:
					break;
				}
			}
				break;
			case L_SIGNUP_CN:
			case L_SIGNUP_INT://注册
				//Cloud->cloudClient->registerUser()
				break;
			case L_RESET_CN:
			case L_RESET_INT: //重置密码
				break;
			default:
				break;
			}
		
	}
	else
	{
		wxLogMessage("error");
	}
	button->SetCursor(wxCursor(wxCURSOR_HAND));//还原
}
void ACCloudLoginDialog::OnCheckBox()
{
	checkBox->SetValue(checkBox->isChecked());//作为下次是否自动登录账号的标志来使用
}
void ACCloudLoginDialog::OnCheckBoxAgree()
{
	checkBoxAgree->SetValue(checkBoxAgree->isChecked());
	button->SetEnable(checkBoxAgree->isChecked());
}
bool ACCloudLoginDialog::CrcAccountPhone()//校验电话号码
{
	if (account->IsEmpty())//账号不为空
	{
		ERRORDLG(_L("Account cannot be empty!"), L("Warning"))
		return false;
	}
	if (!account->GetValue().IsNumber())
	{
		ERRORDLG(_L("Please enter the correct phone number."), L("Warning"))
		return false;
	}
	if (L_LOGIN_CN == layoutMode || L_SIGNUP_CN == layoutMode || L_RESET_CN == layoutMode)//如果为中国区域，检测账号是否为11位
	{
		if (account->GetValue().size() != 11)
		{
			ERRORDLG(_L("Please enter 11 valid phone numbers."), L("Warning"))//请输入11位有效电话号码
			return false;
		}
	}
	return true;
}
bool ACCloudLoginDialog::CrcAccountEmail()//校验邮箱
{
	if (account->IsEmpty())//账号不为空
	{
		ERRORDLG(_L("Account cannot be empty!"), L("Warning"))
		return false;
	}
	wxString str = account->GetValue();
	if (str.Find('@') == wxNOT_FOUND)//没找到@关键字
	{
		ERRORDLG(_L("Please enter the correct email."), L("Warning"))
		return false;
	}
	size_t atIndex = str.find('@');

	if (atIndex != wxString::npos && atIndex > 0 && atIndex < str.length() - 1)//@前后都含有字符
		return true;
	else
	{
		ERRORDLG(_L("Please enter the correct email."), L("Warning"))
		return false;
	}
}
bool ACCloudLoginDialog::CrcVerification()//校验验证码
{
	if (verication->IsEmpty())//校验码不为空
	{
		ERRORDLG(_L("Verication cannot be empty!"), L("Warning"))
		return false;
	}
	if (verication->GetValue().size() != 6 || !verication->GetValue().IsNumber())//验证码不为6位或者不全为数字
	{
		ERRORDLG(_L("Please enter the correct verication."), L("Warning"))//请输入正确的验证码
		return false;
	}
	return true;
}
bool ACCloudLoginDialog::CrcName()        //校验昵称
{
	if (name->IsEmpty())//账号不为空
	{
		ERRORDLG(_L("Name cannot be empty!"), L("Warning"))
		return false;
	}
	return true;
}
bool ACCloudLoginDialog::CrcPassword()    //校验密码
{
	if (password->IsEmpty())//账号不为空
	{
		ERRORDLG(_L("Password cannot be empty!"), L("Warning"))
		return false;
	}
	return true;
}
bool ACCloudLoginDialog::CrcConfirm()     //校验密码确认
{
	if (confirm->IsEmpty())//账号不为空
	{
		ERRORDLG(_L("Confirm cannot be empty!"), L("Warning"))
		return false;
	}
	if (password->GetValue() != confirm->GetValue())
	{
		ERRORDLG(_L("The two passwords entered are inconsistent."), L("Warning"))//两次密码输入不一致
		return false;
	}
	return true;
}
//void ACCloudLoginDialog::OnComboBoxServer(wxCommandEvent& event)
//{
//	int selectedIndex = event.GetSelection();
//	if (0 == selectedIndex)//INT
//	{
//		if (L_LOGIN_CN == layoutMode)
//			ReLayout(L_LOGIN_INT);
//		else if (L_SIGNUP_CN == layoutMode)
//			ReLayout(L_SIGNUP_INT);
//		else if (L_RESET_CN == layoutMode)
//			ReLayout(L_RESET_INT);
//	}
//	else
//	{
//		if (L_LOGIN_INT == layoutMode)
//			ReLayout(L_LOGIN_CN);
//		else if (L_SIGNUP_INT == layoutMode)
//			ReLayout(L_SIGNUP_CN);
//		else if (L_RESET_INT == layoutMode)
//			ReLayout(L_RESET_CN);
//	}
//
//	//#if defined(__WXMSW__)
//	//		HWND hWnd = (HWND)password->GetHandle();
//	//		::SendMessage(hWnd, EM_SETPASSWORDCHAR, selectedIndex ? 0 : 0x25cf, 0); // 0x25cf is ● character
//	//		password->Refresh();
//	//#endif
//
//
//
//			//password->SetValue(password->GetValue());
//			/*if (selectedIndex)
//			{
//				password->SetWindowStyleFlag(wxTE_PROCESS_ENTER | wxTE_PASSWORD);
//			}
//			else
//				password->SetWindowStyleFlag(wxTE_PROCESS_ENTER);
//			password->Refresh();*/
//			// 在这里编写选中事件的处理代码
//			//wxLogMessage(wxT("选中项的索引: %d, 文本: %s"), selectedIndex, s);
//	event.Skip();
//}
void ACCloudLoginDialog::OnTimer(wxTimerEvent& event)
{
	if (seconds > 0)
	{
		seconds--;
		wxString text = _L("Resend") +  wxString::Format(" (%dS)", seconds);
		buttonGetCode->SetLabel(text);
	}
	else
	{
		buttonGetCode->SetEnable(true);
		timer.Stop();
		buttonGetCode->SetLabel(_L("Get Code"));
		buttonGetCode->SetMinSize(wxSize(86, 24));
		Layout();
		Refresh();
	}
}

void ACCloudLoginDialog::msw_rescale()
{
	password->SetMinSize(FromDIP(wxSize(0, 28)));
	password->SetMargins(wxPoint(FromDIP(32), -1));
	account->SetMinSize(FromDIP(wxSize(0, 28)));
	//account->SetMargins(wxPoint(FromDIP(32 + 86), -1));

	checkBox->Rescale();
	checkBox->Layout();
	checkBoxAgree->Rescale();
	checkBoxAgree->Layout();

	wxSize size = FromDIP(wxSize(740, 560));
	SetMinSize(size);

	Fit();
	Layout();
	Refresh();
}
//void ACCloudLoginDialog::CallLogPut(bool isSuccess, int id, const wxString& str)
//{
//	//wxLogMessage(isSuccess ? "successed!" : "failed");
//	wxString s;
//	switch (id)
//	{
//	case EventType::EventSpaceInfo:        ///< 获取空间信息
//		s = "EventSpaceInfo";
//		break;
//	case EventType::EventFilerename:       ///< 文件重命名
//		s = "EventFilerename";
//		break;
//	case EventType::EventFileremove:       ///< 文件删除
//		s = "EventFileremove";
//		break;
//	case EventType::EventFilelist:         ///< 文件列表数据
//		s = "EventFilelist";
//		break;
//	case EventType::EventFileLock:         ///< 文件空间锁定
//		s = "EventFileLock";
//		break;
//	case EventType::EventFileUnlock:       ///< 文件空间解锁
//		s = "EventFileUnlock";
//		break;
//	case EventType::EventFileConfirmation: ///< 文件上传确认
//		s = "EventFileConfirmation";
//		break;
//	case EventType::EventFileDownload:     ///< 文件下载
//		s = "EventFileDownload";
//		break;
//	case EventType::EventSliceInfo:        ///< 切片文件信息
//		s = "EventSliceInfo";
//		break;
//	case EventType::EventSliceStatus:      ///< 切片文件状态
//		s = "EventSliceStatus";
//		break;
//	case EventType::EventModelInfo:        ///< 模型文件信息
//		s = "EventModelInfo";
//		break;
//	case EventType::EventLogin:            ///< 登录
//		s = "EventLogin";
//		break;
//	case EventType::EventLogout:           ///< 登出
//		s = "EventLogout";
//		break;
//	case EventType::EventCheckNick:        ///< 检查昵称
//		s = "EventCheckNick";
//		break;
//	case EventType::EventCAPTCHA:          ///< 获取验证码
//		s = "EventCAPTCHA";
//		break;
//	case EventType::EventResetPWD:         ///< 重置密码
//		s = "EventResetPWD";
//		break;
//	case EventType::EventRegister:         ///< 用户注册
//		s = "EventRegister";
//		break;
//	case EventType::EventPrinterList:      ///< 打印机列表
//		s = "EventPrinterList";
//		break;
//	case EventType::EventAddPrinter:       ///< 添加打印机
//		s = "EventAddPrinter";
//		break;
//	case EventType::EventDelPrinter:       ///< 删除打印机
//		s = "EventDelPrinter";
//		break;
//	case EventType::EventRenamePrinter:    ///< 重命名打印机
//		s = "EventRenamePrinter";
//		break;
//	case EventType::EventFeedback:         ///< 反馈
//		s = "EventFeedback";
//		break;
//	case EventType::EventCountries:        ///< 国家列表
//		s = "EventCountries";
//		break;
//	case EventType::EventMax:
//		s = "EventMax";
//		break;
//	default:
//		break;
//	}
//	s = s + " " + str;
//	textLog->SetLabel(s);
//	update();
//}

} // GUI
} // Slic3r

#undef ERRORDLG