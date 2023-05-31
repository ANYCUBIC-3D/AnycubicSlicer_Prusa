#ifndef slic3r_GUI_ACListBox_hpp_
#define slic3r_GUI_ACListBox_hpp_

#include <wx/textctrl.h>
#include "ACStaticBox.hpp"
#include "ACButton.hpp"
#include <vector>
#include <wx/event.h>

wxDECLARE_EVENT(EVT_ACLISTBOX_SEL_CHANGED, wxCommandEvent);

class ACListBox : public wxNavigationEnabled<ACStaticBox>
{
    std::vector<ACButton*> m_buttons;
    std::vector<wxString> m_imgNames;
    std::vector<wxString> m_hover_imgNames;
    std::vector<wxString> m_dis_imgNames;

    ACStateColor m_bgColor_nor;
    ACStateColor m_bgColor_sel;

    ACStateColor m_textColor_nor;
    ACStateColor m_textColor_sel;

    static const int ListBoxItemsMargin = 12;
    static const int ListBoxItemsGap = 10;

    static const int ListItemWidth = 22;
    static const int ListItemHeight = 3.6;

    int m_itemSpacing = 10;
    int m_itemPadding = 12;
    wxSize m_itemMinSize = wxSize(220, 36);


    wxBoxSizer* m_mainSizer {nullptr};

    int m_currentSelIndex = -1;

    std::function<void(int)> m_cb_ItemSelected = nullptr;
public:
    ACListBox(wxWindow* parent,
             wxWindowID      id        = wxID_ANY,
             const wxPoint & pos       = wxDefaultPosition,
             const wxSize &  size      = wxDefaultSize, 
             long style = 0);

public:
    void Create(wxWindow *     parent,
              const wxPoint &pos   = wxDefaultPosition,
              const wxSize & size  = wxDefaultSize,
              long           style = 0);

    void SelectItem(int id);
    int AppendItem(const wxString& text, const wxString& imgName);
    int AppendItem(const wxString &text,
                   const wxString &imgName,
                   const wxString &hover_imgName,
                   const wxString &dis_imgName);

    int GetButtonsCount() { return m_buttons.size(); }
    std::vector<ACButton *> GetButtons() { return m_buttons; }

    virtual void Rescale();

    int GetSelection();

    wxString GetItemText(int index);

    void DeleteChildren();

    void onItemSelected(std::function<void(int)> cb_ItemSelected);
protected:
    void messureSize();
private:

};

#endif // !slic3r_GUI_TextInput_hpp_
