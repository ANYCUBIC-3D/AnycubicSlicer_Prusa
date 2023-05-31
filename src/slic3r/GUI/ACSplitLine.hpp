#ifndef slic3r_GUI_ACSplitLine_hpp_
#define slic3r_GUI_ACSplitLine_hpp_

#include <wx/event.h>
#include "ACStaticBox.hpp"


class ACButton;
class ACSplitLine : public ACStaticBox
{
public:
    enum SplitDir
    {
        SplitHorizontal = wxHORIZONTAL,
        SplitVertical = wxVERTICAL,
    };

public:
    ACSplitLine(wxWindow* parent, SplitDir dir = SplitHorizontal, int lineLen = 14, int lineWidth = 1, long style = 0);

    bool Create(wxWindow* parent, SplitDir dir = SplitHorizontal, int lineLen = 14, int lineWidth = 1, long style = 0);

    void setLineColour(const wxColour& c);
    void setLinePadding(int padding);

    void sys_color_changed();
protected:
    void Rescale();
    void messureSize();
    void render(wxDC& dc);

private:
    SplitDir m_splitDir = SplitHorizontal;
    int m_padding = 4;
    int m_lineWidth = 1;
    int m_lineLen = 14;
    wxColour m_lineColour = wxColour(0,0,0);
};



#endif // !slic3r_GUI_ACSplitLine_hpp_
