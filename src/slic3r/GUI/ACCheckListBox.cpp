#include "ACCheckListBox.hpp"

#include <wx/dcgraph.h>
#include "ACDefines.h"
#include "ACStateColor.hpp"

//BEGIN_EVENT_TABLE(ACCheckListBox, ACStaticBox)
//
////EVT_PAINT(ACCheckListBox::paintEvent)
//
//END_EVENT_TABLE()

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

//wxDEFINE_EVENT(EVT_ACCHECKLISTBOX_SEL_CHANGED, wxCommandEvent);
//wxDEFINE_EVENT(EVT_ACCHECKLISTBOX_CHECK_CHANGED, wxCommandEvent);

#define TitleSpacing 14
#define ItemSpacing  6
//#define ItemPanding  6

ACCheckListBox::ACCheckListBox(wxWindow *     parent,
                    const wxString& title,
                    ACButton::AC_BUTTON_TYPE        itemBtType,
                    ACButton::AC_BUTTON_CHECK_STYLE itemBtStyle,
                    int nItemCols,
                    wxWindowID      id,
                    const wxPoint &pos,
                    const wxSize  &size,
                    long           style)
    : m_itemBtType(itemBtType)
    , m_itemBtStyle(itemBtStyle)
    , m_flexCol(nItemCols)
{
    Create(parent, title, pos, size, style);
}

void ACCheckListBox::Create(wxWindow *     parent,
                    const wxString& title,
                    const wxPoint &pos,
                    const wxSize  &size,
                    long           style)
{

    ACStaticBox::Create(parent, wxID_ANY, pos, size, style);

    //SetCornerRadius(14);
    SetBackgroundColour(AC_COLOR_WHITE);

    if (m_flexCol == 1) 
        m_itemsSizer = new wxBoxSizer (wxVERTICAL);
    else
        m_itemsSizer = new wxFlexGridSizer(m_flexCol, ItemSpacing, ItemSpacing);
    
    messureSize();

    SetSizerAndFit(m_itemsSizer);

}

void ACCheckListBox::Rescale()
{
    for (ACButton* bt : m_buttons) {
        bt->Rescale();
    }

    float dipSpacing = FromDIP(ItemSpacing);

    ((wxFlexGridSizer*)m_itemsSizer)->SetHGap(dipSpacing);
    ((wxFlexGridSizer*)m_itemsSizer)->SetHGap(dipSpacing);



    Refresh();
}

void ACCheckListBox::messureSize()
{
    wxSize minSize(0,0);
    
    float dipSpacing = FromDIP(ItemSpacing);

    int maxW = 0;
    int itemsTotalH = 0;
    if (m_flexCol == 1) {
        if (m_buttons.size() ) {
            for (int i = 0; i < m_buttons.size(); ++i) 
            {
                wxSize minSize = m_buttons[i]->GetMinSize();
                maxW = std::max(maxW, minSize.GetWidth());
                itemsTotalH += minSize.GetHeight();
            }
            minSize.y += itemsTotalH;
            minSize.x = std::max(minSize.x, maxW);    
        }
    } else {
        int rows =  std::ceil((float) m_buttons.size() / m_flexCol);

        int index = 0;
        for (int i = 0; i < rows; ++i) {
            ACButton** rowStart = (m_buttons.data() + index++);
            int maxH = rowStart[0]->GetMinSize().GetHeight();
            int w = rowStart[0]->GetMinSize().GetWidth();
            for (int j = 1; j < m_flexCol && index < m_buttons.size(); ++j, index++) {
                w += dipSpacing;
                w += rowStart[j]->GetMinSize().GetWidth();
                maxH = std::max(maxH, rowStart[j]->GetMinSize().GetHeight());
            }
            itemsTotalH += maxH;
            maxW = std::max(maxW, w);
        }
        minSize.y += (rows-1)*dipSpacing + itemsTotalH;
        minSize.x = std::max(minSize.x, maxW);    
    }

    wxSize curSize = GetSize();
    SetMinSize(minSize);

    if (curSize.x < minSize.x || curSize.y < minSize.y)
        SetSize(std::max(curSize.x, minSize.x), std::max(curSize.y, minSize.y));
}


