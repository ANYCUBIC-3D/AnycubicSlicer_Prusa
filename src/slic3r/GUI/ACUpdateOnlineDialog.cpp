#include "ACUpdateOnlineDialog.hpp"

#include "GUI.hpp"
#include "GUI_App.hpp"
#include "I18N.hpp"
#include "MainFrame.hpp"
#include "libslic3r/Utils.hpp"
#include "ACLabel.hpp"
#include "format.hpp"

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/statbmp.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/dcclient.h>
#include "libslic3r/format.hpp"
#include "format.hpp"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/cursor.h>

#include <thread>
#include <chrono>
#include "MsgDialog.hpp"

namespace Slic3r
{

namespace GUI
{

PrivateDrawBackground::PrivateDrawBackground(wxWindow* parent) : DPIPanelEX(parent,
	wxID_ANY,
	wxDefaultPosition,
	parent->GetClientSize(),
	wxNO_BORDER/*wxCLIP_CHILDREN*/),
	m_parent { parent }
{
	wxInitAllImageHandlers();
	std::string name = "airPlane";
	if (boost::filesystem::exists(name + ".png"))
		filePath = name + ".png";
	else
		filePath = Slic3r::var(name + ".png");
	//filePath = std::string(filePath.c_str());
	imageHigh = wxImage(from_u8(filePath), wxBITMAP_TYPE_PNG);

	image = imageHigh;
	image.Rescale(FromDIP(460), FromDIP(335), wxIMAGE_QUALITY_HIGH);
	bitmap = wxBitmap(image);

	//wxStaticBitmap temp(this, wxID_ANY, *get_bmp_bundle("airPlane", wxSize(60.0f * em, 42.0f * em)));
	//bitmap = temp.GetBitmap();

	Bind(wxEVT_PAINT,     &PrivateDrawBackground::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &PrivateDrawBackground::OnMouseLeftDown, this);
	Bind(wxEVT_LEFT_UP,   &PrivateDrawBackground::OnMouseLeftUp, this);
	Bind(wxEVT_MOTION,    &PrivateDrawBackground::OnMouseMove, this);
}

void PrivateDrawBackground::OnPaint(wxPaintEvent& event)
{
	image = imageHigh;
	image.Rescale(FromDIP(460), FromDIP(335), wxIMAGE_QUALITY_HIGH);
	bitmap = wxBitmap(image);
	wxAutoBufferedPaintDC paintDC(this);
	wxGraphicsContext* gc = wxGraphicsContext::Create(paintDC);
	gc->ClearRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight());
	if (gc)
	{
		gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
		//gc->SetBrush(wxBrush(wxColour(0, 0, 0, 0)));
		//gc->DrawRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight());
		gc->DrawBitmap(bitmap, 0, 0, bitmap.GetWidth(), bitmap.GetHeight());
		gc->DrawRoundedRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight(), 16);
		delete gc;
	}
}

void PrivateDrawBackground::OnMouseLeftDown(wxMouseEvent& event)
{
	if (event.GetY() < 60)
	{
		wxPoint mouse_pos = ::wxGetMousePosition();
		wxPoint frame_pos = m_parent->GetScreenPosition();
		m_delta = mouse_pos - frame_pos;

		CaptureMouse();
	}
	event.Skip();
}

void PrivateDrawBackground::OnMouseLeftUp(wxMouseEvent& event)
{
	wxPoint mouse_pos = ::wxGetMousePosition();
	if (HasCapture())
		ReleaseMouse();

	event.Skip();
}

void PrivateDrawBackground::OnMouseMove(wxMouseEvent& event)
{
	wxPoint mouse_pos = ::wxGetMousePosition();

	if (!HasCapture())
	{
		event.Skip();
		return;
	}

	if (event.Dragging() && event.LeftIsDown())
	{
		m_parent->Move(mouse_pos - m_delta);
	}
	event.Skip();
}
void PrivateDrawBackground::msw_rescale()
{
	SetMinSize(FromDIP(wxSize(460, 335)));
	Fit();
	Refresh();
}

