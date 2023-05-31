#include "Notebook.hpp"

// #ifdef _WIN32

#include "GUI_App.hpp"
#include "wxExtensions.hpp"

#include <wx/button.h>
#include <wx/sizer.h>
#include "ACDefines.h"
#include "ACButton.hpp"

wxDEFINE_EVENT(wxCUSTOMEVT_NOTEBOOK_SEL_CHANGED, wxCommandEvent);

ButtonsListCtrl::ButtonsListCtrl(wxWindow *parent/*, bool add_mode_buttons = false*/) :
    wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTAB_TRAVERSAL)
{
#ifdef __WINDOWS__
    SetDoubleBuffered(true);
#endif //__WINDOWS__

    m_btn_margin  = 0;
    m_line_margin = 0;

    this->SetBackgroundColour(AC_COLOR_LIGHTBLUE);

    m_sizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(m_sizer);

    m_sizer->AddStretchSpacer(1);

    m_buttons_sizer = new wxFlexGridSizer(1, m_btn_margin, m_btn_margin);
    m_sizer->Add(m_buttons_sizer, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxBOTTOM, m_btn_margin);

    m_sizer->AddStretchSpacer(1);

}

void ButtonsListCtrl::Rescale()
{
    for (ACButton* btn : m_pageButtons)
        btn->Rescale();

    m_sizer->Layout();
}

void ButtonsListCtrl::OnColorsChanged()
{
    for (ACButton* btn : m_pageButtons)
        btn->sys_color_changed();

    m_sizer->Layout();
}

void ButtonsListCtrl::SetSelection(int sel)
{
    if (m_selection == sel)
        return;
    if (m_selection >= 0) {
        m_pageButtons[m_selection]->SetChecked(false);
        ACStateColor bg_color = ACStateColor( std::pair{wxColour(204, 224, 255), (int) ACStateColor::Normal} );
        m_pageButtons[m_selection]->SetBackgroundColor(bg_color);
        m_pageButtons[m_selection]->SetIcon(m_pageButtons_img[m_selection]);
    }

    m_selection = sel;

    if (m_selection >= 0) {
        m_pageButtons[m_selection]->SetChecked(true);
        ACStateColor bg_color = ACStateColor( std::pair{wxColour(255, 255, 255), (int) ACStateColor::Normal} );
        m_pageButtons[m_selection]->SetBackgroundColor(bg_color);
        m_pageButtons[m_selection]->SetIcon(m_pageButtons_img_click[m_selection]);
    }
    Refresh();
}

bool ButtonsListCtrl::InsertPage(size_t             n,
                                 const wxString &   text,
                                 bool               bSelect /* = false*/,
                                 const std::string &bmp_name /* = ""*/,
                                 const std::string &bmp_nameh_over /* = ""*/,
                                 const std::string &bmp_name_inactive)
{
    ACButton *btn = new ACButton(this, text.empty() ? text : " " + text, bmp_name, bmp_nameh_over, bmp_name_inactive,  wxNO_BORDER, wxSize(28,28));

    btn->SetCornerRadius(10, ACButton::CornerTop);
    btn->SetBackgroundColour(wxColour(204, 224, 255)); // ac light blue
    btn->SetPaddingSize(wxSize(25, 11));
    btn->SetAlignCenter(true);

    int em = em_unit(this);
    // BBS set size for button
    //btn->SetMinSize({(text.empty() ? 40 : 136) * em / 10, 36 * em / 10});

    ACStateColor bg_color = ACStateColor(
        std::pair{wxColour(204, 224, 255), (int) ACStateColor::Normal});
    btn->SetBackgroundColor(bg_color);

    ACStateColor text_color = ACStateColor(
        std::pair{wxColour(29, 105, 224), (int) ACStateColor::Pressed},
        std::pair{wxColour(57, 134, 255), (int) ACStateColor::Hovered},
        std::pair{wxColour(62, 81, 116), (int) ACStateColor::Normal}
        );
    btn->SetTextColor(text_color);

    btn->SetChecked(false);

    btn->Bind(wxEVT_BUTTON, [this, btn](wxCommandEvent& event) {
        if (auto it = std::find(m_pageButtons.begin(), m_pageButtons.end(), btn); it != m_pageButtons.end()) {
            auto sel = it - m_pageButtons.begin();
            //do it later
            //SetSelection(sel);
            wxCommandEvent evt = wxCommandEvent(wxCUSTOMEVT_NOTEBOOK_SEL_CHANGED);
            evt.SetId(sel);
            wxPostEvent(this->GetParent(), evt);
            Refresh();
        }
    });

    //Slic3r::GUI::wxGetApp().UpdateDarkUI(btn);

    m_pageButtons.insert(m_pageButtons.begin() + n, btn);
    m_pageButtons_img.insert(m_pageButtons_img.begin() + n, bmp_name);
    m_pageButtons_img_click.insert(m_pageButtons_img_click.begin() + n, bmp_nameh_over);
    m_buttons_sizer->Insert(n, new wxSizerItem(btn));
    m_buttons_sizer->SetCols(m_buttons_sizer->GetCols() + 1);
    m_sizer->Layout();
    return true;
}

void ButtonsListCtrl::RemovePage(size_t n)
{
    ACButton* btn = m_pageButtons[n];
    m_pageButtons.erase(m_pageButtons.begin() + n);
    m_buttons_sizer->Remove(n);
#if __WXOSX__
    RemoveChild(btn);
#else
    btn->Reparent(nullptr);
#endif
    btn->Destroy();
    m_sizer->Layout();
}

bool ButtonsListCtrl::SetPageImage(size_t n, const std::string& bmp_name) const
{
    if (n >= m_pageButtons.size())
        return false;
     //return m_pageButtons[n]->SetBitmap_(bmp_name);

     ScalableBitmap bitmap(NULL, bmp_name);
     // m_pageButtons[n]->SetBitmap_(bitmap);
     return true;
}

void ButtonsListCtrl::SetPageText(size_t n, const wxString& strText)
{
    ACButton* btn = m_pageButtons[n];
    btn->SetLabel(strText);
}

wxString ButtonsListCtrl::GetPageText(size_t n) const
{
    ACButton* btn = m_pageButtons[n];
    return btn->GetLabel();
}

// #endif // _WIN32


