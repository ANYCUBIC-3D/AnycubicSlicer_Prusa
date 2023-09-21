#include "ACTopbar.hpp"
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
#include <wx/display.h>
//#include "WebViewDialog.hpp"
//#include "PartPlate.hpp"

#include "ACDefines.h"

#define TOPBAR_HEIGHT  24
#define TOPBAR_PADDING  2
#define TOPBAR_SPACING  2
#define TOPBAR_ICON_SIZE  26
#define TOPBAR_MENU_WIDTH  41
//#define TOPBAR_TITLE_WIDTH  300

using namespace Slic3r;

//struct ButtonMutexManager
//{
//    std::vector<ACButton*> buttons;
//
//    void add(ACButton* bt) {
//        bt->Bind(wxEVT_LEAVE_WINDOW, [&](wxMouseEvent& event){
//            
//        });
//    }
//};


ACTopbar::ACTopbar(wxFrame* parent) 
    : ACStaticBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxWANTS_CHARS)
{ 
    radius = 0;
    Init(parent);
}

void ACTopbar::setTitleStyle(ACButton* bt) 
{
    bt->SetButtonType(ACButton::AC_BUTTON_LABEL);
    bt->SetFont(bt->GetFont().Bold());
}

void ACTopbar::setMenuStyle(ACButton* bt) 
{
    bt->SetButtonType(ACButton::AC_BUTTON_SEL);
    bt->setTakeFocusedAsHovered(true);
    //bt->SetCheckStyle(ACButton::CHECKSTYLE_ON);
}
void ACTopbar::setBtStyle  (ACButton* bt) 
{
    //ACStateColor background_color (
    //    std::make_pair(AC_COLOR_MAIN_BLUE, (int) ACStateColor::Checked),
    //    std::make_pair(AC_COLOR_MAIN_BLUE, (int) ACStateColor::Hovered), 
    //    std::make_pair(AC_COLOR_WHITE, (int) ACStateColor::Normal));
    //bt->SetBackgroundColor(background_color);
    //bt->SetSpacing(0);
    /*bt->SetPaddingSize(wxSize(2,2));*/

    bt->SetButtonType(ACButton::AC_BUTTON_ICON);
}