ACUpdateOnlineDialog::ACUpdateOnlineDialog(wxWindow* parent) :
	DPIDialog(parent,
		wxID_ANY,
		wxString(""),
		wxDefaultPosition,
		wxSize(460, 335),
		wxFRAME_NO_TASKBAR | wxNO_BORDER /*| wxSTAY_ON_TOP*/),
		/*wxFRAME_NO_TASKBAR | wxFRAME_SHAPED*/
		/*wxBORDER_NONE*/
		/*wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER wxDEFAULT_FRAME_STYLE*/
	origin       { parent },
	panel        { new PrivateDrawBackground(this) },
	//panel         { new wxPanel(this) },
	text1        { new ACButton(panel, _L("Software Update")) },
	text2        { new ACButton(panel, " ")},
	checkBox     { new ACCheckBox(panel) },
	text3        { new ACButton(panel, _L("Automatically check the latest at startup"))},
	button       { new ACButton(panel, _L("Check For Updates"))},
	buttonClose  { new ACButton(panel, "", "icon-close_40-nor", "icon-close-hover-blue", "icon-close_40-nor", wxBORDER_NONE, wxSize(50, 50))},
	text4        { new ACButton(panel, _L("Check Up Your Network And Try Again."))},
	buttonWarning{ new ACButton(panel, "", "icon-warning-red","", "", wxBORDER_NONE, wxSize(14, 14))},
	sizer        { new wxBoxSizer(wxVERTICAL) },
	sizerMain    { new wxBoxSizer(wxVERTICAL) },
	hBox         { new wxBoxSizer(wxHORIZONTAL) },
	hBox1        { new wxBoxSizer(wxHORIZONTAL) },
	vBox         { new wxBoxSizer(wxVERTICAL) },
	animationCtrl{ nullptr },
	parent       { nullptr }
{
	Init();
	Connect();
}
ACUpdateOnlineDialog::~ACUpdateOnlineDialog()
{
	origin = nullptr;
	animationCtrl->Stop();
	wxDELETE(text1);
	wxDELETE(text2);
	wxDELETE(checkBox);
	wxDELETE(text3);
	wxDELETE(button);
	wxDELETE(buttonClose);
	wxDELETE(text4);
	wxDELETE(buttonWarning);
	//wxDELETE(hBox);
	//wxDELETE(sizer);
	wxDELETE(panel);
	//wxDELETE(animationCtrl);
}
void ACUpdateOnlineDialog::Connect()
{
	button->     Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnUpdateCheck(); });
	buttonClose->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnClose(); });
	checkBox->   Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) { OnCheckBox(); });
}
void ACUpdateOnlineDialog::Build()
{

}
void ACUpdateOnlineDialog::Init()
{
	SetMinSize(FromDIP(wxSize(460, 335)));
//#ifdef __WXMSW__
//	HWND hwnd = (HWND)GetHandle();
//	::SetWindowLong(hwnd, GWL_EXSTYLE, ::GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
//#endif
	//setMarkWindow(origin, this, true, true);//set mask
//	SetExtraStyle(this->GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);
    AddWindowDrakEdg(this);

	std::string filePath;
	std::string name = "loading_white";
	if (boost::filesystem::exists(name + ".gif"))
		filePath = name + ".gif";
	else
		filePath = Slic3r::var(name + ".gif");
	animationCtrl = new wxAnimationCtrl(button, wxID_ANY, wxAnimation(from_u8(filePath), wxANIMATION_TYPE_GIF), wxDefaultPosition, wxSize(22, 22)/*wxDefaultSize*/, wxBORDER_NONE| wxAC_NO_AUTORESIZE);
	animationCtrl->SetMinSize(wxSize(22, 22));
	animationCtrl->SetWindowVariant(wxWINDOW_VARIANT_NORMAL);
	animationCtrl->Play();
	
	
	//button->SetEnable(false);
	//button->SetEnable(true);
	text1->SetMinSize(wxSize(120, 18));
	text2->SetMinSize(wxSize(450, 21));
	text3->SetMinSize(wxSize(120, 15));
	button->SetMinSize(wxSize(200, 30));
	text4->SetMinSize(wxSize(209, 15));

	text1->SetButtonType        (ACButton::AC_BUTTON_TYPE::AC_BUTTON_LABEL);
	text2->SetButtonType        (ACButton::AC_BUTTON_TYPE::AC_BUTTON_LABEL);
	button->SetButtonType       (ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV0);
	buttonClose->SetButtonType  (ACButton::AC_BUTTON_TYPE::AC_BUTTON_ICON);
	text3->SetButtonType        (ACButton::AC_BUTTON_TYPE::AC_BUTTON_LABEL);
	text4->SetButtonType        (ACButton::AC_BUTTON_TYPE::AC_BUTTON_LABEL);
	buttonWarning->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_ICON);

	text1->SetPaddingSize(wxSize(0, 0));
	text2->SetPaddingSize(wxSize(0, 0));
	button->SetPaddingSize(wxSize(0, 0));
	text4->SetPaddingSize(wxSize(0, 0));
	buttonWarning->SetPaddingSize(wxSize(0, 0));
	text1->SetSpacing(0);
	text2->SetSpacing(0);
	button->SetSpacing(0);
	text4->SetSpacing(0);
	buttonWarning->SetSpacing(0);


	//buttonClose->SetPaddingSize(wxSize(0, 0));
	buttonWarning->SetMinSize(wxSize(14, 14));

	//layout gif
	vBox->AddStretchSpacer(1);
	vBox->Add(animationCtrl, 0, /*wxEXPAND | wxALL*/wxALIGN_CENTER | wxALL, 0);
	vBox->AddStretchSpacer(1);
	button->SetSizer(vBox, true);
	//reset Disable color
	ACStateColor bgColor;
	bgColor.append(AC_COLOR_BT_L0_BG_NOR, ACStateColor::Disabled);
	bgColor.append(AC_COLOR_BT_L0_BG_PRE, ACStateColor::Pressed);
	bgColor.append(AC_COLOR_BT_L0_BG_HOV, ACStateColor::Hovered);
	bgColor.append(AC_COLOR_BT_L0_BG_NOR, ACStateColor::Normal);
	button->SetBackgroundColor(bgColor);

	text1->SetCornerRadius(0);
	text2->SetCornerRadius(0);
	text4->SetCornerRadius(0);
	text3->SetCornerRadius(0);
	buttonWarning->SetCornerRadius(0);
	buttonWarning->SetBackgroundColor(ACStateColor(wxColour(255, 255, 255)));
	
	buttonClose->SetCornerRadius(0);
	buttonClose->SetPaddingSize(wxSize(0, 0));
	buttonClose->SetBackgroundColor(ACStateColor(wxColour(150, 191, 255)));

	
	//checkbox
	text3->SetSpacing(0);
	text3->SetPaddingSize(wxSize(0, 0));
	checkBox->SetMinSize(FromDIP(wxSize(16, 16)));
	checkBox->SetValue(true);//init set checked
	//warning
	//text4->SetMinSize(wxSize(500, 27));
	text4->SetTextColor(ACStateColor(wxColour(220, 0, 0)));
	hBox1->Add(buttonWarning, 0, wxALIGN_CENTER | wxTOP, 0);
	hBox1->AddSpacer(FromDIP(3));
	hBox1->Add(text4, 0, wxALIGN_CENTER | wxTOP, 0);
	text4->SetLabel("");
	/***********Hide**********/
	buttonWarning->Hide();

	//Set mouse cursor style
	wxCursor cursor(wxCURSOR_HAND);
	button->SetCursor(cursor);
	buttonClose->SetCursor(cursor);
	checkBox->SetCursor(cursor);
	hBox->AddSpacer(FromDIP(1));
	hBox->Add(checkBox, 0, wxALIGN_CENTER | wxALL, 0);
	hBox->AddSpacer(FromDIP(3));
	hBox->Add(text3, 0, wxALIGN_CENTER | wxALL, 0);
	hBox->AddSpacer(FromDIP(1));

	text1->SetTextColor(ACStateColor(wxColour(51, 51, 51)));
	//Layout
	wxFont fontBold = ACLabel::sysFont(14, true);
	wxFont font     = ACLabel::sysFont(13, false);
	
	text1->SetFont(fontBold);
	text2->SetFont(font);
	button->SetFont(font);
	text3->SetFont(font);
	text4->SetFont(font);

	panel->SetSize(GetClientSize());

	
	
	sizer->Add(buttonClose, 0, wxALIGN_RIGHT | wxALIGN_BOTTOM | wxALL, 0);
	sizer->AddStretchSpacer(1);
	//sizer->Add(0, 0, 1, wxEXPAND, 0);
	sizer->Add(text1,       0, wxALIGN_CENTER | wxALL, 0);
	sizer->AddSpacer(FromDIP(20));
	sizer->Add(text2,       0, wxALIGN_CENTER | wxALL, 0);
	sizer->AddSpacer(FromDIP(6));
	sizer->Add(hBox,        0, wxALIGN_CENTER | wxALL, 0);
	sizer->AddSpacer(FromDIP(21));
	sizer->Add(button,      0, wxALIGN_CENTER | wxALL, 0);
	sizer->AddSpacer(FromDIP(4));
	sizer->Add(hBox1,       0, wxALIGN_CENTER | wxALL, 0);
	sizer->AddSpacer(FromDIP(11));
	
	panel->SetSizer(sizer);
    
	//Hide
	//SetHideWarning(false);
	SetWaitGIF(false);

	//Layout
	sizerMain->Add(panel, 1, wxEXPAND | wxALL, 1);
	SetSizerAndFit(sizerMain);

	SetSize(FromDIP(wxSize(460, 335)));
	Layout();
}

void ACUpdateOnlineDialog::OnUpdateCheckEvent() 
{
    button->SetLabel("");
    animationCtrl->Show();
    SetWaitGIF(true);
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    if (parent)
        parent->StartCheckForUpdates();
}

void ACUpdateOnlineDialog::OnClose()
{
    this->EndModal(wxID_NO);
	//Close();
}
void ACUpdateOnlineDialog::OnUpdateCheck()
{
    //this->EndModal(wxID_YES);
	button->SetLabel("");
	animationCtrl->Show();
	SetWaitGIF(true);

	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	if (parent)
		parent->StartCheckForUpdates();
}
void ACUpdateOnlineDialog::OnCheckBox()
{
	if (parent)
	{
		checkBox->update();
		parent->SetCheckEnableState(checkBox->isChecked());
	}
}
void ACUpdateOnlineDialog::SetWaitGIF(bool isEnable)
{
	if (animationCtrl)
		animationCtrl->Show(isEnable);
	
	button->Layout();
	button->Fit();
	button->SetEnable(!isEnable);
}
void ACUpdateOnlineDialog::msw_rescale()
{
	animationCtrl->SetMinSize(wxSize(22, 22));
	animationCtrl->SetSize(wxSize(22, 22));
	checkBox->SetMinSize(FromDIP(wxSize(16, 16)));
    checkBox->Rescale();
    checkBox->Layout();
	//button->SetMinSize(wxSize(20.0f * em, 3.2f * em));
	//button->SetMaxSize(wxSize(20.0f * em, 3.2f * em));
	SetMinSize(FromDIP(wxSize(460, 335)));
	Fit();
	Layout();
	Refresh();
}

