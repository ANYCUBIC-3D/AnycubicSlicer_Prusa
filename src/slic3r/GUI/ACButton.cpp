#include "ACButton.hpp"
#include "ACStateColor.hpp"
#include "ACLabel.hpp"
#include "ACStateHandler.hpp"

#include <wx/dcgraph.h>

#include "ACDefines.h"

BEGIN_EVENT_TABLE(ACButton, ACStaticBox)

EVT_LEFT_DOWN(ACButton::mouseDown)
EVT_LEFT_UP(ACButton::mouseReleased)
EVT_MOUSE_CAPTURE_LOST(ACButton::mouseCaptureLost)
EVT_KEY_DOWN(ACButton::keyDownUp)
EVT_KEY_UP(ACButton::keyDownUp)

//// catch paint events
//EVT_PAINT(ACButton::paintEvent)

END_EVENT_TABLE()

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

//ACButton::ACButton()
//    : m_paddingSize(10, 10)
//    , m_minSize(-1, -1)
//{
//    //background_color = ACStateColor(
//    //    std::make_pair(0xF0F0F0, (int) ACStateColor::Disabled),
//    //    std::make_pair(0x37EE7C, (int) ACStateColor::Hovered | ACStateColor::Checked),
//    //    std::make_pair(0x00AE42, (int) ACStateColor::Checked),
//    //    std::make_pair(*wxLIGHT_GREY, (int) ACStateColor::Hovered), 
//    //    std::make_pair(*wxWHITE, (int) ACStateColor::Normal));
//    //text_color       = ACStateColor(
//    //    std::make_pair(*wxLIGHT_GREY, (int) ACStateColor::Disabled), 
//    //    std::make_pair(*wxBLACK, (int) ACStateColor::Normal));
//}
static int acButtonCounter = 0;

ACButton::~ACButton()
{
    //pressedDown = false;
    //state_handler.clearAttach();
    //state_handler.update_binds();
    printf("ACButton Desdroyed.... %d \n", --acButtonCounter);
}


ACButton::ACButton(wxWindow* parent, wxString text, wxString icon, wxString hover_icon, wxString dis_icon, long style, wxSize iconSize)
    : m_paddingSize(10, 10)
    , m_minSize(-1, -1)
{
    acButtonCounter++;
    Create(parent, text, icon,hover_icon, dis_icon, style, iconSize);
}

bool ACButton::Create(wxWindow* parent, wxString text, wxString icon, wxString hover_icon, wxString dis_icon, long style, wxSize iconSize)
{
    ACStaticBox::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);

    m_iconSize = iconSize;
    if ((style & (int)AC_ALIGN_LEFT) == AC_ALIGN_LEFT)
        m_alignCenter = false;

    if ((style & (int)wxVERTICAL) == wxVERTICAL)
        m_isHorizontal = false;

    state_handler.attach({&text_color});
    state_handler.update_binds();

    //BBS set default font
    SetFont(ACLabel::Body_14);

    wxWindow::SetLabel(text);

    SetIcon(icon);
    SetInactiveIcon(dis_icon.IsEmpty() ? icon : dis_icon);
    SetHoverIcon(hover_icon.IsEmpty() ? icon : hover_icon);

    setupCheckImg();

    m_sizeValid = false;
    messureSize();
    Refresh();
    return true;
}

void ACButton::setupCheckImg()
{
    m_ckBoxIconSize = wxSize(16,16);

    m_checkOnImg        = ScalableBitmap(this, m_ckBoxIconSize, "checkbox-on-nor" );
    m_checkOnImgHover   = ScalableBitmap(this, m_ckBoxIconSize, "checkbox-on-focused" );
    m_checkOnImgDis     = ScalableBitmap(this, m_ckBoxIconSize, "checkbox-on-disable" );
    m_checkOffImg       = ScalableBitmap(this, m_ckBoxIconSize, "checkbox-off-nor" );
    m_checkOffImgHover  = ScalableBitmap(this, m_ckBoxIconSize, "checkbox-off-focused" );
    m_checkOffImgDis    = ScalableBitmap(this, m_ckBoxIconSize, "checkbox-off-disable" );
    m_checkHalfImg      = ScalableBitmap(this, m_ckBoxIconSize, "checkbox-half_on-nor" );
    m_checkHalfImgHover = ScalableBitmap(this, m_ckBoxIconSize, "checkbox-half_on-focused" );
    m_checkHalfImgDis   = ScalableBitmap(this, m_ckBoxIconSize, "checkbox-half_on-disable" );
}

void ACButton::SetLabel(const wxString& label)
{
    wxWindow::SetLabel(label);
    m_sizeValid = false;
    Refresh();
}

