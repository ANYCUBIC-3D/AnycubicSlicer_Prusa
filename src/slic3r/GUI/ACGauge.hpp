#ifndef slic3r_ACGauge_hpp_
#define slic3r_ACGauge_hpp_

#include "GUI.hpp"
#include "GUI_App.hpp"
#include "GUI_Utils.hpp"

#include "ACStateHandler.hpp"

#include <wx/window.h>


class ACGauge : public wxWindow
{
public:
    ACGauge(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size);
    void SetValue(int value);
	int GetValue();
	void SetShow(bool show);
    wxPoint GetArcPoint(int value);
    void Rescale();
    wxPoint m_endPoint;
    void    InstallNowEvent(wxMouseEvent &event);

protected:
    void paintEvent(wxPaintEvent &evt);
    DECLARE_EVENT_TABLE()

private:
    int m_value; 
    wxSize m_size;
    int    m_penWidth;
    bool   m_gaugeIndex{false};
};


#endif /* slic3r_ACGauge_hpp_ */
