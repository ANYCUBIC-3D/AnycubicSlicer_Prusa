#include "ACDropDown.hpp"
#include "ACLabel.hpp"

#include <wx/dcgraph.h>
#include <wx/display.h>
#include <wx/dcbuffer.h>

#include "wx/defs.h"
#include "ACDefines.h"
#include "GUI_App.hpp"

namespace Slic3r { namespace GUI {

wxDEFINE_EVENT(EVT_DISMISS, wxCommandEvent);

BEGIN_EVENT_TABLE(ACDropDown, wxPopupTransientWindow)

EVT_LEFT_DOWN(ACDropDown::mouseDown)
EVT_LEFT_UP(ACDropDown::mouseReleased)
EVT_MOUSE_CAPTURE_LOST(ACDropDown::mouseCaptureLost)
EVT_MOTION(ACDropDown::mouseMove)
EVT_MOUSEWHEEL(ACDropDown::mouseWheelMoved)

// catch paint events
EVT_PAINT(ACDropDown::paintEvent)

END_EVENT_TABLE()

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

ACDropDown::ACDropDown(
    wxWindow *parent, std::vector<wxString> &texts, std::vector<const wxBitmapBundle *> &icons, wxSize iconSize, long style)
    : texts(texts)
    , icons(icons)
    , m_iconSize(iconSize)
    , state_handler(this)
    , text_color(std::make_pair(AC_COLOR_DROPDOWN_FG_NOR, (int) ACStateColor::Normal))
    , selector_border_color(std::make_pair(AC_COLOR_DROPDOWN_BD_HOV, (int) ACStateColor::Hovered),
                            std::make_pair(AC_COLOR_DROPDOWN_BD_SEL, (int) ACStateColor::Checked),
                            std::make_pair(AC_COLOR_DROPDOWN_BD_NOR, (int) ACStateColor::Normal))
    , selector_background_color(std::make_pair(AC_COLOR_DROPDOWN_ITEM_BG_HOV, (int) ACStateColor::Hovered),
                                std::make_pair(AC_COLOR_DROPDOWN_ITEM_BG_SEL, (int) ACStateColor::Checked),
                                std::make_pair(AC_COLOR_DROPDOWN_ITEM_BG_NOR, (int) ACStateColor::Normal))
{
    Create(parent, style);
}

void ACDropDown::Create(wxWindow *parent, long style)
{
    wxPopupTransientWindow::Create(parent);
    SetWindowStyle(wxFRAME_SHAPED | wxCLIP_CHILDREN);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    // SetTransparent(true);
    state_handler.attach({/*&border_color, */ &text_color, &selector_border_color, &selector_background_color});
    state_handler.update_binds();
    // if ((style & DD_NO_CHECK_ICON) == 0)
    //     check_bitmap = get_bmp_bundle("checked", 16); //= wxBitmapBundle(this, "checked", 16);
    // text_off = style & DD_NO_TEXT;

    // BBS set default font
    SetFont(ACLabel::Body_13);
// #ifdef __WIN32__
#ifdef __WXOSX__
    // none
    // #else
    // wxPopupTransientWindow releases mouse on idle, which may cause various problems,
    //  such as losting mouse move, and dismissing soon on first LEFT_DOWN event.
    Bind(wxEVT_IDLE, [](wxIdleEvent &evt) { /*std::printf("idle. %d", evt.GetId());*/ });
#endif
}

void ACDropDown::Invalidate(bool clear)
{
    m_sizeValid = false;
    if (clear) {
        selection = hover_item = -1;
        m_offsetY              = 0;
    }

    // need_sync = true;
}

void ACDropDown::SetSelection(int n)
{
    if (n < 0 || n >= texts.size())
        return;

    if (selection == n)
        return;

    selection = n;

    // paintNow();
}

wxString ACDropDown::GetValue() const { return selection >= 0 ? texts[selection] : wxString(); }

void ACDropDown::SetValue(const wxString &value)
{
    auto i    = std::find(texts.begin(), texts.end(), value);
    selection = i == texts.end() ? -1 : std::distance(texts.begin(), i);
    m_sizeValid       = false;
    messureSize();
}

void ACDropDown::SetCornerRadius(double radius)
{
    this->radius = radius;
    Refresh();
}

void ACDropDown::SetBorderColor(ACStateColor const &color)
{
    border_color = color;
    state_handler.update_binds();
    Refresh();
}

void ACDropDown::SetSelectorBorderColor(ACStateColor const &color)
{
    selector_border_color = color;
    state_handler.update_binds();
    Refresh();
}

void ACDropDown::SetTextColor(ACStateColor const &color)
{
    text_color = color;
    state_handler.update_binds();
    Refresh();
}

void ACDropDown::SetSelectorBackgroundColor(ACStateColor const &color)
{
    selector_background_color = color;
    state_handler.update_binds();
    Refresh();
}

void ACDropDown::SetUseContentWidth(bool use)
{
    if (use_content_width == use)
        return;
    use_content_width = use;
    m_sizeValid       = false;
    messureSize();
}
//
// void ACDropDown::SetAlignIcon(bool align)
//{
//    align_icon = align;
//    messureSize();
//}

// void ACDropDown::SetIconSize(wxSize iconSize)
//{
//     this->m_iconSize = iconSize;
//     messureSize();
// }

void ACDropDown::Rescale() { m_sizeValid = false; }

bool ACDropDown::HasDismissLongTime()
{
    auto now = boost::posix_time::microsec_clock::universal_time();
    return !IsShown() && (now - dismissTime).total_milliseconds() >= 200;
}

void ACDropDown::paintEvent(wxPaintEvent &evt)
{
    // depending on your system you may need to look at double-buffered dcs
    wxBufferedPaintDC dc(this);
    render(dc);
}

/*
 * Alternatively, you can use a clientDC to paint on the panel
 * at any time. Using this generally does not free you from
 * catching paint events, since it is possible that e.g. the window
 * manager throws away your drawing when the window comes to the
 * background, and expects you will redraw it when the window comes
 * back (by sending a paint event).
 */
// void ACDropDown::paintNow()
//{
//     // depending on your system you may need to look at double-buffered dcs
//     // wxClientDC dc(this);
//     // render(dc);
//     Refresh();
// }

/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void ACDropDown::render(wxDC &dc)
{
    if (texts.size() == 0)
        return;

    int    states = state_handler.states();
    wxSize size   = GetSize();
    //// blit parent
    // wxWindow* parent = this->GetParent();
    // wxClientDC parentDC(parent);
    // dc.Blit(wxPoint(0,0), GetSize(), &parentDC, wxPoint(0,0));

    // draw background
    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.SetPen(AC_COLOR_BD_BLACK);
    dc.SetBrush(*wxWHITE_BRUSH);

    dc.DrawRectangle(0, 0, size.x, size.y);

    // draw hover rectangle
    // int ofsY = texts.size() > m_maxRows ? m_rowSize.y / 4 : 0;

    wxRect rectRowContent = {{0, m_offsetY + m_padding}, m_rowSize};
    if (hover_item >= 0 && (states & ACStateColor::Hovered)) {
        wxRect rectRowHover = rectRowContent;
        rectRowHover.y += m_rowSize.y * hover_item;
        if (rectRowHover.GetBottom() > 0 && rectRowHover.y < size.y) {
            if (selection == hover_item)
                dc.SetBrush(wxBrush(selector_background_color.colorForStates(states | ACStateColor::Checked)));
            dc.SetPen(wxPen(selector_border_color.colorForStates(states)));
            rectRowHover.Deflate(m_padding, 1);
            dc.DrawRoundedRectangle(rectRowHover, radius);
        }
    }
    // draw checked rectangle
    if (selection >= 0 && (selection != hover_item || (states & ACStateColor::Hovered) == 0)) {
        wxRect rectRowSel = rectRowContent;
        rectRowSel.y += m_rowSize.y * selection;
        if (rectRowSel.GetBottom() > 0 && rectRowSel.y < size.y) {
            dc.SetBrush(wxBrush(selector_background_color.colorForStates(states | ACStateColor::Checked)));
            dc.SetPen(wxPen(selector_border_color.colorForStates(states)));
            rectRowSel.Deflate(m_padding, 1);
            dc.DrawRoundedRectangle(rectRowSel, radius);
        }
    }

    // draw position bar
    if (texts.size() > m_maxRows) {
        int    height = m_rowSize.y * texts.size() + m_padding * 2;
        wxRect rect   = {size.x - m_scrollWidth, -m_offsetY * size.y / height, m_scrollBarWidth, size.y * size.y / height};
        dc.SetPen(wxPen(border_color.defaultColor()));
        dc.SetBrush(wxBrush(*wxLIGHT_GREY));
        dc.DrawRoundedRectangle(rect, 2);
    }

    // rectRowContent.Deflate(m_padding*2, 0);
    //  draw texts & icons
    dc.SetTextForeground(text_color.colorForStates(states));
    for (int i = 0; i < texts.size(); ++i) {
        if (rectRowContent.GetBottom() < 0) {
            rectRowContent.y += m_rowSize.y;
            continue;
        }
        if (rectRowContent.y > size.y)
            break;

        wxPoint pt = rectRowContent.GetLeftTop();
        pt.x += m_spacing * 2;

        if (icons[i] && icons[i]->IsOk()) {
            pt.y = rectRowContent.y + (m_rowSize.y - m_iconSize.y) / 2;
            dc.DrawBitmap(icons[i]->GetBitmap(m_iconSize), pt);
        }

        if (m_iconExist)
            pt.x += m_iconSize.x + m_spacing;

        auto text = texts[i];
        if (!text.IsEmpty()) {
            // wxSize tSize = dc.GetTextExtent(text);
            // if (pt.x + tSize.x > rectRowContent.GetRight()) {
            //     text = wxControl::Ellipsize(text, dc, wxELLIPSIZE_END, rectRowContent.GetRight() - pt.x);
            // }
            pt.y = rectRowContent.y + (m_rowSize.y - m_textSize.y) / 2;
            dc.SetFont(GetFont());
            dc.DrawText(text, pt);
        }

        rectRowContent.y += m_rowSize.y;
    }
}

void ACDropDown::messureSize()
{
    if (m_sizeValid)
        return;

    wxClientDC dc(this);

    m_iconExist = false;
    m_textSize  = wxSize(0, 0);

    for (size_t i = 0; i < texts.size(); ++i) {
        wxSize t_size = dc.GetTextExtent(texts[i]);
        m_textSize.x  = std::max(t_size.x, m_textSize.x);
        m_textSize.y  = std::max(t_size.y, m_textSize.y);
        if (icons[i] && icons[i]->IsOk()) {
            m_iconExist = true;
        }
    }

    wxSize szRowContent = wxSize(4 * m_padding, 2 * m_padding);

    if (m_iconExist) {
        szRowContent.x += m_iconSize.x + m_spacing + m_textSize.x;
        szRowContent.y += std::max(m_iconSize.y, m_textSize.y);
    } else {
        szRowContent += m_textSize;
    }
    m_rowSize   = szRowContent;
    m_rowSize.x = std::max(m_rowSize.x, GetParent() ? GetParent()->GetSize().x - (texts.size() > m_maxRows ? m_scrollWidth : 0) : 0);

    wxSize szContent = m_rowSize;

    if (texts.size() > m_maxRows)
        szContent.x += m_scrollWidth;

    szContent.y *= std::min((size_t) m_maxRows, texts.size());
    szContent.y += 2 * m_padding; //(texts.size() > m_maxRows ? m_rowSize.y / 2 : 0);

    //wxSize curSize = wxWindow::GetSize();
    wxWindow::SetMinSize(szContent);
    wxWindow::SetSize(szContent);

    //if (curSize.x < szContent.x || curSize.y < szContent.y)
    //    wxWindow::SetSize(std::max(curSize.x, szContent.x), std::max(curSize.y, szContent.y));

    m_sizeValid = true;
}

void ACDropDown::autoPosition()
{
    m_sizeValid = false;
    messureSize();

    wxPoint pos  = GetParent()->ClientToScreen(wxPoint(0, 0));
    wxPoint old  = GetPosition();
    wxSize  size = GetSize();

    Position(pos, {0, GetParent()->GetSize().y + m_padding});
}

void ACDropDown::mouseDown(wxMouseEvent &event)
{
    // Receivce unexcepted LEFT_DOWN on Mac after OnDismiss
    if (!IsShown())
        return;
    // force calc hover item again
    mouseMove(event);
    pressedDown = true;
    CaptureMouse();
    dragStart = event.GetPosition();
}

void ACDropDown::mouseReleased(wxMouseEvent &event)
{
    if (pressedDown) {
        dragStart   = wxPoint();
        pressedDown = false;
        if (HasCapture())
            ReleaseMouse();
        if (hover_item >= 0) { // not moved
            int item = hover_item;
            hover_item = -1;
            DismissAndNotify();
            sendDropDownEvent(item);
        }
    }
}

void ACDropDown::mouseCaptureLost(wxMouseCaptureLostEvent &event)
{
    wxMouseEvent evt;
    mouseReleased(evt);
}

void ACDropDown::mouseMove(wxMouseEvent &event)
{
    int     rectHeight = m_padding * 2 + m_rowSize.y * texts.size();
    wxPoint pt         = event.GetPosition();
    if (pressedDown) { // drag
        int posY  = m_offsetY + (pt.y - dragStart.y);
        dragStart = pt;
        if (posY > 0)
            posY = 0;
        else if (posY + rectHeight < GetSize().y)
            posY = GetSize().y - rectHeight;

        if (posY != m_offsetY) {
            m_offsetY  = posY;
            hover_item = -1; // moved
        } else {
            return;
        }
    }

    //
    if (!pressedDown || hover_item >= 0) {
        int hoverIdx = (pt.y - m_padding - m_offsetY) / m_rowSize.y;
        if (hoverIdx >= (int) texts.size())
            hoverIdx = -1;
        if (hoverIdx == hover_item)
            return;
        hover_item = hoverIdx;
        // if (hover >= 0)
        //     SetToolTip(texts[hover]);
    }
    Refresh();
}

void ACDropDown::mouseWheelMoved(wxMouseEvent &event)
{
    if (texts.size() <= m_maxRows)
        return;

    int  rectHeight = m_padding * 2 + m_rowSize.y * texts.size();
    auto delta      = event.GetWheelRotation() > 0 ? m_rowSize.y : -m_rowSize.y;
    int  posY       = m_offsetY + delta;
    if (posY > 0)
        posY = 0;
    else if (posY + rectHeight < GetSize().y)
        posY = GetSize().y - rectHeight;

    if (posY != m_offsetY) {
        m_offsetY = posY;
    } else {
        return;
    }
    int hover = (event.GetPosition().y - m_padding - m_offsetY) / m_rowSize.y;
    if (hover >= (int) texts.size())
        hover = -1;
    if (hover != hover_item) {
        hover_item = hover;
        // if (hover >= 0) SetToolTip(texts[hover]);
    }
    Refresh();
}

// currently unused events
void ACDropDown::sendDropDownEvent(int item)
{
    selection = item;
    wxCommandEvent event(wxEVT_COMBOBOX);
    event.SetEventObject(this);
    event.SetInt(selection);
    event.SetString(GetValue());
    GetEventHandler()->ProcessEvent(event);
}

void ACDropDown::OnDismiss()
{
    dismissTime = boost::posix_time::microsec_clock::universal_time();
    hover_item  = -1;
    wxCommandEvent e(EVT_DISMISS);
    GetEventHandler()->ProcessEvent(e);
}

}} // namespace Slic3r::GUI