void ACButton::SetIcon(const wxString& iconName)
{
    if (!iconName.IsEmpty()) {
        //BBS set button icon default size to 20
        this->active_icon = ScalableBitmap(this, m_iconSize, iconName.ToStdString());
    }
    else
    {
        this->active_icon = ScalableBitmap();
    }
    m_sizeValid = false;
    Refresh();
}

void ACButton::SetInactiveIcon(const wxString &iconName)
{
    if (!iconName.IsEmpty()) {
        // BBS set button icon default size to 20
        this->inactive_icon = ScalableBitmap(this, m_iconSize, iconName.ToStdString());
    } else {
        this->inactive_icon = ScalableBitmap();
    }
    m_sizeValid = false;
    Refresh();
}

void ACButton::SetHoverIcon(const wxString &iconName)
{
    if (!iconName.IsEmpty()) {
        // BBS set button icon default size to 20
        this->hover_icon = ScalableBitmap(this, m_iconSize, iconName.ToStdString());
    } else {
        this->hover_icon = ScalableBitmap();
    }
    m_sizeValid = false;
    Refresh();
}

void ACButton::SetMinSize(const wxSize& size)
{
    m_minSize = size;
    m_sizeValid = false;
    Refresh();
}

void ACButton::SetPaddingSize(const wxSize& size)
{
    m_paddingSize = size;
    m_sizeValid = false;
    Refresh();
}
    
void ACButton::SetSpacing(const int& value)
{
    m_spacing = value;
    m_sizeValid = false;
    Refresh();
}