ACInstallWizardDialog::ACInstallWizardDialog(wxWindow* parent) : DPIDialog(parent,
	wxID_ANY,
	L"",
	wxDefaultPosition,
	wxSize(590, 380),
	/*wxDEFAULT_DIALOG_STYLE | wxMINIMIZE_BOX | wxRESIZE_BORDER |*/wxFRAME_NO_TASKBAR | wxNO_BORDER /*| wxSTAY_ON_TOP*/),
	origin         { parent },
	progressBar    { new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL) },
	//logoBitmap     { new wxStaticBitmap(this, wxID_ANY, *get_bmp_bundle("logo_64px", 3.2f * em)) },
	logoBitmap     { new ACButton(this, "", "logo_128px", "logo_128px", "logo_128px", wxNO_BORDER, wxSize(42, 42)) },
	text           { new wxStaticText(this, wxID_ANY, format_wxstr(_L("Installing Updates For %1%"), wxGetApp().appName())) },
	installingText { new wxStaticText(this, wxID_ANY, _L("Installing Updates")) },
	progressText   { new wxStaticText(this, wxID_ANY, "")},
	buttonCancel   { new ACButton(this, _L("Cancel")) },
	//buttonIcon     { new ACButton(this, "", "software_minimization-nor", "software_minimization-hover", "software_minimization-nor", wxNO_BORDER, wxSize(26, 26)) },
	buttonMax      { new ACButton(this, "", "software_window-nor", "software_window-hover", "software_window-hover", wxNO_BORDER, wxSize(26, 26)) },
	buttonClose    { new ACButton(this, "", "software_close-nor", "software_close-hover-blue", "software_close-nor", wxNO_BORDER, wxSize(26, 26)) },
	vBox           { new wxBoxSizer(wxVERTICAL) },
	hBox           { new wxBoxSizer(wxHORIZONTAL) }
{
	Init();
	Connect();
}
ACInstallWizardDialog::~ACInstallWizardDialog()
{
	origin = nullptr;
	wxDELETE(progressBar);
	wxDELETE(logoBitmap);
	wxDELETE(text);
	wxDELETE(installingText);
	wxDELETE(progressText);
	wxDELETE(buttonCancel);
	//wxDELETE(buttonIcon);
	wxDELETE(buttonMax);
	wxDELETE(buttonClose);
}
void ACInstallWizardDialog::Build()
{

}
void ACInstallWizardDialog::Init()
{
	SetMinSize(FromDIP(wxSize(590, 380)));
//#ifdef __WXMSW__
//	HWND hwnd = (HWND)GetHandle();
//	::SetWindowLong(hwnd, GWL_EXSTYLE, ::GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
//#endif
	//setMarkWindow(origin, this, true, true);//set mask
//	SetExtraStyle(this->GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);
	logoBitmap->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_ICON);
	logoBitmap->SetPaddingSize(wxSize(0, 0));
	logoBitmap->SetSpacing(0);
	logoBitmap->SetCornerRadius(0);
    AddWindowDrakEdg(this);

	logoBitmap->SetMinSize(wxSize(42, 42));
	logoBitmap->SetBackgroundColor(AC_COLOR_WHITE);

	//close max ... button
	//buttonIcon-> SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_ICON);
	buttonMax->  SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_ICON);
	buttonClose->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_ICON);
	//buttonIcon-> SetPaddingSize(wxSize(0, 0));
	buttonMax->  SetPaddingSize(wxSize(0, 0));
	buttonClose->SetPaddingSize(wxSize(0, 0));
	buttonMax->SetEnable(false);

	wxFont fontBold = ACLabel::sysFont(14, true);
	wxFont font     = ACLabel::sysFont(14, false);
	text->SetFont(font);
    text->SetForegroundColour(AC_COLOR_BLACK);

	hBox->Add(logoBitmap, 0, wxTOP, FromDIP(20));
	hBox->AddSpacer(FromDIP(10));
	hBox->Add(text, 1, wxTOP, FromDIP(32));

	//hBox->Add(buttonIcon, 0, wxTOP, FromDIP(10));
	//hBox->AddSpacer(FromDIP(10));
	//hBox->Add(buttonMax, 0, wxTOP, FromDIP(10));
	buttonMax->Hide();
	hBox->AddSpacer(FromDIP(10));
	hBox->Add(buttonClose, 0, wxTOP | wxRIGHT, FromDIP(10));
	vBox->Add(hBox, 0, wxLEFT | wxEXPAND, FromDIP(20));



	

	
	vBox->AddSpacer(FromDIP(45));
	

	// add "Installing Updates" text
	installingText->SetFont(fontBold);
    installingText->SetForegroundColour(AC_COLOR_BLACK);
	//installingText->SetForegroundColour(wxColour(57, 134, 255));
	vBox->Add(installingText, 0, wxLEFT, FromDIP(20));
	vBox->AddSpacer(FromDIP(20));

	// add download progress and display text
	progressText->SetFont(ACLabel::sysFont(13, false));
    progressText->SetForegroundColour(AC_COLOR_BLACK);
	vBox->Add(progressText, 0, wxLEFT, FromDIP(20));
	progressBar->SetValue(30);
	progressBar->SetMinSize(FromDIP(wxSize(500, 18)));
	vBox->AddSpacer(FromDIP(10));
	vBox->Add(progressBar, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(20));
	//vBox->AddSpacer(FromDIP(140));
	vBox->AddStretchSpacer(1);

	//spring


	//set stylesheet
	buttonCancel->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV0);
	buttonCancel->SetMinSize(wxSize(110, 30));
	buttonCancel->SetCornerRadius(4);
	buttonCancel->SetPaddingSize(wxSize(0, 0));
	buttonCancel->SetBackgroundColor(ACStateColor(wxColour(244, 248, 254)));
	buttonCancel->SetTextColor(ACStateColor(wxColour(57, 134, 255)));
	buttonCancel->SetBorderColor(ACStateColor(wxColour(57, 134, 255)));
	buttonCancel->SetFont(font);
	wxCursor cursor(wxCURSOR_HAND);
	buttonCancel->SetCursor(cursor);
	//buttonIcon->SetCursor(cursor);
	buttonClose->SetCursor(cursor);
	
	cursor = wxCursor(wxCURSOR_NONE);
	buttonMax->SetCursor(cursor);
	// add cancel button
	vBox->Add(buttonCancel, 0, wxALIGN_RIGHT | wxRIGHT, FromDIP(20));
	vBox->AddSpacer(FromDIP(20));
	SetSizerAndFit(vBox);
	SetSize(FromDIP(wxSize(590, 380)));
	Layout();
}
void ACInstallWizardDialog::Connect()
{
	buttonCancel->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) {
        if (parent) {
            parent->GetOnlineForceUpgradeStyle() ? OnCancelWin() : OnCancel();
        }
    });
    buttonClose->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) {
        if (parent) {
            parent->GetOnlineForceUpgradeStyle() ? OnCancelWin() : OnCancel();
        }
    });
	//buttonIcon->  Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnIcon(); });
	//Bind(wxEVT_CLOSE_WINDOW, InstallWizardDialog::OnClose, this);

	Bind(wxEVT_LEFT_DOWN, &ACInstallWizardDialog::OnMouseLeftDown, this);
	Bind(wxEVT_LEFT_UP,   &ACInstallWizardDialog::OnMouseLeftUp, this);
	Bind(wxEVT_MOTION,    &ACInstallWizardDialog::OnMouseMove, this);
}

void ACInstallWizardDialog::OnCancelWin()
{

	RichMessageDialog dialog(this, _L("This version cannot be used without updating. Do you want to continue?"), _L("Update Notification"), wxYES_NO);
#ifdef _WIN32
    dialog.SetYesBtnLabel(_L("OK"));
    dialog.SetNoBtnLabel(_L("Cancel"));
#else
    dialog.SetYesNoLabels(_L("OK"), _L("Cancel"));
#endif
    dialog.Bind(wxEVT_CLOSE_WINDOW, [&dialog](wxCloseEvent &event) { dialog.EndModal(wxID_CANCEL); });
    m_dialog = &dialog;
    int result = m_dialog->ShowModal();
    m_dialog   = nullptr;
    if (result == wxID_YES)
        this->EndModal(result);

}

