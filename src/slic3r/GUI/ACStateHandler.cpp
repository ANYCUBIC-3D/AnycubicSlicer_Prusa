#include "ACStateHandler.hpp"

wxDEFINE_EVENT(EVT_ENABLE_CHANGED, wxCommandEvent);

ACStateHandler::ACStateHandler(wxWindow * owner)
    : owner_(owner)
{
    owner_->PushEventHandler(this);
    if (owner->IsEnabled())
        states_ |= Enabled;
    if (owner->HasFocus())
        states_ |= Focused;
}

ACStateHandler::~ACStateHandler() 
{ 
    owner_->RemoveEventHandler(this); 

    colors_.clear();

    update_binds();
}

void ACStateHandler::attach(ACStateColor const &color)
{
    colors_.push_back(&color);
}

void ACStateHandler::attach(std::vector<ACStateColor const *> const & colors)
{
    colors_.insert(colors_.end(), colors.begin(), colors.end());
}

void ACStateHandler::attach_child(wxWindow *child)
{
    auto ch = new ACStateHandler(this, child);
    children_.emplace_back(ch);
    ch->update_binds();
    states2_ |= ch->states();
}

void ACStateHandler::remove_child(wxWindow *child)
{
    children_.erase(std::remove_if(children_.begin(), children_.end(), 
            [child](auto &c) { return c->owner_ == child; }), children_.end());
    states2_ = 0;
    for (auto & c : children_) states2_ |= c->states();
}

void ACStateHandler::update_binds()
{
    int bind_states = parent_ ? (parent_->bind_states_ & ~Enabled) : 0;
    for (auto c : colors_) {
        bind_states |= c->states();
    }
    bind_states = bind_states | (bind_states >> 16);
    int diff = bind_states ^ bind_states_;
    State       states[] = {Enabled, Checked, Focused, Hovered, Pressed};
    wxEventType events[] = {EVT_ENABLE_CHANGED, wxEVT_CHECKBOX, wxEVT_SET_FOCUS, wxEVT_ENTER_WINDOW, wxEVT_LEFT_DOWN};
    wxEventType events2[] = {{0}, {0}, wxEVT_KILL_FOCUS, wxEVT_LEAVE_WINDOW, wxEVT_LEFT_UP};
    for (int i = 0; i < 5; ++i) {
        int s = states[i];
        if (diff & s) {
            if (bind_states & s) {
                Bind(events[i], &ACStateHandler::changed, this);
                if (events2[i])
                    Bind(events2[i], &ACStateHandler::changed, this);
            } else {
                Unbind(events[i], &ACStateHandler::changed, this);
                if (events2[i])
                    owner_->Unbind(events2[i], &ACStateHandler::changed, this);
            }
        }
    }
    bind_states_ = bind_states;
    for (auto &c : children_) c->update_binds();
}

ACStateHandler::ACStateHandler(ACStateHandler *parent, wxWindow *owner)
    : ACStateHandler(owner)
{
    states_ &= ~Enabled;
    parent_ = parent;
}

#include <iostream>
void debugEvent(int eventIdx, bool index1 = true) {
    wxString events[] = {"EVT_ENABLE_CHANGED", "wxEVT_CHECKBOX", "wxEVT_SET_FOCUS", "wxEVT_ENTER_WINDOW", "wxEVT_LEFT_DOWN"};
    wxString events2[] = {"", "", "wxEVT_KILL_FOCUS", "wxEVT_LEAVE_WINDOW", "wxEVT_LEFT_UP"};
    
    if (index1) {
        std::cout << events[eventIdx].c_str() << std::endl;
    } else {
        std::cout << events2[eventIdx].c_str() << std::endl;
    }
}


wxString eventTypeToStr(int t) 
{
    wxString str = "UNKNOW";

    if (t == EVT_ENABLE_CHANGED) str = "EVT_ENABLE_CHANGED"; 
    if (t == wxEVT_CHECKBOX    ) str = "wxEVT_CHECKBOX    "; 
    if (t == wxEVT_SET_FOCUS   ) str = "wxEVT_SET_FOCUS   "; 
    if (t == wxEVT_ENTER_WINDOW) str = "wxEVT_ENTER_WINDOW"; 
    if (t == wxEVT_LEFT_DOWN   ) str = "wxEVT_LEFT_DOWN   "; 
    if (t == wxEVT_KILL_FOCUS  ) str = "wxEVT_KILL_FOCUS  "; 
    if (t == wxEVT_LEAVE_WINDOW) str = "wxEVT_LEAVE_WINDOW"; 
    if (t == wxEVT_LEFT_UP     ) str = "wxEVT_LEFT_UP     "; 

    return str;
}

void ACStateHandler::changed(wxEvent &event)
{
    event.Skip();
    wxEventType events[] = {EVT_ENABLE_CHANGED, wxEVT_CHECKBOX, wxEVT_SET_FOCUS, wxEVT_ENTER_WINDOW, wxEVT_LEFT_DOWN};
    wxEventType events2[] = {{0}, {0}, wxEVT_KILL_FOCUS, wxEVT_LEAVE_WINDOW, wxEVT_LEFT_UP};
    int old = states_;
    // some events are from another window (ex: text_ctrl of TextInput), save state in states2_ to avoid conflicts
    for (int i = 0; i < 5; ++i) {
        if (events2[i]) {
            if (event.GetEventType() == events[i]) {
                states_ |= 1 << i;
                break;
            } else if (event.GetEventType() == events2[i]) {
                states_ &= ~(1 << i);
                break;
            }
        }
        else { // events2[i] == 0
            if (i == 1) {
                if (event.GetEventType() == events[i] ) {
                    wxCommandEvent* ce = (wxCommandEvent*)&event;
                    if (ce->GetInt()) {
                        states_ |= 1 << i;
                    } else { // == 0
                        states_ &= ~(1 << i);
                    }
                    break;
                }
            } else {
                if (event.GetEventType() == events[i] ) {
                    states_ ^= (1 << i);
                    break;
                }            
            }

        }
    }
    if (old != states_ && (old | states2_) != (states_ | states2_)) {
        if (parent_) {
            printf("ACStateHandler::event: %s, change \n", eventTypeToStr(event.GetEventType()).ToStdString().c_str());
            parent_->changed(states_ | states2_);
        }
        else {
            printf("ACStateHandler::event: %s, Refresh \n", eventTypeToStr(event.GetEventType()).ToStdString().c_str());
            owner_->Refresh();
        }
    }
}

void ACStateHandler::changed(int)
{
    int old = states2_;
    states2_ = 0;
    for (auto &c : children_) states2_ |= c->states();
    if (old != states2_ && (old | states_) != (states_ | states2_)) {
        if (parent_) {
            printf("ACStateHandler::int, change \n");
            parent_->changed(states_ | states2_);
        }
        else {
            printf("ACStateHandler::int, Refresh \n");
            owner_->Refresh();
        }
    }
}
