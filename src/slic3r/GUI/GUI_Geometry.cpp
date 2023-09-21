#include "libslic3r/libslic3r.h"
#include "GUI_Geometry.hpp"
#include "I18N.hpp"

namespace Slic3r {
namespace GUI {

const double in_to_mm = 25.4;
const double mm_to_in = 1 / in_to_mm;

wxString coordinate_type_str(ECoordinatesType type)
{
    switch (type)
    {
    case ECoordinatesType::World:    { return _L("World coordinates"); }
    case ECoordinatesType::Instance: { return _L("Object coordinates"); }
    case ECoordinatesType::Local:    { return _L("Part coordinates"); }
    default:                         { assert(false); return _L("Unknown"); }
    }
}

} // namespace Slic3r
} // namespace GUI
