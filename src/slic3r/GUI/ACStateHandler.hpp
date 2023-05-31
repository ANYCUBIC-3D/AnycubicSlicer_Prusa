#ifndef slic3r_GUI_ACStateHandler_hpp_
#define slic3r_GUI_ACStateHandler_hpp_

#include <wx/event.h>

#include "ACStateColor.hpp"

#include <memory>

wxDECLARE_EVENT(EVT_ENABLE_CHANGED, wxCommandEvent);

class ACStateHandler : public wxEvtHandler
{
public:
    enum State {
        Enabled    = 1,
        Checked    = 2,
        Focused    = 4,
        Hovered    = 8,
        Pressed    = 16,
        Disabled   = 1 << 16,
        NotChecked = 2 << 16,
        NotFocused = 4 << 16,
        NotHovered = 8 << 16,
        NotPressed = 16 << 16,
    };

public:
    ACStateHandler(wxWindow *owner);

    ~ACStateHandler();

public:
    void attach(ACStateColor const &color);

    void attach(std::vector<ACStateColor const *> const &colors);

    void attach_child(wxWindow *child);

    void remove_child(wxWindow *child);

    void update_binds();

    int states() const { return states_ | states2_; }

    void clearAttach() { colors_.clear(); };

    void changed(wxEvent &event);
private:
    ACStateHandler(ACStateHandler *parent, wxWindow *owner);


    void changed(int state2);

private:
    wxWindow                                    *owner_;
    std::vector<ACStateColor const *>            colors_;
    int                                          bind_states_ = 0;
    int                                          states_      = 0;
    int                                          states2_     = 0; // from children
    std::vector<std::unique_ptr<ACStateHandler>> children_;
    ACStateHandler                              *parent_ = nullptr;
};

#endif // !slic3r_GUI_StateHandler_hpp_
