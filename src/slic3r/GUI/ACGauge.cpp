#include "ACGauge.hpp"

#include "ACDefines.h"
#include <wx/dcgraph.h>
#include "ACDefines.h"
#include "wxExtensions.hpp"

#include "ACStaticBox.hpp"
#include "ACStateColor.hpp"


BEGIN_EVENT_TABLE(ACGauge, wxWindow)

EVT_PAINT(ACGauge::paintEvent)
EVT_LEFT_DOWN(ACGauge::InstallNowEvent)
END_EVENT_TABLE()

ACGauge::ACGauge(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
    :wxWindow(parent, id, pos, size)
{
    const int &em = GUI::wxGetApp().em_unit();
    m_size        = size / em;
    m_value = 0;
    m_penWidth    = int(0.2 * em);
    SetBackgroundColour(AC_COLOR_WHITE);
}

void ACGauge::SetValue(int value)
{
    if (!m_gaugeIndex)
        m_gaugeIndex = !m_gaugeIndex;
    m_value = value;
    Refresh();
}

int ACGauge::GetValue()
{
	return m_value;
}

void ACGauge::SetShow(bool isShow)
{
	m_gaugeIndex = isShow;
	Refresh();
}

void ACGauge::Rescale()
{
    const int &em = GUI::wxGetApp().em_unit();
    wxSize     _size = m_size * em;

    this->SetMinSize(_size);

    Fit();

    this->Layout();

    Refresh();
}

void ACGauge::InstallNowEvent(wxMouseEvent &event)
{ 
    if (m_value == 100) {
        GUI::wxGetApp().sendShowCheckUpdateProgressFinishDialogEvent();
    } else {
        GUI::wxGetApp().sendShowCheckUpdateProgressDownLoadEvent();
    }
    event.Skip();
}

wxPoint ACGauge::GetArcPoint(int value)
{

    int x_point, y_point,new_value;
    int centerX = GetClientSize().GetWidth() / 2;
    int centerY = GetClientSize().GetHeight() / 2;

    if (value < 25) {
        new_value = value;
        x_point   = centerX - centerX * new_value / 25 + m_penWidth;
        y_point   = centerY * new_value / 25;
    } else if (value < 50) {
        new_value = value - 25;
        x_point   = centerX * new_value / 25 + m_penWidth;
        y_point   = centerY + centerY * new_value / 25;
    } else if (value < 75) {
        new_value = value - 50;
        x_point   = centerX * new_value / 25 + centerX - m_penWidth;
        if (x_point < centerX + m_penWidth)
            x_point = centerX + m_penWidth;
        y_point   = 2 * centerY - centerY * new_value / 25;
    }else {
        new_value = value - 75;
        x_point   = 2 * centerX - centerX * new_value / 25 - m_penWidth;
        y_point   = centerY - centerY * new_value / 25;
    }
   
    return wxPoint(int(x_point), int(y_point));
}

void ACGauge::paintEvent(wxPaintEvent &evt)
{
    if (m_gaugeIndex) {
        wxPaintDC dc(this);
        wxGCDC    gcdc(dc);
        gcdc.SetBackground(wxBrush(GetBackgroundColour()));
        gcdc.Clear();

        int width   = std::min(GetClientSize().GetWidth(), GetClientSize().GetHeight());
        int radius  = width / 2 - m_penWidth;
        int centerX = GetClientSize().GetWidth() / 2;
        int centerY = GetClientSize().GetHeight() / 2;

        wxColour circleColor(AC_COLOR_MAIN_BLUE);
        if (m_value <= 1 + m_penWidth)
            circleColor = AC_COLOR_BOX_GRAY;
        gcdc.SetPen(wxPen(circleColor, m_penWidth));
        gcdc.DrawCircle(centerX, centerY, radius);
        if (m_value < 98 - m_penWidth && m_value > 1 + m_penWidth) {
            gcdc.SetPen(wxPen(wxColour(AC_COLOR_BOX_GRAY), m_penWidth));
            gcdc.SetBrush(*wxTRANSPARENT_BRUSH);
            wxPoint start_point(centerX, m_penWidth);
            m_endPoint = GetArcPoint(100 - m_value);
            wxPoint center_point(centerX, centerY);
            gcdc.DrawArc(start_point, m_endPoint, center_point);
        }

        wxColour insideColor(AC_COLOR_BOX_GRAY);
        if (m_value == 100)
            insideColor = AC_COLOR_MAIN_BLUE;
        gcdc.SetPen(wxPen(insideColor, 1));
        gcdc.SetBrush(insideColor);
        wxPoint _downRectPoint(centerX * 0.7, centerY * 1.4);
        wxPoint _upRectPoint(centerX - 2, centerY * 0.5);
        gcdc.DrawRectangle(_downRectPoint, wxSize(centerX * 0.7, 3));
        gcdc.DrawRectangle(_upRectPoint, wxSize(4, centerY * 0.5));
        wxPoint _poly1(centerX * 0.7, centerY);
        wxPoint _poly2(centerX, centerY * 1.3);
        wxPoint _poly3(centerX * 1.3, centerY);
        wxPoint triangle[3] = {_poly1, _poly2, _poly3};
        gcdc.DrawPolygon(3, triangle);
    }
}



