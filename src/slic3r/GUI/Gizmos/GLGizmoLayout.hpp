#ifndef slic3r_GLGizmoLayout_hpp_
#define slic3r_GLGizmoLayout_hpp_

#include "GLGizmoBase.hpp"


namespace Slic3r {
namespace GUI {

#if ENABLE_WORLD_COORDINATE
class Selection;
#endif // ENABLE_WORLD_COORDINATE
class GizmoObjectManipulation;
class GLGizmoLayout3D : public GLGizmoBase
{

public:
    GLGizmoLayout3D(GLCanvas3D& parent, const std::string& icon_filename, unsigned int sprite_id, GizmoObjectManipulation* obj_manipulation);
    virtual ~GLGizmoLayout3D() = default;

protected:
    virtual bool on_init() override;
    virtual std::string on_get_name() const override;
    virtual void on_render() override;
    virtual bool on_is_activable() const override;
    virtual void on_render_input_window(float x, float y, float bottom_limit) override;
private:
protected:
    GizmoObjectManipulation* m_object_manipulation = nullptr;
};


} // namespace GUI
} // namespace Slic3r

#endif // slic3r_GLGizmoLayout_hpp_