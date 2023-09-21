/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/statbox.cpp
// Purpose:     ACGroupBox
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "ACDefines.h"
// For compilers that support precompilation, includes "wx.h".

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <wx/statbox.h>
#include <wx/wxprec.h>
#include <wx/notebook.h>
#include <wx/sysopt.h>

#ifndef WX_PRECOMP
#include <wx/app.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/image.h>
#include <wx/sizer.h>
#endif // WX_PRECOMP

#ifdef WIN32
#include <windows.h> // needed by GET_X_LPARAM and GET_Y_LPARAM macros

#include <wx/msw/uxtheme.h>
#include <wx/msw/private.h>
#include <wx/msw/missing.h>
#include <wx/msw/dc.h>
#include <wx/msw/private/winstyle.h>
#endif // WIN32

#include "ACGroupbox.hpp"

#include "ACButton.hpp"

wxIMPLEMENT_CLASS(ACGroupBoxSizer, wxBoxSizer);

namespace
{

// Offset of the first pixel of the label from the box left border.
//
// FIXME: value is hardcoded as this is what it is on my system, no idea if
//        it's true everywhere
const int LABEL_HORZ_OFFSET = 9;
const int LABEL_VERT_OFFSET = 9;

// Extra borders around the label on left/right and bottom sides.
const int LABEL_HORZ_BORDER = 2;
const int LABEL_VERT_BORDER = 2;

// Offset of the box contents from left/right/bottom edge (top one is
// different, see ACGetBordersForSizer()). This one is completely arbitrary.
const int CHILDREN_OFFSET = 5;

} // anonymous namespace

// ----------------------------------------------------------------------------
// wxWin macros
// ----------------------------------------------------------------------------

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// ACGroupBox
// ----------------------------------------------------------------------------

static int groupboxCounter = 0;
bool ACGroupBox::Create(wxWindow *parent,
                         const wxString& label,
                         wxWindowID id,
                         const wxPoint& pos,
                         const wxSize& size,
                         long style)
{
    this->SetCornerRadius(10);

    wxString iconName = "title-circle";
    m_labelWin = new ACButton(this, label, iconName, iconName, iconName, ACButton::AC_ALIGN_LEFT, wxSize(10, 10));
    m_labelWin->SetSpacing(6);
    m_labelWin->SetPaddingSize(wxSize(0,0));
    m_labelWin->SetButtonType(ACButton::AC_BUTTON_LABEL);

    //Bind(wxEVT_PAINT, [&](wxPaintEvent& WXUNUSED(event)){
    //    OnPaint();
    //});
    PositionLabelWindow();

    groupboxCounter++;
    return true;
}
bool ACGroupBox::Create(wxWindow *                                          parent,
                        const wxString &                                    label,
                        wxWindowID                                          id,
                        bool                                                createIndex,
                        const wxPoint &pos,
                        const wxSize & size,
                        long                                            style)
{
    this->SetCornerRadius(10);

    wxString iconName = "title-circle";
    m_labelWin        = new ACButton(this, label, iconName, iconName, iconName, ACButton::AC_ALIGN_LEFT, wxSize(10, 10));
    m_labelWin->SetSpacing(6);
    m_labelWin->SetPaddingSize(wxSize(0, 0));
    m_labelWin->SetButtonType(ACButton::AC_BUTTON_LABEL);
    if (createIndex) {
        wxString rest_iconName = "reset-click";
        m_circleBack           = new ACButton(this, "", rest_iconName, rest_iconName, rest_iconName, wxNO_BORDER, wxSize(24, 24));
        m_circleBack->SetPaddingSize(wxSize(0, 0));
        m_circleBack->SetButtonType(ACButton::AC_BUTTON_ICON);
        m_circleBack->SetBackgroundColour(wxColor(255,255,255));
        m_circleBack->Hide();
    }
    // Bind(wxEVT_PAINT, [&](wxPaintEvent& WXUNUSED(event)){
    //    OnPaint();
    //});
    PositionLabelWindow();

    groupboxCounter++;
    return true;
}
ACGroupBox::~ACGroupBox()
{
    printf("ACGroupBox Desdroyed.... %d \n", --groupboxCounter);

    delete m_labelWin;
    delete m_circleBack;
}

void ACGroupBox::PositionLabelWindow()
{
    m_labelWin->Move(m_padding, m_padding);
    if (m_createIndex) {
        wxSize labelSize = m_labelWin->GetSize();
        m_circleBack->Move(labelSize.x, (labelSize.y - m_circleBack->GetSize().y) / 2 + m_padding);
    }
}


