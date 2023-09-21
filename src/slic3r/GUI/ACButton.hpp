#ifndef slic3r_GUI_ACButton_hpp_
#define slic3r_GUI_ACButton_hpp_

#include "wxExtensions.hpp"
#include "ACStaticBox.hpp"
#include <wx/event.h>

class ACButton : public wxNavigationEnabled<ACStaticBox>
{

public:
    ACButton(wxWindow* parent, wxString text, wxString icon = "", wxString hover_icon = "", wxString dis_icon = "", long style = 0, wxSize iconSize = wxSize(24,24));

    ~ACButton();

    enum AC_ALIGN_STYLE
    {
        AC_ALIGN_LEFT = 0x10,
    };

    enum AC_BUTTON_TYPE
    {
        AC_BUTTON_LV0,
        AC_BUTTON_LV1,
        AC_BUTTON_LV2,
        AC_BUTTON_LV3,
        AC_BUTTON_LV0_N, // NOT SELECT
        AC_BUTTON_ICON, // 
        AC_BUTTON_LIST_ITEM_L0,
        AC_BUTTON_LIST_ITEM_L1,
        AC_BUTTON_LIST_ITEM_L2,
        AC_BUTTON_TABLE_ITEM,
        AC_BUTTON_DROPDOWM_ITEM,
        AC_BUTTON_CHECK_L0,
        AC_BUTTON_CHECK_L1,
        AC_BUTTON_CHECK_IMG,
        AC_BUTTON_SEL,
        AC_BUTTON_LABEL,
        AC_BUTTON_LABEL_2,
        AC_BUTTON_UPDOWN,
    };

    enum AC_BUTTON_CHECK_STYLE
    {
        CHECKSTYLE_NO_CHECK = 0,
        CHECKSTYLE_ON,
        CHECKSTYLE_ON_BOX,     // checkbox
        CHECKSTYLE_ON_HALF,
        CHECKSTYLE_ON_MARK,
    };

    enum AC_BUTTON_CHECK_STATE
    {
        CHECKSTATE_OFF = 0,
        CHECKSTATE_ON,
        CHECKSTATE_ON_HALF,
    };

    bool Create(wxWindow* parent, wxString text, wxString icon = "", wxString hover_icon = "", wxString dis_icon = "", long style = 0, wxSize iconSize = wxSize(24,24));

    void setupCheckImg();

    void SetLabel(const wxString& label) override;

    void SetIcon(const wxString& iconName);

    void SetInactiveIcon(const wxString& iconName);

    void SetHoverIcon(const wxString& iconName);

    void SetMinSize(const wxSize& size) override;
    
    void SetPaddingSize(const wxSize& size);
    
    void SetSpacing(const int& value);

    void SetButtonType(AC_BUTTON_TYPE);

    //void SetOffset(const wxSize& size);
    
    void SetTextColor(ACStateColor const &color);

    void SetTextColorNormal(wxColor const &color);

    void SetCheckStyle(AC_BUTTON_CHECK_STYLE checkStyle);

    void SetChecked(bool selected = true, bool sendEvents = false);
    void SetChecked(AC_BUTTON_CHECK_STATE state, bool sendEvents = false);

    void SetHalfChecked(bool sendEvents = false);

    void SetTextAtLeft(bool atLeft = true);

    bool GetChecked() const { return m_checkState != CHECKSTATE_OFF; }

    bool checkable() const { return m_checkStyle != CHECKSTYLE_NO_CHECK; }

    bool GetHalfChecked() { return m_checkState == CHECKSTATE_ON_HALF; }

    void SetAlignCenter(bool align) { m_alignCenter = align; }

    void setCheckedMarkImg(const wxString& imgNameCheckedOn, const wxString& imgNameCheckedHover, int imgSize);

    bool Enable(bool enable = true) override;
    bool SetEnable(bool enable = true) ;

    void SetCanFocus(bool canFocus) override;

    void Rescale();

    void clearColor();

    void setTakeFocusedAsHovered(bool as);

    void sys_color_changed();

    void setDrawCircle(bool canShow) ;
    void setCircleVisiable(bool show);

    void setSizeValid(bool valid, bool calcNow = true);

    void messureSize();

    void setFixedSizePar(int index) { m_fixedWidth = index; }

protected:
#ifdef __WIN32__
    WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam) override;
#endif

    bool AcceptsFocus() const override;

    //void paintEvent(wxPaintEvent& evt);

    virtual void render(wxDC& dc) override;

    

    // some useful events
    void mouseDown(wxMouseEvent& event);
    void mouseReleased(wxMouseEvent& event);
    void mouseCaptureLost(wxMouseCaptureLostEvent &event);
    void keyDownUp(wxKeyEvent &event);

    void sendButtonEvent();


    DECLARE_EVENT_TABLE()

private:
    wxString firstPart;
    wxString secondPart;
    int m_fixedWidth = 0;
    wxSize m_szText = wxSize(0,0);
    wxSize m_minSize; // set by outer
    wxSize m_paddingSize;
    wxSize m_szContent;
    wxSize m_iconSize = wxSize(24, 24);
    wxSize m_ckMarkIconSize = wxSize(8, 8);
    wxSize m_ckBoxIconSize = wxSize(16, 16);
    //wxSize m_offset;
    ScalableBitmap active_icon;
    ScalableBitmap inactive_icon;
    ScalableBitmap hover_icon;

    ScalableBitmap m_checkedMarkImg;
    ScalableBitmap m_checkedMarkImgHover;

    ScalableBitmap m_checkOnImg;
    ScalableBitmap m_checkOnImgHover;
    ScalableBitmap m_checkOnImgDis;

    ScalableBitmap m_checkOffImg;
    ScalableBitmap m_checkOffImgHover;
    ScalableBitmap m_checkOffImgDis;

    ScalableBitmap m_checkHalfImg;
    ScalableBitmap m_checkHalfImgHover;
    ScalableBitmap m_checkHalfImgDis;

    ACStateColor   text_color;


    bool pressedDown = false;
    bool canFocus  = true;
    bool m_alignCenter = true;
    int m_spacing = 5;
    int m_circleSize = 6;
    bool m_isHorizontal = true;
    bool m_textAtLeft = true;
    AC_BUTTON_CHECK_STYLE m_checkStyle = CHECKSTYLE_NO_CHECK;
    AC_BUTTON_CHECK_STATE m_checkState = CHECKSTATE_OFF;
    bool                  m_draw_circle    = false;
    bool                  m_circle_visiable    = false;
};



#endif // !slic3r_GUI_ACButton_hpp_
