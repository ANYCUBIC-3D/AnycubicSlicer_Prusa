#include "GLGizmoLayout.hpp"
#include "slic3r/GUI/Gizmos/GizmoObjectManipulation.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/Plater.hpp"

namespace Slic3r {
namespace GUI {

GLGizmoLayout3D::GLGizmoLayout3D(GLCanvas3D& parent, const std::string& icon_filename, unsigned int sprite_id, GizmoObjectManipulation* obj_manipulation)
    : GLGizmoBase(parent, icon_filename, sprite_id)
    , m_object_manipulation(obj_manipulation)
{
}

bool GLGizmoLayout3D::on_init() 
{
    m_shortcut_key = WXK_CONTROL_A;
    return true;
}

bool GLGizmoLayout3D::on_is_activable() const
{
    return wxGetApp().plater()->can_arrange();
}

std::string GLGizmoLayout3D::on_get_name() const 
{
    return _u8L("Layout");
}

void GLGizmoLayout3D::on_render() 
{
    // do nothing
}

void GLGizmoLayout3D::on_render_input_window(float x, float y, float bottom_limit)
{
    if (m_object_manipulation)
        m_object_manipulation->do_render_ac_layout_window(m_imgui, get_name(), x, y, bottom_limit);
}


} // namespace GUI
} // namespace Slic3r