void ACButton::SetButtonType(ACButton::AC_BUTTON_TYPE type)
{
    ACStateColor bgColor;
    ACStateColor bdColor;
    ACStateColor fgColor;

    switch (type)
    {
    case ACButton::AC_BUTTON_LV0:
        bgColor.append(AC_COLOR_BT_L0_BG_DIS, ACStateColor::Disabled);
        bgColor.append(AC_COLOR_BT_L0_BG_PRE, ACStateColor::Pressed);
        bgColor.append(AC_COLOR_BT_L0_BG_HOV, ACStateColor::Hovered);
        bgColor.append(AC_COLOR_BT_L0_BG_NOR, ACStateColor::Normal);

        fgColor.append(AC_COLOR_BT_L0_FG_DIS, ACStateColor::Disabled);
        fgColor.append(AC_COLOR_BT_L0_FG_PRE, ACStateColor::Pressed);
        fgColor.append(AC_COLOR_BT_L0_FG_HOV, ACStateColor::Hovered);
        fgColor.append(AC_COLOR_BT_L0_FG_NOR, ACStateColor::Normal);
        break;
    case ACButton::AC_BUTTON_LV1:
        bgColor.append(AC_COLOR_BT_L1_BG_DIS, ACStateColor::Disabled);
        bgColor.append(AC_COLOR_BT_L1_BG_PRE, ACStateColor::Pressed);
        bgColor.append(AC_COLOR_BT_L1_BG_HOV, ACStateColor::Hovered);
        bgColor.append(AC_COLOR_BT_L1_BG_NOR, ACStateColor::Normal);

        bdColor.append(AC_COLOR_BT_L1_BD_DIS, ACStateColor::Disabled);
        bdColor.append(AC_COLOR_BT_L1_BD_PRE, ACStateColor::Pressed);
        bdColor.append(AC_COLOR_BT_L1_BD_HOV, ACStateColor::Hovered);
        bdColor.append(AC_COLOR_BT_L1_BD_NOR, ACStateColor::Normal);

        fgColor.append(AC_COLOR_BT_L1_FG_DIS, ACStateColor::Disabled);        
        fgColor.append(AC_COLOR_BT_L1_FG_PRE, ACStateColor::Pressed);
        fgColor.append(AC_COLOR_BT_L1_FG_HOV, ACStateColor::Hovered);
        fgColor.append(AC_COLOR_BT_L1_FG_NOR, ACStateColor::Normal);
        break;
    case ACButton::AC_BUTTON_LV2:
        bgColor.append(AC_COLOR_BT_L2_BG_DIS, ACStateColor::Disabled);
        bgColor.append(AC_COLOR_BT_L2_BG_PRE, ACStateColor::Pressed);
        bgColor.append(AC_COLOR_BT_L2_BG_HOV, ACStateColor::Hovered);
        bgColor.append(AC_COLOR_BT_L2_BG_NOR, ACStateColor::Normal);
        bdColor.append(AC_COLOR_BT_L2_BD_DIS, ACStateColor::Disabled);
        bdColor.append(AC_COLOR_BT_L2_BD_PRE, ACStateColor::Pressed);
        bdColor.append(AC_COLOR_BT_L2_BD_HOV, ACStateColor::Hovered);
        bdColor.append(AC_COLOR_BT_L2_BD_NOR, ACStateColor::Normal);
        fgColor.append(AC_COLOR_BT_L2_FG_DIS, ACStateColor::Disabled);        
        fgColor.append(AC_COLOR_BT_L2_FG_PRE, ACStateColor::Pressed);
        fgColor.append(AC_COLOR_BT_L2_FG_HOV, ACStateColor::Hovered);
        fgColor.append(AC_COLOR_BT_L2_FG_NOR, ACStateColor::Normal);
        break;
    case ACButton::AC_BUTTON_LV3:
        bgColor.append(AC_COLOR_BT_L3_BG_DIS, ACStateColor::Disabled);
        bgColor.append(AC_COLOR_BT_L3_BG_PRE, ACStateColor::Pressed);
        bgColor.append(AC_COLOR_BT_L3_BG_HOV, ACStateColor::Hovered);
        bgColor.append(AC_COLOR_BT_L3_BG_NOR, ACStateColor::Normal);
        fgColor.append(AC_COLOR_BT_L3_FG_DIS, ACStateColor::Disabled);        
        fgColor.append(AC_COLOR_BT_L3_FG_PRE, ACStateColor::Pressed);
        fgColor.append(AC_COLOR_BT_L3_FG_HOV, ACStateColor::Hovered);
        fgColor.append(AC_COLOR_BT_L3_FG_NOR, ACStateColor::Normal);
        break;
    case ACButton::AC_BUTTON_LV0_N:
        //bgColor.append(AC_COLOR_BT_L0N_BG_DIS, ACStateColor::Disabled);
        //bgColor.append(AC_COLOR_BT_L0N_BG_PRE, ACStateColor::Pressed);
        //bgColor.append(AC_COLOR_BT_L0N_BG_HOV, ACStateColor::Hovered);
        //bgColor.append(AC_COLOR_BT_L0N_BG_NOR, ACStateColor::Normal );
        //bdColor.append(AC_COLOR_BT_L0N_BD_DIS, ACStateColor::Disabled);
        //bdColor.append(AC_COLOR_BT_L0N_BD_PRE, ACStateColor::Pressed);
        //bdColor.append(AC_COLOR_BT_L0N_BD_HOV, ACStateColor::Hovered);
        //bdColor.append(AC_COLOR_BT_L0N_BD_NOR, ACStateColor::Normal );
        fgColor.append(AC_COLOR_BT_L0N_FG_DIS, ACStateColor::Disabled);        
        fgColor.append(AC_COLOR_BT_L0N_FG_PRE, ACStateColor::Pressed);
        fgColor.append(AC_COLOR_BT_L0N_FG_HOV, ACStateColor::Hovered);
        fgColor.append(AC_COLOR_BT_L0N_FG_NOR, ACStateColor::Normal );
        break;
    case ACButton::AC_BUTTON_ICON:
        background_color.clear();

        bdColor.append(AC_COLOR_BT_L2_BD_DIS, ACStateColor::Disabled);
        bdColor.append(AC_COLOR_BT_L2_BD_PRE, ACStateColor::Pressed);
        bdColor.append(AC_COLOR_BT_L2_BD_HOV, ACStateColor::Hovered);
        bdColor.append(AC_COLOR_BT_L2_BD_NOR, ACStateColor::Normal);
        break;
    case ACButton::AC_BUTTON_LIST_ITEM_L0:
        bgColor.append(AC_COLOR_MLIST_ITEM_L0_BG_PRE, ACStateColor::Pressed);
        bgColor.append(AC_COLOR_MLIST_ITEM_L0_BG_HOV, ACStateColor::Hovered);
        bgColor.append(AC_COLOR_MLIST_ITEM_L0_BG_SEL, ACStateColor::Checked);
        bgColor.append(AC_COLOR_MLIST_ITEM_L0_BG_NOR, ACStateColor::Normal );

        fgColor.append(AC_COLOR_MLIST_ITEM_L0_FG_SEL, ACStateColor::Disabled);        
        fgColor.append(AC_COLOR_MLIST_ITEM_L0_FG_PRE, ACStateColor::Pressed);
        fgColor.append(AC_COLOR_MLIST_ITEM_L0_FG_HOV, ACStateColor::Hovered);
        //fgColor.append(AC_COLOR_MLIST_ITEM_L0_FG_PRE, ACStateColor::Checked);
        fgColor.append(AC_COLOR_MLIST_ITEM_L0_FG_NOR, ACStateColor::Normal);
        break;
    case ACButton::AC_BUTTON_LIST_ITEM_L1:
        bgColor.append(AC_COLOR_MLIST_ITEM_L1_BG_PRE, ACStateColor::Pressed);
        bgColor.append(AC_COLOR_MLIST_ITEM_L1_BG_HOV, ACStateColor::Hovered);
        bgColor.append(AC_COLOR_MLIST_ITEM_L1_BG_SEL, ACStateColor::Checked);
        bgColor.append(AC_COLOR_MLIST_ITEM_L1_BG_NOR, ACStateColor::Normal );

        fgColor.append(AC_COLOR_MLIST_ITEM_L1_FG_PRE, ACStateColor::Pressed);
        fgColor.append(AC_COLOR_MLIST_ITEM_L1_FG_HOV, ACStateColor::Hovered);
        fgColor.append(AC_COLOR_MLIST_ITEM_L1_FG_SEL, ACStateColor::Checked);        
        fgColor.append(AC_COLOR_MLIST_ITEM_L1_FG_NOR, ACStateColor::Normal);
        break;
    case ACButton::AC_BUTTON_LIST_ITEM_L2:
        bgColor.append(AC_COLOR_MLIST_ITEM_L2_BG_PRE, ACStateColor::Pressed);
        bgColor.append(AC_COLOR_MLIST_ITEM_L2_BG_HOV, ACStateColor::Hovered);
        bgColor.append(AC_COLOR_MLIST_ITEM_L2_BG_SEL, ACStateColor::Checked);
        bgColor.append(AC_COLOR_MLIST_ITEM_L2_BG_NOR, ACStateColor::Normal );

        fgColor.append(AC_COLOR_MLIST_ITEM_L2_FG_PRE, ACStateColor::Pressed);
        fgColor.append(AC_COLOR_MLIST_ITEM_L2_FG_HOV, ACStateColor::Hovered);
        fgColor.append(AC_COLOR_MLIST_ITEM_L2_FG_SEL, ACStateColor::Checked);        
        fgColor.append(AC_COLOR_MLIST_ITEM_L2_FG_NOR, ACStateColor::Normal);
        break;
    case ACButton::AC_BUTTON_TABLE_ITEM:
        bgColor.append(AC_COLOR_TAB_ITEM_BG_SEL, ACStateColor::Checked);
        bgColor.append(AC_COLOR_TAB_ITEM_BG_NOR, ACStateColor::Normal );

        fgColor.append(AC_COLOR_TAB_ITEM_FG_PRE, ACStateColor::Pressed);
        fgColor.append(AC_COLOR_TAB_ITEM_FG_HOV, ACStateColor::Hovered);
        fgColor.append(AC_COLOR_TAB_ITEM_FG_SEL, ACStateColor::Checked);        
        fgColor.append(AC_COLOR_TAB_ITEM_FG_NOR, ACStateColor::Normal );
        break;
    case ACButton::AC_BUTTON_DROPDOWM_ITEM:
        bgColor.append(AC_COLOR_DROPDOWN_BG_DIS, ACStateColor::Disabled);
        bgColor.append(AC_COLOR_DROPDOWN_BG_PRE, ACStateColor::Pressed );
        bgColor.append(AC_COLOR_DROPDOWN_BG_HOV, ACStateColor::Hovered );
        bgColor.append(AC_COLOR_DROPDOWN_BG_SEL, ACStateColor::Checked );
        bgColor.append(AC_COLOR_DROPDOWN_BG_NOR, ACStateColor::Normal  );

        bdColor.append(AC_COLOR_DROPDOWN_BD_DIS, ACStateColor::Disabled);
        bdColor.append(AC_COLOR_DROPDOWN_BD_PRE, ACStateColor::Pressed );
        bdColor.append(AC_COLOR_DROPDOWN_BD_HOV, ACStateColor::Hovered );
        bdColor.append(AC_COLOR_DROPDOWN_BD_SEL, ACStateColor::Checked );
        bdColor.append(AC_COLOR_DROPDOWN_BD_NOR, ACStateColor::Normal  );

        fgColor.append(AC_COLOR_DROPDOWN_FG_DIS, ACStateColor::Disabled);        
        fgColor.append(AC_COLOR_DROPDOWN_FG_PRE, ACStateColor::Pressed );
        fgColor.append(AC_COLOR_DROPDOWN_FG_HOV, ACStateColor::Hovered );
        fgColor.append(AC_COLOR_DROPDOWN_FG_SEL, ACStateColor::Checked );
        fgColor.append(AC_COLOR_DROPDOWN_FG_NOR, ACStateColor::Normal  );
        break;
    case ACButton::AC_BUTTON_CHECK_L0:
        bgColor.append(AC_COLOR_BT_CHECK_L0_BG_CHK, ACStateColor::Checked );
        bgColor.append(AC_COLOR_BT_CHECK_L0_BG_NOR, ACStateColor::Normal  );
        fgColor.append(AC_COLOR_BT_CHECK_L0_FG_CHK, ACStateColor::Checked );
        fgColor.append(AC_COLOR_BT_CHECK_L0_FG_NOR, ACStateColor::Normal  );
    case ACButton::AC_BUTTON_CHECK_L1:
        bgColor.append(AC_COLOR_BT_CHECK_L1_BG_CHK, ACStateColor::Checked );
        bgColor.append(AC_COLOR_BT_CHECK_L1_BG_NOR, ACStateColor::Normal  );
        bdColor.append(AC_COLOR_BT_CHECK_L1_BD_CHK, ACStateColor::Checked );
        bdColor.append(AC_COLOR_BT_CHECK_L1_BD_NOR, ACStateColor::Normal  );
        fgColor.append(AC_COLOR_BT_CHECK_L1_FG_CHK, ACStateColor::Checked );
        fgColor.append(AC_COLOR_BT_CHECK_L1_FG_NOR, ACStateColor::Normal  );
        break;
    case ACButton::AC_BUTTON_CHECK_IMG:
        SetBackgroundColour(AC_COLOR_SEL_IMG_BG);
        bgColor.clear();

        bdColor.append(AC_COLOR_SEL_IMG_BD_DIS, ACStateColor::Disabled);
        //bdColor.append(AC_COLOR_SEL_IMG_BD_PRE, ACStateColor::Pressed );
        bdColor.append(AC_COLOR_SEL_IMG_BD_HOV, ACStateColor::Hovered );
        bdColor.append(AC_COLOR_SEL_IMG_BD_SEL, ACStateColor::Checked );
        bdColor.append(AC_COLOR_SEL_IMG_BD_NOR, ACStateColor::Normal  );

        fgColor.append(AC_COLOR_SEL_IMG_FG_DIS, ACStateColor::Disabled);        
        //fgColor.append(AC_COLOR_SEL_IMG_FG_PRE, ACStateColor::Pressed );
        fgColor.append(AC_COLOR_SEL_IMG_FG_HOV, ACStateColor::Hovered );
        fgColor.append(AC_COLOR_SEL_IMG_FG_SEL, ACStateColor::Checked );
        fgColor.append(AC_COLOR_SEL_IMG_FG_NOR, ACStateColor::Normal  );
        break;
    case ACButton::AC_BUTTON_SEL:
        bgColor.append(AC_COLOR_BT_SEL_BG_DIS, ACStateColor::Disabled);
        bgColor.append(AC_COLOR_BT_SEL_BG_PRE, ACStateColor::Pressed);
        bgColor.append(AC_COLOR_BT_SEL_BG_HOV, ACStateColor::Hovered);
        bgColor.append(AC_COLOR_BT_SEL_BG_NOR, ACStateColor::Normal );
        bdColor.append(AC_COLOR_BT_SEL_BD_DIS, ACStateColor::Disabled);
        bdColor.append(AC_COLOR_BT_SEL_BD_PRE, ACStateColor::Pressed);
        bdColor.append(AC_COLOR_BT_SEL_BD_HOV, ACStateColor::Hovered);
        bdColor.append(AC_COLOR_BT_SEL_BD_NOR, ACStateColor::Normal );
        fgColor.append(AC_COLOR_BT_SEL_FG_DIS, ACStateColor::Disabled);        
        fgColor.append(AC_COLOR_BT_SEL_FG_PRE, ACStateColor::Pressed);
        fgColor.append(AC_COLOR_BT_SEL_FG_HOV, ACStateColor::Hovered);
        fgColor.append(AC_COLOR_BT_SEL_FG_NOR, ACStateColor::Normal );
        break;
    case ACButton::AC_BUTTON_LABEL:
        bgColor.append(AC_COLOR_WHITE, ACStateColor::Normal );
        fgColor.append(AC_COLOR_BLACK, ACStateColor::Normal );
        break;
    case ACButton::AC_BUTTON_LABEL_2:
        fgColor.append(AC_COLOR_BLACK, ACStateColor::Normal );
        break;
    case ACButton::AC_BUTTON_UPDOWN:
        bgColor.append(AC_COLOR_WHITE, ACStateColor::Disabled);
        bgColor.append(AC_COLOR_MAIN_BLUE_TRANS_TWO, ACStateColor::Pressed);
        bgColor.append(AC_COLOR_MAIN_BLUE_TRANS_HOVER, ACStateColor::Hovered);
        bgColor.append(AC_COLOR_WHITE, ACStateColor::Normal);

        fgColor.append(AC_COLOR_WHITE, ACStateColor::Disabled);
        fgColor.append(AC_COLOR_MAIN_BLUE_TRANS_TWO, ACStateColor::Pressed);
        fgColor.append(AC_COLOR_MAIN_BLUE_TRANS_HOVER, ACStateColor::Hovered);
        fgColor.append(AC_COLOR_WHITE, ACStateColor::Normal);
        break;
    default:
        break;
    }
    SetBackgroundColor(bgColor);
    SetBorderColor(bdColor);
    SetTextColor(fgColor);

    Refresh();
}

