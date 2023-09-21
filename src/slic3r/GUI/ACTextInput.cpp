#include "ACTextInput.hpp"
#include "ACLabel.hpp"

#include <wx/dcgraph.h>
#include "ACDefines.h"

BEGIN_EVENT_TABLE(ACTextInput, ACStaticBox)

EVT_PAINT(ACTextInput::paintEvent)

END_EVENT_TABLE()

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

ACTextInput::ACTextInput()
    : label_color(
        std::make_pair(AC_COLOR_TEXTINPUT_FG_DIS, (int) ACStateColor::Disabled), 
        std::make_pair(AC_COLOR_TEXTINPUT_FG_HOV, (int) ACStateColor::Hovered),
        std::make_pair(AC_COLOR_TEXTINPUT_FG_NOR, (int) ACStateColor::Normal)
    ) , text_color(             
        std::make_pair(AC_COLOR_TEXTINPUT_FG_DIS, (int) ACStateColor::Disabled), 
        std::make_pair(AC_COLOR_TEXTINPUT_FG_HOV, (int) ACStateColor::Hovered),
        std::make_pair(AC_COLOR_TEXTINPUT_FG_NOR, (int) ACStateColor::Normal)
    )
{
    radius = 0;
    border_width = 1;
    border_color = ACStateColor(std::make_pair(AC_COLOR_TEXTINPUT_BD_DIS, (int) ACStateColor::Disabled)
                              , std::make_pair(AC_COLOR_TEXTINPUT_BD_HOV, (int) ACStateColor::Hovered)
                              , std::make_pair(AC_COLOR_TEXTINPUT_BD_NOR, (int) ACStateColor::Normal));
    background_color = ACStateColor(std::make_pair(AC_COLOR_TEXTINPUT_BG_DIS, (int) ACStateColor::Disabled)
                                  , std::make_pair(AC_COLOR_TEXTINPUT_BG_HOV, (int) ACStateColor::Hovered)
                                  , std::make_pair(AC_COLOR_TEXTINPUT_BG_NOR, (int) ACStateColor::Normal));
    SetFont(ACLabel::Body_13);
}

ACTextInput::ACTextInput(wxWindow *     parent,
                     wxString       text,
                     wxString       label,
                     wxString       iconName,
                     wxSize         iconSize,
                     const wxPoint &pos,
                     const wxSize & size,
                     long           style)
    : ACTextInput()
{
    Create(parent, text, label, iconName, iconSize, pos, size, style);
}

void ACTextInput::Create(wxWindow *     parent,
                       wxString       text,
                       wxString       label,
                       wxString       iconName,
                       wxSize         iconSize,
                       const wxPoint &pos,
                       const wxSize & size,
                       long           style)
{
    //m_itemMinSize = size;

    m_iconSize = iconSize;
    ACStaticBox::Create(parent, wxID_ANY, pos, size, style);
    wxWindow::SetLabel(label);
    style &= ~wxRIGHT;
    state_handler.attach({&label_color, & text_color});
    state_handler.update_binds();
    text_ctrl = new wxTextCtrl(this, wxID_ANY, text, {m_padding, m_padding}, wxDefaultSize, style | wxBORDER_NONE | wxTE_PROCESS_ENTER);
    text_ctrl->SetFont(ACLabel::Body_13);
    text_ctrl->SetInitialSize(text_ctrl->GetBestSize());
    text_ctrl->SetBackgroundColour(background_color.colorForStates(state_handler.states()));
    text_ctrl->SetForegroundColour(text_color.colorForStates(state_handler.states()));
    state_handler.attach_child(text_ctrl);
    text_ctrl->Bind(wxEVT_KILL_FOCUS, [this](auto &e) {
        OnEdit();
        e.SetId(GetId());
        ProcessEventLocally(e);
        e.Skip();
    });
    text_ctrl->Bind(wxEVT_SET_FOCUS, [this](auto &e) {
        OnEdit();
        e.SetId(GetId());
        ProcessEventLocally(e);
    });
    text_ctrl->Bind(wxEVT_TEXT_ENTER, [this](auto &e) {
        OnEdit();
        e.SetId(GetId());
        ProcessEventLocally(e);
    });
    text_ctrl->Bind(wxEVT_RIGHT_DOWN, [this](auto &e) {
    }); // disable context menu
    
    if (!iconName.IsEmpty()) {
        this->m_icon = ScalableBitmap(this, m_iconSize, iconName.ToStdString());
    }
    messureSize();
}


void ACTextInput::SetCornerRadius(double radius)
{
    this->radius = radius;
    Refresh();
}

void ACTextInput::SetLabel(const wxString& label)
{
    wxWindow::SetLabel(label);
    text_ctrl->SetLabel(label);
    messureSize();
    Refresh();
}

void ACTextInput::SetIcon(const wxString& iconName, wxSize iconSize)
{
    wxASSERT(iconName.IsEmpty() == false);

    m_iconSize = iconSize;
    this->m_icon = ScalableBitmap(this, m_iconSize, iconName.ToStdString());
    messureSize();
    Rescale();
}

void ACTextInput::SetLabelColor(ACStateColor const &color)
{
    label_color = color;
    state_handler.update_binds();
}

void ACTextInput::SetTextColor(ACStateColor const& color)
{
    text_color= color;
    state_handler.update_binds();
}

