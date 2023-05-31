#include "ACDialogTopbar.hpp"
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
//#include "WebViewDialog.hpp"
//#include "PartPlate.hpp"


using namespace Slic3r;

ACDialogTopbar::ACDialogTopbar(wxWindow *parent, const wxString &title, int toolbarW, int toolbarH) 
    : wxWindow(parent, ID_DIALOG_TOP_BAR, wxDefaultPosition, wxDefaultSize)
    , m_frame(nullptr)
    , m_title(title)
    , m_toolbar_h(toolbarH)
    , m_toolbar_w(toolbarW)
    , m_mainSizer(nullptr)
    , m_title_item(nullptr)
    , m_close_button(nullptr)
    , m_point(0,0)
{ 
    Init(parent);
}

ACDialogTopbar::~ACDialogTopbar()
{
}

void ACDialogTopbar::Init(wxWindow* parent) 
{
    m_frame = parent;
    SetMinSize(wxSize(-1, m_toolbar_h));

    SetBackgroundColour(AC_COLOR_LIGHTBLUE);

    //ACStateColor background_color = ACStateColor(
    //    std::make_pair(AC_COLOR_MAIN_BLUE, (int) ACStateColor::Checked),
    //    std::make_pair(AC_COLOR_BG_LIGHTGRAY, (int) ACStateColor::Hovered), 
    //    std::make_pair(wxTransparentColor, (int) ACStateColor::Normal));

    //ACStateColor color();


    m_title_item = new ACButton(this, m_title, "", "", "", wxNO_BORDER);
    m_title_item->SetCanFocus(false);
    m_title_item->SetEnable(false);
    m_title_item->SetButtonType(ACButton::AC_BUTTON_LABEL_2);
    m_title_item->SetFont(ACLabel::Body_16);
    m_title_item->SetPaddingSize(wxSize(0, 0));


    //wxSize titleTextSize = m_title_item->GetTextSize();

    //wxBitmap close_bitmap = create_scaled_bitmap("software_close-nor", nullptr, TOPBAR_ICON_SIZE);
    //wxBitmap close_bitmap_hover = create_scaled_bitmap("software_close-hover", nullptr, TOPBAR_ICON_SIZE);
    wxString btIconNameCloseNor = "icon-close_40-nor";
    wxString btIconNameCloseHover = "software_close-hover";

    int buttonSize = 40;
    m_close_button = new ACButton(this, "", btIconNameCloseNor, btIconNameCloseHover, btIconNameCloseNor, wxBORDER_NONE,
                                  wxSize(buttonSize, buttonSize));
    m_close_button->SetPaddingSize(wxSize((0), (0)));
    m_close_button->SetButtonType(ACButton::AC_BUTTON_ICON);
    //m_close_button->SetAlignCenter(true);
    //m_close_button->SetBackgroundColor(background_color);

    m_mainSizer = new wxBoxSizer(wxHORIZONTAL);
    m_mainSizer->Add(m_title_item, 1, wxEXPAND);
    m_mainSizer->Add(m_close_button, 0, wxALIGN_CENTER_VERTICAL);
    m_mainSizer->AddSpacer(16);
    SetSizer(m_mainSizer);
    Fit();
    msw_rescale();
    //SetMinSize(wxSize(-1, m_toolbar_h));
    //SetSize(m_frame->GetSize().GetWidth(), m_toolbar_h);

    Bind(wxEVT_BUTTON, &ACDialogTopbar::OnClose, this, m_close_button->GetId());
    
    //Bind(wxEVT_SIZE, [this](wxSizeEvent &event) {
    //    Layout();
    //    event.Skip();
    //});

    //m_mainSizer->Layout();
    //Layout();

    m_title_item->Bind(wxEVT_LEFT_DOWN, &ACDialogTopbar::OnMouseLeftDown, this);
    m_title_item->Bind(wxEVT_LEFT_UP, &ACDialogTopbar::OnMouseLeftUp, this);
    m_title_item->Bind(wxEVT_MOTION, &ACDialogTopbar::OnMouseMotion, this);

    Bind(wxEVT_LEFT_DOWN, &ACDialogTopbar::OnMouseLeftDown, this);
    Bind(wxEVT_LEFT_UP, &ACDialogTopbar::OnMouseLeftUp, this);
    Bind(wxEVT_MOTION, &ACDialogTopbar::OnMouseMotion, this);
}

void ACDialogTopbar::msw_rescale()
{
    
    const int &em = em_unit(GetParent());

    SetMinSize(wxSize(m_toolbar_w * em, m_toolbar_h));
    SetSize(wxSize(m_toolbar_w, m_toolbar_h));
    m_mainSizer->Layout();
    Layout();
}
void ACDialogTopbar::SetTitle(wxString title)
{
    m_title_item->SetLabel(title);

    Refresh();
}

void ACDialogTopbar::SetShowCloseButton(bool show)
{
    m_close_button->Show(show);
    m_mainSizer->Layout();
    Refresh();
}

void ACDialogTopbar::OnClose(wxEvent& event) 
{ 
    m_frame->Close();
}

void ACDialogTopbar::OnMouseLeftDown(wxMouseEvent& event)
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
        ::PostMessage((HWND) m_frame->GetHandle(), WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(mouse_pos.x, mouse_pos.y));
        return;
#endif //  __WXMSW__
    }
    event.GetPosition(&m_point.x, &m_point.y);
    event.Skip();

}

void ACDialogTopbar::OnMouseLeftUp(wxMouseEvent& event)
{
    wxPoint mouse_pos = ::wxGetMousePosition();
    if (HasCapture())
    {
        ReleaseMouse();
    }

    event.Skip();
}

void ACDialogTopbar::OnMouseMotion(wxMouseEvent& event)
{
    wxPoint mouse_pos = ::wxGetMousePosition();

    if (!HasCapture()) {
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
        m_frame->Move(x - m_point.x , y - m_point.y);
    }
    event.Skip();
}