void ACTopbar::Init(wxFrame* parent) 
{
    m_frame = parent;
    m_toolbar_h = TOPBAR_HEIGHT;
    int client_w = parent->GetClientSize().GetWidth();
    this->SetSize(client_w, m_toolbar_h);

    SetBackgroundColour(AC_COLOR_WHITE);


    std::string iconName_logo = "AnycubicSlicer";
    std::string filePath;
    if (boost::filesystem::exists(iconName_logo + ".png"))
        filePath = iconName_logo + ".png";
    else
        filePath = Slic3r::var(iconName_logo + ".png");
    wxImage imageHigh = wxImage(from_u8(filePath), wxBITMAP_TYPE_PNG);
    imageHigh.Rescale(FromDIP(TOPBAR_ICON_SIZE), FromDIP(TOPBAR_ICON_SIZE), wxIMAGE_QUALITY_HIGH);

    //wxBitmap logo_bitmap = create_scaled_bitmap("AnycubicSlicer", nullptr, TOPBAR_ICON_SIZE);
    wxStaticBitmap *btLogo = new wxStaticBitmap(this, wxID_ANY, wxBitmap(imageHigh));
    //ACButton *btLogo = new ACButton(this, "", iconName_logo, iconName_logo, iconName_logo, wxNO_BORDER, wxSize(TOPBAR_ICON_SIZE, TOPBAR_ICON_SIZE));
    //btLogo->SetEnable(false);
    btLogo->SetCanFocus(false);
    //btLogo->SetPaddingSize(wxSize(0, 0));

    
    m_btFile    = new ACButton(this, _L("File")    , "", "", "", wxNO_BORDER, wxSize(0,0));
    m_btEdit    = new ACButton(this, _L("Edit")    , "", "", "", wxNO_BORDER, wxSize(0,0));
    m_btView    = new ACButton(this, _L("View")    , "", "", "", wxNO_BORDER, wxSize(0,0));
    m_btSettings= new ACButton(this, _L("Settings"), "", "", "", wxNO_BORDER, wxSize(0,0));
    m_btHelp    = new ACButton(this, _L("Help")    , "", "", "", wxNO_BORDER, wxSize(0,0));

    m_btFile    ->SetMinSize(wxSize(TOPBAR_MENU_WIDTH, m_toolbar_h));
    m_btFile->SetPaddingSize(wxSize(10, 0));
    m_btFile->SetCornerRadius(4);
    m_btEdit    ->SetMinSize(wxSize(TOPBAR_MENU_WIDTH, m_toolbar_h));
    m_btEdit->SetPaddingSize(wxSize(10, 0));
    m_btEdit->SetCornerRadius(4);
    m_btView    ->SetMinSize(wxSize(TOPBAR_MENU_WIDTH, m_toolbar_h));
    m_btView->SetPaddingSize(wxSize(10, 0));
    m_btView->SetCornerRadius(4);
    m_btSettings->SetMinSize(wxSize(TOPBAR_MENU_WIDTH, m_toolbar_h));
    m_btSettings->SetPaddingSize(wxSize(10, 0));
    m_btSettings->SetCornerRadius(4);
    m_btHelp    ->SetMinSize(wxSize(TOPBAR_MENU_WIDTH, m_toolbar_h));
    m_btHelp->SetPaddingSize(wxSize(10, 0));
    m_btHelp->SetCornerRadius(4);

    setMenuStyle(m_btFile    );
    setMenuStyle(m_btEdit    );
    setMenuStyle(m_btSettings);
    setMenuStyle(m_btView    );
    setMenuStyle(m_btHelp    );

    wxBoxSizer * hSizer_left = new wxBoxSizer(wxHORIZONTAL);
    hSizer_left->Add(btLogo, 0,wxALIGN_CENTER_VERTICAL);
    hSizer_left->Add(m_btFile,0, wxALIGN_CENTER_VERTICAL|wxLEFT,10);
    hSizer_left->Add(m_btEdit,0, wxALIGN_CENTER_VERTICAL);
    hSizer_left->Add(m_btSettings,0, wxALIGN_CENTER_VERTICAL);
    hSizer_left->Add(m_btView,0, wxALIGN_CENTER_VERTICAL);
    hSizer_left->Add(m_btHelp, 0,wxALIGN_CENTER_VERTICAL);
    if (wxGetApp().is_gcode_viewer())
        m_btHelp->Hide();


    wxString iconName_Iconize      = "software_minimization-nor"  ;
    wxString iconName_IconizeHover = "software_minimization-hover";

    m_iconName_Maximize      = "software_maximization-nor"  ;
    m_iconName_MaximizeHover = "software_maximization-hover";
    m_iconName_Window        = "software_window-nor"  ;
    m_iconName_WindowHover   = "software_window-hover";

    wxString iconName_Close      = "software_close-nor"  ;
    wxString iconName_CloseHover = "software_close-hover";

    m_btIconize  = new ACButton(this, "",  iconName_Iconize   ,  iconName_IconizeHover   , iconName_Iconize   , wxNO_BORDER, wxSize(TOPBAR_ICON_SIZE, TOPBAR_ICON_SIZE));
    m_btMaximize = new ACButton(this, "", m_iconName_Window, m_iconName_WindowHover, m_iconName_WindowHover , wxNO_BORDER,wxSize(TOPBAR_ICON_SIZE, TOPBAR_ICON_SIZE));
    m_btClose    = new ACButton(this, "",  iconName_Close     ,  iconName_CloseHover     , iconName_Close     , wxNO_BORDER, wxSize(TOPBAR_ICON_SIZE, TOPBAR_ICON_SIZE));

    setBtStyle(m_btIconize );
    setBtStyle(m_btMaximize);
    setBtStyle(m_btClose   );
    m_btIconize->SetPaddingSize(wxSize(0, 0));
    m_btMaximize->SetPaddingSize(wxSize(0, 0));
    m_btClose->SetPaddingSize(wxSize(0, 0));
    wxBoxSizer * hSizer_right = new wxBoxSizer(wxHORIZONTAL);
    hSizer_right->Add(m_btIconize, wxALIGN_CENTER_VERTICAL);
    hSizer_right->AddSpacer(10);
    hSizer_right->Add(m_btMaximize, wxALIGN_CENTER_VERTICAL);
    hSizer_right->AddSpacer(10);
    hSizer_right->Add(m_btClose, wxALIGN_CENTER_VERTICAL);

    if (m_frame->IsMaximized()) {
        m_btMaximize->SetIcon(m_iconName_Maximize);
        m_btMaximize->SetHoverIcon(m_iconName_MaximizeHover);
    }
 
    m_title_item = new ACButton(this, "", "", "", "", wxNO_BORDER);
    m_title_item->SetCanFocus(false);
    m_title_item->SetEnable(false);
    setTitleStyle(m_title_item);
    m_title_item->SetPaddingSize(wxSize(0, 0));
    
    wxBoxSizer * mainSizer = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->AddSpacer(10);
    mainSizer->Add(hSizer_left, 0);
    mainSizer->Add(m_title_item, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND);
    mainSizer->Add(hSizer_right, 0);
    mainSizer->AddSpacer(10);

    SetSizer(mainSizer);

    this->Layout();


    m_btFile    ->Bind(wxEVT_BUTTON, &ACTopbar::OnFileToolItem, this);
    m_btEdit    ->Bind(wxEVT_BUTTON, &ACTopbar::OnEditToolItem, this);
    m_btView    ->Bind(wxEVT_BUTTON, &ACTopbar::OnViewToolItem, this);
    m_btSettings->Bind(wxEVT_BUTTON, &ACTopbar::OnSetsToolItem, this);
    m_btHelp    ->Bind(wxEVT_BUTTON, &ACTopbar::OnHelpToolItem, this);
    
    m_btIconize ->Bind(wxEVT_BUTTON, &ACTopbar::OnIconize, this);
    m_btMaximize->Bind(wxEVT_BUTTON, &ACTopbar::OnFullScreen, this);
    m_btClose   ->Bind(wxEVT_BUTTON, &ACTopbar::OnCloseFrame, this);

//    m_frame->Bind(wxEVT_SIZE, [this](wxSizeEvent&) {
//        //BOOST_LOG_TRIVIAL(trace) << "mainframe: size changed, is maximized = " << this->IsMaximized();
//#ifndef __APPLE__
//        if (this->IsMaximized()) {
//            m_topbar->SetWindowSize();
//        } else {
//            m_topbar->SetMaximizedSize();
//        }
//#endif
//        Refresh();
//        Layout();
//    });
    
    this->Bind(wxEVT_MOTION, &ACTopbar::OnMouseMotion, this);
    this->Bind(wxEVT_LEFT_DCLICK, &ACTopbar::OnMouseLeftDClock, this);
    this->Bind(wxEVT_LEFT_DOWN, &ACTopbar::OnMouseLeftDown, this);
    this->Bind(wxEVT_LEFT_UP, &ACTopbar::OnMouseLeftUp, this);
    this->Bind(wxEVT_MENU_CLOSE,&ACTopbar::OnMenuClose,this);
}
void ACTopbar::OnMenuClose(wxMenuEvent &event) { 
   if (now_showBtn != nullptr && event.GetId() < 0) {
        static_cast<MainFrame *>(m_frame)->SetPopWinShowIndex(false);
        setMenuStyle(now_showBtn);
       now_showBtn = nullptr;
   }
}
ACTopbar::~ACTopbar()
{
    m_file_menu = nullptr;
}

