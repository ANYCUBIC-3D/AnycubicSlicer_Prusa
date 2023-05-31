/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/statbox.h
// Purpose:     ACGroupBox class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_ACGROUPBOX_H_
#define _WX_MSW_ACGROUPBOX_H_

//#include "wx/compositewin.h"
//#include "wx/statbox.h"

#include "ACStaticBox.hpp"

class ACButton;
// Group box
class ACGroupBox : public ACStaticBox
{
public:
    ACGroupBox(wxWindow *parent
                ,  const wxString& label,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0)
        : ACStaticBox(parent, id, pos, size, style)
    {
        Create(parent, label, id, pos, size, style);
    }

    ~ACGroupBox();

    bool Create(wxWindow *parent
                , const wxString& label
                , wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0);

    /// Implementation only
    void ACGetBordersForSizer(int *borderTop, int *borderOther) const;

    virtual bool SetFont(const wxFont& font) wxOVERRIDE;

    // returns true if the platform should explicitly apply a theme border
    //virtual bool CanApplyThemeBorder() const wxOVERRIDE { return false; }

    void PositionLabelWindow();
protected:
    virtual wxSize DoGetBestSize() const;

private:

    ACButton* m_labelWin = nullptr;

    int m_padding = 12;
    int m_spacing = 12;
};

// Indicate that we have the ctor overload taking wxWindow as label.
#define wxHAS_WINDOW_LABEL_IN_STATIC_BOX

//
class ACGroupBoxSizer : public wxBoxSizer
{
public:
    ACGroupBoxSizer(ACGroupBox *box, int orient);
    virtual ~ACGroupBoxSizer();

    virtual wxSize CalcMin();
    virtual void   RepositionChildren(const wxSize &minSize);

    ACGroupBox *GetGroupBox() const { return m_groupBox; }

    // override to hide/show the static box as well
    virtual void ShowItems(bool show);
    virtual bool AreAnyItemsShown() const;

    virtual bool Detach(wxWindow *window);
    virtual bool Detach(wxSizer *sizer) { return wxBoxSizer::Detach(sizer); }
    virtual bool Detach(int index) { return wxBoxSizer::Detach(index); }

protected:
    ACGroupBox *m_groupBox;

private:
    wxDECLARE_CLASS(ACGroupBoxSizer);
    wxDECLARE_NO_COPY_CLASS(ACGroupBoxSizer);
};

#endif // _WX_MSW_ACGROUPBOX_H_