void ACButton::SetTextColor(ACStateColor const& color)
{
    text_color = color;
    state_handler.update_binds();
    Refresh();
}

void ACButton::SetTextColorNormal(wxColor const &color)
{
    text_color.setColorForStates(color, 0);
    Refresh();
}

void ACButton::SetCheckStyle(AC_BUTTON_CHECK_STYLE checkStyle) 
{
    m_checkStyle = checkStyle; 
    m_sizeValid = false;
    Refresh(); 
}

void ACButton::SetChecked(AC_BUTTON_CHECK_STATE state, bool sendEvents) 
{
    if (m_checkState == state)
        return;

    m_checkState = state;

    wxCommandEvent ckEvent(wxEVT_CHECKBOX, GetId());
    ckEvent.SetEventObject(this);
    ckEvent.SetInt(m_checkState);
    if (sendEvents)
        GetEventHandler()->ProcessEvent(ckEvent);
    else
        this->state_handler.changed(ckEvent);

    Refresh();
}

void ACButton::SetChecked(bool selected, bool sendEvents) 
{ 
    AC_BUTTON_CHECK_STATE state = selected ? CHECKSTATE_ON : CHECKSTATE_OFF;

    SetChecked(state, sendEvents);
}
void ACButton::SetHalfChecked(bool sendEvents) 
{ 
    SetChecked(CHECKSTATE_ON_HALF, sendEvents);
}

