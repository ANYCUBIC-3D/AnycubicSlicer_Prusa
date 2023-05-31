#ifndef slic3r_GUI_ACCheckListBox_hpp_
#define slic3r_GUI_ACCheckListBox_hpp_

#include <wx/textctrl.h>
#include "ACStaticBox.hpp"
#include "ACButton.hpp"
#include <vector>
#include <wx/event.h>
#include <map>


class ACCheckListBox : public wxNavigationEnabled<ACStaticBox>
{
    std::vector<ACButton*> m_buttons;
    std::map<ACButton*, const std::string* > m_buttonsData;

    int m_itemSpacing = 10;
    int m_itemPadding = 12;

    wxSize m_itemMinSize = wxSize(220, 36);

    wxSizer * m_itemsSizer = nullptr;
    int m_flexCol = 4;

    int m_currentSelIndex = -1;

    std::function<void(int)> m_cb_ItemChecked = nullptr;

    int m_flexCountPerRow = 4;

    bool m_isMultSel = false;


    ACButton::AC_BUTTON_TYPE        m_itemBtType ;
    ACButton::AC_BUTTON_CHECK_STYLE m_itemBtStyle;
public:
    ACCheckListBox(wxWindow* parent,
            const wxString& title,
            ACButton::AC_BUTTON_TYPE        itemBtType,
            ACButton::AC_BUTTON_CHECK_STYLE itemBtStyle,
            int nItemCols,
            wxWindowID      id        = wxID_ANY,
            const wxPoint & pos       = wxDefaultPosition,
            const wxSize &  size      = wxDefaultSize, 
            long style = 0);


    enum ListItemLayoutType {
        LayoutFlex,
        LayoutList,
    };

public:
    void Create(wxWindow *     parent,
            const wxString& title,
            const wxPoint &pos   = wxDefaultPosition,
            const wxSize & size  = wxDefaultSize,
            long           style = 0);

    int AppendItem(const wxString& text, const wxString& imgName, const std::string* data);

    int Append(const wxString& text, const std::string* data = nullptr);
    int Append(const std::string& text, const std::string* data = nullptr) { return Append(wxString(text), data); }

    int GetButtonsCount() { return m_buttons.size(); }

    virtual void Rescale();

    void setMultSel(bool mult) { m_isMultSel = mult; }

    void ClearSelection();
    void SetSelection(int index, bool sel = true);
    void SetCheck(int index, bool ck);

    int GetCount() { return m_buttons.size(); }

    int GetSelection();
    int GetSelections(std::vector<int>& selections);

    bool IsChecked(int index);

    wxString GetItemText(int index);
    const std::string& get_data(int index);

    int GetItemIndex(const ACButton* bt) const;

    int find(const std::string& text);


    void Clear();

    void onItemChecked(std::function<void(int)> cb_ItemChecked);

    void SetTitle(const wxString& title) const;
protected:

private:
    void messureSize();
};

#endif // !slic3r_GUI_ACCheckListBox_hpp_