int ACCheckListBox::AppendItem(const wxString& text, const wxString& imgName, const std::string* data)
{
    int style = m_itemBtStyle == ACButton::CHECKSTYLE_ON_BOX ? ACButton::AC_ALIGN_LEFT : 0;
    ACButton *bt = new ACButton(this, text, imgName, imgName, imgName, style);
    bt->SetButtonType(m_itemBtType);
    bt->SetCheckStyle(m_itemBtStyle);

    if (m_itemBtStyle == ACButton::CHECKSTYLE_ON_MARK) {
        bt->setCheckedMarkImg("Label-check-nor", "Label-check-hover", 14);
        bt->SetMinSize(wxSize(std::max(bt->GetMinSize().x, 80), 34));
        bt->SetCornerRadius(15);
    }

    bt->Bind(wxEVT_CHECKBOX, [bt, this](wxCommandEvent& event)
    {
        if (event.GetInt() == ACButton::CHECKSTATE_ON) 
        {
            if (m_isMultSel == false) 
            {
                for (ACButton* itBt : m_buttons) 
                {
                    if (itBt == bt || itBt->GetChecked() == false) 
                        continue;
                    itBt->SetChecked(false);
                }            
            }
        }

        wxCommandEvent lbEvent(wxEVT_LISTBOX, GetId());
        lbEvent.SetEventObject(this);
        lbEvent.SetInt(GetItemIndex(bt));
        GetEventHandler()->ProcessEvent(lbEvent);
    });

    m_itemsSizer->Add(bt, 0, wxEXPAND);    

    m_buttons.push_back(bt);

    m_buttonsData.insert(std::make_pair(bt, data));

    messureSize();

    Layout();

    Refresh();

    return -1;
}

int ACCheckListBox::Append(const wxString& text, const std::string* data)
{
    return AppendItem(text, "", data);
}


void ACCheckListBox::onItemChecked(std::function<void(int)> cb_ItemChecked)
{
    m_cb_ItemChecked = cb_ItemChecked;
}


void ACCheckListBox::ClearSelection()
{
    for (ACButton* bt : m_buttons) {
        if (bt->GetChecked())
            bt->SetChecked(false);
    }
}

void ACCheckListBox::SetSelection(int index, bool sel )
{
    if (index == -1) {
        ClearSelection();
        return;
    }

    m_buttons[index]->SetChecked(sel);
}

void ACCheckListBox::SetCheck(int index, bool ck)
{
    if (index == -1) {
        ClearSelection();
        return;
    }

    m_buttons[index]->SetChecked(ck);
}

int ACCheckListBox::GetSelection()
{
    for (int i = 0; i < m_buttons.size(); i++) 
    {
        ACButton* bt = m_buttons[i];
        if (bt->GetChecked()) {
            return i;
        }    
    }
    return -1;
}

int ACCheckListBox::GetSelections(std::vector<int>& selections)
{
    selections.clear();

    int nSel = 0;

    for (int i = 0; i < m_buttons.size(); i++) 
    {
        ACButton* bt = m_buttons[i];
        if (bt->GetChecked()) {
            selections.push_back(i);
            nSel++;
        }    
    }

    return nSel;
}


wxString ACCheckListBox::GetItemText(int index)
{
    if(index < 0 || index >= m_buttons.size())
        return "";

    return m_buttons[index]->GetLabel();
}

const std::string& ACCheckListBox::get_data(int index)
{
    wxASSERT(index >= 0 && index < m_buttons.size())
    return *m_buttonsData[m_buttons[index]];
}




int ACCheckListBox::GetItemIndex(const ACButton* bt) const
{
    int index = 0;
    for (auto btIt : m_buttons) {
        if (btIt == bt) {
            return index;
        }
        index++;
    }
    return -1;
}

int ACCheckListBox::find(const std::string& text)
{
    int index = 0;
    for (ACButton* btIt : m_buttons) 
    {
        const std::string& value = *m_buttonsData[btIt];
        if ( value == text) {
            return index;
        }
        index++;
    }
    return -1;
}

bool ACCheckListBox::IsChecked(int index)
{
    if(index < 0 || index >= m_buttons.size())
        return false;
    return m_buttons[index]->GetChecked();
}


void ACCheckListBox::Clear()
{
    m_itemsSizer->Clear(true);
    m_buttons.clear();
    m_buttonsData.clear();
}