bool ACButton::Enable(bool enable)
{
    bool result = wxWindow::Enable(enable);
    if (result) {
        wxCommandEvent e(EVT_ENABLE_CHANGED);
        e.SetEventObject(this);
        e.SetInt(enable);
        GetEventHandler()->ProcessEvent(e);
    }
    return result;
}

bool ACButton::SetEnable(bool enable) 
{
    return Enable(enable);
}

void ACButton::SetCanFocus(bool canFocus) { this->canFocus = canFocus; }

void ACButton::Rescale()
{
    //if (this->active_icon.get_bitmap().IsOk())
    //    this->active_icon.msw_rescale();

    //if (this->inactive_icon.get_bitmap().IsOk())
    //    this->inactive_icon.msw_rescale();

    m_sizeValid = false;

    Refresh();
}

void ACButton::clearColor()
{
    text_color.clear();
    background_color.clear();
    background_color2.clear();
    border_color.clear();

    state_handler.update_binds();
    Refresh();
}
void ACButton::setTakeFocusedAsHovered(bool as)
{
    text_color       .setTakeFocusedAsHovered(as);
    background_color .setTakeFocusedAsHovered(as);
    background_color2.setTakeFocusedAsHovered(as);
    border_color     .setTakeFocusedAsHovered(as);
    Refresh();
}