void ACInstallWizardDialog::OnCancel()
{
	//Hide();
    this->EndModal(wxID_NO);
	//wxGetApp().sendDownLoadFinishEvent();
}
void ACInstallWizardDialog::OnIcon()
{
	if (!IsIconized())
		Iconize(true);
}
void ACInstallWizardDialog::SetProgressValue(int value)
{
	progressBar->SetValue(value);
}
void ACInstallWizardDialog::OnMouseLeftDown(wxMouseEvent& event)
{
	if (event.GetY() < 108)
	{
		wxPoint mouse_pos = ::wxGetMousePosition();
		wxPoint frame_pos = this->GetScreenPosition();
		m_delta = mouse_pos - frame_pos;

		CaptureMouse();
	}
	event.Skip();
}

void ACInstallWizardDialog::OnMouseLeftUp(wxMouseEvent& event)
{
	wxPoint mouse_pos = ::wxGetMousePosition();
	if (HasCapture())
		ReleaseMouse();

	event.Skip();
}

void ACInstallWizardDialog::OnMouseMove(wxMouseEvent& event)
{
	wxPoint mouse_pos = ::wxGetMousePosition();

	if (!HasCapture())
	{
		event.Skip();
		return;
	}

	if (event.Dragging() && event.LeftIsDown())
	{
		Move(mouse_pos - m_delta);
	}
	event.Skip();
}
void ACInstallWizardDialog::msw_rescale()
{
	SetMinSize(FromDIP(wxSize(590, 380)));
	Fit();
	Layout();
	Refresh();
}

ACUpdateOnlineInfoDialog::ACUpdateOnlineInfoDialog(wxWindow* parent) :
	DPIDialog(parent,
		wxID_ANY,
		L"",
		wxDefaultPosition,
		wxSize(460, 400),
		wxNO_BORDER /*| wxSTAY_ON_TOP*//*wxDEFAULT_DIALOG_STYLE | wxMINIMIZE_BOX | wxRESIZE_BORDER*/),
	origin        { parent },
	hBottomBox    { new wxBoxSizer(wxHORIZONTAL) },
	vBox          { new wxBoxSizer(wxVERTICAL) },
	vContext      { new wxBoxSizer(wxVERTICAL) },
	topBar        { new ACDialogTopbar1(this, "", 0, 84) },
	//logoBitmap    { new wxStaticBitmap(this, wxID_ANY, *get_bmp_bundle("logo_128px"/*logo_64px*/, 6.4f * em))},
	//logoBitmap    { new ACScaleBitmapHighQuality(this, wxID_ANY, std::string("logo_128px")) },
	onlineVersion { nullptr/*new wxStaticText(nullptr, wxID_ANY, "")*/ },
	textTime      { nullptr/*new wxStaticText(nullptr, wxID_ANY, _L(""))*/ },
	buttonLater   { new ACButton(this, _L("Later")) },
	buttonUpdate  { new ACButton(this, _L("Update")) },
	box           { new ACStaticBox(this) },
	text          { new wxTextCtrl(box, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_RICH | wxTE_MULTILINE | wxNO_BORDER | wxTE_READONLY) },
	parent        { nullptr }
{
	Init();
	Connect();
}
ACUpdateOnlineInfoDialog::~ACUpdateOnlineInfoDialog()
{
	wxDELETE(onlineVersion);
	wxDELETE(textTime);
	wxDELETE(topBar);
	wxDELETE(buttonLater);
	wxDELETE(buttonUpdate);
	wxDELETE(text);
	wxDELETE(box);

	//wxDELETE(vTopBox);
	//wxDELETE(hBox);
	//wxDELETE(vContext);
	//wxDELETE(vBox);
}
void ACUpdateOnlineInfoDialog::Build()
{

}
void ACUpdateOnlineInfoDialog::Init()
{
	SetMinSize(FromDIP(wxSize(460, 400)));
//#ifdef __WXMSW__
//	HWND hwnd = (HWND)GetHandle();
//	::SetWindowLong(hwnd, GWL_EXSTYLE, ::GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
//#endif
	//setMarkWindow(origin, this, true, true);//set mask
//	SetExtraStyle(this->GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);
    AddWindowDrakEdg(this);
	topBar->SetToolBarH(84);
	topBar->SetCustomBackgroundColour(wxColour(255, 255, 255));

	onlineVersion = new wxStaticText(box, wxID_ANY, "");
	textTime = new wxStaticText(box, wxID_ANY, "");
	//current and latest version
	wxFont fontBold = ACLabel::sysFont(13, true);
	wxFont font     = ACLabel::sysFont(13, false);

	onlineVersion->SetFont(fontBold);
    onlineVersion->SetForegroundColour(AC_COLOR_BLACK);
    textTime->SetFont(font);
    textTime->SetForegroundColour(AC_COLOR_BLACK);


	//button later set stylesheet 
	buttonLater->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV1);
	buttonLater->SetPaddingSize(wxSize(0, 0));
	buttonLater->SetMinSize(wxSize(110, 30));
	buttonLater->SetSpacing(0);
	buttonLater->SetCornerRadius(4);
	buttonLater->SetBackgroundColor(ACStateColor(wxColour(244, 248, 254)));
	buttonLater->SetTextColor(ACStateColor(wxColour(57, 134, 255)));
	buttonLater->SetBorderColor(ACStateColor(wxColour(57, 134, 255)));

	buttonLater->SetFont(font);

	//button update now set stylesheet 
	buttonUpdate->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV0);
	buttonUpdate->SetPaddingSize(wxSize(0, 0));
	buttonUpdate->SetMinSize(wxSize(110, 30));
	buttonUpdate->SetSpacing(0);
	buttonUpdate->SetCornerRadius(4);
	//buttonUpdateNow->SetBackgroundColor(ACStateColor(wxColour(255, 255, 255)));
	buttonUpdate->SetTextColor(ACStateColor(wxColour(255, 255, 255)));
	buttonUpdate->SetBorderColor(ACStateColor(wxColour(57, 134, 255)));

	buttonUpdate->SetFont(font);

	wxCursor cursor(wxCURSOR_HAND);
	buttonLater->SetCursor(cursor);
	buttonUpdate->SetCursor(cursor);

	hBottomBox->AddStretchSpacer(1);
	hBottomBox->Add(buttonLater, 0, wxLEFT | wxRIGHT | wxBOTTOM, 0);
	hBottomBox->AddSpacer(20);
	hBottomBox->Add(buttonUpdate, 0, wxLEFT | wxRIGHT | wxBOTTOM, 0);

	//vBox->Add(hBox, 0, wxEXPAND | wxALL, 20);
	text->SetMinSize(FromDIP(wxSize(400, 100)));
	text->SetFont(font);
	text->SetBackgroundColour(wxColour(240, 240, 240));
    text->SetForegroundColour(AC_COLOR_BLACK);

	onlineVersion->SetMinSize(FromDIP(wxSize(160, 16)));
	onlineVersion->SetBackgroundColour(wxColour(240, 240, 240));
	textTime->SetBackgroundColour(wxColour(240, 240, 240));
	textTime->SetMinSize(FromDIP(wxSize(160, 16)));
	vContext->AddSpacer(FromDIP(14));
	vContext->Add(onlineVersion, 0, wxLEFT, FromDIP(10));
	vContext->AddSpacer(FromDIP(6));
	vContext->Add(textTime, 0, wxLEFT, FromDIP(10));
	vContext->AddSpacer(FromDIP(10));
	vContext->Add(text, 1, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(10));
	text->Layout();
	
	box->SetCornerRadius(12, ACStaticBox::CornerRadiusType::CornerAll);
	box->SetMinSize(FromDIP(wxSize(420, 246)));
	SetContext("");
	box->SetSizer(vContext);
	box->SetBackgroundColor(ACStateColor(wxColour(240, 240, 240)));
	vBox->Add(topBar, 0, wxEXPAND | wxLEFT|wxTOP|wxRIGHT, 1);
	vBox->Add(box, 1, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(20));
	vBox->AddSpacer(FromDIP(20));
	vBox->Add(hBottomBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(20));

	SetSizerAndFit(vBox);

	SetSize(FromDIP(wxSize(460, 400)));
	Layout();
}
void ACUpdateOnlineInfoDialog::Connect()
{
    topBar->m_close_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) {
        if (parent) {
            parent->GetOnlineForceUpgradeStyle() ? OnCloseWin() : OnClose();
        }
    });
    buttonLater->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) {
        if (parent) {
            parent->GetOnlineForceUpgradeStyle() ? OnCloseWin() : OnClose();
        }
    });
	buttonUpdate->          Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnUpdate(); });
}

