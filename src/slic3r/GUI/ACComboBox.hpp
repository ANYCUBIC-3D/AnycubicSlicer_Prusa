#ifndef slic3r_GUI_ACComboBox_hpp_
#define slic3r_GUI_ACComboBox_hpp_

#include "ACTextInput.hpp"
#include "ACDropDown.hpp"

#define CB_NO_DROP_ICON DD_NO_CHECK_ICON
#define CB_NO_TEXT DD_NO_TEXT
namespace Slic3r { 
namespace GUI {

class ACComboBox : public wxWindowWithItems<ACTextInput, wxItemContainer>
{
    std::vector<wxString>         texts;
    std::vector<const wxBitmapBundle*>   icons;
    std::vector<void *>           datas;
    std::vector<wxClientDataType> types;

    ACDropDown drop;
    bool     drop_down = false;
    bool     text_off = false;

    int m_padding = 4;
    int m_spacing = 4;
public:
    ACComboBox(wxWindow *      parent,
             wxWindowID      id,
             const wxString &value     = wxEmptyString,
             const wxPoint & pos       = wxDefaultPosition,
             const wxSize &  size      = wxDefaultSize,
             int             n         = 0,
             const wxString  choices[] = NULL,
             long            style     = 0);
    
    ACDropDown & GetDropDown() { return drop; }

    virtual bool SetFont(wxFont const & font) override;

public:

    int Append(const wxString &item);

    int Append(const wxString &item, const wxBitmapBundle *bitmapBundle);

    int Append(const wxString &item, const wxBitmapBundle *bitmapBundle, void *clientData);

    unsigned int GetCount() const override;

    int  GetSelection() const override;

    void SetSelection(int n) override;

    virtual void Rescale() override;

    wxString GetValue() const;
    void     SetValue(const wxString &value);

    void SetLabel(const wxString &label) override;
    wxString GetLabel() const override;

    void SetTextLabel(const wxString &label);
    wxString GetTextLabel() const;

    wxString GetString(unsigned int n) const override;
    void     SetString(unsigned int n, wxString const &value) override;

    const wxBitmapBundle* GetItemBitmapBundle(unsigned int n);

    int GetItemsWidth();

	void SetCornerRadius(int value);
protected:
    virtual int  DoInsertItems(const wxArrayStringsAdapter &items,
                               unsigned int                 pos,
                               void **                      clientData,
                               wxClientDataType             type) override;
    virtual void DoClear() override;

    void DoDeleteOneItem(unsigned int pos) override;

    void *DoGetItemClientData(unsigned int n) const override;
    void  DoSetItemClientData(unsigned int n, void *data) override;
    
    void OnEdit() override;

#ifdef __WIN32__
    WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam) override;
#endif

private:

    // some useful events
    void mouseDown(wxMouseEvent &event);
    void mouseWheelMoved(wxMouseEvent &event);
    void keyDown(wxKeyEvent &event);

    void sendComboBoxEvent();

    DECLARE_EVENT_TABLE()
};
} // GUI
} // Slic3r

#endif // !slic3r_GUI_ComboBox_hpp_