void ACButton::sys_color_changed()
{
    Rescale();
}

void ACButton::setDrawCircle(bool canShow) 
{ 
    m_draw_circle = canShow; 
    m_sizeValid = false;
    Refresh();
}

void ACButton::setCircleVisiable(bool show) 
{ 
    m_circle_visiable = show; 
    m_sizeValid = false;
    Refresh();
}

void ACButton::setSizeValid(bool valid, bool calcNow )
{
    m_sizeValid = valid;
    if (calcNow)
        messureSize();
}


//void ACButton::paintEvent(wxPaintEvent& evt)
//{
//    // depending on your system you may need to look at double-buffered dcs
//    wxPaintDC dc(this);
//    render(dc);
//}

/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void ACButton::render(wxDC& dc)
{
    if (m_sizeValid == false)
        messureSize();

    // background
    ACStaticBox::render(dc);

    // draw icon and text
    int states = state_handler.states();
    wxSize size = GetSize();
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    // calc content size
    // text
    auto text = GetLabel();
    wxSize dipIconSize = FromDIP(m_iconSize);
    wxSize dipPadding = FromDIP(m_paddingSize);
    float dipSpacing = FromDIP(m_spacing);

    const ScalableBitmap* icon = &active_icon;
    if ((states & (int)ACStateColor::State::Disabled) != 0) {
        icon = &inactive_icon;
    } else if ((states & (int)ACStateColor::State::Hovered) != 0) {
        icon = &hover_icon;
    }

    wxRect rcContent = { {0, 0}, size };
    
    if (m_alignCenter) {
        rcContent.Deflate((size-m_szContent)/2);
    } else {
        rcContent.Deflate(dipPadding);
    }
    // start draw
    // icon
    wxPoint pt = rcContent.GetLeftTop();
    if (icon->get_bitmap().IsOk()) {
        wxBitmap bmp = icon->bmp().GetBitmap(dipIconSize);

        if (m_isHorizontal) {
            pt.y += (rcContent.height - dipIconSize.y) / 2;
            dc.DrawBitmap(bmp, pt);

            pt.x += dipIconSize.x + dipSpacing;
            pt.y = rcContent.GetTop();
        } else {
            pt.x += (rcContent.width - dipIconSize.x) / 2;
            dc.DrawBitmap(bmp, pt);

            pt.x = rcContent.GetLeft();
            pt.y = rcContent.GetTop() + dipIconSize.y + dipSpacing;
        }
    }

    if (!text.IsEmpty()) {
        if (m_isHorizontal) {
            pt.y += (rcContent.height - m_szText.y) / 2;
        }
        else { 
            pt.x += (rcContent.width - m_szText.x) / 2;
        }

        int textValidSpace = rcContent.GetWidth();
        if (m_szText.x > textValidSpace)
            text = wxControl::Ellipsize(text, dc, wxELLIPSIZE_END, textValidSpace);

        dc.SetFont(GetFont());
        dc.SetTextForeground(text_color.colorForStates(states));
        dc.DrawText(text, pt);
    }
    if (m_draw_circle && m_circle_visiable) {
        int dipCircleSize = FromDIP(m_circleSize);
        wxPoint circle_pos(size.x - dipPadding.x - dipCircleSize / 2, size.y / 2);
        dc.SetBrush(text_color.colorForStates(states));
        dc.DrawCircle(circle_pos, dipCircleSize / 2);
    }
    if (checkable()) {
        if (m_checkStyle == CHECKSTYLE_ON_MARK) {
            if (GetChecked()) // show mark on selected
            {
                // mark
                ScalableBitmap* sbmp = (states & (int)ACStateColor::State::Hovered) ? &m_checkedMarkImgHover : &m_checkedMarkImg; 
                if (sbmp->bmp().IsOk()) {
                    wxPoint pt(size.x-m_ckMarkIconSize.x-1, 1);
                    dc.DrawBitmap(sbmp->bmp().GetBitmap(m_ckMarkIconSize), pt);
                }
            }
        } else if (m_checkStyle == CHECKSTYLE_ON_BOX || m_checkStyle == CHECKSTYLE_ON_HALF){
            if (m_isHorizontal) 
            {
                ScalableBitmap* sbmp = nullptr;

                switch (m_checkState)
                {
                case CHECKSTATE_ON:
                    sbmp = (states & (int)ACStateColor::State::Disabled) ?  &m_checkOnImgDis : (states & (int)ACStateColor::State::Hovered) ? &m_checkOnImgHover : &m_checkOnImg;
                    break;
                case CHECKSTATE_ON_HALF:
                    sbmp = (states & (int)ACStateColor::State::Disabled) ?  &m_checkHalfImgDis : (states & (int)ACStateColor::State::Hovered) ? &m_checkHalfImgHover : &m_checkHalfImg;
                    break;
                default: // no check
                    sbmp = (states & (int)ACStateColor::State::Disabled) ?  &m_checkOffImgDis : (states & (int)ACStateColor::State::Hovered) ? &m_checkOffImgHover : &m_checkOffImg;
                    break;
                }

                if (sbmp->bmp().IsOk()) {
                    wxSize dipSize = FromDIP(m_ckBoxIconSize);
                    pt.x = size.x - dipPadding.x - dipSize.x;
                    pt.y = (size.y- dipSize.y)/2;

                    dc.DrawBitmap(sbmp->bmp().GetBitmap(dipSize), pt);
                }         
            }
        }



    }
}

