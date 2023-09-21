#ifndef slic3r_ACGcode_hpp_
#define slic3r_ACGcode_hpp_

#include "GUI.hpp"
#include "GUI_App.hpp"
#include "GUI_Utils.hpp"


namespace Slic3r {

namespace GUI {

class Plater;

enum class ACGcodeMode : int {
    ACGcode_None = 0,
    ACGcode_Show

};
struct ACGcode_Params
{
    ACGcode_Params();

    std::string last_gcode_file;

    ACGcodeMode mode;
};
class ACGcode
{
public:
    ACGcode();

    ~ACGcode() {}

    void ac_load_gcode(const wxString &filename);

    void ac_export_gcode();

    void ac_upload_gcode();

    void ac_remote_print();

    Plater *plater;
};
}} // namespace Slic3r

#endif /* slic3r_ACGcode_hpp_ */
