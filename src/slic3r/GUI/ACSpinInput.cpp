#include "ACSpinInput.hpp"
#include "ACLabel.hpp"
#include "ACButton.hpp"

#include <wx/dcgraph.h>
#include <wx/spinctrl.h>

#include "ACDefines.h"

BEGIN_EVENT_TABLE(ACSpinInput, ACStaticBox)

EVT_KEY_DOWN(ACSpinInput::keyPressed)
EVT_MOUSEWHEEL(ACSpinInput::mouseWheelMoved)

EVT_PAINT(ACSpinInput::paintEvent)

END_EVENT_TABLE()

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

ACSpinInput::ACSpinInput()
    : label_color(std::make_pair(0x909090, (int) ACStateColor::Disabled), std::make_pair(0x6B6B6B, (int) ACStateColor::Normal))
    , text_color(std::make_pair(0x909090, (int) ACStateColor::Disabled), std::make_pair(0x262E30, (int) ACStateColor::Normal))
{
    radius           = 0;
    border_width     = 1;
    border_color     = ACStateColor(std::make_pair(AC_COLOR_SPINBOX_BD_DIS, (int) ACStateColor::Disabled),
                                    std::make_pair(AC_COLOR_SPINBOX_BD_HOV, (int) ACStateColor::Hovered),
                                    std::make_pair(AC_COLOR_SPINBOX_BD_NOR, (int) ACStateColor::Normal));
    background_color = ACStateColor(std::make_pair(AC_COLOR_SPINBOX_BG_DIS, (int) ACStateColor::Disabled),
                                    std::make_pair(AC_COLOR_SPINBOX_BG_HOV, (int) ACStateColor::Hovered),
                                    std::make_pair(AC_COLOR_SPINBOX_BG_NOR, (int) ACStateColor::Normal));
}

ACSpinInput::ACSpinInput(
    wxWindow *parent, wxString text, wxString label, const wxPoint &pos, const wxSize &size, long style, int min, int max, int initial)
    : ACSpinInput()
{
    Create(parent, text, label, pos, size, style, min, max, initial);
}

void ACSpinInput::Create(
    wxWindow *parent, wxString text, wxString label, const wxPoint &pos, const wxSize &size, long style, int min, int max, int initial)
{
    ACStaticBox::Create(parent, wxID_ANY, pos, size);
    SetFont(ACLabel::Body_12);
    wxWindow::SetLabel(label);
    state_handler.attach({&label_color, &text_color});
    state_handler.update_binds();
    text_ctrl = new wxTextCtrl(this, wxID_ANY, text, {20, 4}, wxDefaultSize, style | wxBORDER_NONE | wxTE_PROCESS_ENTER,
                               wxTextValidator(wxFILTER_DIGITS));
    text_ctrl->SetFont(ACLabel::Body_14);
    text_ctrl->SetBackgroundColour(background_color.colorForStates(state_handler.states()));
    text_ctrl->SetForegroundColour(text_color.colorForStates(state_handler.states()));
    text_ctrl->SetInitialSize(text_ctrl->GetBestSize());
    state_handler.attach_child(text_ctrl);
    text_ctrl->Bind(wxEVT_KILL_FOCUS, &ACSpinInput::onTextLostFocus, this);
    text_ctrl->Bind(wxEVT_TEXT_ENTER, &ACSpinInput::onTextEnter, this);
    text_ctrl->Bind(wxEVT_KEY_DOWN, &ACSpinInput::keyPressed, this);
    text_ctrl->Bind(wxEVT_RIGHT_DOWN, [this](auto &e) {}); // disable context menu
    ACButton_inc = createACButton(true);
    ACButton_dec = createACButton(false);
    delta        = 0;
    timer.Bind(wxEVT_TIMER, &ACSpinInput::onTimer, this);

    long initialFromText;
    if (text.ToLong(&initialFromText))
        initial = initialFromText;
    SetRange(min, max);
    SetValue(initial);
    messureSize();
}

void ACSpinInput::SetCornerRadius(double radius)
{
    this->radius = radius;
    Refresh();
}

void ACSpinInput::SetLabel(const wxString &label)
{
    wxWindow::SetLabel(label);
    messureSize();
    Refresh();
}

