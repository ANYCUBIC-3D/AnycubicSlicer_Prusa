#include "Universal.hpp"
#include "slic3r/GUI/Cloud/ACCloudLoginPrivate.hpp"
#include "slic3r/GUI/Cloud/ACCloudMachine.hpp"
#include "ACStateColor.hpp"

#include <wx/notebook.h>

namespace Slic3r { 
namespace GUI {

UniversalToolBar::UniversalToolBar(wxWindow* parent, MainFrame* main_frame) :
	wxPanel(parent, wxID_ANY),
	//buttonUser		{ new ACButton(this, "m", "", "", "", wxBORDER_NONE, wxSize(36, 36))},
	buttonUser		{ new ACButtonRoundUser(this, wxID_ANY, "m", wxDefaultPosition, wxSize(36, 36), wxBORDER_NONE) },
	buttonReturn	{ new ACButton(this, "", "icon-close_40-nor_black", "icon-close_40-hover", "icon-close_40-dis_black", wxBORDER_NONE, wxSize(48, 48)) },
	text			{ new wxStaticText(this, wxID_ANY, "morecpp")},
	p				{ main_frame },
	popup			{ new ACQuickResponseDialog(this, wxSize(440, 312)) }
{
	popup->Hide();
	//SetBackgroundColour(wxColour(235, 237, 240));

	buttonUser->SetMinSize(FromDIP(wxSize(36, 36)));
	/*buttonUser->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_LABEL);
	buttonUser->SetCornerRadius(18);
	buttonUser->SetPaddingSize(wxSize(0, 0));
	buttonUser->SetSpacing(4);
	
	ACStateColor bgColor1;
	bgColor1.append(AC_COLOR_BT_L1_BG_DIS,  ACStateColor::Disabled);
	bgColor1.append(wxColour(57, 134, 255), ACStateColor::Pressed);
	bgColor1.append(wxColour(57, 134, 255), ACStateColor::Hovered);
	bgColor1.append(wxColour(57, 134, 255), ACStateColor::Normal);
	buttonUser->SetBackgroundColor(bgColor1);*/

	wxCursor cursor(wxCURSOR_HAND);
	buttonUser->SetCursor(cursor);
	buttonReturn->SetCursor(cursor);
	buttonReturn->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_ICON);
	buttonReturn->SetCornerRadius(24);
	buttonReturn->SetPaddingSize(wxSize(0, 0));
	//button->SetBackgroundColor(ACStateColor(wxColour(255, 255, 255)));
	buttonReturn->SetMinSize(wxSize(48, 48));//不需要使用FromDIP

	ACStateColor bgColor;
	bgColor.append(AC_COLOR_BT_L1_BG_DIS, ACStateColor::Disabled);
	bgColor.append(wxColour(229, 46, 46), ACStateColor::Pressed);
	bgColor.append(wxColour(236, 91, 86), ACStateColor::Hovered);
	bgColor.append(*wxWHITE, ACStateColor::Normal);
	buttonReturn->SetBackgroundColor(bgColor);

	text->SetFont(ACLabel::sysFont(14, false));
	buttonUser->SetFont(ACLabel::sysFont(16, false));
	

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);// (wxVERTICAL);
	sizer->AddSpacer(FromDIP(40));
	sizer->Add(buttonUser, 0, wxALIGN_CENTER | wxALL, 0);
	sizer->AddSpacer(FromDIP(6));
	sizer->Add(text, 0, wxALIGN_CENTER | wxALL, 0);
	sizer->AddStretchSpacer(1);
	sizer->Add(buttonReturn, 0, wxALIGN_CENTER | wxALL, FromDIP(18));
	sizer->AddSpacer(FromDIP(30));
	SetSizerAndFit(sizer);
	Layout();
	Refresh();

	buttonReturn->Bind(wxEVT_BUTTON, &UniversalToolBar::OnButtonReturn, this);
	buttonUser  ->Bind(wxEVT_BUTTON, &UniversalToolBar::OnButtonUser, this);
}

UniversalToolBar::~UniversalToolBar()
{
	wxDELETE(buttonUser);
	wxDELETE(buttonReturn);
	wxDELETE(text);
	wxDELETE(popup);
	p = nullptr;
}
void UniversalToolBar::OnButtonReturn(wxCommandEvent& event)
{
	//wxLogMessage("HELLO WORLD");
	if (p)
		p->select_main_panel(0);
}
void UniversalToolBar::OnButtonUser(wxCommandEvent& event)
{
	/**显示 二维码界面**/
	popup->SetPosition(this->GetScreenPosition() + wxPoint(40, FromDIP(65)/*GetSize().GetHeight()*/));
	popup->Show();
}

Universal::Universal(wxWindow* parent, MainFrame* main_frame)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition/*, wxGetApp().get_min_size()*/),
	m_space_panel	{ new wxPanel(this) },
	m_tool_bar		{ new UniversalToolBar(this, main_frame) },
	m_cloud_machine	{ new ACCloudMachine(this, main_frame) }
{
	GUI::wxGetApp().set_cloud_machine(m_cloud_machine);//pass printer
	SetBackgroundColour(wxColour(235, 237, 240));
	m_space_panel->SetBackgroundColour(*wxWHITE);
	//new Notebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxTAB_TRAVERSAL | wxNB_NOPAGETHEME);
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	m_space_panel->SetMinSize(FromDIP(wxSize(100, 2)));
	m_tool_bar->SetMinSize(FromDIP(wxSize(100, 87)));
	sizer->Add(m_space_panel,   0, wxALIGN_CENTER | wxEXPAND | wxALL, 0);
	sizer->Add(m_tool_bar,      0, wxALIGN_CENTER | wxEXPAND | wxALL, 0);
	sizer->Add(m_cloud_machine, 1, wxALIGN_CENTER | wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, FromDIP(20));
	SetSizer(sizer);
	Layout();
	Refresh();
}
Universal::~Universal()
{
	wxDELETE(m_tool_bar);
	wxDELETE(m_book_ctrl);
	wxDELETE(m_cloud_machine);
}
void Universal::UpdatePrinterList()
{
	if (m_cloud_machine && m_cloud_machine->IsShown())//界面show的状态才执行
		m_cloud_machine->UpdatePrinterList();
}

} // namespace GUI
} // namespace Slic3r
