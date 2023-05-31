#pragma once

#include "wx/wx.h"

#include "ACLabel.hpp"
#include "ACButton.hpp"
#include "GUI_Utils.hpp"

using namespace Slic3r::GUI;

class ACDialogTopbar : public wxWindow
{
public:
    ACDialogTopbar(wxWindow *parent, const wxString &title, int toolbarW, int toolbarH = 62);
    ~ACDialogTopbar(void) ;

    void Init(wxWindow* parent);

    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseLeftUp(wxMouseEvent& event);
    void OnMouseMotion(wxMouseEvent& event);

    void SetTitle(wxString title);
    void SetShowCloseButton(bool show);
    void msw_rescale();

public:
    void OnClose(wxEvent &event);
private:
    wxWindow*   m_frame;

    wxString    m_title;
    int         m_toolbar_h;
    int         m_toolbar_w;

    wxBoxSizer * m_mainSizer;
    ACButton *  m_title_item;
    ACButton *  m_close_button;

    // drag move
    wxPoint m_delta;
    wxPoint m_point;
};
