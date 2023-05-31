#include "ACListBox.hpp"

#include <wx/dcgraph.h>
#include "ACDefines.h"
#include "ACStateColor.hpp"

//BEGIN_EVENT_TABLE(ACListBox, ACStaticBox)
//
////EVT_PAINT(ACListBox::paintEvent)
//
//END_EVENT_TABLE()

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

wxDEFINE_EVENT(EVT_ACLISTBOX_SEL_CHANGED, wxCommandEvent);


ACListBox::ACListBox(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
{
    Create(parent, pos, size, style);
}

void ACListBox::Create(wxWindow *     parent,
                       const wxPoint &pos,
                       const wxSize  &size,
                       long           style)
{

    ACStaticBox::Create(parent, wxID_ANY, pos, size, style);

    ACStateColor bgColor;
    bgColor.append(AC_COLOR_PANEL_BG, ACStateColor::Normal);
    SetBackgroundColor(bgColor);
    SetCornerRadius(14);

    m_mainSizer = new wxBoxSizer(wxVERTICAL);
    m_mainSizer->AddSpacer(m_itemPadding);

    SetSizerAndFit(m_mainSizer);
    Layout();
    messureSize();
}

void ACListBox::Rescale()
{
    for (ACButton* bt : m_buttons) {
        bt->Rescale();
    }

    messureSize();
    Refresh();
}
int ACListBox::AppendItem(const wxString& text, const wxString& imgName)
{

    return AppendItem(text, imgName, imgName, imgName);
}


int ACListBox::AppendItem(const wxString &text,
    const wxString &imgName,
    const wxString &hover_imgName,
    const wxString &dis_imgName)
{
    
    m_bgColor_nor = ACStateColor(
        std::make_pair(AC_COLOR_LISTBOX_ITEM_BG_PRE, (int) ACStateColor::Pressed),
        std::make_pair(AC_COLOR_LISTBOX_ITEM_BG_HOV, (int) ACStateColor::Hovered), 
        std::make_pair(AC_COLOR_LISTBOX_ITEM_BG_NOR, (int) ACStateColor::Normal)
    );

    m_bgColor_sel = ACStateColor(
        std::make_pair(AC_COLOR_LISTBOX_ITEM_BG_SEL, (int) ACStateColor::Pressed),
        std::make_pair(AC_COLOR_LISTBOX_ITEM_BG_SEL, (int) ACStateColor::Hovered), 
        std::make_pair(AC_COLOR_LISTBOX_ITEM_BG_SEL, (int) ACStateColor::Normal)
    );

    m_textColor_nor = ACStateColor(
        std::make_pair(AC_COLOR_LISTBOX_ITEM_FG_PRE, (int) ACStateColor::Pressed),
        std::make_pair(AC_COLOR_LISTBOX_ITEM_FG_HOV, (int) ACStateColor::Hovered), 
        std::make_pair(AC_COLOR_LISTBOX_ITEM_FG_NOR, (int) ACStateColor::Normal)
    );

    m_textColor_sel = ACStateColor(
        std::make_pair(AC_COLOR_LISTBOX_ITEM_FG_SEL, (int) ACStateColor::Pressed),
        std::make_pair(AC_COLOR_LISTBOX_ITEM_FG_SEL, (int) ACStateColor::Hovered), 
        std::make_pair(AC_COLOR_LISTBOX_ITEM_FG_SEL, (int) ACStateColor::Normal)
    );

    ACButton *button = new ACButton(this, text, imgName, imgName, imgName, wxBORDER_NONE|ACButton::AC_ALIGN_LEFT, wxSize(36, 36));
    button->SetPaddingSize(wxSize(10, 0));
    button->SetCornerRadius(10);
    button->SetBackgroundColour(AC_COLOR_PANEL_BG);
    button->DisableFocusFromKeyboard();

    button->SetBackgroundColor(m_bgColor_nor);
    button->SetTextColor(m_textColor_nor);

    const int &em = em_unit(GetParent());
    button->SetMinSize(wxSize(ListItemWidth * em, int(ListItemHeight * em)));
    button->setDrawCircle(true);

    button->Bind(wxEVT_BUTTON, [&,  button](wxEvent& event){
        int index = -1;
        int i=0;
        for (auto it = m_buttons.begin(); it !=  m_buttons.end(); ++it, ++i) {
            if (button == *it) {
                index = i;
            }
        }

        if (index == -1)
            return;

        SelectItem(index);

        event.Skip();

    });

    if (m_buttons.size() != 0)
        m_mainSizer->AddSpacer(m_itemSpacing);

    m_mainSizer->Add(button, 0, wxLEFT|wxRIGHT, m_itemPadding);
    
    m_buttons.push_back(button);
    m_imgNames.push_back(imgName);
    m_hover_imgNames.push_back(hover_imgName);
    m_dis_imgNames.push_back(dis_imgName);
    if (m_currentSelIndex == -1) {
        SelectItem(0);
    }

    messureSize();
    Layout();
    Refresh();
    return m_buttons.size()-1;
}

void ACListBox::onItemSelected(std::function<void(int)> cb_ItemSelected)
{
    m_cb_ItemSelected = cb_ItemSelected;
}

void ACListBox::SelectItem(int index)
{
    if (index < 0 || index > m_buttons.size()-1 || m_buttons.size() == 0) {
        // out of range
        return;
    }
    ACButton* oldButton = m_currentSelIndex == -1 ? nullptr : m_buttons[m_currentSelIndex];
    if (oldButton) {
        oldButton->SetBackgroundColor(m_bgColor_nor);
        oldButton->SetTextColor(m_textColor_nor);
        oldButton->SetIcon(m_imgNames[m_currentSelIndex]);
        oldButton->SetHoverIcon(m_imgNames[m_currentSelIndex]);
        oldButton->setCircleVisiable(false);
    }

    m_currentSelIndex = index;
    ACButton* button = m_buttons[m_currentSelIndex];

    button->SetBackgroundColor(m_bgColor_sel);
    button->SetTextColor(m_textColor_sel);
    button->SetIcon(m_hover_imgNames[m_currentSelIndex]);
    button->SetHoverIcon(m_hover_imgNames[m_currentSelIndex]);
    if (m_cb_ItemSelected) {
        m_cb_ItemSelected(m_currentSelIndex);
    }
    button->setCircleVisiable(true);
    Refresh();
}


int ACListBox::GetSelection()
{
    return m_currentSelIndex;
}

wxString ACListBox::GetItemText(int index)
{
    if(index < 0)
        return "";

    return m_buttons[index]->GetLabel();
}

void ACListBox::DeleteChildren()
{
    m_mainSizer->Clear(true);
    m_mainSizer->AddSpacer(m_itemPadding);

    m_currentSelIndex = -1;
    m_buttons.clear();

    messureSize();
    Layout();
}



void ACListBox::messureSize()
{
    wxSize oldSize = GetSize();
    const int &em      = em_unit(GetParent());
    int nItems = m_buttons.size();

    wxSize minSize;
    minSize.x = ListItemWidth * em + m_itemPadding * 2;
    minSize.y = nItems * int(ListItemHeight * em) + (nItems - 1) * m_itemSpacing + m_itemPadding * 2;
    
    int btH = m_itemPadding*2 + (nItems-1)*m_itemSpacing;
    for (const ACButton* bt : m_buttons) {
        wxSize btSize = bt->GetMinSize();
        minSize.x = std::max(minSize.x, btSize.x+m_itemPadding*2);
        btH += btSize.y;
    }

    wxSize curSize = GetSize();
    SetMinSize(minSize);

    if (curSize.x < minSize.x || curSize.y < minSize.y)
        SetSize(std::max(curSize.x, minSize.x), std::max(curSize.y, minSize.y));

}