void ACSpinInput::SetLabelColor(ACStateColor const &color)
{
    label_color = color;
    state_handler.update_binds();
}

void ACSpinInput::SetTextColor(ACStateColor const &color)
{
    text_color = color;
    state_handler.update_binds();
}

void ACSpinInput::SetSize(wxSize const &size)
{
    wxWindow::SetSize(size);
    Rescale();
}

void ACSpinInput::SetValue(const wxString &text) { text_ctrl->SetValue(text); }

void ACSpinInput::SetValue(int value)
{
    if (value < min)
        value = min;
    else if (value > max)
        value = max;
    this->val = value;
    text_ctrl->SetValue(wxString::FromDouble(value));
}

int ACSpinInput::GetValue() const { return val; }

wxString ACSpinInput::GetTextValue() { return text_ctrl->GetValue(); }

void ACSpinInput::SetRange(int min, int max)
{
    this->min = min;
    this->max = max;
}

void ACSpinInput::DoSetToolTipText(wxString const &tip)
{
    wxWindow::DoSetToolTipText(tip);
    text_ctrl->SetToolTip(tip);
}

void ACSpinInput::Rescale()
{
    ACButton_inc->Rescale();
    ACButton_dec->Rescale();
    messureSize();
    Refresh();
}

bool ACSpinInput::Enable(bool enable)
{
    text_ctrl->SetEditable(enable);
    bool result = /*text_ctrl->Enable(enable) &&*/ wxWindow::Enable(enable);
    if (result) {
        wxCommandEvent e(EVT_ENABLE_CHANGED);
        e.SetEventObject(this);
        GetEventHandler()->ProcessEvent(e);
        text_ctrl->SetBackgroundColour(background_color.colorForStates(state_handler.states()));
        text_ctrl->SetForegroundColour(text_color.colorForStates(state_handler.states()));
        ACButton_inc->Enable(enable);
        ACButton_dec->Enable(enable);
    }
    return result;
}

// void ACSpinInput::paintEvent(wxPaintEvent& evt)
//{
//     // depending on your system you may need to look at double-buffered dcs
//     wxPaintDC dc(this);
//     render(dc);
// }
//
///*
// * Here we do the actual rendering. I put it in a separate
// * method so that it can work no matter what type of DC
// * (e.g. wxPaintDC or wxClientDC) is used.
// */
// void ACSpinInput::render(wxDC& dc)
//{
//    ACStaticBox::render(dc);
//    int    states = state_handler.states();
//    wxSize size = GetSize();
//    // draw seperator of ACButtons
//    wxPoint pt = ACButton_inc->GetPosition();
//    pt.y = size.y / 2;
//    dc.SetPen(wxPen(border_color.defaultColor()));
//    dc.DrawLine(pt, pt + wxSize{ACButton_inc->GetSize().x - 2, 0});
//    // draw label
//    auto label = GetLabel();
//    if (!label.IsEmpty()) {
//        pt.x = size.x - labelSize.x - 5;
//        pt.y = (size.y - labelSize.y) / 2;
//        dc.SetFont(GetFont());
//        dc.SetTextForeground(label_color.colorForStates(states));
//        dc.DrawText(label, pt);
//    }
//}

void ACSpinInput::messureSize()
{
    wxSize size     = GetSize();
    wxSize textSize = text_ctrl == nullptr ? wxSize(0,0) : text_ctrl->GetSize();
    int    h        = textSize.y + 8;
    if (size.y < h) {
        size.y = h;
        SetSize(size);
        SetMinSize(size);
    } else {
        textSize.y = size.y * 14 / 24;
    }


    wxSize btnSize = {14, (size.y - 6) / 2};
    btnSize.x      = btnSize.x * btnSize.y / 10;
    wxClientDC dc(this);
    labelSize  = dc.GetMultiLineTextExtent(GetLabel());
    textSize.x = size.x - labelSize.x - btnSize.x - 16;
    if (text_ctrl) {
        text_ctrl->SetSize(textSize);
        text_ctrl->SetPosition({6, (size.y - textSize.y) / 2});
    }
    //text_ctrl->SetPosition({6 + btnSize.x, (size.y - textSize.y) / 2});
    if (ACButton_inc && ACButton_dec) {
        ACButton_inc->SetSize(btnSize);
        ACButton_inc->SetPosition({size.x - btnSize.x - 6, size.y / 2 - btnSize.y - 1});
        ACButton_dec->SetSize(btnSize);
        ACButton_dec->SetPosition({size.x - btnSize.x - 6, size.y / 2 + 1});    
    }
    //ACButton_inc->SetPosition({3, size.y / 2 - btnSize.y - 1});
    //ACButton_dec->SetPosition({3, size.y / 2 + 1});
}