void ACUpdateOnlineInfoDialog::OnCloseWin() 
{
    RichMessageDialog dialog(this, _L("This version cannot be used without updating."), _L("Update Notification"), wxYES_NO);
#ifdef _WIN32
    dialog.SetYesBtnLabel(_L("Update Now"));
    dialog.SetNoBtnLabel(_L("Quit"));
#else
    dialog.SetYesNoLabels(_L("Update Now"), _L("Quit"));
#endif
    dialog.Bind(wxEVT_CLOSE_WINDOW, [&dialog](wxCloseEvent &event) { dialog.EndModal(wxID_CANCEL); });

    int       result = dialog.ShowModal();
	if (result == wxID_YES || result == wxID_NO)
        this->EndModal(result);
}

void ACUpdateOnlineInfoDialog::OnClose()
{
    this->EndModal(wxID_NO);
	/*Close();
    wxGetApp().sendThreadCancelEvent(true);*/
}
void ACUpdateOnlineInfoDialog::OnUpdate()
{
    this->EndModal(wxID_YES);
	/*if (parent)
	{
		std::string version = parent->GetOnlineVersion().to_string();
		wxGetApp().sendDownLoadCanceEvent(version);
	}*/
}
void ACUpdateOnlineInfoDialog::SetContext(const wxString& context)
{
	//text->SetLabelText(context);
	text->SetValue(context);
}
void ACUpdateOnlineInfoDialog::msw_rescale()
{
	
	SetMinSize(FromDIP(wxSize(460, 400)));
	Fit();
	Layout();
	Refresh();
}



ACHint1Dialog::ACHint1Dialog(wxWindow* parent) :
	DPIDialog(parent,
		wxID_ANY,
		L"",
		wxDefaultPosition,
		wxSize(460, 200),
		wxCLIP_CHILDREN | wxNO_BORDER /*| wxSTAY_ON_TOP*//*wxDEFAULT_DIALOG_STYLE | wxMINIMIZE_BOX | wxRESIZE_BORDER*/),
	origin         { parent },
	topbar         { new ACDialogTopbar(this, _L("Software Update"), 40) },
	hBox1          { new wxBoxSizer(wxHORIZONTAL) },
	hBox2          { new wxBoxSizer(wxHORIZONTAL) },
	vBox1          { new wxBoxSizer(wxVERTICAL) },
	vBox2          { new wxBoxSizer(wxVERTICAL)},
	buttonCancel   { new ACButton(this, _L("Cancel")) },
	buttonUpdateNow{ new ACButton(this, _L("Install Now")) },
	//logoBitmap     { new wxStaticBitmap(this, wxID_ANY, *get_bmp_bundle("logo_128px"/*logo_64px*/, 64))},
	logoBitmap     { new ACButton(this, "", "logo_128px", "logo_128px", "logo_128px", wxNO_BORDER, wxSize(42, 42)) },
	textTop        { new wxStaticText(this, wxID_ANY, _L("Prepare to install the update")) },
	textBottom     { new wxStaticText(this, wxID_ANY, _L("Update content is download. Do you want to install now?")) }
{
	Init();
	Connect();
}
ACHint1Dialog::~ACHint1Dialog()
{
	origin = nullptr;
	wxDELETE(topbar);
	wxDELETE(buttonCancel);
	wxDELETE(buttonUpdateNow);
	wxDELETE(textBottom);
	wxDELETE(logoBitmap);
}
void ACHint1Dialog::Build()
{

}
void ACHint1Dialog::Init()
{
//#ifdef __WXMSW__
//	HWND hwnd = (HWND)GetHandle();
//	::SetWindowLong(hwnd, GWL_EXSTYLE, ::GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
//#endif
	SetMinSize(FromDIP(wxSize(460, 200)));
	//setMarkWindow(origin, this, true, true);//set mask
//	SetExtraStyle(this->GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);
    AddWindowDrakEdg(this);
	logoBitmap->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_ICON);
	logoBitmap->SetPaddingSize(wxSize(0, 0));
	logoBitmap->SetSpacing(0);
	logoBitmap->SetCornerRadius(0);
	logoBitmap->SetMinSize(wxSize(42, 42));
	logoBitmap->SetBackgroundColor(AC_COLOR_WHITE);

	hBox1->AddSpacer(FromDIP(10));
	hBox1->Add(logoBitmap, 0, wxTOP, 4);
	hBox1->AddSpacer(FromDIP(10));

	wxFont fontBold = ACLabel::sysFont(14, true);
	wxFont font     = ACLabel::sysFont(13, false);
	textTop->SetFont(fontBold);
    textTop->SetForegroundColour(AC_COLOR_BLACK);
	vBox1->Add(textTop, 0, wxALL, 0);

	textBottom->SetFont(font);
    textBottom->SetForegroundColour(AC_COLOR_BLACK);
	vBox1->AddSpacer(FromDIP(10));
	vBox1->Add(textBottom, 0, wxALL, 0);

	hBox1->Add(vBox1, 0, wxEXPAND | wxRIGHT, FromDIP(18));

	topbar->GetTextPtr()->SetFont(fontBold);
	//topbar->GetTextPtr()->SetTextColor(ACStateColor(wxColour(255, 255, 255)));
	topbar->GetTextPtr()->SetPaddingSize(wxSize(0, 0));
	topbar->GetButtonPtr()->SetPaddingSize(wxSize(0, 0));
	topbar->GetButtonPtr()->SetMinSize(wxSize(36, 36));
	topbar->GetButtonPtr()->SetSpacing(0);

	//topbar->GetTextPtr()->SetBackgroundColor(ACStateColor(wxColour(57, 134, 255)));
	//topbar->GetButtonPtr()->SetBackgroundColor(ACStateColor(wxColour(57, 134, 255)));
	topbar->LayoutLeft();
	//topbar->SetBackgroundColour(wxColour(57, 134, 255));
	vBox2->Add(topbar, 0, wxEXPAND | wxALL, 1);
	vBox2->AddSpacer(FromDIP(20));
	vBox2->Add(hBox1, 0, wxEXPAND | wxALL, 1);


	//button cancel set stylesheet 
	buttonCancel->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV1);
	buttonCancel->SetPaddingSize(wxSize(0, 0));
	buttonCancel->SetMinSize(wxSize(110, 30));
	buttonCancel->SetSpacing(0);
	buttonCancel->SetCornerRadius(4);
	buttonCancel->SetBackgroundColor(ACStateColor(wxColour(244, 248, 254)));
	buttonCancel->SetTextColor(ACStateColor(wxColour(57, 134, 255)));
	buttonCancel->SetBorderColor(ACStateColor(wxColour(57, 134, 255)));
	buttonCancel->SetFont(font);

	//button update now set stylesheet 
	buttonUpdateNow->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV0);
	buttonUpdateNow->SetPaddingSize(wxSize(0, 0));
	buttonUpdateNow->SetMinSize(wxSize(110, 30));
	buttonUpdateNow->SetSpacing(0);
	buttonUpdateNow->SetCornerRadius(4);
	//buttonUpdateNow->SetBackgroundColor(ACStateColor(wxColour(255, 255, 255)));
	buttonUpdateNow->SetTextColor(ACStateColor(wxColour(255, 255, 255)));
	buttonUpdateNow->SetBorderColor(ACStateColor(wxColour(57, 134, 255)));
	buttonUpdateNow->SetFont(font);

	wxCursor cursor(wxCURSOR_HAND);
	buttonCancel->SetCursor(cursor);
	buttonUpdateNow->SetCursor(cursor);

	hBox2->AddStretchSpacer(1);
	hBox2->Add(buttonCancel, 0, wxALL, 0);
	hBox2->AddSpacer(FromDIP(20));
	hBox2->Add(buttonUpdateNow, 0, wxALL, 0);
	hBox2->AddSpacer(FromDIP(20));
	
	vBox2->AddStretchSpacer(1);
	vBox2->Add(hBox2, 0, wxEXPAND|wxALL, 1);
	vBox2->AddSpacer(FromDIP(20));

	SetBackgroundColour(AC_COLOR_WHITE);
	SetSizerAndFit(vBox2);
	SetSize(FromDIP(wxSize(460, 200)));
	Layout();
}
void ACHint1Dialog::Connect()
{
	buttonCancel->          Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnClose(); });
	buttonUpdateNow->       Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnUpdateNow(); });
	topbar->GetButtonPtr()->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnClose(); });
}

