#include "ACDialogTopbar1.hpp"
#include "wx/artprov.h"
#include "wx/aui/framemanager.h"
#include "I18N.hpp"
#include "GUI_App.hpp"
#include "GUI.hpp"
#include "wxExtensions.hpp"
#include "Plater.hpp"
#include "MainFrame.hpp"
#include "wx/dc.h"
#include "wx/dcgraph.h"


namespace Slic3r
{

namespace GUI
{

ACDialogTopbar1::ACDialogTopbar1(wxWindow* parent, const wxString& title, int toolbarW, int toolbarH)
	: wxWindow(parent, ID_DIALOG_TOP_BAR, wxDefaultPosition, wxDefaultSize),
	m_frame			{ nullptr },
	m_title			{ title },
	m_toolbar_w		{ toolbarW },
	m_toolbar_h		{ toolbarH },
	m_mainSizer		{ nullptr },
	m_v_Sizer		{ nullptr },
	m_title_item	{ nullptr },
	m_subtitle_item	{ nullptr },
	m_close_button	{ nullptr },
	m_point			{ 0, 0 }
{
	Init(parent);
}

ACDialogTopbar1::~ACDialogTopbar1()
{
}

void ACDialogTopbar1::Init(wxWindow* parent)
{
	m_frame = parent;
	SetMinSize(wxSize(m_toolbar_w, m_toolbar_h));

	//SetBackgroundColour(AC_COLOR_LIGHTBLUE);

	m_title_item = new ACButton(this, m_title, "", "", "", wxNO_BORDER);
	m_title_item->SetCanFocus(false);
	m_title_item->SetEnable(false);
	m_title_item->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
	m_title_item->SetPaddingSize(wxSize(0, 0));

	wxFont fontBold = ACLabel::sysFont(14, true);
	wxFont font     = ACLabel::sysFont(14, false);
	m_title_item->SetFont(fontBold);

	m_subtitle_item = new ACButton(this, m_title, "", "", "", wxNO_BORDER);
	m_subtitle_item->SetCanFocus(false);
	m_subtitle_item->SetEnable(false);
	m_subtitle_item->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
	m_subtitle_item->SetPaddingSize(wxSize(0, 0));

	font = m_subtitle_item->GetFont();
	m_subtitle_item->SetFont(font);
	

	//wxSize titleTextSize = m_title_item->GetTextSize();

	//wxBitmap close_bitmap = create_scaled_bitmap("software_close-nor", nullptr, TOPBAR_ICON_SIZE);
	//wxBitmap close_bitmap_hover = create_scaled_bitmap("software_close-hover", nullptr, TOPBAR_ICON_SIZE);
	wxString btIconNameCloseNor = "icon-close_40-nor";
	wxString btIconNameCloseHover = "software_close-hover";

	m_close_button = { new ACButton(this, "", "icon-close_40-nor_black", "icon-close-hover-blue", "icon-close_40-nor_black", wxBORDER_NONE, wxSize(50, 50)) },
	m_close_button->SetPaddingSize(wxSize((0), (0)));
	m_close_button->SetButtonType(ACButton::AC_BUTTON_ICON);

	m_logo_button = new ACButton(this, "", "logo_128px", "logo_128px", "logo_128px", wxBORDER_NONE, wxSize(42, 42));
	m_logo_button->SetButtonType(ACButton::AC_BUTTON_TYPE::AC_BUTTON_ICON);
	m_logo_button->SetPaddingSize(wxSize(0, 0));
	m_logo_button->SetSpacing(0);
	m_logo_button->SetCornerRadius(0);

	wxCursor cursor(wxCURSOR_HAND);
	m_close_button->SetCursor(cursor);

	m_v_Sizer = new wxBoxSizer(wxVERTICAL);
	m_v_Sizer->AddSpacer(FromDIP(20));
	m_v_Sizer->Add(m_title_item, 0, wxALIGN_LEFT | wxALL, 0);
    m_v_Sizer->Add(m_subtitle_item, 0, wxALIGN_LEFT | wxTOP, FromDIP(6));

	m_mainSizer = new wxBoxSizer(wxHORIZONTAL);
    m_mainSizer->Add(m_logo_button, 0, wxEXPAND | wxTop|wxLEFT, FromDIP(20));
	m_mainSizer->AddSpacer(FromDIP(10));
	m_mainSizer->Add(m_v_Sizer, 1, wxALL, 0);
	m_mainSizer->Add(m_close_button, 0, wxALL, 0);

	SetSizer(m_mainSizer);

	SetSize(FromDIP(wxSize(m_toolbar_w, m_toolbar_h)));

	Layout();


	//Bind(wxEVT_BUTTON, &ACDialogTopbar1::OnClose, this, m_close_button->GetId());

	//m_title_item->Bind(wxEVT_LEFT_DOWN, &ACDialogTopbar1::OnMouseLeftDown, this);
	//m_title_item->Bind(wxEVT_LEFT_UP,   &ACDialogTopbar1::OnMouseLeftUp,   this);
	//m_title_item->Bind(wxEVT_MOTION,    &ACDialogTopbar1::OnMouseMotion,   this);

	Bind(wxEVT_LEFT_DOWN, &ACDialogTopbar1::OnMouseLeftDown, this);
	Bind(wxEVT_LEFT_UP,   &ACDialogTopbar1::OnMouseLeftUp,   this);
	Bind(wxEVT_MOTION,    &ACDialogTopbar1::OnMouseMotion,   this);
}

void ACDialogTopbar1::SetToolBarH(int h)
{
	m_toolbar_h = h;
	SetMinSize(FromDIP(wxSize(m_toolbar_w, m_toolbar_h)));
}

void ACDialogTopbar1::msw_rescale()
{
	SetMinSize(FromDIP(wxSize(m_toolbar_w, m_toolbar_h)));
	//SetSize(FromDIP(wxSize(m_toolbar_w, m_toolbar_h)));
	m_mainSizer->Layout();
	m_v_Sizer->Layout();
	Fit();
	Layout();
	Refresh();
}

void ACDialogTopbar1::SetTitle(const wxString& title)
{
	m_title_item->SetLabel(title);
    Layout();
}

void ACDialogTopbar1::SetSubTitle(const wxString& subTitle)
{
	m_subtitle_item->SetLabel(subTitle);
    Layout();
}

void ACDialogTopbar1::SetCustomBackgroundColour(const wxColour& colour)
{
	SetBackgroundColour(colour);
	m_logo_button->SetBackgroundColor(ACStateColor(colour));
	m_title_item->SetBackgroundColor(ACStateColor(colour));
	m_subtitle_item->SetBackgroundColor(ACStateColor(colour));
	m_close_button->SetBackgroundColor(ACStateColor(colour));
}

void ACDialogTopbar1::SetShowCloseButton(bool show)
{
	m_close_button->Show(show);
	m_mainSizer->Layout();
	Refresh();
}

void ACDialogTopbar1::OnMouseLeftDown(wxMouseEvent& event)
{
	wxPoint mouse_pos = ::wxGetMousePosition();
	wxPoint frame_pos = m_frame->GetScreenPosition();
	static wxPoint m_delta;
	m_delta = mouse_pos - frame_pos;
	bool isAtbuttonPos = m_close_button != nullptr && m_close_button->GetRect().Contains(event.GetPosition()) ? true : false;
	CaptureMouse();
	if (isAtbuttonPos)
	{
#ifdef __WXMSW__
		ReleaseMouse();
		::PostMessage((HWND)m_frame->GetHandle(), WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(mouse_pos.x, mouse_pos.y));
		return;
#endif //  __WXMSW__
	}
	event.GetPosition(&m_point.x, &m_point.y);
	event.Skip();

}

void ACDialogTopbar1::OnMouseLeftUp(wxMouseEvent& event)
{
	wxPoint mouse_pos = ::wxGetMousePosition();
	if (HasCapture())
	{
		ReleaseMouse();
	}

	event.Skip();
}

void ACDialogTopbar1::OnMouseMotion(wxMouseEvent& event)
{
	wxPoint mouse_pos = ::wxGetMousePosition();

	if (!HasCapture())
	{
		//m_frame->OnMouseMotion(event);

		event.Skip();
		return;
	}

	if (event.Dragging() && event.LeftIsDown())
	{
		int x, y;
		event.GetPosition(&x, &y);
		ClientToScreen(&x, &y);
		//m_frame->Move(mouse_pos - m_delta);
		m_frame->Move(x - m_point.x, y - m_point.y);
	}
	event.Skip();
}


}
}