ACButton *ACSpinInput::createACButton(bool inc)
{
    wxString btName      = inc ? "arrow-up-nor" : "arrow-down-nor";
    wxString btNameHover = inc ? "arrow-up-hover" : "arrow-down-hover";
    wxString btNameDis   = inc ? "arrow-up-nor" : "arrow-down-nor";
    auto     btn         = new ACButton(this, "", btName, btNameHover, btNameDis, wxBORDER_NONE, wxSize(6, 6));
    btn->SetButtonType(ACButton::AC_BUTTON_UPDOWN);
    btn->setTakeFocusedAsHovered(true);
    btn->SetCornerRadius(2);
    btn->DisableFocusFromKeyboard();
    btn->SetPaddingSize(wxSize(0,0));
    btn->setSizeValid(false, true);
    btn->Bind(wxEVT_LEFT_DOWN, [=](auto &e) {
        delta = inc ? 1 : -1;
        SetValue(val + delta);
        text_ctrl->SetFocus();
        btn->CaptureMouse();
        delta *= 8;
        timer.Start(100);
        sendSpinEvent();
    });
    btn->Bind(wxEVT_LEFT_DCLICK, [=](auto &e) {
        delta = inc ? 1 : -1;
        btn->CaptureMouse();
        SetValue(val + delta);
        sendSpinEvent();
    });
    btn->Bind(wxEVT_LEFT_UP, [=](auto &e) {
        btn->ReleaseMouse();
        timer.Stop();
        text_ctrl->SelectAll();
        delta = 0;
    });
    return btn;
}

void ACSpinInput::onTimer(wxTimerEvent &evnet)
{
    if (delta < -1 || delta > 1) {
        delta /= 2;
        return;
    }
    SetValue(val + delta);
    sendSpinEvent();
}

void ACSpinInput::onTextLostFocus(wxEvent &event)
{
    timer.Stop();
    for (auto *child : GetChildren())
        if (auto btn = dynamic_cast<ACButton *>(child))
            if (btn->HasCapture())
                btn->ReleaseMouse();
    wxCommandEvent e;
    onTextEnter(e);
    // pass to outer
    event.SetId(GetId());
    ProcessEventLocally(event);
    e.Skip();
}

void ACSpinInput::onTextEnter(wxCommandEvent &event)
{
    long value;
    if (!text_ctrl->GetValue().ToLong(&value)) {
        value = val;
    }
    if (text_ctrl->GetValue().Length() <= 0) {
        SetValue(val);
        sendSpinEvent();
    } else {
        if (value != val) {
            SetValue(value);
            sendSpinEvent();
        }
    }
    
    event.SetId(GetId());
    ProcessEventLocally(event);
}

void ACSpinInput::mouseWheelMoved(wxMouseEvent &event)
{
    auto delta = (event.GetWheelRotation() < 0 == event.IsWheelInverted()) ? 1 : -1;
    SetValue(val + delta);
    sendSpinEvent();
    text_ctrl->SetFocus();
}

void ACSpinInput::keyPressed(wxKeyEvent &event)
{
    switch (event.GetKeyCode()) {
    case WXK_UP:
    case WXK_DOWN:
        long value;
        if (!text_ctrl->GetValue().ToLong(&value)) {
            value = val;
        }
        if (event.GetKeyCode() == WXK_DOWN && value > min) {
            --value;
        } else if (event.GetKeyCode() == WXK_UP && value + 1 < max) {
            ++value;
        }
        if (value != val) {
            SetValue(value);
            sendSpinEvent();
        }
        break;
    default: event.Skip(); break;
    }
}

void ACSpinInput::sendSpinEvent()
{
    wxCommandEvent event(wxEVT_SPINCTRL, GetId());
    event.SetEventObject(this);
    GetEventHandler()->ProcessEvent(event);
}