void ACButton::messureSize()
{
    wxClientDC dc(this);

    wxSize dipPadding = FromDIP(m_paddingSize);
    float dipSpacing = FromDIP(m_spacing);

    int states = state_handler.states();
    const ScalableBitmap* icon = &active_icon;
    if ((states & (int)ACStateColor::State::Disabled) != 0) {
        icon = &inactive_icon;
    } else if ((states & (int)ACStateColor::State::Hovered) != 0) {
        icon = &hover_icon;
    }

    wxString text = GetLabel();
    if (!text.IsEmpty()) {
        wxSize textSize = dc.GetTextExtent(text);
        m_szText = textSize;
    } else {
        m_szText = wxSize(0,0);
    }

    wxSize szIcon(0,0);
    if (icon->get_bitmap().IsOk()) {
        wxSize dipIconSize = FromDIP(m_iconSize);
        szIcon = dipIconSize;
    }

    m_szContent = szIcon;

    if (m_isHorizontal) {
        if (m_szText.x > 0) {
            m_szContent.x = m_szContent.x + dipSpacing + m_szText.x;
        }

        if (m_szText.y > m_szContent.y)
            m_szContent.y = m_szText.y;

        if (m_checkStyle == CHECKSTYLE_ON_BOX || m_checkStyle == CHECKSTYLE_ON_HALF) {
            wxSize dipSize = FromDIP(m_ckBoxIconSize);
            m_szContent.x = m_szContent.x + dipSpacing + dipSize.x;
            m_szContent.y = std::max(m_szContent.y, dipSize.y);
        }
    } else {
        if (m_szText.y > 0) {
            m_szContent.y = m_szContent.y + dipSpacing + m_szText.y;
        }

        if (m_szText.x > m_szContent.x)
            m_szContent.x = m_szText.x;
    }

    if (m_draw_circle) {
        m_szContent.x += dipSpacing + FromDIP(m_circleSize);
    }

    wxSize minSize = m_szContent + dipPadding * 2;

    minSize.x = std::max(minSize.x, FromDIP(m_minSize.x));
    minSize.y = std::max(minSize.y, FromDIP(m_minSize.y));

    wxSize curSize = wxWindow::GetSize();
    wxWindow::SetMinSize(minSize);

    if (curSize.x < minSize.x || curSize.y < minSize.y)
        wxWindow::SetSize(std::max(curSize.x, minSize.x), std::max(curSize.y, minSize.y));

    m_sizeValid = true;
}

