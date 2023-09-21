#include "ACStaticBoxBuffered.hpp"
#include "ACStateColor.hpp"
#include <wx/dcgraph.h>
#include "ACDefines.h"
#include "wxExtensions.hpp"

BEGIN_EVENT_TABLE(ACStaticBoxBuffered, wxWindow)

// catch paint events
//EVT_ERASE_BACKGROUND(ACStaticBox::eraseEvent)
EVT_PAINT(ACStaticBoxBuffered::paintEvent)
EVT_SIZE(ACStaticBoxBuffered::onSizeChanged)

END_EVENT_TABLE()

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

ACStaticBoxBuffered::ACStaticBoxBuffered() : state_handler(this), radius(8), radiusType(CornerAll)
{
	//border_color = ACStateColor(
	//    std::make_pair(AC_COLOR_BLACK_DISABLE, (int) ACStateColor::Disabled), 
	//    //std::make_pair(AC_COLOR_MAIN_BLUE_HOVER, (int) ACStateColor::Hovered), 
	//    std::make_pair(AC_COLOR_BLACK, (int) ACStateColor::Normal));
}

ACStaticBoxBuffered::ACStaticBoxBuffered(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, wxString name)
	: ACStaticBoxBuffered()
{
	Create(parent, id, pos, size, style);
}

bool ACStaticBoxBuffered::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, wxString name)
{
	if (style & wxBORDER_NONE)
		border_width = 0;
	wxWindow::Create(parent, id, pos, size, style);
	state_handler.attach({ &border_color, &background_color, &background_color2 });
	state_handler.update_binds();
	SetBackgroundColour(GetParentBackgroundColor(parent));
	//SetAutoLayout(true);
	SetBackgroundStyle(wxBG_STYLE_PAINT);

	this->Bind(wxEVT_SIZE, [this](wxEvent& evt) { evt.Skip(); Refresh(); });
	return true;
}

void ACStaticBoxBuffered::SetCornerRadius(double radius)
{
	this->radius = radius;
	Refresh();
}

void ACStaticBoxBuffered::SetCornerRadiusType(CornerRadiusType type)
{
	this->radiusType = (int)type;
	Refresh();
}

void ACStaticBoxBuffered::SetCornerRadius(double radius, CornerRadiusType type)
{
	this->radius = radius;
	this->radiusType = (int)type;
	Refresh();
}

void ACStaticBoxBuffered::SetBorderWidth(int width)
{
	border_width = width;
	Refresh();
}

void ACStaticBoxBuffered::SetBorderColor(ACStateColor const& color)
{
	border_color = color;
	state_handler.update_binds();
	Refresh();
}

void ACStaticBoxBuffered::SetBorderColorNormal(wxColor const& color)
{
	border_color.setColorForStates(color, 0);
	Refresh();
}

void ACStaticBoxBuffered::SetBackgroundColor(ACStateColor const& color)
{
	background_color = color;
	state_handler.update_binds();
	Refresh();
}

void ACStaticBoxBuffered::SetBackgroundColorNormal(wxColor const& color)
{
	background_color.setColorForStates(color, 0);
	state_handler.update_binds();
	Refresh();
}

void ACStaticBoxBuffered::SetBackgroundColor2(ACStateColor const& color)
{
	background_color2 = color;
	state_handler.update_binds();
	Refresh();
}

void ACStaticBoxBuffered::clearColor()
{
	background_color.clear();
	background_color2.clear();
	border_color.clear();
	state_handler.update_binds();
	Refresh();
}

void ACStaticBoxBuffered::setTakeFocusedAsHovered(bool as)
{
	background_color .setTakeFocusedAsHovered(as);
	background_color2.setTakeFocusedAsHovered(as);
	border_color     .setTakeFocusedAsHovered(as);
	Refresh();
}


wxColor ACStaticBoxBuffered::GetParentBackgroundColor(wxWindow* parent)
{
    if (auto box = dynamic_cast<ACStaticBoxBuffered*>(parent)) {
        if (box->background_color.count() > 0) {
            if (box->background_color2.count() == 0)
                return box->background_color.defaultColor();

            auto s = box->background_color.defaultColor();
            auto e = box->background_color2.defaultColor();
            int r = (s.Red() + e.Red()) / 2;
            int g = (s.Green() + e.Green()) / 2;
            int b = (s.Blue() + e.Blue()) / 2;
            return wxColor(r, g, b);
        }
    }
    if (parent)
        return parent->GetBackgroundColour();
    return *wxWHITE;
}