void ACHint1Dialog::OnUpdateNow()
{
    this->EndModal(wxID_YES);
	//wxGetApp().sendUpdateNowEvent();
}
void ACHint1Dialog::OnClose()
{
    this->EndModal(wxID_NO);
	//Close();
}
void ACHint1Dialog::msw_rescale()
{
	SetMinSize(FromDIP(wxSize(460, 200)));
	Fit();
	Layout();
	Refresh();
}



Hint2Dialog::Hint2Dialog(wxWindow* parent) :
	DPIDialog(parent,
		wxID_ANY,
		L"",
		wxDefaultPosition,
		wxSize(460, 200),
		wxNO_BORDER | wxSTAY_ON_TOP/*wxDEFAULT_DIALOG_STYLE | wxMINIMIZE_BOX | wxRESIZE_BORDER*/),
	topbar        { new ACDialogTopbar(this, "AnycubicSlicer", 40) },
	hBox1         { new wxBoxSizer(wxHORIZONTAL) },
	hBox2         { new wxBoxSizer(wxHORIZONTAL) },
	vBox          { new wxBoxSizer(wxVERTICAL) },
	buttonCancel  { new ACButton(this, _L("Cancel")) },
	buttonDiscard { new ACButton(this, _L("Discard")) },
	buttonSave    { new ACButton(this, _L("Save")) },
	logoBitmap    { new wxStaticBitmap(this, wxID_ANY, *get_bmp_bundle("logo_128px"/*logo_64px*/, 64))},
	text          { new wxStaticText(this, wxID_ANY, "")}
	//_L("Off Mission Anycubic Slicer. The current project has been modified. Do you want me to change and save it to \"Hello AC Men\"")

{
	/** get_bmp_bundle(wxGetApp().logo_name()*/
	Init();
	Connect();
}
Hint2Dialog::~Hint2Dialog()
{
	wxDELETE(topbar);
	wxDELETE(buttonCancel);
	wxDELETE(buttonDiscard);
	wxDELETE(buttonSave);
	wxDELETE(logoBitmap);
}
void Hint2Dialog::Init()
{
	hBox1->Add(logoBitmap, 0, wxLEFT | wxRIGHT | wxTOP, 10);

	wxFont font = text->GetFont();
	font.SetPointSize(12);
	text->SetFont(font);
	text->Wrap(400);
	hBox1->Add(text, 0, wxLEFT | wxRIGHT | wxTOP, 10);

	font.SetPointSize(16);
	topbar->GetTextPtr()->SetFont(font);
	topbar->GetTextPtr()->SetTextColor(ACStateColor(wxColour(255, 255, 255)));
	topbar->GetTextPtr()->SetPaddingSize(wxSize(0, 0));
	topbar->GetButtonPtr()->SetPaddingSize(wxSize(0, 0));
	topbar->GetButtonPtr()->SetMinSize(wxSize(36, 36));
	topbar->GetButtonPtr()->SetSpacing(0);
	topbar->GetButtonPtr()->SetMaxClientSize(wxSize(36, 36));
	topbar->GetButtonPtr()->SetClientSize(24, 24);

	topbar->GetTextPtr()->SetBackgroundColor(ACStateColor(wxColour(57, 134, 255)));
	topbar->GetButtonPtr()->SetBackgroundColor(ACStateColor(wxColour(57, 134, 255)));
	topbar->LayoutLeft();
	topbar->SetBackgroundColour(wxColour(57, 134, 255));
	vBox->Add(topbar, 0, wxEXPAND);
	vBox->Add(hBox1, 0, wxEXPAND | wxALL, 10);


	//button cancel set stylesheet 
	buttonCancel->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV1);
	buttonCancel->SetPaddingSize(wxSize(0, 0));
	buttonCancel->SetMinSize(wxSize(110, 30));
	buttonCancel->SetMaxSize(wxSize(110, 30));
	buttonCancel->SetSpacing(0);
	buttonCancel->SetCornerRadius(4);
	buttonCancel->SetClientSize(wxSize(110, 30));
	buttonCancel->SetBackgroundColor(ACStateColor(wxColour(244, 248, 254)));
	buttonCancel->SetTextColor(ACStateColor(wxColour(57, 134, 255)));
	buttonCancel->SetBorderColor(ACStateColor(wxColour(57, 134, 255)));
	font.SetPointSize(11);
	buttonCancel->SetFont(font);

	//button discard set stylesheet 
	buttonDiscard->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV1);
	buttonDiscard->SetPaddingSize(wxSize(0, 0));
	buttonDiscard->SetMinSize(wxSize(110, 30));
	buttonDiscard->SetMaxSize(wxSize(110, 30));
	buttonDiscard->SetSpacing(0);
	buttonDiscard->SetCornerRadius(4);
	buttonDiscard->SetClientSize(wxSize(110, 30));
	buttonDiscard->SetBackgroundColor(ACStateColor(wxColour(244, 248, 254)));
	buttonDiscard->SetTextColor(ACStateColor(wxColour(57, 134, 255)));
	buttonDiscard->SetBorderColor(ACStateColor(wxColour(57, 134, 255)));
	buttonDiscard->SetFont(font);

	//button save set stylesheet 
	buttonSave->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LV0);
	buttonSave->SetPaddingSize(wxSize(0, 0));
	buttonSave->SetMinSize(wxSize(110, 30));
	buttonSave->SetMaxSize(wxSize(110, 30));
	buttonSave->SetSpacing(0);
	buttonSave->SetCornerRadius(4);
	buttonSave->SetClientSize(wxSize(110, 30));
	buttonSave->SetTextColor(ACStateColor(wxColour(255, 255, 255)));
	buttonSave->SetBorderColor(ACStateColor(wxColour(57, 134, 255)));
	buttonSave->SetFont(font);

	wxCursor cursor(wxCURSOR_HAND);
	buttonCancel->SetCursor(cursor);
	buttonDiscard->SetCursor(cursor);
	buttonSave->SetCursor(cursor);

	vBox->AddSpacer(30);

	hBox2->AddStretchSpacer(1);
	hBox2->Add(buttonCancel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 0);
	hBox2->AddSpacer(20);
	hBox2->Add(buttonDiscard, 0, wxLEFT | wxRIGHT | wxBOTTOM, 0);
	hBox2->AddSpacer(20);
	hBox2->Add(buttonSave, 0, wxLEFT | wxRIGHT | wxBOTTOM, 0);
	hBox2->AddSpacer(20);

	vBox->Add(hBox2, 0, wxEXPAND, 0);
	vBox->AddSpacer(20);

	SetBackgroundColour(AC_COLOR_WHITE);
	SetSizerAndFit(vBox);

}
void Hint2Dialog::Connect()
{
	buttonCancel-> Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnCancel(); });
	buttonDiscard->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnDiscard(); });
	buttonSave->   Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnSave(); });

	topbar->GetButtonPtr()->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnClose(); });
}
void Hint2Dialog::OnCancel()
{
	//Test ...
	int a = 1;
}
void Hint2Dialog::SetContext(const wxString& t)
{
	text->SetLabelText(t);
}
void Hint2Dialog::OnDiscard()
{
	//Test ...
	int a = 1;
}
void Hint2Dialog::OnSave()
{
	//Test ...
	int a = 1;
}
void Hint2Dialog::OnClose()
{
	//Test ...
	int a = 1;
	Close();
}