wxSize ACGroupBox::DoGetBestSize() const
{

    // Calculate the size needed by the label
    wxSize labelSize = m_labelWin->GetMinSize();
    wxSize best = wxSize(m_padding*2, m_padding*2) + labelSize;
    if (m_createIndex) {
        best = labelSize;
    }
    // If there is a sizer then the base best size is the sizer's minimum
    if (GetSizer() != NULL)
    {
        best.y += m_spacing;
        // children in sizer's Size
        wxSize childrenSize = GetSizer()->CalcMin();
        best = wxSize(std::max(labelSize.x, childrenSize.x), labelSize.y+childrenSize.y);
    } else {
        best.y += m_spacing;
        if (m_createIndex) {
            best.y -= m_spacing;
        }
    }
    // otherwise the best size falls back to the label size
    return best;
}

void ACGroupBox::ACGetBordersForSizer(int *borderTop, int *borderOther) const
{
    // Base class version doesn't leave enough space at the top when the label
    // is empty, so we can't use it here, even though the code is pretty
    // similar.
    if ( m_labelWin )
    {
        *borderTop = m_padding + m_labelWin->GetSize().y + m_spacing;
        if (m_createIndex) {
            *borderTop = m_labelWin->GetSize().y;
        }
    }
    else // No label window nor text.
    {
        *borderTop = m_padding;
    }

    *borderOther = m_padding;
}


bool ACGroupBox::SetFont(const wxFont& font)
{
    // We need to reposition the label as its size may depend on the font.
    if ( m_labelWin )
    {
        m_labelWin->SetFont(font);
        PositionLabelWindow();
    }

    return true;
}

static int groupboxSizerCounter = 0;

ACGroupBoxSizer::ACGroupBoxSizer(ACGroupBox *box, int orient) : wxBoxSizer(orient), m_groupBox(box)
{
    wxASSERT_MSG(box, wxT("ACGroupBoxSizer needs a static box"));

    // do this so that our Detach() is called if the static box is destroyed
    // before we are
    m_groupBox->SetContainingSizer(this);

    groupboxSizerCounter++;
}

ACGroupBoxSizer::~ACGroupBoxSizer()
{
    // As an exception to the general rule that sizers own other sizers that
    // they contain but not the windows managed by them, this sizer does own
    // the static box associated with it (which is not very logical but
    // convenient in practice and, most importantly, can't be changed any more
    // because of compatibility). However we definitely should not destroy the
    // children of this static box when we're being destroyed, as this would be
    // unexpected and break the existing code which worked with the windows
    // created as siblings of the static box instead of its children in the
    // previous wxWidgets versions, so ensure they are left alive.

    if ( m_groupBox )
        delete m_groupBox;
    printf("ACGroupBox Desdroyed.... %d \n", --groupboxSizerCounter);
}

void ACGroupBoxSizer::RepositionChildren(const wxSize &minSize)
{
    if (m_groupBox == nullptr)
        return;


    wxSize old_size( m_size );
    wxPoint old_pos( m_position );

    int top_border = 0, other_border = 0;

    m_groupBox->ACGetBordersForSizer(&top_border, &other_border);

    m_groupBox->PositionLabelWindow();

    m_groupBox->SetSize( m_position.x, m_position.y, m_size.x, m_size.y );

    m_position = wxPoint(0,0);

    m_size.x -= (2*other_border);
    m_size.y -= (top_border + other_border);

    m_position.x += other_border;
    m_position.y += top_border;

    wxBoxSizer::RepositionChildren(minSize);

    m_position = old_pos;
    m_size     = old_size;
}

wxSize ACGroupBoxSizer::CalcMin()
{
    int top_border, other_border;
    if (m_groupBox)
        m_groupBox->ACGetBordersForSizer(&top_border, &other_border);

    wxSize ret( wxBoxSizer::CalcMin() );

    ret.x += 2*other_border;

    // ensure that we're wide enough to show the static box label (there is no
    // need to check for the static box best size in vertical direction though)
    const int boxWidth = m_groupBox == nullptr ? 0 : m_groupBox->GetBestSize().x;
    ret.x = std::max(boxWidth, ret.x);

    ret.y += other_border + top_border;

    return ret;
}

void ACGroupBoxSizer::ShowItems(bool show)
{
    if (m_groupBox)
        m_groupBox->Show( show );
    wxBoxSizer::ShowItems( show );
}

bool ACGroupBoxSizer::AreAnyItemsShown() const
{

    // We don't need to check the status of our child items: if the box is
    // shown, this sizer should be considered shown even if all its elements
    // are hidden (or, more prosaically, there are no elements at all). And,
    // conversely, if the box is hidden then all our items, which are its
    // children, are hidden too.
    return m_groupBox && m_groupBox->IsShown();
}

bool ACGroupBoxSizer::Detach(wxWindow *window)
{
    wxASSERT_MSG( window, wxT("window can not be null") );
    // avoid deleting m_groupBox in our dtor if it's being detached from the
    // sizer (which can happen because it's being already destroyed for
    // example)
    if (window == m_groupBox )
    {
        m_groupBox = nullptr;
        return true;
    }

    return wxSizer::Detach(window);
}