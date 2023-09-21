#include "ACComboBox.hpp"
#include "ACLabel.hpp"

#include <wx/dcgraph.h>
#include "ACDefines.h"

namespace Slic3r { 
namespace GUI {

BEGIN_EVENT_TABLE(ACComboBox, ACTextInput)

EVT_LEFT_DOWN(ACComboBox::mouseDown)
//EVT_MOUSEWHEEL(ACComboBox::mouseWheelMoved)
EVT_KEY_DOWN(ACComboBox::keyDown)

// catch paint events
END_EVENT_TABLE()

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */


ACComboBox::ACComboBox(wxWindow *      parent,
                   wxWindowID      id,
                   const wxString &value,
                   const wxPoint & pos,
                   const wxSize &  size,
                   int             n,
                   const wxString  choices[],
                   long            style)
    : drop(this, texts, icons, wxSize(32,32))
{
    if (style & wxCB_READONLY)
        style |= wxRIGHT;
    text_off = style & CB_NO_TEXT;

    ACTextInput::Create(parent, "", value, "arrow-expand-nor", wxSize(16, 16), wxDefaultPosition, size,
                      style | wxTE_PROCESS_ENTER);

    SetBackgroundColour(AC_COLOR_WHITE);

    if (parent)
        parent->SetBackgroundColour(AC_COLOR_WHITE);

    drop.SetCornerRadius(6);
    drop.SetBackgroundColour(AC_COLOR_WHITE);
    
    ACTextInput::SetCornerRadius(6);
    if (style & wxCB_READONLY) {
        GetTextCtrl()->Hide();
        ACTextInput::SetBorderColor(ACStateColor(
            std::make_pair(AC_COLOR_DROPDOWN_BD_DIS, (int) ACStateColor::Disabled),
            std::make_pair(AC_COLOR_DROPDOWN_BD_PRE, (int) ACStateColor::Pressed),
            std::make_pair(AC_COLOR_DROPDOWN_BD_HOV, (int) ACStateColor::Hovered),
            std::make_pair(AC_COLOR_DROPDOWN_BD_NOR, (int) ACStateColor::Normal)));
        ACTextInput::SetBackgroundColor(ACStateColor(
            std::make_pair(AC_COLOR_DROPDOWN_BG_DIS, (int) ACStateColor::Disabled),
            std::make_pair(AC_COLOR_DROPDOWN_BG_PRE, (int) ACStateColor::Pressed),
            std::make_pair(AC_COLOR_DROPDOWN_BG_HOV, (int) ACStateColor::Hovered),
            std::make_pair(AC_COLOR_DROPDOWN_BG_NOR, (int) ACStateColor::Normal)));
        ACTextInput::SetLabelColor(ACStateColor(
            std::make_pair(AC_COLOR_DROPDOWN_FG_DIS, (int) ACStateColor::Disabled),
            std::make_pair(AC_COLOR_DROPDOWN_FG_PRE, (int) ACStateColor::Pressed),
            std::make_pair(AC_COLOR_DROPDOWN_FG_HOV, (int) ACStateColor::Hovered),
            std::make_pair(AC_COLOR_DROPDOWN_FG_NOR, (int) ACStateColor::Normal)));
    } else {
        GetTextCtrl()->Bind(wxEVT_KEY_DOWN, &ACComboBox::keyDown, this);
    }
    drop.Bind(wxEVT_COMBOBOX, [this](wxCommandEvent &e) {
        SetSelection(e.GetInt());

        wxCommandEvent ev(wxEVT_COMBOBOX);
        ev.SetEventObject(this);
        ev.SetInt(e.GetInt());
        ev.SetString(GetValue());
        GetEventHandler()->ProcessEvent(ev);
    });
    drop.Bind(EVT_DISMISS, [this](auto &) {
        drop_down = false;
        wxCommandEvent e(wxEVT_COMBOBOX_CLOSEUP);
        GetEventHandler()->ProcessEvent(e);
    });

    for (int i = 0; i < n; ++i) Append(choices[i], nullptr);
}

int ACComboBox::GetSelection() const { return drop.GetSelection(); }

void ACComboBox::SetSelection(int n)
{
    drop.SetSelection(n);
    SetLabel(drop.GetValue());
    //if (drop.selection >= 0)
    //    SetIcon(icons[drop.selection]);
}

void ACComboBox::Rescale()
{
    ACTextInput::Rescale();
    drop.Rescale();
}

wxString ACComboBox::GetValue() const
{
    return drop.GetSelection() >= 0 ? drop.GetValue() : GetLabel();
}

void ACComboBox::SetValue(const wxString &value)
{
    drop.SetValue(value);
    SetLabel(value);
    //if (drop.selection >= 0)
    //    SetIcon(icons[drop.selection]);
}

void ACComboBox::SetLabel(const wxString &value)
{
    if (GetTextCtrl()->IsShown() || text_off) {
        GetTextCtrl()->SetValue(value);
        ACTextInput::messureSize();
    }
    else
        ACTextInput::SetLabel(value);

    if (this->GetParent())
        this->GetParent()->Refresh();
}

wxString ACComboBox::GetLabel() const
{
    wxString str = "";
    if (GetTextCtrl()->IsShown() || text_off)
        str = GetTextCtrl()->GetValue();
    else
        str = ACTextInput::GetLabel();
    return str;
}

void ACComboBox::SetTextLabel(const wxString& label)
{
    ACTextInput::SetLabel(label);
}

wxString ACComboBox::GetTextLabel() const
{
    return ACTextInput::GetLabel();
}

bool ACComboBox::SetFont(wxFont const& font)
{
    if (GetTextCtrl() && GetTextCtrl()->IsShown())
        return GetTextCtrl()->SetFont(font);
    else
        return ACTextInput::SetFont(font);
}



int ACComboBox::Append(const wxString &item,
                     const wxBitmapBundle *bitmapBundle,
                     void *          clientData)
{
    texts.push_back(item);
    icons.push_back(bitmapBundle);
    datas.push_back(clientData);
    types.push_back(wxClientData_None);
    drop.Invalidate();
    return texts.size() - 1;
}

int ACComboBox::Append(const wxString &item, const wxBitmapBundle *bitmapBundle)
{
    return Append(item, bitmapBundle, nullptr);
}
int ACComboBox::Append(const wxString &item)
{
    return Append(item, nullptr);
}


void ACComboBox::DoClear()
{
    texts.clear();
    icons.clear();
    datas.clear();
    types.clear();
    drop.Invalidate(true);
}

void ACComboBox::DoDeleteOneItem(unsigned int pos)
{
    if (pos < 0 || pos >= texts.size()) 
        return;

    texts.erase(texts.begin() + pos);
    icons.erase(icons.begin() + pos);
    datas.erase(datas.begin() + pos);
    types.erase(types.begin() + pos);
    drop.Invalidate(true);
}

unsigned int ACComboBox::GetCount() const { return texts.size(); }

wxString ACComboBox::GetString(unsigned int n) const
{
    return n < texts.size() ? texts[n] : wxString{};
}

void ACComboBox::SetString(unsigned int n, wxString const &value)
{
    if (n >= texts.size()) return;
    texts[n]  = value;
    drop.Invalidate();
    if (n == drop.GetSelection()) SetLabel(value);
}

const wxBitmapBundle* ACComboBox::GetItemBitmapBundle(unsigned int n) {
    return icons[n];
}

int ACComboBox::GetItemsWidth()
{
    drop.messureSize();
    return drop.GetSize().GetWidth();
}


void ACComboBox::SetCornerRadius(int value)
{
	if (value < 0)
		return;
	drop.SetCornerRadius(value);
	ACTextInput::SetCornerRadius(value);
}

int ACComboBox::DoInsertItems(const wxArrayStringsAdapter &items,
                            unsigned int                 pos,
                            void **                      clientData,
                            wxClientDataType             type)
{
    if (pos > texts.size()) return -1;

    for (int i = 0; i < items.GetCount(); ++i) {
        texts.insert(texts.begin() + pos, items[i]);
        icons.insert(icons.begin() + pos, nullptr);
        datas.insert(datas.begin() + pos, clientData ? clientData[i] : NULL);
        types.insert(types.begin() + pos, type);
        ++pos;
    }
    drop.Invalidate(true);
    return pos - 1;
}

void *ACComboBox::DoGetItemClientData(unsigned int n) const { return n < texts.size() ? datas[n] : NULL; }

void ACComboBox::DoSetItemClientData(unsigned int n, void *data)
{
    if (n < texts.size())
        datas[n] = data;
}

void ACComboBox::mouseDown(wxMouseEvent &event)
{
    SetFocus();
    if (drop_down) {
        drop.Hide();
    } else if (drop.HasDismissLongTime()) {
        drop.autoPosition();
        drop_down = true;
#ifdef __APPLE__
        drop.SetWindowStyleFlag(wxPOPUP_WINDOW | wxBORDER_NONE | wxSTAY_ON_TOP);
#endif
        drop.Popup();
        wxCommandEvent e(wxEVT_COMBOBOX_DROPDOWN);
        GetEventHandler()->ProcessEvent(e);
    }
}


void ACComboBox::mouseWheelMoved(wxMouseEvent &event)
{
    event.Skip();
    if (drop_down) return;
    auto delta = (event.GetWheelRotation() < 0 == event.IsWheelInverted()) ? -1 : 1;
    unsigned int n = GetSelection() + delta;
    if (n < GetCount()) {
        SetSelection((int) n);
        sendComboBoxEvent();
    }
}

void ACComboBox::keyDown(wxKeyEvent& event)
{
    switch (event.GetKeyCode()) {
        case WXK_RETURN:
        case WXK_SPACE:
            if (drop_down) {
                drop.DismissAndNotify();
            } else if (drop.HasDismissLongTime()) {
                drop.autoPosition();
                drop_down = true;
                drop.Popup();
                wxCommandEvent e(wxEVT_COMBOBOX_DROPDOWN);
                GetEventHandler()->ProcessEvent(e);
            }
            break;
        case WXK_UP:
        case WXK_DOWN:
        case WXK_LEFT:
        case WXK_RIGHT:
            if ((event.GetKeyCode() == WXK_UP || event.GetKeyCode() == WXK_LEFT) && GetSelection() > 0) {
                SetSelection(GetSelection() - 1);
            } else if ((event.GetKeyCode() == WXK_DOWN || event.GetKeyCode() == WXK_RIGHT) && GetSelection() + 1 < texts.size()) {
                SetSelection(GetSelection() + 1);
            } else {
                break;
            }
            {
                wxCommandEvent e(wxEVT_COMBOBOX);
                e.SetEventObject(this);
                e.SetId(GetId());
                e.SetInt(GetSelection());
                GetEventHandler()->ProcessEvent(e);
            }
            break;
        case WXK_TAB:
            HandleAsNavigationKey(event);
            break;
        default:
            event.Skip();
            break;
    }
}

void ACComboBox::OnEdit()
{
    auto value = GetTextCtrl()->GetValue();
    SetValue(value);
}

#ifdef __WIN32__

WXLRESULT ACComboBox::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
    if (nMsg == WM_GETDLGCODE) {
        return DLGC_WANTALLKEYS;
    }
    return ACTextInput::MSWWindowProc(nMsg, wParam, lParam);
}

#endif

void ACComboBox::sendComboBoxEvent()
{
    wxCommandEvent event(wxEVT_COMBOBOX, GetId());
    event.SetEventObject(this);
    event.SetInt(drop.GetSelection());
    event.SetString(drop.GetValue());
    GetEventHandler()->ProcessEvent(event);
}

} // GUI
} // Slic3r
