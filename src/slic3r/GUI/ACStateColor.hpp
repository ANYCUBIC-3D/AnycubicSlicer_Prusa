#ifndef slic3r_GUI_ACStateColor_hpp_
#define slic3r_GUI_ACStateColor_hpp_

#include <wx/colour.h>

class ACStateColor
{
public:
    enum State {
        Normal = 0, 
        Enabled = 1,
        Checked = 2,
        Focused = 4,
        Hovered = 8,
        Pressed = 16,
        Disabled = 1 << 16,
        NotChecked = 2 << 16,
        NotFocused = 4 << 16,
        NotHovered = 8 << 16,
        NotPressed = 16 << 16,
    };

public:
    template<typename ...Colors>
    ACStateColor(std::pair<Colors, int>... colors) {
        fill(colors...);
    }

    // single color
    ACStateColor(wxColour const & color);

    // single color
    ACStateColor(wxString const &color);

    // single color
    ACStateColor(unsigned long color );

    ACStateColor(){}
public:
    void append(wxColour const & color, int states);

    void append(wxString const &color, int states);

    void append(unsigned long color, int states);

    void clear();

public:
    int count() const { return statesList_.size(); }

    int states() const;

public:
    wxColour defaultColor();

    wxColour colorForStates(int states);

    int colorIndexForStates(int states);

    bool setColorForStates(wxColour const & color, int states);

    void setTakeFocusedAsHovered(bool set);

private:
    template<typename Color, typename ...Colors>
    void fill(std::pair<Color, int> color, std::pair<Colors, int>... colors) {
        fillOne(color);
        fill(colors...);
    }

    template<typename Color>
    void fillOne(std::pair<Color, int> color) {
        append(color.first, color.second);
    }

    void fill() {
    }

private:
    std::vector<int> statesList_;
    std::vector<wxColour> colors_;
    bool takeFocusedAsHovered_ = false;
};

#endif // !slic3r_GUI_ACStateColor_hpp_
