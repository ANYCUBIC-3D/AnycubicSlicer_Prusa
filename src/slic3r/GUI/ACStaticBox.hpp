#ifndef slic3r_GUI_ACStaticBox_hpp_
#define slic3r_GUI_ACStaticBox_hpp_

#include "ACStateHandler.hpp"

#include <wx/window.h>

class ACStaticBox : public wxWindow
{
public:
    ACStaticBox();

    ACStaticBox(wxWindow* parent,
             wxWindowID      id        = wxID_ANY,
             const wxPoint & pos       = wxDefaultPosition,
             const wxSize &  size      = wxDefaultSize, 
             long style = 0,
             wxString name = "");

    enum CornerRadiusType {
        CornerTopLeft     = 0x01,
        CornerTopRight    = 0x02,
        CornerBottomLeft  = 0x04,
        CornerBottomRight = 0x08,
        CornerTop         = CornerTopLeft | CornerTopRight,
        CornerBottom      = CornerBottomLeft | CornerBottomRight,
        CornerLeft        = CornerTopLeft | CornerBottomLeft,
        CornerRight       = CornerTopRight | CornerBottomRight,
        CornerAll         = CornerLeft | CornerRight,
    };

    bool Create(wxWindow* parent,
        wxWindowID      id        = wxID_ANY,
        const wxPoint & pos       = wxDefaultPosition,
        const wxSize &  size      = wxDefaultSize, 
        long style = 0,
    wxString name = "");

    void SetCornerRadius(double radius);

    void SetCornerRadiusType(CornerRadiusType type);

    void SetCornerRadius(double radius, CornerRadiusType type);

    void SetBorderWidth(int width);

    void SetBorderColor(ACStateColor const & color);

    void SetBorderColorNormal(wxColor const &color);

    void SetBackgroundColor(ACStateColor const &color);

    void SetBackgroundColorNormal(wxColor const &color);

    void SetBackgroundColor2(ACStateColor const &color);

    static wxColor GetParentBackgroundColor(wxWindow * parent);

    void clearColor();

    void setTakeFocusedAsHovered(bool);

protected:
    void eraseEvent(wxEraseEvent& evt);

    void paintEvent(wxPaintEvent& evt);

    void onSizeChanged(wxSizeEvent& e);
    virtual void render(wxDC& dc);

    //virtual void doRender(wxDC& dc);
    virtual void messureSize(){};
protected:
    double radius;
    int radiusType;
    int border_width = 1;
    ACStateHandler state_handler;
    ACStateColor   border_color;
    ACStateColor   background_color;
    ACStateColor   background_color2;

    bool m_sizeValid = false;
    DECLARE_EVENT_TABLE()
};

#endif // !slic3r_GUI_StaticBox_hpp_
