#ifndef slic3r_GUI_ACRADIOBOX_hpp_
#define slic3r_GUI_ACRADIOBOX_hpp_

#include "wxExtensions.hpp"

#include <wx/tglbtn.h>

namespace Slic3r { 
namespace GUI {

class ACRadioBox : public wxBitmapToggleButton
{
public:
    ACRadioBox(wxWindow *parent);

public:
    void SetValue(bool value) override;
	bool GetValue();
    void Rescale();
    bool Disable() { 
        return wxBitmapToggleButton::Disable(); 
    }
    bool Enable() { 
        return wxBitmapToggleButton::Enable(); 
    }

private:
    void update();

private:
    ScalableBitmap m_on;
    ScalableBitmap m_off;
};

}}



#endif // !slic3r_GUI_CheckBox_hpp_