void ACButton::mouseDown(wxMouseEvent& event)
{
    event.Skip();
    pressedDown = true;
    if (canFocus)
        SetFocus();
    CaptureMouse();
}

void ACButton::mouseReleased(wxMouseEvent& event)
{
    event.Skip();
    if (pressedDown) 
    {
        pressedDown = false;
        if (HasCapture())
            ReleaseMouse();
        if (wxRect({0, 0}, GetSize()).Contains(event.GetPosition()))
            sendButtonEvent();
    }
}

void ACButton::mouseCaptureLost(wxMouseCaptureLostEvent &event)
{
    wxMouseEvent evt;
    mouseReleased(evt);
}

void ACButton::keyDownUp(wxKeyEvent &event)
{
    if (event.GetKeyCode() == WXK_SPACE || event.GetKeyCode() == WXK_RETURN) {
        wxMouseEvent evt(event.GetEventType() == wxEVT_KEY_UP ? wxEVT_LEFT_UP : wxEVT_LEFT_DOWN);
        event.SetEventObject(this);
        GetEventHandler()->ProcessEvent(evt);
        return;
    }
    if (event.GetEventType() == wxEVT_KEY_DOWN &&
        (event.GetKeyCode() == WXK_TAB || event.GetKeyCode() == WXK_LEFT || event.GetKeyCode() == WXK_RIGHT 
        || event.GetKeyCode() == WXK_UP || event.GetKeyCode() == WXK_DOWN))
        HandleAsNavigationKey(event);
    else
        event.Skip();
}

void ACButton::sendButtonEvent()
{
    wxCommandEvent event(wxEVT_BUTTON, GetId());
    event.SetEventObject(this);
    GetEventHandler()->ProcessEvent(event);

    if (checkable()) {    
        AC_BUTTON_CHECK_STATE checkState = AC_BUTTON_CHECK_STATE(m_checkStyle == CHECKSTYLE_ON_HALF ? (m_checkState+1)%3 : (m_checkState+1)%2);
        SetChecked(checkState, true);
    }

    Refresh();
}

#ifdef __WIN32__

WXLRESULT ACButton::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
    if (nMsg == WM_GETDLGCODE) { return DLGC_WANTMESSAGE; }
    if (nMsg == WM_KEYDOWN) {
        wxKeyEvent event(CreateKeyEvent(wxEVT_KEY_DOWN, wParam, lParam));
        switch (wParam) {
        case WXK_RETURN: { // WXK_RETURN key is handled by default button
            GetEventHandler()->ProcessEvent(event);
            return 0;
        }
        }
    }
    return wxWindow::MSWWindowProc(nMsg, wParam, lParam);
}

#endif

bool ACButton::AcceptsFocus() const { return canFocus; }


void ACButton::setCheckedMarkImg(const wxString& imgNameCheckedOn, const wxString& imgNameCheckedHover, int imgSize)
{
    m_ckMarkIconSize = wxSize(imgSize, imgSize);
    if (!imgNameCheckedOn.IsEmpty()) {
        m_checkedMarkImg = ScalableBitmap(this, imgNameCheckedOn.ToStdString(), imgSize);
    } else {
        m_checkedMarkImg = ScalableBitmap();
    }
    if (!imgNameCheckedHover.IsEmpty()) {
        m_checkedMarkImgHover = ScalableBitmap(this, imgNameCheckedHover.ToStdString(), imgSize);
    } else {
        m_checkedMarkImgHover = ScalableBitmap();
    }
    m_sizeValid = true;
    Refresh();
}

