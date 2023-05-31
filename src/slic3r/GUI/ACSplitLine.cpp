#include "ACSplitLine.hpp"
#include "ACStateColor.hpp"
#include "ACLabel.hpp"
#include "ACStateHandler.hpp"
#include "wxExtensions.hpp"

#include <wx/dcgraph.h>
#include "ACButton.hpp"

#include "ACDefines.h"


ACSplitLine::ACSplitLine(wxWindow* parent, SplitDir dir, int lineLen, int lineWidth, long style)
{
    Create(parent, dir, lineLen, lineWidth, style);
}

bool ACSplitLine::Create(wxWindow* parent, SplitDir dir, int lineLen, int lineWidth, long style)
{
    ACStaticBox::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);

    m_splitDir = dir;
    m_lineLen = lineLen;
    m_lineWidth = lineWidth;

    messureSize();

    return true;
}

void ACSplitLine::sys_color_changed()
{
    // AC : TODO
}


void ACSplitLine::Rescale()
{
    messureSize();
}

void ACSplitLine::render(wxDC& dc)
{
    ACStaticBox::render(dc);

    wxPen pen = dc.GetPen();
    pen.SetWidth(m_lineWidth);
    pen.SetColour(m_lineColour);
    dc.SetPen(pen);

    if (m_splitDir == wxHORIZONTAL) {
        dc.DrawLine(wxPoint(m_padding+1, 0), wxPoint(m_padding+1, m_lineLen));
    } else {
        dc.DrawLine(wxPoint(0, m_padding+1), wxPoint(m_lineLen, m_padding+1));
    }
}
//
void ACSplitLine::setLineColour(const wxColour& c)
{
    m_lineColour = c;
    Refresh();
}
void ACSplitLine::setLinePadding(int padding)
{
    m_padding = padding;
    messureSize();
    Refresh();
}

void ACSplitLine::messureSize()
{
    wxSize oldSize = GetSize();

    wxSize minSize;
    
    if (m_splitDir == wxHORIZONTAL) {
        minSize.x = m_padding*2 + m_lineWidth;
        minSize.y = m_lineLen;
    } else {
        minSize.y = m_padding*2 + m_lineWidth;
        minSize.x = m_lineLen;
    }

    wxSize curSize = GetSize();
    SetMinSize(minSize);

    if (curSize.x < minSize.x || curSize.y < minSize.y)
        SetSize(std::max(curSize.x, minSize.x), std::max(curSize.y, minSize.y));
}