void ACTopbar::SetFileMenu(wxMenu* file_menu)
{
    m_file_menu = file_menu;
}

void ACTopbar::SetEditMenu(wxMenu* edit_menu) 
{ 
    m_edit_menu = edit_menu;
}

void ACTopbar::SetViewMenu(wxMenu* view_menu) 
{
    m_view_menu = view_menu;
}

void ACTopbar::SetSetsMenu(wxMenu* sets_menu) 
{
    m_sets_menu = sets_menu;
}

void ACTopbar::SetHelpMenu(wxMenu* help_menu) 
{ 
    m_help_menu = help_menu;
}

void ACTopbar::SetEditHide()
{
    m_btEdit->Hide();
    this->Layout();
    Refresh();
}


void ACTopbar::SetTitle(wxString title)
{
    wxSize itemSize = m_title_item->GetSize();

    wxGCDC dc(this);
    title = wxControl::Ellipsize(title, dc, wxELLIPSIZE_END, itemSize.GetWidth());

    m_title_item->SetLabel(title);

    this->Layout();
    this->Refresh();
}

void ACTopbar::OnFullScreen(wxCommandEvent& event)
{
    if (m_frame->IsMaximized() || m_frame->IsFullScreen()) {
        m_btMaximize->SetIcon     (m_iconName_Window);
        m_btMaximize->SetHoverIcon(m_iconName_WindowHover);
        //m_frame->Restore();
        if (m_frame->IsFullScreen()) {
            m_frame->ShowFullScreen(false);
            return;
        }
        m_frame->Maximize(false);

    }
    else { // isWindow
        m_btMaximize->SetIcon     (m_iconName_Maximize);
        m_btMaximize->SetHoverIcon(m_iconName_MaximizeHover);

        wxDisplay display(this);
        auto      size = display.GetClientArea().GetSize();
        m_frame->SetMaxSize(size + wxSize{16, 16});
        m_normalRect = m_frame->GetRect();
        m_frame->Maximize();

    }
}

