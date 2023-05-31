#include "ACLabel.hpp"
#include "ACStaticBox.hpp"

#include <wx/settings.h>
#include <wx/dc.h>
#include <wx/dcclient.h>

wxFont ACLabel::sysFont(int size, bool bold)
{
//#ifdef __linux__
//    return wxFont{};
//#endif
#ifndef __APPLE__
    size = size * 4 / 5;
#endif

    auto   face = wxString::FromUTF8("HONORSansCN");
    wxFont font{size, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL, false, face};
    font.SetFaceName(face);
    if (!font.IsOk()) {
        font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
        if (bold)
            font.MakeBold();
        font.SetPointSize(size);
    }
    return font;
}
wxFont ACLabel::Head_24;
wxFont ACLabel::Head_20;
wxFont ACLabel::Head_18;
wxFont ACLabel::Head_16;
wxFont ACLabel::Head_15;
wxFont ACLabel::Head_14;
wxFont ACLabel::Head_13;
wxFont ACLabel::Head_12;
wxFont ACLabel::Head_10;

wxFont ACLabel::Body_16;
wxFont ACLabel::Body_15;
wxFont ACLabel::Body_14;
wxFont ACLabel::Body_13;
wxFont ACLabel::Body_12;
wxFont ACLabel::Body_11;
wxFont ACLabel::Body_10;
wxFont ACLabel::Body_9;

void ACLabel::initSysFont()
{
    Head_24 = ACLabel::sysFont(24, true);
    Head_20 = ACLabel::sysFont(20, true);
    Head_18 = ACLabel::sysFont(18, true);
    Head_16 = ACLabel::sysFont(16, true);
    Head_15 = ACLabel::sysFont(15, true);
    Head_14 = ACLabel::sysFont(14, true);
    Head_13 = ACLabel::sysFont(13, true);
    Head_12 = ACLabel::sysFont(12, true);
    Head_10 = ACLabel::sysFont(10, true);

    Body_16 = ACLabel::sysFont(16, false);
    Body_15 = ACLabel::sysFont(15, false);
    Body_14 = ACLabel::sysFont(14, false);
    Body_13 = ACLabel::sysFont(13, false);
    Body_12 = ACLabel::sysFont(12, false);
    Body_11 = ACLabel::sysFont(11, false);
    Body_10 = ACLabel::sysFont(10, false);
    Body_9  = ACLabel::sysFont(9, false);
}

wxSize ACLabel::split_lines(wxDC &dc, int width, const wxString &text, wxString &multiline_text)
{
    multiline_text = text;
    if (width > 0 && dc.GetTextExtent(text).x > width) {
        size_t start = 0;
        while (true) {
            size_t idx = size_t(-1);
            for (size_t i = start; i < multiline_text.Len(); i++) {
                if (multiline_text[i] == ' ') {
                    if (dc.GetTextExtent(multiline_text.SubString(start, i)).x < width)
                        idx = i;
                    else {
                        if (idx == size_t(-1))
                            idx = i;
                        break;
                    }
                }
            }
            if (idx == size_t(-1))
                break;
            multiline_text[idx] = '\n';
            start               = idx + 1;
            if (dc.GetTextExtent(multiline_text.Mid(start)).x < width)
                break;
        }
    }
    return dc.GetMultiLineTextExtent(multiline_text);
}

ACLabel::ACLabel(wxWindow *parent, wxString const &text, long style) : ACLabel(parent, Body_14, text, style) {}

ACLabel::ACLabel(wxWindow *parent, wxFont const &font, wxString const &text, long style)
    : wxStaticText(parent, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, style)
{
    this->font = font;
    SetFont(font);
    SetBackgroundColour(ACStaticBox::GetParentBackgroundColor(parent));
    Bind(wxEVT_ENTER_WINDOW, [this](auto &e) {
        if (GetWindowStyle() & LB_HYPERLINK) {
            SetFont(this->font.Underlined());
            Refresh();
        }
    });
    Bind(wxEVT_LEAVE_WINDOW, [this](auto &e) {
        if (GetWindowStyle() & LB_HYPERLINK) {
            SetFont(this->font);
            Refresh();
        }
    });
}

void ACLabel::SetWindowStyleFlag(long style)
{
    if (style == GetWindowStyle())
        return;
    wxStaticText::SetWindowStyleFlag(style);
    if (style & LB_HYPERLINK) {
        this->color = GetForegroundColour();
        static wxColor clr_url("#00AE42");
        SetForegroundColour(clr_url);
    } else {
        SetForegroundColour(this->color);
        SetFont(this->font);
    }
    Refresh();
}

wxSize ACLabel::GetTextSize()
{
    wxClientDC dc(this);
    wxSize     textSize = dc.GetTextExtent(GetLabel());

    return textSize;
}


void ACLabel::Rescale()
{
    wxStaticText::Refresh();
}