void ACStaticBoxBuffered::eraseEvent(wxEraseEvent& evt)
{
	// for transparent background, but not work
#ifdef __WXMSW__
	wxDC* dc = evt.GetDC();
	wxSize size = GetSize();
	wxClientDC dc2(GetParent());
	dc->Blit({ 0, 0 }, size, &dc2, GetPosition());
#endif
}
#include <wx/dcgraph.h>
#include "wx/defs.h"
#include <wx/dcbuffer.h>
void ACStaticBoxBuffered::paintEvent(wxPaintEvent& evt)
{
	// depending on your system you may need to look at double-buffered dcs
	//wxPaintDC dc(this);
	//wxGCDC gcdc(dc);
	//render(gcdc);
	wxAutoBufferedPaintDC paintDC(this);
	wxGraphicsContext* gc = wxGraphicsContext::Create(paintDC);
	render(gc);
	if (gc)
	{
		delete gc;
		gc = nullptr;
	}
}

void ACStaticBoxBuffered::onSizeChanged(wxSizeEvent& e)
{
	e.Skip();

	m_sizeValid = false;
	messureSize();
}



void ACStaticBoxBuffered::render(wxGraphicsContext* dc)
{
	wxSize size = GetSize();
	wxRectF rc(0.0, 0.0, double(size.x), double(size.y));
	wxColour parentColour = GetParentBackgroundColor(GetParent());
	dc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
	//dc.SetBackground(parentColour);
	dc->SetBrush(parentColour);
	dc->SetPen(*wxTRANSPARENT_PEN);
	dc->DrawRectangle(rc.x, rc.y, rc.w, rc.h);

	int dipBoardWidth = border_width == 0 ? 0 : FromDIP(border_width);
	int dipRadius = radius == 0 ? 0 : FromDIP(radius);


	int states = state_handler.states();
	if (background_color2.count() == 0)
	{
		//if (( border_color.count() > 0) || background_color.count() > 0) {
		if (dipBoardWidth && border_color.count() > 0)
		{
			rc.Deflate(dipBoardWidth);

			dc->SetPen(wxPen(border_color.colorForStates(states), dipBoardWidth));
		}
		else
		{
			dc->SetPen(*wxTRANSPARENT_PEN);
		}

		if (background_color.count() > 0)
			dc->SetBrush(wxBrush(background_color.colorForStates(states)));
		else
			dc->SetBrush(wxBrush(GetBackgroundColour()));

		if (dipRadius == 0)
		{
			dc->DrawRectangle(rc.x, rc.y, rc.w, rc.h);
		}
		else
		{
			dc->DrawRoundedRectangle(rc.x, rc.y, rc.w, rc.h, dipRadius);

			if (radiusType != CornerAll)
			{
				if ((radiusType & (int)CornerTopLeft) == 0)
				{
					wxRectF src(rc.x, rc.y, rc.w / 2.0, rc.h / 2.0);
					dc->DrawRectangle(src.x, src.y, src.w, src.h);
				}
				if ((radiusType & (int)CornerTopRight) == 0)
				{
					wxRectF src(rc.x + rc.w / 2.0/* + 1.0*/, rc.y, rc.w / 2.0, rc.h / 2.0);
					dc->DrawRectangle(src.x, src.y, src.w, src.h);
				}
				if ((radiusType & (int)CornerBottomLeft) == 0)
				{
					wxRectF src(rc.x, rc.y + rc.h / 2.0/* + 1.0*/, rc.w / 2.0, rc.h / 2.0);
					dc->DrawRectangle(src.x, src.y, src.w, src.h);
				}
				if ((radiusType & (int)CornerBottomRight) == 0)
				{
					wxRectF src(rc.x + rc.w / 2.0/* + 1.0*/, rc.y + rc.h / 2.0/* + 1.0*/, rc.w / 2, rc.h / 2);
					dc->DrawRectangle(src.x, src.y, src.w, src.h);
				}
			}
		}
		//}
	}
	/*else
	{
		wxColor start = background_color.colorForStates(states);
		wxColor stop = background_color2.colorForStates(states);
		int r = start.Red(), g = start.Green(), b = start.Blue();
		int dr = (int)stop.Red() - r, dg = (int)stop.Green() - g, db = (int)stop.Blue() - b;
		int lr = 0, lg = 0, lb = 0;
		for (int y = 0; y < size.y; ++y)
		{
			dc->SetPen(wxPen(wxColor(r, g, b)));
			dc->DrawLine(0, y, size.x, y);
			lr += dr; while (lr >= size.y) { ++r, lr -= size.y; } while (lr <= -size.y) { --r, lr += size.y; }
			lg += dg; while (lg >= size.y) { ++g, lg -= size.y; } while (lg <= -size.y) { --g, lg += size.y; }
			lb += db; while (lb >= size.y) { ++b, lb -= size.y; } while (lb <= -size.y) { --b, lb += size.y; }
		}
	}*/
}