void ACTopbar::UpdateToolbarWidth(int width)
{
    this->SetSize(width, m_toolbar_h);
}

void ACTopbar::Rescale() {

    m_btFile    ->Rescale();
    m_btEdit    ->Rescale();
    m_btView    ->Rescale();
    m_btSettings->Rescale();
    m_btHelp    ->Rescale();


    m_title_item->Rescale();
    
    m_btIconize ->Rescale();
    m_btMaximize->Rescale();
    m_btClose   ->Rescale();

    
}
void ACTopbar::ChangeIconStyle() {
    if (m_frame->IsMaximized() || m_frame->IsFullScreen()) {
        m_btMaximize->SetIcon(m_iconName_Maximize);
        m_btMaximize->SetHoverIcon(m_iconName_MaximizeHover);
    } else { // isWindow
        m_btMaximize->SetIcon(m_iconName_Window);
        m_btMaximize->SetHoverIcon(m_iconName_WindowHover);
    }
}
void ACTopbar::OnIconize(wxCommandEvent &event) {
    m_frame->Iconize();
}


void ACTopbar::OnCloseFrame(wxCommandEvent& event)
{
    m_frame->Close();
}

void ACTopbar::OnFileToolItem(wxCommandEvent& event)
{
    m_btFile->SetBackgroundColor(AC_COLOR_BT_SEL_BG_HOV);
    m_btFile->SetTextColor(AC_COLOR_BT_SEL_FG_HOV);
    now_showBtn = m_btFile;
    static_cast<MainFrame*>(m_frame)->SetPopWinShowIndex(true);
    this->PopupMenu(m_file_menu, wxPoint(m_btFile->GetPosition().x, this->GetSize().GetHeight() + 2));
    //setMenuStyle(m_btFile);

}

void ACTopbar::OnEditToolItem(wxCommandEvent& event)
{
    m_btEdit->SetBackgroundColor(AC_COLOR_BT_SEL_BG_HOV);
    m_btEdit->SetTextColor(AC_COLOR_BT_SEL_FG_HOV);
    now_showBtn = m_btEdit;
    static_cast<MainFrame *>(m_frame)->SetPopWinShowIndex(true);
    this->PopupMenu(m_edit_menu, wxPoint(m_btEdit->GetPosition().x, this->GetSize().GetHeight() + 2));
    //setMenuStyle(m_btEdit);
}

void ACTopbar::OnViewToolItem(wxCommandEvent& event)
{
    m_btView->SetBackgroundColor(AC_COLOR_BT_SEL_BG_HOV);
    m_btView->SetTextColor(AC_COLOR_BT_SEL_FG_HOV);
    now_showBtn = m_btView;
    static_cast<MainFrame *>(m_frame)->SetPopWinShowIndex(true);
    this->PopupMenu(m_view_menu, wxPoint(m_btView->GetPosition().x, this->GetSize().GetHeight() + 2));
    //setMenuStyle(m_btView);
}

void ACTopbar::OnSetsToolItem(wxCommandEvent& event)
{
    m_btSettings->SetBackgroundColor(AC_COLOR_BT_SEL_BG_HOV);
    m_btSettings->SetTextColor(AC_COLOR_BT_SEL_FG_HOV);
    now_showBtn = m_btSettings;
    static_cast<MainFrame *>(m_frame)->SetPopWinShowIndex(true);
    this->PopupMenu(m_sets_menu, wxPoint(m_btSettings->GetPosition().x, this->GetSize().GetHeight() + 2));
    //setMenuStyle(m_btSettings);
}

