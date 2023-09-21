#pragma once

#include "ACStaticBox.hpp"
#include "ACButton.hpp"

using namespace Slic3r::GUI;

class ACTopbar : public ACStaticBox
{
public:
    ACTopbar(wxFrame* parent);
    void Init(wxFrame *parent);
    ~ACTopbar();
    void UpdateToolbarWidth(int width);

    void SetFileMenu(wxMenu* file_menu);
    void SetEditMenu(wxMenu* edit_menu);
    void SetViewMenu(wxMenu* view_menu);
    void SetSetsMenu(wxMenu* sets_menu);
    void SetHelpMenu(wxMenu* help_menu);

    void SetEditHide();

    void SetTitle(wxString title);

    void Rescale();
    void ChangeIconStyle();

    void OnMenuClose(wxMenuEvent &event);
private:
    void OnFileToolItem(wxCommandEvent& event);
    void OnEditToolItem(wxCommandEvent& event);
    void OnViewToolItem(wxCommandEvent& event);
    void OnSetsToolItem(wxCommandEvent& event);
    void OnHelpToolItem(wxCommandEvent& event);

    void OnFullScreen(wxCommandEvent& event);
    void OnIconize   (wxCommandEvent& event);
    void OnCloseFrame(wxCommandEvent& event);

    void OnMouseLeftDClock(wxMouseEvent& mouse);
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseLeftUp(wxMouseEvent& event);
    void OnMouseMotion(wxMouseEvent& event);

    void setTitleStyle(ACButton* bt);
    void setMenuStyle(ACButton* bt);
    void setBtStyle  (ACButton* bt);
private:
    wxFrame* m_frame;

    wxRect m_normalRect;
    wxPoint m_delta;

    ACButton * m_btFile    ;
    ACButton * m_btEdit    ;
    ACButton * m_btView    ;
    ACButton * m_btSettings;
    ACButton * m_btHelp    ;
    ACButton * now_showBtn = nullptr;

    wxMenu m_top_menu;
    wxMenu* m_file_menu = nullptr;
    wxMenu* m_edit_menu = nullptr;
    wxMenu* m_view_menu = nullptr;
    wxMenu* m_sets_menu = nullptr;
    wxMenu* m_help_menu = nullptr;

    wxString m_iconName_Maximize     ;
    wxString m_iconName_MaximizeHover;
    wxString m_iconName_Window       ;
    wxString m_iconName_WindowHover  ;

    ACButton* m_title_item;
    
    ACButton* m_btIconize ;
    ACButton* m_btMaximize;
    ACButton* m_btClose   ;

    wxBitmap maximize_bitmap;
    wxBitmap maximize_bitmap_hover;

    wxBitmap window_bitmap;
    wxBitmap window_bitmap_hover;

    int m_toolbar_h;
};

//
//class TitleBar : public wxControl
//{
//public:
//    TitleBar(wxWindow* parent);
//    ~TitleBar() {}
//
//
//    ACTopbar* GetMenubar() { return &m_menuBar; }
//private:
//    ACTopbar m_menuBar;
//    wxBoxSizer* m_sizer;
//
//};