void ACTextInput::Rescale()
{
    //if (!this->m_icon.name().empty())
    //    this->m_icon.msw_rescale();
    messureSize();
    Refresh();
}

bool ACTextInput::Enable(bool enable)
{
    bool result = /*text_ctrl->Enable(enable) && */wxWindow::Enable(enable);
    if (result) {
        wxCommandEvent e(EVT_ENABLE_CHANGED);
        e.SetEventObject(this);
        GetEventHandler()->ProcessEvent(e);
        wxColour disColor = background_color.colorForStates(state_handler.states());
        wxColour texColor = text_color.colorForStates(state_handler.states());


        text_ctrl->SetBackgroundColour(disColor);
        text_ctrl->SetForegroundColour(texColor);
    }
    return result;
}
    
//void ACTextInput::setItemSize(const wxSize& itemSize)
//{
//    m_itemMinSize = itemSize;
//
//
//
//    messureSize();
//    Refresh();
//}

void ACTextInput::SetMinSize(const wxSize& size)
{
    wxSize size2 = size;
    if (size2.y < 0) {
#ifdef __WXMAC__
        if (GetPeer()) // peer is not ready in Create on mac
#endif
        size2.y = GetSize().y;
    }
    wxWindow::SetMinSize(size2);
}

void ACTextInput::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
    wxWindow::DoSetSize(x, y, width, height, sizeFlags);
 
    if (text_ctrl) {
        int dipPadding = FromDIP(m_padding);
        int dipSpacing = FromDIP(m_spacing);
        wxSize size = GetSize();
        wxPoint textPos = {dipPadding, 0};
        wxSize iconSize = m_icon.get_bitmap().IsOk() ? m_iconSize : wxSize(0,0);

        wxSize textSize = text_ctrl->GetSize();
        textSize.x = size.x - dipPadding*2 - (iconSize.x == 0 ? 0 : (m_spacing + iconSize.x));
        text_ctrl->SetSize(textSize);
        text_ctrl->SetPosition({textPos.x, (size.y - textSize.y) / 2});
    }
}

void ACTextInput::DoSetToolTipText(wxString const &tip)
{
    wxWindow::DoSetToolTipText(tip);
    text_ctrl->SetToolTip(tip);
}

void ACTextInput::paintEvent(wxPaintEvent &evt)
{
    // depending on your system you may need to look at double-buffered dcs
    wxPaintDC dc(this);
    render(dc);
}

/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void ACTextInput::render(wxPaintDC& dc)
{
    ACStaticBox::render(dc);

    dc.SetBackground(*wxTRANSPARENT_BRUSH);

    int dipPadding = FromDIP(m_padding);
    //float dipSpacing = FromDIP(m_spacing);
    int states = state_handler.states();
    wxSize size = GetSize();

    wxRect rcContent = { {0, 0}, size };
    rcContent.Deflate(dipPadding);



    //bool   align_right = GetWindowStyle() & wxRIGHT;
    // start draw
    wxPoint pt = {dipPadding, 0};

    auto text = wxWindow::GetLabel();
    if (!text.IsEmpty()) {
        int textValidSpace = rcContent.GetWidth() - (m_icon.get_bitmap().IsOk() ? (m_iconSize.x+m_spacing) : 0);
        if (m_labelSize.x > textValidSpace)
            text = wxControl::Ellipsize(text, dc, wxELLIPSIZE_END, textValidSpace);

        pt.y = (size.y - m_labelSize.y) / 2;
        dc.SetTextForeground(label_color.colorForStates(states));
        dc.SetFont(GetFont());
        dc.DrawText(text, pt);
    }

    if (m_icon.get_bitmap().IsOk()) {
        wxBitmap bmp = m_icon.bmp().GetBitmap(m_iconSize);
        pt.x = size.x - dipPadding - m_iconSize.x; 
        pt.y = (size.y - m_iconSize.y) / 2;
        dc.DrawBitmap(bmp, pt);
    }
}

void ACTextInput::messureSize()
{
    wxSize oldSize = GetSize();

    wxClientDC dc(this);

    float dipPadding = FromDIP(m_padding);
    float dipSpacing = FromDIP(m_spacing);

    m_labelSize = dc.GetTextExtent(wxWindow::GetLabel().IsEmpty() ? "place_hold" : wxWindow::GetLabel());

    wxSize minIconSize(0,0);
    if (m_icon.get_bitmap().IsOk()) {
        minIconSize = m_iconSize;
    }

    wxSize curMinSize = GetMinSize();
    wxSize minSize = curMinSize;

    if (curMinSize.x <= 0)
        minSize.x = m_labelSize.x + 2*dipPadding + (minIconSize.x == 0 ? 0 : (dipSpacing + minIconSize.x));
    if (curMinSize.y <= 0)
        minSize.y = 2*dipPadding + std::max(m_labelSize.y, minIconSize.y);

    SetMinSize(minSize);

    if (oldSize.x < minSize.x || oldSize.y < minSize.y) {
        wxSize newSize = wxSize(std::max(oldSize.x, minSize.x), std::max(oldSize.y, minSize.y));
        SetSize(newSize);

        //if (text_ctrl) {
        //    text_ctrl->SetPosition(wxPoint(m_padding, (newSize.y - m_labelSize.y)/2));
        //    text_ctrl->SetSize(wxSize(newSize.x - 2*m_padding - (minIconSize.x == 0 ? 0 : (dipSpacing + minIconSize.x)), m_labelSize.y));
        //}
    }

}