void ACTopbar::OnHelpToolItem(wxCommandEvent& event)
{
    m_btHelp->SetBackgroundColor(AC_COLOR_BT_SEL_BG_HOV);
    m_btHelp->SetTextColor(AC_COLOR_BT_SEL_FG_HOV);
    now_showBtn = m_btHelp;
    static_cast<MainFrame *>(m_frame)->SetPopWinShowIndex(true);
    this->PopupMenu(m_help_menu, wxPoint(m_btHelp->GetPosition().x, this->GetSize().GetHeight() + 2));
    //setMenuStyle(m_btHelp);
}


void ACTopbar::OnMouseLeftDClock(wxMouseEvent& mouse)
{
    //wxPoint mouse_pos = ::wxGetMousePosition();
    bool atTitlePos = mouse.GetPosition().x > m_btHelp->GetRect().GetRight() && mouse.GetPosition().x < m_btIconize->GetRect().GetLeft();
    // check whether mouse is not on any tool item
    if (atTitlePos == false) {
        mouse.Skip();
        return;
    }



//#ifdef __W1XMSW__
//    ::PostMessage((HWND) m_frame->GetHandle(), WM_NCLBUTTONDBLCLK, HTCAPTION, MAKELPARAM(mouse_pos.x, mouse_pos.y));
//    return;
//#endif //  __WXMSW__

    if (m_frame->IsMaximized() || m_frame->IsFullScreen()) {
        m_btMaximize->SetIcon     (m_iconName_Window);
        m_btMaximize->SetHoverIcon(m_iconName_WindowHover);
        //m_frame->Restore();
        if (m_frame->IsFullScreen()) {
            m_frame->ShowFullScreen(false);
            return;
        }
        m_frame->Maximize(false);
    }
    else {
        m_btMaximize->SetIcon     (m_iconName_Maximize);
        m_btMaximize->SetHoverIcon(m_iconName_MaximizeHover);

        wxDisplay display(this);
        auto      size = display.GetClientArea().GetSize();
        m_frame->SetMaxSize(size + wxSize{16, 16});
        m_normalRect = m_frame->GetRect();
        m_frame->Maximize();
    }
}

void ACTopbar::OnMouseLeftDown(wxMouseEvent& event)
{
    wxPoint mouse_pos = ::wxGetMousePosition();
    wxPoint frame_pos = m_frame->GetScreenPosition();
    m_delta = mouse_pos - frame_pos;

    wxRect btRectA = m_btHelp->GetRect();
    wxRect btRectB = m_btIconize->GetRect();

    bool atTitlePos = event.GetPosition().x > btRectA.GetRight() && event.GetPosition().x < btRectB.GetLeft();

    if (atTitlePos && !m_frame->IsMaximized())
    {
        CaptureMouse();
//#ifdef __WXMSW__
//        ReleaseMouse();
//        ::PostMessage((HWND) m_frame->GetHandle(), WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(mouse_pos.x, mouse_pos.y));
//        return;
//#endif //  __WXMSW__
    }
    
    event.Skip();
}

void ACTopbar::OnMouseLeftUp(wxMouseEvent& event)
{
    wxPoint mouse_pos = ::wxGetMousePosition();
    if (HasCapture())
    {
        ReleaseMouse();
    }

    event.Skip();
}

void ACTopbar::OnMouseMotion(wxMouseEvent& event)
{
    wxPoint mouse_pos = ::wxGetMousePosition();

    if (!HasCapture()) {
        //m_frame->OnMouseMotion(event);
        event.Skip();
        return;
    }

    if (event.Dragging() && event.LeftIsDown())
    {
        // leave max state and adjust position 
        //if (m_frame->IsMaximized()) {
        //    wxRect rect = m_frame->GetRect();
        //    // Filter unexcept mouse move
        //    if (m_delta + rect.GetLeftTop() != mouse_pos) {
        //        m_delta = mouse_pos - rect.GetLeftTop();
        //        m_delta.x = m_delta.x * m_normalRect.width / rect.width;
        //        m_delta.y = m_delta.y * m_normalRect.height / rect.height;
        //        m_frame->Restore();
        //    }
        //}
        m_frame->Move(mouse_pos - m_delta);
    }
    event.Skip();
}