ACUpdateManger::ACUpdateManger(wxWindow* parent) :
	dlg         { new ACUpdateOnlineDialog(parent) },
	dlgInfo     { new ACUpdateOnlineInfoDialog(parent) },
	dlgProgress { new ACInstallWizardDialog(parent) },
	dlgFinish   { new ACHint1Dialog(parent) }
{
	m_app_config = get_app_config();

	GetNowSoftwareVersionInfo();

	dlg->SetACUpdateManger(this);
    dlgProgress->SetACUpdateManger(this);
	dlgInfo->SetACUpdateManger(this);
}
void ACUpdateManger::SetOriginParent(wxWindow* p)
{
	dlg->SetParent(p);
	dlgInfo->SetParent(p);
	dlgProgress->SetParent(p);
	dlgFinish->SetParent(p);
}
void ACUpdateManger::SetUpdateOnlineDialog()
{

    wxString softwareVersion(format_wxstr(wxGetApp().buildID()));
	//wxString softwareVersion = from_u8((boost::format(_u8L("AnycubicSlicer_%s")) % GetNowVersionString()).str());
	dlg->SetText2(softwareVersion);
    dlg->SetText4("");
	dlg->SetCheckBoxChecked(GetCheckEnableState());
}
void ACUpdateManger::SetUpdateOnlineInfoDialog()
{
	//dlgInfo->Freeze();

	wxString nowSoftwareVersion = from_u8((boost::format(_u8L("Current version: %s")) % GetNowVersionString()).str());
	dlgInfo->SetCurrentVersion(nowSoftwareVersion);

	wxString onLineSoftwareVersion = format_wxstr(wxGetApp().appName()) + ": " + wxString(GetOnlineVersionString().to_string());
	dlgInfo->SetLatestVersion(onLineSoftwareVersion);

	wxString onLineSoftwareVersionUpdate = from_u8((boost::format(_u8L("V%s update")) % GetOnlineVersionString()).str());
	dlgInfo->SetOnlineVersion(onLineSoftwareVersionUpdate);//

	wxString onLineSoftwareVesionTime = GetOnlineVesionTime();
	dlgInfo->SetTimeText(onLineSoftwareVesionTime);

	const int& em = dlgInfo->em_unit();
	
	wxString context = "";
	std::vector<wxString> info;
    auto                  language = GetNowSoftwareLanguageInfo();
	if (language.empty() || language == "en")
		info = GetImproveAndBeginInfoList_EN();
	else
		info = GetImproveAndBeginInfoList_ZH();

	for (wxString improveInfo : info)
		context += improveInfo + "\n";
	dlgInfo->SetContext(context);
	dlgInfo->Refresh();
	
	//m_checkUpdateInfoDialog->msw_rescale();
}
void ACUpdateManger::SetInstallWizardDialog()
{
	SetDownloadProgressPercentage(0.0f);
}
void ACUpdateManger::SetHint1Dialog()
{

}




bool ACUpdateManger::StartCheckForUpdates()
{
	wxGetApp().app_version_check_public(true);
	return true;
}

std::string ACUpdateManger::GetNowSoftwareLanguageInfo()
{
    wxString language = m_app_config->get("translation_language");
    if (language.empty()) {
        const wxLanguage lang_system = wxLanguage(wxLocale::GetSystemLanguage());
        if (lang_system != wxLANGUAGE_UNKNOWN) {
            const wxLanguageInfo *m_language_info_system = wxLocale::GetLanguageInfo(lang_system);
            return m_language_info_system->Description.ToStdString();
        }
        return "";
    }
    return language.ToStdString();
}

void ACUpdateManger::GetNowSoftwareVersionInfo()
{
    /*if (m_app_config->orig_version().valid()) {
        m_now_version = m_app_config->orig_version();
    } else {
        m_now_version = *Semver::parse(SLIC3R_VERSION);
    }*/
    m_now_version = *Semver::parse(SLIC3R_VERSION);
}

bool ACUpdateManger::GetCheckEnableState()
{
	if (m_app_config->get("notify_release") != "none")
		return true;
	return false;
}
void ACUpdateManger::SetIniFileDownInfo(wxString info)
{
	if (dlg->IsShown())
		dlg->SetText4(info);
}
void ACUpdateManger::SetTexe4Color(bool red)
{
	if (dlg)//set color
		dlg->SetText4Color(red);
}
void ACUpdateManger::SetDownloadProgressPercentage(float percentage)
{

	int      softSize = GetOnlineSoftwareSize();
	wxString _nowSumSize = wxString::Format("%.2f MB of ", percentage > 0 ? percentage * softSize / 1024 / 1024 : 0.0f);
	wxString _fileSumSize = wxString::Format("%.2f MB,", softSize * 1.0f / 1024 / 1024);
	wxString _gap = wxString::Format("%d", percentage > 0 ? int(percentage * 100) : 0);

	wxString downLoadInfo = from_u8((boost::format(_u8L("Downloading(%s complete)")) % (_nowSumSize + _fileSumSize + _gap + "%")).str());
	dlgProgress->SetProgressText(downLoadInfo);

	dlgProgress->SetProgressValue(int(percentage * 100));
}

static bool isContainString(const wxString& str, const wxString& target) { return str.Contains(target); }

static std::vector<std::string> GetListInfoSelectInfo(std::vector<std::string>& info, wxString start_str, wxString end_str)
{
	std::vector<std::string> new_info;
	auto start_it = std::find(info.begin(), info.end(), start_str);
	auto end_it = std::find(info.begin(), info.end(), end_str);

	if (start_it != info.end() && end_it != info.end())
	{
		++start_it;
		while (start_it != end_it)
		{
			std::cout << *start_it << std::endl;
			new_info.push_back(*start_it);
			++start_it;
		}
	}
	return new_info;
}

static bool isContainBetween(const wxString& str, wxString target1, const wxString target2)
{
	int index1 = str.Find(target1);
	int index2 = str.Find(target2);
	if ((index1 != wxNOT_FOUND || index2 != wxNOT_FOUND))
	{
		return index1 > 0 ? index1 : index2;
	}
	return false;
}

static std::vector<wxString> findLinesContainBetween(std::vector<std::string>& lines, const wxString target1, const wxString target2)
{
	bool                  index = false;
	std::vector<wxString> result;
	std::vector<wxString> new_result;
	for (const std::string& line : lines)
	{
		int resultIndex = isContainBetween(line, target1, target2);
		if (resultIndex > 0 || index)
		{
			if (resultIndex > 0)
				index = !index;
			wxString wxstr(line.c_str(), wxConvUTF8);
			result.push_back(wxstr);
		}
	}
	if (result.size() > 0)
	{
		for (int i = 1; i <= result.size() - 2; ++i)
		{
			new_result.push_back(result[i]);
		}
	}
	return new_result;
}

