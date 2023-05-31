#ifndef slic3r_GUI_ACDropDown_hpp_
#define slic3r_GUI_ACDropDown_hpp_

#include <wx/stattext.h>
#include "wxExtensions.hpp"
#include "ACStateHandler.hpp"
#include "wx/popupwin.h"
#include <wx/bmpbndl.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#define DD_NO_CHECK_ICON 0x0001
#define DD_NO_TEXT 0x0002
#define DD_STYLE_MASK 0x0003

namespace Slic3r { namespace GUI {

wxDECLARE_EVENT(EVT_DISMISS, wxCommandEvent);

class ACDropDown : public wxPopupTransientWindow
{
    std::vector<wxString>               &texts;
    std::vector<const wxBitmapBundle *> &icons;
    bool                                 need_sync  = false;
    int                                  selection  = -1;
    int                                  hover_item = -1;

    double radius            = 6;
    int m_padding = 4;
    int m_spacing = 4;
    int m_scrollWidth = 6;
    int m_scrollBarWidth = 4;
    int m_maxRows = 15;
    bool   use_content_width = false;
    //bool   align_icon        = false;
    //bool   text_off          = false;

    wxSize m_textSize;
    wxSize m_rowSize;
    wxSize m_iconSize;
    bool   m_iconExist = false;
    //wxSize m_checkIconSize;

    ACStateHandler  state_handler;
    ACStateColor    text_color;
    ACStateColor    border_color;
    ACStateColor    selector_border_color;
    ACStateColor    selector_background_color;
    //wxBitmapBundle *check_bitmap = nullptr;

    bool                     pressedDown = false;
    boost::posix_time::ptime dismissTime;
    int                      m_offsetY = 0; // x not used
    wxPoint                  dragStart;
    bool                     m_sizeValid = false;
public:
    ACDropDown(wxWindow *parent, std::vector<wxString> &texts, std::vector<const wxBitmapBundle *> &icons, wxSize iconSize, long style = 0);

    void Create(wxWindow *parent, long style = 0);

public:
    void Invalidate(bool clear = false);

    int GetSelection() const { return selection; }

    void SetSelection(int n);

    wxString GetValue() const;
    void     SetValue(const wxString &value);

    //wxSize GetItemIconSize() const { return m_iconSize; }

public:
    void SetCornerRadius(double radius);

    void SetBorderColor(ACStateColor const &color);

    void SetSelectorBorderColor(ACStateColor const &color);

    void SetSelectorBackgroundColor(ACStateColor const &color);

    void SetTextColor(ACStateColor const &color);

    void SetUseContentWidth(bool use);

    //void SetAlignIcon(bool align);

    //void SetIconSize(wxSize iconSize) ;

    //virtual bool HasTransparentBackground() { return true; }
    //virtual bool IsTransparentBackgroundSupported(wxString *reason = NULL) const { return true; }
    //virtual bool CanSetTransparent() { return true; }
    // virtual bool SetTransparent(wxByte WXUNUSED(alpha)) { return true; }
public:
    void Rescale();

    bool HasDismissLongTime();

protected:
    void OnDismiss() override;

private:
    void paintEvent(wxPaintEvent &evt);
    //void paintNow();

    void render(wxDC &dc);

    friend class ACComboBox;
    void messureSize();
    void autoPosition();

    // some useful events
    void mouseDown(wxMouseEvent &event);
    void mouseReleased(wxMouseEvent &event);
    void mouseCaptureLost(wxMouseCaptureLostEvent &event);
    void mouseMove(wxMouseEvent &event);
    void mouseWheelMoved(wxMouseEvent &event);

    void sendDropDownEvent(int item);

    DECLARE_EVENT_TABLE()
};

}} // namespace Slic3r::GUI

#endif // !slic3r_GUI_ACDropDown_hpp_
