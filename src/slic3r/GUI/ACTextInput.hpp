#ifndef slic3r_GUI_ACTextInput_hpp_
#define slic3r_GUI_ACTextInput_hpp_

#include <wx/textctrl.h>
#include "ACStaticBox.hpp"
#include <wx/bmpbndl.h>
#include <wx/renderer.h>
#include "wxExtensions.hpp"

class ACTextInput : public wxNavigationEnabled<ACStaticBox>
{

    wxSize m_labelSize;
    ACStateColor     label_color;
    ACStateColor     text_color;
    wxTextCtrl * text_ctrl = nullptr;

    ScalableBitmap   m_icon ;
    wxSize           m_iconSize;

    int m_padding = 4;
    int m_spacing = 4;

    wxSize m_itemMinSize;
public:
    ACTextInput();

    ACTextInput(wxWindow *     parent,
              wxString       text,
              wxString       label = "",
              wxString       icon  = "",
              wxSize         iconSize = wxSize(16, 16),
              const wxPoint &pos   = wxDefaultPosition,
              const wxSize & size  = wxDefaultSize,
              long           style = 0);
   
public:
    void Create(wxWindow *     parent,
              wxString       text,
              wxString       label = "",
              wxString       icon  = "",
              wxSize         iconSize = wxSize(16, 16),
              const wxPoint &pos   = wxDefaultPosition,
              const wxSize & size  = wxDefaultSize,
              long           style = 0);

    void SetCornerRadius(double radius);


    void SetLabel(const wxString& label);

    void SetIcoSize(const wxSize &icoSize) { m_iconSize = icoSize; }

    void SetValue(const wxString& lable) { SetLabel(lable); }

    void SetIcon(const wxString& iconName, wxSize size);

    void SetLabelColor(ACStateColor const &color);

    void SetTextColor(ACStateColor const &color);

    virtual void Rescale();

    virtual bool Enable(bool enable = true) override;

    virtual void SetMinSize(const wxSize& size) override;

    wxTextCtrl *GetTextCtrl() { return text_ctrl; }

    wxTextCtrl const *GetTextCtrl() const { return text_ctrl; }

    wxString GetValue() const { return  wxWindow::GetLabel(); }

    void SetEditable(bool) const { }


protected:
    virtual void OnEdit() {}

    virtual void DoSetSize(
        int x, int y, int width, int height, int sizeFlags = wxSIZE_AUTO);

    void DoSetToolTipText(wxString const &tip) override;

    void messureSize();
private:
    void paintEvent(wxPaintEvent& evt);

    virtual void render(wxPaintDC& dc);

    DECLARE_EVENT_TABLE()
};

#endif // !slic3r_GUI_TextInput_hpp_