static std::string getSubstringAfterChar(std::string& str, char target, int _index = 0)
{
	size_t index = str.find(target);
	if (index != std::string::npos)
	{
		return str.substr(index + 1 + _index);
	}
	return "";
}
bool ACUpdateManger::SetIniFileInfo(std::vector<std::string>& info, const std::string &downLoadUrl, bool showDialog)
{
#ifdef WIN32
	std::vector<std::string> software = GetListInfoSelectInfo(info, "[WIN64]", "[OSX]");
#else
	std::vector<std::string> software = GetListInfoSelectInfo(info, "[OSX]", "[INFO]");
#endif
	if (software.size() >= 4)
	{
		SetOnlineVersion(getSubstringAfterChar(software[0], '='));
		if (GetNowVersion() < GetOnlineVersion())
		{
            SetOnlineSoftwarePath(downLoadUrl + getSubstringAfterChar(software[1], '=', 1));
			SetOnlineSoftwareSize(getSubstringAfterChar(software[3], '='));
			SetnlineVesionTime(getSubstringAfterChar(software[2], '='));
			SetImproveAndBeginInfoList_EN(findLinesContainBetween(info, "begin_en", "end_en"));
			SetImproveAndBeginInfoList_ZH(findLinesContainBetween(info, "begin_zh", "end_zh"));
            if (software.size() > 4) {
                SetOnlineForceUpgradeStyle(getSubstringAfterChar(software[4], '='));
            }
            if (showDialog || (m_app_config->get("notify_release") != "none" && !showDialog) || m_online_ForceUpgrade)
			{
				//dlg->SetText4("");
				wxGetApp().sendUpdateDialogText4ValueEvent("");
				wxGetApp().sendShowUpdateDialogEvent();
			}
				//if (!showDialog) {
    //                while (true) {
    //                    bool          m_isShow = true;
    //                    wxWindowList &children = wxGetApp().mainframe->GetChildren();
    //                    for (wxWindowList::iterator it = children.begin(); it != children.end(); ++it) {
    //                        wxWindow *child = *it;
    //                        if (child->IsKindOf(wxClassInfo::FindClass(wxT("wxDialog")))) {
    //                            wxDialog *dialog = wxDynamicCast(child, wxDialog);
    //                            if (dialog && dialog->IsShown()) {
    //                                m_isShow = false;
    //                            }
    //                        }
    //                    }
    //                    if (m_isShow) {
    //                        wxGetApp().sendShowUpdateDialogEvent();
    //                        //ShowCheckUpdateInfoDialog();
    //                        break;
    //                    } else if (wxGetApp().mainframe->checkUpdate_dialog->dlgInfo->IsShown() ||
    //                               wxGetApp().mainframe->checkUpdate_dialog->dlgProgress->IsShown() ||
    //                               wxGetApp().mainframe->checkUpdate_dialog->dlgFinish->IsShown()) {
    //                        break;
    //                    }
    //                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    //                }
    //            }else { 
				//	wxGetApp().sendShowUpdateDialogEvent();
				//	//ShowCheckUpdateInfoDialog();
				//}
    //            
    //        }
			return true;
		}
	}
	//SetTexe4Color(false);
	//SetIniFileDownInfo(_L("AnycubicSlicer is up to date."));
	wxGetApp().sendUpdateDialogText4ColorEvent(wxString("green"));
	wxGetApp().sendUpdateDialogText4ValueEvent(format_wxstr(_L("%1% is up to date."),wxGetApp().appName()));
	return false;
}

void ACUpdateManger::SetCheckEnableState(bool index)
{
	m_app_config->set("notify_release", index ? "all" : "none");
}

void ACUpdateManger::CloseAllDialog()
{
	if (dlg->IsShown())
		dlg->Close();
	if (dlgInfo->IsShown())
		dlgInfo->Close();
    if (dlgProgress->IsShown()) 
        dlgProgress->Close();
	if (dlgFinish->IsShown())
		dlgFinish->Close();
}


void ACUpdateManger::SetWindowPosEvent(wxWindow *w) {
    int x = (wxGetApp().mainframe->GetSize().GetWidth() - w->GetSize().GetWidth()) / 2;
    int y = (wxGetApp().mainframe->GetSize().GetHeight() - w->GetSize().GetHeight()) / 2;
    w->SetPosition(wxGetApp().mainframe->GetPosition() + wxPoint(x, y));
}

void ACUpdateManger::ShowCheckVersionDialog()
{
	//CloseAllDialog();
	//if (!dlg->IsShown())
	//{
	//	SetUpdateOnlineDialog();
	//	//dlg->ShowModal();
	//	dlg->Raise();
	//	//dlg->Show();
	//	SetShowCenter(dlg);
	//}
    SetUpdateOnlineDialog();
    SetWindowPosEvent(dlg);
    if (dlg->ShowModal() == wxID_NO) {
        dlg->Close();
    }
}
void ACUpdateManger::ShowCheckUpdateInfoDialog()
{
	//CloseAllDialog();
	//if (!dlgInfo->IsShown())
	//{
	//	SetUpdateOnlineInfoDialog();
	//	//dlgInfo->ShowModal();
	//	dlgInfo->Raise();
	//	//dlgInfo->Show();
	//	SetShowCenter(dlgInfo);
	//}
    SetUpdateOnlineInfoDialog();
    SetWindowPosEvent(dlgInfo);
	if (dlgInfo->ShowModal() == wxID_YES) {
        std::string version = GetOnlineVersion().to_string();
        wxGetApp().sendDownLoadCanceEvent(version);
    } else {
        if (m_online_ForceUpgrade) {
                wxGetApp().mainframe->Close();
        } else {
            wxGetApp().sendThreadCancelEvent(true);
        }
	}

}
void ACUpdateManger::ShowCheckUpdateProgressDialog()
{
	//CloseAllDialog();
	//if (!dlgProgress->IsShown())
	//{
	//	SetInstallWizardDialog();
	//	//dlgProgress->ShowModal();
	//	dlgProgress->Raise();
	//	//dlgProgress->Show();
	//	SetShowCenter(dlgProgress);
	//}
    SetInstallWizardDialog();
    SetWindowPosEvent(dlgProgress);
    wxGetApp().mainframe->SetOnlineForceUpgradeStatc(m_online_ForceUpgrade);
    if (dlgProgress->ShowModal() == wxID_YES) {
        wxGetApp().sendDownLoadFinishEvent();
        wxGetApp().mainframe->Close();
    } else {
        wxGetApp().sendDownLoadFinishEvent();
    }
    
}
void ACUpdateManger::ShowCheckUpdateProgressFinishDialog()
{
	//CloseAllDialog();
	//if (!dlgFinish->IsShown())
	//{
	//	SetHint1Dialog();
	//	//dlgFinish->ShowModal();
	//	dlgFinish->Raise();
	//	//dlgFinish->Show();
	//	SetShowCenter(dlgFinish);
	//}
	if (dlgProgress && dlgProgress->IsShown()) {
        dlgProgress->EndModal(wxID_NO);
	}
    SetHint1Dialog();
    SetWindowPosEvent(dlgFinish);
    if (dlgFinish->ShowModal() == wxID_YES) {
        wxGetApp().sendUpdateNowEvent();
	} else {
        dlgFinish->Close();
    }
}

void ACUpdateManger::SetShowCenter(wxWindow* w)
{
	int x = (wxGetApp().mainframe->GetSize().GetWidth() - w->GetSize().GetWidth()) / 2;
	int y = (wxGetApp().mainframe->GetSize().GetHeight() - w->GetSize().GetHeight()) / 2;
	w->SetPosition(wxGetApp().mainframe->GetPosition() + wxPoint(x, y));
	w->Show();
}

RichMessageDialog* ACUpdateManger::ShowForceUpgradeDialog(wxString content, wxString title, wxString OKLable, wxString NOLable)
{
    RichMessageDialog dialog(NULL, _L(content), _L(title), wxYES_NO);
#ifdef _WIN32
    dialog.SetYesBtnLabel(_L(OKLable));
    dialog.SetNoBtnLabel(_L(NOLable));
#else
    dialog.SetYesNoLabels(_L(OKLable), _L(NOLable));
#endif
    dialog.Bind(wxEVT_CLOSE_WINDOW, [&dialog](wxCloseEvent &event) { dialog.EndModal(wxID_CANCEL); });

	return &dialog;
        

}


}
}



