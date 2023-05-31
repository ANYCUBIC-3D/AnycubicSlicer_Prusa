#include "slic3r/GUI/ImGuiWrapper.hpp"
#include <imgui/imgui_internal.h>

#include "GizmoObjectManipulation.hpp"
#include "slic3r/GUI/GUI_ObjectList.hpp"
//#include "I18N.hpp"
#include "GLGizmosManager.hpp"
#include "slic3r/GUI/GLCanvas3D.hpp"

#include "slic3r/GUI/GUI_App.hpp"
#include "libslic3r/AppConfig.hpp"

#include "libslic3r/Model.hpp"
#include "libslic3r/Geometry.hpp"
#include "slic3r/GUI/Selection.hpp"
#include "slic3r/GUI/Plater.hpp"
#include "slic3r/GUI/MainFrame.hpp"
#include "slic3r/GUI/I18N.hpp"
#include "libslic3r/Print.hpp"
#include "libslic3r/BuildVolume.hpp"

#include <boost/algorithm/string.hpp>

#define MAX_NUM 9999.99
#define MAX_SIZE "9999.99"

namespace Slic3r
{
namespace GUI
{

const double GizmoObjectManipulation::in_to_mm = 25.4;
const double GizmoObjectManipulation::mm_to_in = 0.0393700787;
const double GizmoObjectManipulation::oz_to_g = 28.34952;
const double GizmoObjectManipulation::g_to_oz = 0.035274;

// Helper function to be used by drop to bed button. Returns lowest point of this
// volume in world coordinate system.
static double get_volume_min_z(const GLVolume* volume)
{
    const Transform3f& world_matrix = volume->world_matrix().cast<float>();

    // need to get the ModelVolume pointer
    const ModelObject* mo = wxGetApp().model().objects[volume->composite_id.object_id];
    const ModelVolume* mv = mo->volumes[volume->composite_id.volume_id];
    const TriangleMesh& hull = mv->get_convex_hull();

    float min_z = std::numeric_limits<float>::max();
    for (const stl_vertex& vert : hull.its.vertices) {
        min_z = std::min(min_z, Vec3f::UnitZ().dot(world_matrix * vert));
    }
    return min_z;
}

GizmoObjectManipulation::GizmoObjectManipulation(GLCanvas3D& glcanvas)
    : m_glcanvas(glcanvas)
{
    m_imperial_units = wxGetApp().app_config->get("use_inches") == "1";
    m_new_unit_string = m_imperial_units ? L("in") : L("mm");
}

void GizmoObjectManipulation::UpdateAndShow(const bool show)
{
	if (show) {
        this->set_dirty();
		this->update_if_dirty();
	}
}

void GizmoObjectManipulation::update_ui_from_settings()
{
    if (m_imperial_units != (wxGetApp().app_config->get("use_inches") == "1")) {
        m_imperial_units = wxGetApp().app_config->get("use_inches") == "1";

        m_new_unit_string = m_imperial_units ? L("in") : L("mm");

        update_buffered_value();
    }
}

void GizmoObjectManipulation::update_settings_value(const Selection& selection)
{
	m_new_move_label_string   = L("Position");
    m_new_rotate_label_string = L("Rotation");
    m_new_scale_label_string  = L("Scale ratios");

    m_world_coordinates = true;

    ObjectList* obj_list = wxGetApp().obj_list();
    if (selection.is_single_full_instance()) {
        // all volumes in the selection belongs to the same instance, any of them contains the needed instance data, so we take the first one
        const GLVolume* volume = selection.get_volume(*selection.get_volume_idxs().begin());
        //Vec3d offset = volume->get_instance_offset();
        //const BoundingBoxf3& box = volume->bounding_box();
        //m_new_position = Vec3d(offset.x(), offset.y(), offset.z()-(box.size()/2).z());

        m_new_position = volume->get_instance_offset();

        if (m_world_coordinates) {
			m_new_rotate_label_string = L("Rotate");
            m_new_rotation = volume->get_instance_rotation() * (180. / M_PI);
			m_new_size     = selection.get_scaled_instance_bounding_box().size();
			m_new_scale    = m_new_size.cwiseProduct(selection.get_unscaled_instance_bounding_box().size().cwiseInverse()) * 100.;
		} 
        else {
			m_new_rotation = volume->get_instance_rotation() * (180. / M_PI);
			m_new_size     = volume->get_instance_transformation().get_scaling_factor().cwiseProduct(wxGetApp().model().objects[volume->object_idx()]->raw_mesh_bounding_box().size());
			m_new_scale    = volume->get_instance_scaling_factor() * 100.;
		}

        m_new_enabled  = true;
        // BBS: change "Instance Operations" to "Object Operations"
        m_new_title_string = L("Object Operations");
    }
    else if (selection.is_single_full_object() && obj_list->is_selected(itObject)) {
        const BoundingBoxf3& box = selection.get_bounding_box();
        //m_new_position = Vec3d(box.center().x(), box.center().y(), box.min.z());
        m_new_position = box.center();
        m_new_rotation = Vec3d::Zero();
        m_new_scale    = Vec3d(100., 100., 100.);
        m_new_size     = box.size();
        m_new_rotate_label_string = L("Rotate");
		m_new_scale_label_string  = L("Scale");
        m_new_enabled  = true;
        m_new_title_string = L("Object Operations");
    }
    else if (selection.is_single_modifier() || selection.is_single_volume()) {
        // the selection contains a single volume
        const GLVolume* volume = selection.get_volume(*selection.get_volume_idxs().begin());
        //Vec3d offset = volume->get_instance_offset();
        //const BoundingBoxf3& box = volume->bounding_box();
        //m_new_position = Vec3d(offset.x(), offset.y(), offset.z()-(box.size()/2).z());

        m_new_position = volume->get_instance_offset();
        m_new_rotation = volume->get_volume_rotation() * (180. / M_PI);
        m_new_scale    = volume->get_volume_scaling_factor() * 100.;
        m_new_size     = volume->get_instance_transformation().get_scaling_factor().cwiseProduct(volume->get_volume_transformation().get_scaling_factor().cwiseProduct(volume->bounding_box().size()));
        m_new_enabled = true;
        m_new_title_string = L("Volume Operations");
    }
    else if (obj_list->multiple_selection() || obj_list->is_selected(itInstanceRoot)) {
        reset_settings_value();
		m_new_move_label_string   = L("Translate");
		m_new_rotate_label_string = L("Rotate");
		m_new_scale_label_string  = L("Scale");
        m_new_size = selection.get_bounding_box().size();
        m_new_enabled  = true;
        m_new_title_string = L("Group Operations");
    }
	else {
        // No selection, reset the cache.
//		assert(selection.is_empty());
		reset_settings_value();
	}
}

void GizmoObjectManipulation::update_buffered_value()
{
    if (this->m_imperial_units)
        m_buffered_position = this->m_new_position * this->mm_to_in;
    else
        m_buffered_position = this->m_new_position;

    m_buffered_rotation = this->m_new_rotation;

    m_buffered_scale = this->m_new_scale;

    if (this->m_imperial_units)
        m_buffered_size = this->m_new_size * this->mm_to_in;
    else
        m_buffered_size = this->m_new_size;
}

void GizmoObjectManipulation::update_if_dirty()
{
    if (! m_dirty)
        return;

    const Selection &selection = m_glcanvas.get_selection();
    this->update_settings_value(selection);
    this->update_buffered_value();

    auto update_label = [](wxString &label_cache, const std::string &new_label) {
        wxString new_label_localized = _(new_label) + ":";
        if (label_cache != new_label_localized) {
            label_cache = new_label_localized;
        }
    };
    update_label(m_cache.move_label_string,   m_new_move_label_string);
    update_label(m_cache.rotate_label_string, m_new_rotate_label_string);
    update_label(m_cache.scale_label_string,  m_new_scale_label_string);

    enum ManipulationEditorKey
    {
        mePosition = 0,
        meRotation,
        meScale,
        meSize
    };

    for (int i = 0; i < 3; ++ i) {
        auto update = [this, i](Vec3d &cached, Vec3d &cached_rounded,  const Vec3d &new_value) {
			//wxString new_text = double_to_string(new_value(i), 2);
			double new_rounded = round(new_value(i)*100)/100.0;
			//new_text.ToDouble(&new_rounded);
			if (std::abs(cached_rounded(i) - new_rounded) > EPSILON) {
				cached_rounded(i) = new_rounded;
                //const int id = key_id*3+i;
                //if (m_imperial_units && (key_id == mePosition || key_id == meSize))
                //    new_text = double_to_string(new_value(i)*mm_to_in, 2);
                //if (id >= 0) m_editors[id]->set_value(new_text);
            }
			cached(i) = new_value(i);
		};
        update(m_cache.position, m_cache.position_rounded,  m_new_position);
        update(m_cache.scale,    m_cache.scale_rounded,     m_new_scale);
        update(m_cache.size,     m_cache.size_rounded,      m_new_size);
        update(m_cache.rotation, m_cache.rotation_rounded,  m_new_rotation);
    }

    update_reset_buttons_visibility();
    //update_mirror_buttons_visibility();

    m_dirty = false;
}

void GizmoObjectManipulation::update_reset_buttons_visibility()
{
    const Selection& selection = m_glcanvas.get_selection();

    if (selection.is_single_full_instance() || selection.is_single_modifier() || selection.is_single_volume()) {
        const GLVolume* volume = selection.get_volume(*selection.get_volume_idxs().begin());
        Vec3d rotation;
        Vec3d scale;
        double min_z = 0.;

        if (selection.is_single_full_instance()) {
            rotation = volume->get_instance_rotation();
            scale = volume->get_instance_scaling_factor();
        }
        else {
            rotation = volume->get_volume_rotation();
            scale = volume->get_volume_scaling_factor();
            min_z = get_volume_min_z(volume);
        }
        m_show_clear_rotation = !rotation.isApprox(Vec3d::Zero());
        m_show_clear_scale = !scale.isApprox(Vec3d::Ones(), EPSILON);
        m_show_drop_to_bed = (std::abs(min_z) > EPSILON);
    }
}


void GizmoObjectManipulation::reset_settings_value()
{
    m_buffered_rotation = Vec3d::Zero();
    m_new_position = Vec3d::Zero();
    m_new_rotation = Vec3d::Zero();
    m_new_scale = Vec3d::Ones() * 100.;
    m_new_size = Vec3d::Zero();
    m_new_enabled = false;
    // no need to set the dirty flag here as this method is called from update_settings_value(),
    // which is called from update_if_dirty(), which resets the dirty flag anyways.
//    m_dirty = true;
}

void GizmoObjectManipulation::change_position_value(int axis, double value)
{
    if (std::abs(m_cache.position_rounded(axis) - value) < EPSILON)
        return;

    Vec3d position = m_cache.position;
    position(axis) = value;

    Selection& selection = m_glcanvas.get_selection();
    selection.start_dragging();
    selection.translate(position - m_cache.position, selection.requires_local_axes());
    m_glcanvas.do_move(L("Set Position"));

    m_cache.position = position;
	m_cache.position_rounded(axis) = DBL_MAX;
    this->UpdateAndShow(true);
}

void GizmoObjectManipulation::change_rotation_value(int axis, double value)
{
    if (std::abs(m_cache.rotation_rounded(axis) - value) < EPSILON)
        return;

    if (std::abs(value) < EPSILON)
        value = 0;

    Vec3d rotation = m_cache.rotation;
    rotation(axis) = value;

    Selection& selection = m_glcanvas.get_selection();

    TransformationType transformation_type(TransformationType::World_Relative_Joint);
    if (selection.is_single_full_instance() || selection.requires_local_axes())
		transformation_type.set_independent();
	if (selection.is_single_full_instance() && ! m_world_coordinates) {
        //FIXME Selection::rotate() does not process absoulte rotations correctly: It does not recognize the axis index, which was changed.
		// transformation_type.set_absolute();
		transformation_type.set_local();
	}

    selection.start_dragging();
	selection.rotate(
		(M_PI / 180.0) * (transformation_type.absolute() ? rotation : rotation - m_cache.rotation), 
		transformation_type);
    m_glcanvas.do_rotate(L("Set Orientation"));

    m_cache.rotation = rotation;
	m_cache.rotation_rounded(axis) = DBL_MAX;
    this->UpdateAndShow(true);
}

void GizmoObjectManipulation::change_scale_value(int axis, double value)
{
    if (std::abs(m_cache.scale_rounded(axis) - value) < EPSILON)
        return;

    Vec3d scale = m_cache.scale;
    if (scale[axis] != 0 && std::abs(m_cache.size[axis] * value / scale[axis]) > MAX_NUM) {
        scale[axis] *= MAX_NUM / m_cache.size[axis];
    }
    else {
        scale(axis) = value;
    }

    //this->do_scale(axis, scale);
    this->do_scale(axis, 0.01 * scale);

    m_cache.scale = scale;
	m_cache.scale_rounded(axis) = DBL_MAX;
	this->UpdateAndShow(true);
}

void GizmoObjectManipulation::change_size_value(int axis, double value)
{
    if (std::abs(m_cache.size_rounded(axis) - value) < EPSILON)
        return;

    Vec3d size = m_cache.size;
    size(axis) = value;

    const Selection& selection = m_glcanvas.get_selection();

    Vec3d ref_size = m_cache.size;
	if (selection.is_single_volume() || selection.is_single_modifier())
        ref_size = selection.get_volume(*selection.get_volume_idxs().begin())->bounding_box().size();
    else if (selection.is_single_full_instance())
		ref_size = m_world_coordinates ? 
            selection.get_unscaled_instance_bounding_box().size() :
            wxGetApp().model().objects[selection.get_volume(*selection.get_volume_idxs().begin())->object_idx()]->raw_mesh_bounding_box().size();
    //this->do_scale(axis, 100. * Vec3d(size(0) / ref_size(0), size(1) / ref_size(1), size(2) / ref_size(2)));
    this->do_scale(axis, size.cwiseQuotient(ref_size));
    m_cache.size = size;
	m_cache.size_rounded(axis) = DBL_MAX;
	this->UpdateAndShow(true);
}

void GizmoObjectManipulation::do_scale(int axis, const Vec3d &scale) const
{
    Selection& selection = m_glcanvas.get_selection();
    Vec3d scaling_factor = scale;

    TransformationType transformation_type(TransformationType::World_Relative_Joint);
    if (selection.is_single_full_instance()) {
        transformation_type.set_absolute();
        if (! m_world_coordinates)
            transformation_type.set_local();
    }

    // BBS: when select multiple objects, uniform scale can be deselected
    if (m_uniform_scale/* || selection.requires_uniform_scale()*/)
        scaling_factor = scale(axis) * Vec3d::Ones();

    selection.start_dragging();
    //selection.scale(scaling_factor * 0.01, transformation_type);
    selection.scale(scaling_factor , transformation_type);
    selection.stop_dragging();
    m_glcanvas.do_scale(L("Set Scale"));
}

void GizmoObjectManipulation::on_change(const std::string& opt_key, int axis, double new_value)
{
    if (!m_cache.is_valid())
        return;

    if (m_imperial_units && (opt_key == "position" || opt_key == "size"))
        new_value *= in_to_mm;

    if (opt_key == "position")
        change_position_value(axis, new_value);
    else if (opt_key == "rotation")
        change_rotation_value(axis, new_value);
    else if (opt_key == "scale")
        change_scale_value(axis, new_value);
    else if (opt_key == "size")
        change_size_value(axis, new_value);
}

void GizmoObjectManipulation::move_position_value_to_center()
{
    const Selection& selection = m_glcanvas.get_selection();
    const BoundingBoxf3& bounding_box = selection.get_bounding_box();
    Vec3d selObjectsCenter = bounding_box.center();

    const BuildVolume& build_volume = wxGetApp().plater()->build_volume();
    Vec2d bedCenter = build_volume.bed_center();

    Vec2d offset = bedCenter - Vec2d(selObjectsCenter.x(), selObjectsCenter.y());

    for (unsigned int idx : selection.get_volume_idxs()) {
        GLVolume* volume = const_cast<GLVolume*>(selection.get_volume(idx));
        ;
        volume->set_instance_offset(volume->get_instance_offset() + Vec3d(offset.x(), offset.y(), 0));
    }


    m_glcanvas.do_move(L("Move To Center"));

    UpdateAndShow(true);
}

void GizmoObjectManipulation::reset_position_value()
{
    Selection& selection = m_glcanvas.get_selection();

    if (selection.is_single_volume() || selection.is_single_modifier()) {
        GLVolume* volume = const_cast<GLVolume*>(selection.get_volume(*selection.get_volume_idxs().begin()));
        volume->set_volume_offset(Vec3d::Zero());
    }
    else if (selection.is_single_full_instance()) {
        for (unsigned int idx : selection.get_volume_idxs()) {
            GLVolume* volume = const_cast<GLVolume*>(selection.get_volume(idx));
            volume->set_instance_offset(Vec3d::Zero());
        }
    }
    else
        return;

    // Copy position values from GLVolumes into Model (ModelInstance / ModelVolume), trigger background processing.
    m_glcanvas.do_move(L("Reset Position"));

    UpdateAndShow(true);
}

void GizmoObjectManipulation::reset_rotation_value()
{
    Selection& selection = m_glcanvas.get_selection();

    if (selection.is_single_volume() || selection.is_single_modifier()) {
        GLVolume* volume = const_cast<GLVolume*>(selection.get_volume(*selection.get_volume_idxs().begin()));
        volume->set_volume_rotation(Vec3d::Zero());
    }
    else if (selection.is_single_full_instance()) {
        for (unsigned int idx : selection.get_volume_idxs()) {
            GLVolume* volume = const_cast<GLVolume*>(selection.get_volume(idx));
            volume->set_instance_rotation(Vec3d::Zero());
        }
    }
    else
        return;

    // Update rotation at the GLVolumes.
    selection.synchronize_unselected_instances(Selection::SyncRotationType::GENERAL);
    selection.synchronize_unselected_volumes();
    // Copy rotation values from GLVolumes into Model (ModelInstance / ModelVolume), trigger background processing.
    m_glcanvas.do_rotate(L("Reset Rotation"));

    UpdateAndShow(true);
}

void GizmoObjectManipulation::reset_scale_value()
{
    Plater::TakeSnapshot snapshot(wxGetApp().plater(), std::string("Reset scale"));

    change_scale_value(0, 100.);
    change_scale_value(1, 100.);
    change_scale_value(2, 100.);
}

void GizmoObjectManipulation::set_uniform_scaling(const bool new_value)
{ 
    const Selection &selection = m_glcanvas.get_selection();
	if (selection.is_single_full_instance() && m_world_coordinates && !new_value) {
        // Verify whether the instance rotation is multiples of 90 degrees, so that the scaling in world coordinates is possible.
        // all volumes in the selection belongs to the same instance, any of them contains the needed instance data, so we take the first one
        const GLVolume* volume = selection.get_volume(*selection.get_volume_idxs().begin());
        // Is the angle close to a multiple of 90 degrees?
		if (! Geometry::is_rotation_ninety_degrees(volume->get_instance_rotation())) {
            // Cannot apply scaling in the world coordinate system.
            // BBS: remove tilt prompt dialog

            // Bake the rotation into the meshes of the object.
            wxGetApp().model().objects[volume->composite_id.object_id]->bake_xy_rotation_into_meshes(volume->composite_id.instance_id);
            // Update the 3D scene, selections etc.
            wxGetApp().plater()->update();
            // Recalculate cached values at this panel, refresh the screen.
            this->UpdateAndShow(true);
        }
    }
    m_uniform_scale = new_value;
}

static const char* label_values[2][3] = {
{ "##position_x", "##position_y", "##position_z"},
{ "##rotation_x", "##rotation_y", "##rotation_z"}
};

static const char* label_scale_values[2][3] = {
{ "##scale_x", "##scale_y", "##scale_z"},
{ "##size_x", "##size_y", "##size_z"}
};

bool GizmoObjectManipulation::reset_button(ImGuiWrapper *imgui_wrapper, float caption_max, float unit_size, float space_size, float end_text_size)
{
    bool        pressed   = false;
    ImTextureID normal_id = m_glcanvas.get_gizmos_manager().get_icon_texture_id(GLGizmosManager::MENU_ICON_NAME::IC_TOOLBAR_RESET);
    ImTextureID hover_id  = m_glcanvas.get_gizmos_manager().get_icon_texture_id(GLGizmosManager::MENU_ICON_NAME::IC_TOOLBAR_RESET_HOVER);

    float font_size = ImGui::GetFontSize();
    ImVec2 button_size = ImVec2(font_size, font_size);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

    pressed = ImGui::ImageButton3(normal_id, hover_id, button_size);

    ImGui::PopStyleVar(1);
    return pressed;
}

 float GizmoObjectManipulation::max_unit_size(int number, Vec3d &vec1, Vec3d &vec2,std::string str)
 {
     if (number <= 1) return -1;
     Vec3d vec[2] = {vec1, vec2};
     float nuit_max[4] = {0};
     float vec_max = 0, unit_size = 0;

     for (int i = 0; i < number; i++)
     {
         char buf[3][64] = {0};
         float buf_size[3] = {0};
         for (int j = 0; j < 3; j++) {
             ImGui::DataTypeFormatString(buf[j], IM_ARRAYSIZE(buf[j]), ImGuiDataType_Double, (void *) &vec[i][j], "%.2f");
             buf_size[j]  = ImGui::CalcTextSize(buf[j]).x;
             vec_max = std::max(buf_size[j], vec_max);
             nuit_max[i]  = vec_max;
         }
         unit_size = std::max(nuit_max[i], unit_size);
     }

     return unit_size + 8.0;
 }

void GizmoObjectManipulation::do_render_ac_move_window(ImGuiWrapper *imgui_wrapper, std::string window_name, float x, float y, float bottom_limit)
{
    static float last_move_win_h = 0.0f;

    const float approx_height = last_move_win_h ;
    y = std::min(y, bottom_limit - approx_height);
    float _scaleIndex         = imgui_wrapper->get_style_scaling();

    imgui_wrapper->set_next_window_pos(x + 10.0f * _scaleIndex, y,ImGuiCond_Always);
    //imgui_wrapper->set_next_window_size(218.0f * _scaleIndex, 215.0f * _scaleIndex,ImGuiCond_None);
    // BBS
    ImGuiWrapper::push_ac_toolwin_style(m_glcanvas.get_scale());
    //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 6.0));

    std::string name = this->m_new_title_string + "##" + window_name;
    //ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImGuiWrapper::COL_AC_BLUE;
    //ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.05f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWrapper::COL_AC_WHITE);
    ImGui::PushFont(imgui_wrapper->bold_font_14);
    imgui_wrapper->begin(_L(window_name), ImGuiWrapper::TOOLBAR_WINDOW_FLAGS_AC_NEW);
    ImGui::PopFont();
    ImGui::PopStyleColor();
    auto update = [this](unsigned int active_id, std::string opt_key, Vec3d original_value, Vec3d new_value) -> int {
        for (int i = 0; i < 3; i++) {
            if (original_value[i] != new_value[i]) {
                if (active_id != m_last_active_item) {
                    on_change(opt_key, i, new_value[i]);
                    return i;
                }
            }
        }
        return -1;
    };
    
    float space_size    = imgui_wrapper->get_style_scaling() * 6;
    //float position_size = imgui_wrapper->calc_text_size(_L("Position")).x + space_size;
    //float World_size    = imgui_wrapper->calc_text_size(_L("World coordinates")).x + space_size;
    //float caption_max   = std::max(position_size, World_size) + 2 * space_size;
    float end_text_size = imgui_wrapper->calc_text_size(this->m_new_unit_string).x;

    // position
    Vec3d original_position;
    if (this->m_imperial_units)
        original_position = this->m_new_position * this->mm_to_in;
    else
        original_position = this->m_new_position;
    Vec3d display_position = m_buffered_position;

    //// Rotation
    //Vec3d rotation   = this->m_buffered_rotation;
    float title_size = imgui_wrapper->calc_text_size(window_name).x ;
    float axis_size = imgui_wrapper->calc_text_size(wxString("X")).x + space_size;
    float unit_size = imgui_wrapper->calc_text_size(wxString(MAX_SIZE)).x + space_size*2;
    int   index      = 1;
    int   index_unit = 1;
    ImGui::PushFont(imgui_wrapper->default_font_13);
    
    ImTextureID normal_id = m_glcanvas.get_gizmos_manager().get_icon_texture_id(GLGizmosManager::MENU_ICON_NAME::IC_TITLE_CIRCLE);

    // render title
    //ImGui::AlignTextToFramePadding();
    unsigned int current_active_id = ImGui::GetActiveID();
    
    //ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2 * _scaleIndex);
    //ImGui::Image(normal_id, ImVec2(_scaleIndex * 6, _scaleIndex * 6));
    //ImGui::SameLine(/*space_size*2*/);
    ////ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 6 * _scaleIndex);
    //ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 8 * _scaleIndex);
    ////ImGui::SetWindowFontScale(ImGui::GetIO().FontGlobalScale * 1.7f);
    //
    //ImGui::TextColored(ImVec4(0,0,0,1), window_name.c_str());
    //// render line 1
    //ImGui::Separator();
    
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10 * _scaleIndex);
    ImGui::TextColored(ImGuiWrapper::COL_AC_RED, "X");
    ImGui::SameLine(/*axis_size + space_size*/);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * _scaleIndex);
    imgui_wrapper->push_ac_inputdouble_style(ImGui::GetCursorPosX() + 10.0f *
                                                 _scaleIndex,
                                             6.0f *
                                                 _scaleIndex,
                                             3.0f * _scaleIndex,
                                             imgui_wrapper->default_font_13);
    ImGui::PushItemWidth(130.0f * _scaleIndex);
    ImGui::InputDouble(label_values[0][0], &display_position[0], 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
    ImGui::PopItemWidth();
    imgui_wrapper->pop_ac_inputdouble_style();
    ImGui::SameLine(/*axis_size + unit_size + space_size*2*/);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5 * _scaleIndex);
    imgui_wrapper->text(this->m_new_unit_string);

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15 * _scaleIndex);
    ImGui::TextColored(ImGuiWrapper::COL_AC_GREEN, "Y");
    ImGui::SameLine(/*axis_size + space_size*/);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * _scaleIndex);
    imgui_wrapper->push_ac_inputdouble_style(ImGui::GetCursorPosX() +10.0f *
                                                 _scaleIndex,
                                             6.0f *
                                                 _scaleIndex,
                                             3.0f * _scaleIndex,
                                             imgui_wrapper->default_font_13);
    ImGui::PushItemWidth(130.0f * _scaleIndex);
    ImGui::InputDouble(label_values[0][1], &display_position[1], 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
    ImGui::PopItemWidth();
    imgui_wrapper->pop_ac_inputdouble_style();
    ImGui::SameLine(/*axis_size + unit_size + space_size*2*/);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5 * _scaleIndex);
    imgui_wrapper->text(this->m_new_unit_string);

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15 * _scaleIndex);
    ImGui::TextColored(ImGuiWrapper::COL_AC_PURPLE, "Z");
    ImGui::SameLine(/*axis_size + space_size*/);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * _scaleIndex);
    imgui_wrapper->push_ac_inputdouble_style(ImGui::GetCursorPosX() +10.0f *
                                                 _scaleIndex,
                                             6.0f *
                                                 _scaleIndex,
                                             3.0f * _scaleIndex,
                                             imgui_wrapper->default_font_13);
    ImGui::PushItemWidth(130.0f * _scaleIndex);
    ImGui::InputDouble(label_values[0][2], &display_position[2], 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
    ImGui::PopItemWidth();
    imgui_wrapper->pop_ac_inputdouble_style();
    ImGui::SameLine(/*axis_size + unit_size + space_size*2*/);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5 * _scaleIndex);
    imgui_wrapper->text(this->m_new_unit_string);

    imgui_wrapper->push_ac_button_style(ImGui::GetCursorPosY() +
                                            20 * _scaleIndex,
                                        42.0f * _scaleIndex,
                                        6.0f * _scaleIndex,
                                        8.0f * _scaleIndex);
    if (imgui_wrapper->button(_L("Move To Center"), 186 * _scaleIndex, 30 * _scaleIndex)) {
        move_position_value_to_center();
    }
    //if (ImGui::Button("Reset")) {
    //    reset_position_value();
    //}
    imgui_wrapper->pop_ac_button_style();
    ImGui::PopFont();
    for (int i = 0;i<display_position.size();i++)
    {
        if (display_position[i] > MAX_NUM)display_position[i] = MAX_NUM;
        if (display_position[i] < -MAX_NUM)display_position[i] = -MAX_NUM;
    }

    m_buffered_position = display_position;

    update(current_active_id, "position", original_position, m_buffered_position);
    // the init position values are not zero, won't add reset button

    // send focus to m_glcanvas
    bool focued_on_text = false;
    for (int j = 0; j < 3; j++) {
        unsigned int id = ImGui::GetID(label_values[0][j]);
        if (current_active_id == id) {
            m_glcanvas.handle_sidebar_focus_event(label_values[0][j] + 2, true);
            focued_on_text = true;
            break;
        }
    }
    if (!focued_on_text) 
        m_glcanvas.handle_sidebar_focus_event("", false);

    m_last_active_item = current_active_id;
    last_move_input_window_width = ImGui::GetWindowWidth();
    last_move_win_h = ImGui::GetWindowHeight();

    imgui_wrapper->end();
    //ImGui::PopStyleVar(1);
    //ImGui::PopStyleVar();

    ImGuiWrapper::pop_ac_toolwin_style();
}

void GizmoObjectManipulation::do_render_ac_rotate_window(ImGuiWrapper *imgui_wrapper, std::string window_name, float x, float y, float bottom_limit)
{
    static float last_move_win_h = 0.0f;

    const float approx_height = last_move_win_h ;
    y = std::min(y, bottom_limit - approx_height);
    float _scaleIndex         = imgui_wrapper->get_style_scaling();
    imgui_wrapper->set_next_window_pos(x+10.f, y, ImGuiCond_Always);
    /*imgui_wrapper->set_next_window_size(218.0f * _scaleIndex,
                                        218.0f * _scaleIndex, ImGuiCond_None);*/
    // BBS
    ImGuiWrapper::push_ac_toolwin_style(m_glcanvas.get_scale());
    //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0, 6.0));

    std::string name = this->m_new_title_string + "##" + window_name;
    //ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImGuiWrapper::COL_AC_BLUE;
    //ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.05f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWrapper::COL_AC_WHITE);
    ImGui::PushFont(imgui_wrapper->bold_font_14);
    imgui_wrapper->begin(_L(window_name), ImGuiWrapper::TOOLBAR_WINDOW_FLAGS_AC_NEW);
    ImGui::PopFont();
    ImGui::PopStyleColor();
    auto update = [this](unsigned int active_id, std::string opt_key, Vec3d original_value, Vec3d new_value) -> int {
        for (int i = 0; i < 3; i++) {
            if (original_value[i] != new_value[i]) {
                if (active_id != m_last_active_item) {
                    on_change(opt_key, i, new_value[i]);
                    return i;
                }
            }
        }
        return -1;
    };
    float space_size    = _scaleIndex * 6;
    float position_size = imgui_wrapper->calc_text_size(_L("Rotation")).x + space_size;
    float World_size    = imgui_wrapper->calc_text_size(_L("World coordinates")).x + space_size;
    float caption_max   = std::max(position_size, World_size) + 2 * space_size;
    float end_text_size = imgui_wrapper->calc_text_size(this->m_new_unit_string).x;

    // position
    //Vec3d original_position;
    //if (this->m_imperial_units)
    //    original_position = this->m_new_position * this->mm_to_in;
    //else
    //    original_position = this->m_new_position;
    //Vec3d display_position = m_buffered_position;
    // Rotation
    Vec3d rotation   = this->m_buffered_rotation;

    if (std::abs(rotation[0]) < EPSILON) rotation[0] = 0;
    if (std::abs(rotation[1]) < EPSILON) rotation[1] = 0;
    if (std::abs(rotation[2]) < EPSILON) rotation[2] = 0;

    float unit_size = imgui_wrapper->calc_text_size(wxString(MAX_SIZE)).x + space_size;
    int   index      = 1;
    int   index_unit = 1;

    ImTextureID normal_id = m_glcanvas.get_gizmos_manager().get_icon_texture_id(GLGizmosManager::MENU_ICON_NAME::IC_TITLE_CIRCLE);
    ImGui::PushFont(imgui_wrapper->default_font_13);
    // render title
    //ImGui::AlignTextToFramePadding();
    unsigned int current_active_id = ImGui::GetActiveID();
   
    //ImGui::SameLine(/*space_size*2*/);

    //ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2 * _scaleIndex);
    //ImGui::Image(normal_id, ImVec2(space_size,space_size));
    //

    //ImGui::SameLine(/*space_size*2*/);
    //ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 8 * _scaleIndex);
    //ImGui::TextColored(ImVec4(0,0,0,1), window_name.c_str());

    //// render Separator
    //ImGui::Separator();
    // render line x
    //ImGui::AlignTextToFramePadding();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15 * _scaleIndex);
    ImGui::TextColored(ImGuiWrapper::COL_AC_RED, "X");
    ImGui::SameLine(/*axis_size + space_size*/);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::BBLInputDouble(label_values[0][0], &display_position[0], 0.0f, 0.0f, "%.2f");
    ImGui::PushItemWidth(130.0f * _scaleIndex);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * _scaleIndex);
    imgui_wrapper->push_ac_inputdouble_style(ImGui::GetCursorPosX() +12.0f *
                                                 _scaleIndex,
                                             6.0f *
                                                 _scaleIndex,
                                             3.0f * _scaleIndex,
                                             imgui_wrapper->default_font_13);
    ImGui::InputDouble(label_values[1][0], &rotation[0], 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
    imgui_wrapper->pop_ac_inputdouble_style();
    ImGui::SameLine(/*axis_size + unit_size + space_size*2*/);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4 * _scaleIndex);
    imgui_wrapper->text(_L("°"));

    // render line y
    //ImGui::AlignTextToFramePadding();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15 * _scaleIndex);
    ImGui::TextColored(ImGuiWrapper::COL_AC_GREEN, "Y");
    ImGui::SameLine(/*axis_size + space_size*/);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::BBLInputDouble(label_values[0][1], &display_position[1], 0.0f, 0.0f, "%.2f");
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * _scaleIndex);
    imgui_wrapper->push_ac_inputdouble_style(ImGui::GetCursorPosX() +12.0f *
                                                 _scaleIndex,
                                             6.0f *
                                                 _scaleIndex,
                                             3.0f * _scaleIndex,
                                             imgui_wrapper->default_font_13);
    ImGui::InputDouble(label_values[1][1], &rotation[1], 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
    imgui_wrapper->pop_ac_inputdouble_style();
    ImGui::SameLine(/*axis_size + unit_size + space_size*2*/);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4 * _scaleIndex);
    imgui_wrapper->text(_L("°"));

    // render line z
    //ImGui::AlignTextToFramePadding();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15 * _scaleIndex);
    ImGui::TextColored(ImGuiWrapper::COL_AC_PURPLE, "Z");
    ImGui::SameLine(/*axis_size + space_size*/);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::BBLInputDouble(label_values[0][2], &display_position[2], 0.0f, 0.0f, "%.2f");
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * _scaleIndex);
    imgui_wrapper->push_ac_inputdouble_style(ImGui::GetCursorPosX() +12.0f *
                                                 _scaleIndex,
                                             6.0f *
                                                 _scaleIndex,
                                             3.0f * _scaleIndex,
                                             imgui_wrapper->default_font_13);
    ImGui::InputDouble(label_values[1][2], &rotation[2], 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
    imgui_wrapper->pop_ac_inputdouble_style();

    ImGui::PopItemWidth();
    ImGui::SameLine(/*axis_size + unit_size + space_size*2*/);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4 * _scaleIndex);
    imgui_wrapper->text(_L("°"));
    imgui_wrapper->push_ac_button_style(ImGui::GetCursorPosY() +
                                            15 * _scaleIndex,
                                        76.0f * _scaleIndex,
                                        6.0f * _scaleIndex,
                                        8.0f * _scaleIndex);
    imgui_wrapper->disabled_begin(m_glcanvas.get_selection().get_volume_idxs().size() != 1);
    if (imgui_wrapper->button(_L("Reset"), 186.0f * _scaleIndex, 30.0f * _scaleIndex)) {
        reset_rotation_value();
    }
    imgui_wrapper->disabled_end();
    imgui_wrapper->pop_ac_button_style();
    ImGui::PopFont();

    ////ImGui::PushItemWidth(caption_max);
    ////imgui_wrapper->text(_L("World coordinates"));
    ////ImGui::SameLine(caption_max + index * space_size);
    ////ImGui::PushItemWidth(unit_size);
    ////ImGui::TextAlignCenter("X");
    //ImGui::SameLine(caption_max + unit_size + (++index) * space_size);
    ////ImGui::PushItemWidth(unit_size);
    //ImGui::TextAlignCenter("Y");
    //ImGui::SameLine(caption_max + (++index_unit) * unit_size + (++index) * space_size);
    ////ImGui::PushItemWidth(unit_size);
    //ImGui::TextAlignCenter("Z");

    //index      = 1;
    //index_unit = 1;

    //// ImGui::PushItemWidth(unit_size * 2);
    //ImGui::AlignTextToFramePadding();
    //imgui_wrapper->text(_L("Rotation"));
    //ImGui::SameLine(caption_max + index * space_size);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::BBLInputDouble(label_values[1][0], &rotation[0], 0.0f, 0.0f, "%.2f");
    //ImGui::SameLine(caption_max + unit_size + (++index) * space_size);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::BBLInputDouble(label_values[1][1], &rotation[1], 0.0f, 0.0f, "%.2f");
    //ImGui::SameLine(caption_max + (++index_unit) * unit_size + (++index) * space_size);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::BBLInputDouble(label_values[1][2], &rotation[2], 0.0f, 0.0f, "%.2f");
    //ImGui::SameLine(caption_max + (++index_unit) * unit_size + (++index) * space_size);

    m_buffered_rotation = rotation;
    update(current_active_id, "rotation", this->m_new_rotation, m_buffered_rotation);

    //if (m_show_clear_rotation) {
    //    //ImGui::SameLine(caption_max + 3 * unit_size + 4 * space_size + end_text_size);
    //    ImGui::SameLine();
    //    if (reset_button(imgui_wrapper, caption_max, unit_size, space_size, end_text_size)) { reset_rotation_value(); }
    //} else {
    //    //ImGui::SameLine(caption_max + 3 * unit_size + 5 * space_size + end_text_size);
    //    ImGui::SameLine();
    //    ImGui::InvisibleButton("", ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize()));
    //}

    // send focus to m_glcanvas
    bool focued_on_text = false;
    for (int j = 0; j < 3; j++) {
        unsigned int id = ImGui::GetID(label_values[1][j]);
        if (current_active_id == id) {
            m_glcanvas.handle_sidebar_focus_event(label_values[1][j] + 2, true);
            focued_on_text = true;
            break;
        }
    }
    if (!focued_on_text) 
        m_glcanvas.handle_sidebar_focus_event("", false);

    m_last_active_item = current_active_id;
    last_rotate_input_window_width = ImGui::GetWindowWidth();
    last_move_win_h = ImGui::GetWindowHeight();
    imgui_wrapper->end();
    //ImGui::PopStyleVar();
    // BBS
    //ImGui::PopStyleVar(1);
    ImGuiWrapper::pop_ac_toolwin_style();
}

void GizmoObjectManipulation::do_render_ac_scale_input_window(ImGuiWrapper* imgui_wrapper, std::string window_name, float x, float y, float bottom_limit)
{
    static float last_move_win_h = 0.0f;

    const float approx_height = last_move_win_h ;
    y = std::min(y, bottom_limit - approx_height);
    float _scaleIndex         = imgui_wrapper->get_style_scaling();
    imgui_wrapper->set_next_window_pos(x+10.0f, y, ImGuiCond_Always);
    /*imgui_wrapper->set_next_window_size(260.0f * _scaleIndex,
                                        250.0f * _scaleIndex, ImGuiCond_None);*/
    //BBS
    //ImGuiWrapper::push_toolbar_style(m_glcanvas.get_scale());
    ImGuiWrapper::push_ac_toolwin_style(m_glcanvas.get_scale());
    //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0, 6.0));

    std::string name = this->m_new_title_string + "##" + window_name;
    //ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImGuiWrapper::COL_AC_BLUE;
    //ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.05f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWrapper::COL_AC_WHITE);
    ImGui::PushFont(imgui_wrapper->bold_font_14);
    imgui_wrapper->begin(_L(window_name), ImGuiWrapper::TOOLBAR_WINDOW_FLAGS_AC_NEW);
    ImGui::PopFont();
    ImGui::PopStyleColor();
    auto update = [this](unsigned int active_id, std::string opt_key, Vec3d original_value, Vec3d new_value)->int {
        for (int i = 0; i < 3; i++)
        {
            if (original_value[i] != new_value[i])
            {
                if (active_id != m_last_active_item)
                {
                    on_change(opt_key, i, new_value[i]);
                    return i;
                }
            }
        }
        return -1;
    };

    float space_size = imgui_wrapper->get_style_scaling() * 6;
    float scale_size = imgui_wrapper->calc_text_size(_L("Scale")).x + space_size;
    float size_len = imgui_wrapper->calc_text_size(_L("Size")).x + space_size;
    float caption_max = std::max(scale_size, size_len) + 2 * space_size;
    float end_text_size = imgui_wrapper->calc_text_size(this->m_new_unit_string).x;

    Vec3d scale = m_buffered_scale;
    Vec3d display_size = m_buffered_size;

    Vec3d display_position = m_buffered_position;

        //Size
    Vec3d original_size;
    if (this->m_imperial_units)
        original_size = this->m_new_size * this->mm_to_in;
    else
        original_size = this->m_new_size;


    float unit_size = imgui_wrapper->calc_text_size(wxString(MAX_SIZE)).x + space_size;
    bool imperial_units = this->m_imperial_units;

    int index      = 2;
    int index_unit = 1;

    ImTextureID normal_id = m_glcanvas.get_gizmos_manager().get_icon_texture_id(GLGizmosManager::MENU_ICON_NAME::IC_TITLE_CIRCLE);

    unsigned int current_active_id = ImGui::GetActiveID();
    ImGui::PushFont(imgui_wrapper->default_font_13);

    //ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2 * _scaleIndex);
    //ImGui::Image(normal_id, ImVec2(space_size,space_size));

    //ImGui::SameLine(/*space_size*2*/);
    //ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 9 * _scaleIndex);
    //ImGui::TextColored(ImVec4(0,0,0,1), window_name.c_str());

    //// render Separator
    //ImGui::Separator();
    // render line x

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5 * _scaleIndex);
    ImGui::TextColored(ImGuiWrapper::COL_AC_RED, "X");
    ImGui::SameLine();


    ImGui::PushItemWidth(70 * _scaleIndex);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * _scaleIndex);
    imgui_wrapper->push_ac_inputdouble_style(ImGui::GetCursorPosX() +10.0f *
                                                 _scaleIndex,
                                             6.0f *
                                                 _scaleIndex,
                                             3.0f * _scaleIndex,
                                             imgui_wrapper->default_font_13);

    ImGui::InputDouble(label_scale_values[1][0], &display_size[0], 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
    imgui_wrapper->pop_ac_inputdouble_style();

    ImGui::SameLine(/*axis_size + unit_size + space_size*2*/);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5 * _scaleIndex);
    imgui_wrapper->text(this->m_new_unit_string);
    ImGui::SameLine(/*axis_size + unit_size + space_size*2*/);
    imgui_wrapper->push_ac_inputdouble_style(ImGui::GetCursorPosX() +10.0f *
                                                 _scaleIndex,
                                             6.0f *
                                                 _scaleIndex,
                                             3.0f * _scaleIndex,
                                             imgui_wrapper->default_font_13);
    ImGui::InputDouble(label_scale_values[0][0], &scale[0], 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
    imgui_wrapper->pop_ac_inputdouble_style();
    ImGui::SameLine(/*axis_size + unit_size + space_size*2*/);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4 * _scaleIndex);
    imgui_wrapper->text(_L("%"));

    // render line y
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20 * _scaleIndex);
    ImGui::TextColored(ImGuiWrapper::COL_AC_GREEN, "Y");
    ImGui::SameLine(/*axis_size + space_size*/);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * _scaleIndex);
    imgui_wrapper->push_ac_inputdouble_style(ImGui::GetCursorPosX() +10.0f *
                                                 _scaleIndex,
                                             6.0f *
                                                 _scaleIndex,
                                             3.0f * _scaleIndex,
                                             imgui_wrapper->default_font_13);
    ImGui::InputDouble(label_scale_values[1][1], &display_size[1], 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
    imgui_wrapper->pop_ac_inputdouble_style();

    ImGui::SameLine(/*axis_size + unit_size + space_size*2*/);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5 * _scaleIndex);
    imgui_wrapper->text(this->m_new_unit_string);
    ImGui::SameLine(/*axis_size + unit_size + space_size*2*/);
    imgui_wrapper->push_ac_inputdouble_style(ImGui::GetCursorPosX() +10.0f *
                                                 _scaleIndex,
                                             6.0f *
                                                 _scaleIndex,
                                             3.0f * _scaleIndex,
                                             imgui_wrapper->default_font_13);
    ImGui::InputDouble(label_scale_values[0][1], &scale[1], 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
    imgui_wrapper->pop_ac_inputdouble_style();

    ImGui::SameLine(/*axis_size + unit_size + space_size*2*/);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4 * _scaleIndex);
    imgui_wrapper->text(_L("%"));

    // render line z
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20 * _scaleIndex);
    ImGui::TextColored(ImGuiWrapper::COL_AC_PURPLE, "Z");
    ImGui::SameLine(/*axis_size + space_size*/);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::BBLInputDouble(label_values[0][2], &display_position[1], 0.0f, 0.0f, "%.2f");
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * _scaleIndex);
    imgui_wrapper->push_ac_inputdouble_style(ImGui::GetCursorPosX() +10.0f *
                                                 _scaleIndex,
                                             6.0f *
                                                 _scaleIndex,
                                             3.0f * _scaleIndex,
                                             imgui_wrapper->default_font_13);
    ImGui::InputDouble(label_scale_values[1][2], &display_size[2], 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
    imgui_wrapper->pop_ac_inputdouble_style();

    ImGui::SameLine(/*axis_size + unit_size + space_size*2*/);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5 * _scaleIndex);
    imgui_wrapper->text(this->m_new_unit_string);
    ImGui::SameLine(/*axis_size + unit_size + space_size*2*/);
    imgui_wrapper->push_ac_inputdouble_style(ImGui::GetCursorPosX() +10.0f *
                                                 _scaleIndex,
                                             6.0f *
                                                 _scaleIndex,
                                             3.0f * _scaleIndex,
                                             imgui_wrapper->default_font_13);
    ImGui::InputDouble(label_scale_values[0][2], &scale[2], 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
    imgui_wrapper->pop_ac_inputdouble_style();

    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4 * _scaleIndex);
    imgui_wrapper->text(_L("%"));

    //ImGui::PushItemWidth(caption_max);
    //ImGui::Dummy(ImVec2(caption_max, -1));
    ////imgui_wrapper->text(_L(" "));
    ////ImGui::PushItemWidth(unit_size * 1.5);
    //ImGui::SameLine(caption_max + space_size);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::TextAlignCenter("X");
    //ImGui::SameLine(caption_max + unit_size + index * space_size);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::TextAlignCenter("Y");
    //ImGui::SameLine(caption_max + (++index_unit) * unit_size + (++index) * space_size);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::TextAlignCenter("Z");

    //index      = 2;
    //index_unit = 1;

    ////ImGui::PushItemWidth(unit_size * 2);
    //ImGui::AlignTextToFramePadding();
    //imgui_wrapper->text(_L("Scale"));
    //ImGui::SameLine(caption_max + space_size);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::BBLInputDouble(label_scale_values[0][0], &scale[0], 0.0f, 0.0f, "%.2f");
    //ImGui::SameLine(caption_max + unit_size + index * space_size);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::BBLInputDouble(label_scale_values[0][1], &scale[1], 0.0f, 0.0f, "%.2f");
    //ImGui::SameLine(caption_max + (++index_unit) *unit_size + (++index) * space_size);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::BBLInputDouble(label_scale_values[0][2], &scale[2], 0.0f, 0.0f, "%.2f");
    //ImGui::SameLine(caption_max + (++index_unit) *unit_size + (++index) * space_size);
    //imgui_wrapper->text(_L("%"));
    

    /*if (m_show_clear_scale) {
        ImGui::SameLine(caption_max + 3 * unit_size + 4 * space_size + end_text_size);
        if (reset_button(imgui_wrapper, caption_max, unit_size, space_size, end_text_size))
            reset_scale_value();
    } else {
        ImGui::SameLine(caption_max + 3 * unit_size + 5 * space_size + end_text_size);
        ImGui::InvisibleButton("", ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize()));
    }*/


    //index              = 2;
    //index_unit         = 1;
    ////ImGui::PushItemWidth(unit_size * 2);
    //ImGui::AlignTextToFramePadding();
    //imgui_wrapper->text(_L("Size"));
    //ImGui::SameLine(caption_max + space_size);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::BBLInputDouble(label_scale_values[1][0], &display_size[0], 0.0f, 0.0f, "%.2f");
    //ImGui::SameLine(caption_max + unit_size + index * space_size);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::BBLInputDouble(label_scale_values[1][1], &display_size[1], 0.0f, 0.0f, "%.2f");
    //ImGui::SameLine(caption_max + (++index_unit) *unit_size + (++index) * space_size);
    //ImGui::PushItemWidth(unit_size);
    //ImGui::BBLInputDouble(label_scale_values[1][2], &display_size[2], 0.0f, 0.0f, "%.2f");
    //ImGui::SameLine(caption_max + (++index_unit) *unit_size + (++index) * space_size);
    //imgui_wrapper->text(this->m_new_unit_string);

    

    bool uniform_scale = this->m_uniform_scale;
    
    // BBS: when select multiple objects, uniform scale can be deselected
    //const Selection &selection = m_glcanvas.get_selection();
    //bool uniform_scale_only    = selection.is_multiple_full_object() || selection.is_multiple_full_instance() || selection.is_mixed() || selection.is_multiple_volume() ||
    //                          selection.is_multiple_modifier();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10 * _scaleIndex);
    ImGui::PushFont(imgui_wrapper->default_font_13);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                        ImVec2(0.0f * _scaleIndex, 0.0f * _scaleIndex));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,4.0f * _scaleIndex);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing,
                        ImVec2(3.0f * _scaleIndex, 3.0f * _scaleIndex));
    /*if (uniform_scale) {
        imgui_wrapper->disabled_begin(true);
        imgui_wrapper->bbl_ac_checkbox(_L("uniform scalling"), uniform_scale);
        imgui_wrapper->disabled_end();
    } else {
        imgui_wrapper->bbl_ac_checkbox(_L("uniform scalling"), uniform_scale);
    }*/

    imgui_wrapper->bbl_ac_checkbox(_L("uniform scale"), uniform_scale);
    ImGui::PopFont();
    ImGui::PopStyleVar(3);
    /*ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 2 * _scaleIndex);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3 * _scaleIndex);
    ImGui::PushFont(imgui_wrapper->default_font_13);
    imgui_wrapper->text(_L("uniform scalling"));
    ImGui::PopFont();*/


    m_buffered_size = display_size;
    for (int i = 0; i < display_size.size(); i++) {
        if (std::abs(display_size[i]) > MAX_NUM) display_size[i] = MAX_NUM;
    }
    // m_buffered_size = display_size;
    int size_sel = update(current_active_id, "size", original_size, m_buffered_size);

    int scale_sel = -1;
    if (size_sel == -1) {
        m_buffered_scale = scale;
        scale_sel    = update(current_active_id, "scale", this->m_new_scale, m_buffered_scale);
    }
    //int scale_sel    = 0;




    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2 * _scaleIndex);
    imgui_wrapper->push_ac_button_style(ImGui::GetCursorPosY() +
                                            15 * _scaleIndex,
                                        96.0f * _scaleIndex,
                                        6.0f * _scaleIndex,
                                        8.0f * _scaleIndex);
									
    imgui_wrapper->disabled_begin(m_glcanvas.get_selection().get_volume_idxs().size() != 1);
    if (imgui_wrapper->button(_L("Reset"), 228.0f * _scaleIndex, 30.0f * _scaleIndex)) { 
        reset_scale_value();
    }
    imgui_wrapper->disabled_end();
    imgui_wrapper->pop_ac_button_style();

    ImGui::PopItemWidth();
    ImGui::PopFont();

    if (uniform_scale != this->m_uniform_scale) { this->set_uniform_scaling(uniform_scale); }

     // for (int index = 0; index < 3; index++)
    //    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << ",before_index="<<index <<boost::format(",scale %1%, buffered %2%, original_id %3%, new_id %4%\n") %
    //    this->m_new_scale[index] % m_buffered_scale[index] % m_last_active_item % current_active_id;
    
    if ((scale_sel >= 0)) {
        // for (int index = 0; index < 3; index++)
        //    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << ",after_index="<<index <<boost::format(",scale %1%, buffered %2%, original_id %3%, new_id %4%\n") %
        //    this->m_new_scale[index] % m_buffered_scale[index] % m_last_active_item % current_active_id;
        for (int i = 0; i < 3; ++i) {
            if (i != scale_sel) ImGui::ClearInputTextInitialData(label_scale_values[0][i], m_buffered_scale[i]);
            ImGui::ClearInputTextInitialData(label_scale_values[1][i], m_buffered_size[i]);
        }
    }

    if ((size_sel >= 0)) {
        // for (int index = 0; index < 3; index++)
        //    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << ",after_index="<<index <<boost::format(",scale %1%, buffered %2%, original_id %3%, new_id %4%\n") %
        //    this->m_new_scale[index] % m_buffered_scale[index] % m_last_active_item % current_active_id;
        for (int i = 0; i < 3; ++i) {
            ImGui::ClearInputTextInitialData(label_scale_values[0][i], m_buffered_scale[i]);
            if (i != size_sel) ImGui::ClearInputTextInitialData(label_scale_values[1][i], m_buffered_size[i]);
        }
    }

    //send focus to m_glcanvas
    bool focued_on_text = false;
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
        {
            unsigned int id = ImGui::GetID(label_scale_values[i][j]);
            if (current_active_id == id)
            {
                m_glcanvas.handle_sidebar_focus_event(label_scale_values[i][j] + 2, true);
                focued_on_text = true;
                break;
            }
        }
    if (!focued_on_text)
        m_glcanvas.handle_sidebar_focus_event("", false);

    m_last_active_item = current_active_id;

    last_scale_input_window_width = ImGui::GetWindowWidth();
    last_move_win_h = ImGui::GetWindowHeight();
    imgui_wrapper->end();
    //ImGui::PopStyleVar();

    //BBS
    //ImGuiWrapper::pop_toolbar_style();
    ImGuiWrapper::pop_ac_toolwin_style();
}


void GizmoObjectManipulation::do_render_ac_layout_window(ImGuiWrapper *imgui_wrapper, std::string window_name, float x, float y, float bottom_limit)
{
    static float last_move_win_h = 0.0f;
    float        _scaleIndex     = imgui_wrapper->get_style_scaling();
    const float approx_height = last_move_win_h ;
    y = std::min(y, bottom_limit - approx_height);

    std::string dist_key = "min_object_distance";
    std::string rot_key = "enable_rotation";
    std::string postfix;
    float dist_min = 0;
    const PrintConfig config = m_glcanvas.fff_print()->config();
    auto co_opt = config.complete_objects;
    if (co_opt && co_opt.value) {
        dist_min     = float(min_object_distance(config));
        postfix      = "_fff_seq_print";
    } else {
        dist_min     = 0.f;
        postfix     = "_fff";
    }

    imgui_wrapper->set_next_window_pos(x + 10.0f * _scaleIndex, y, ImGuiCond_Always);
    ImGuiWrapper::push_ac_toolwin_style(m_glcanvas.get_scale());

    std::string name = this->m_new_title_string + "##" + window_name;
    ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWrapper::COL_AC_WHITE);
    ImGui::PushFont(imgui_wrapper->bold_font_14);
    imgui_wrapper->begin(_L(window_name), ImGuiWrapper::TOOLBAR_WINDOW_FLAGS_AC_NEW);
    ImGui::PopFont();
    ImGui::PopStyleColor();

    auto &appcfg = wxGetApp().app_config;
    GLCanvas3D::ArrangeSettings settings = m_glcanvas.get_arrange_settings();
    GLCanvas3D::ArrangeSettings &settings_out = m_glcanvas.get_arrange_settings_ref(&m_glcanvas);
    ImGui::PushFont(imgui_wrapper->default_font_13);
    //ImGui::AlignTextToFramePadding();
    
    imgui_wrapper->text(_L("Model Spacing"));
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10 * _scaleIndex);

    //ImGui::AlignTextToFramePadding();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGuiWrapper::COL_AC_LIGHTGRAY);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 4.0f);
    ImGui::PushItemWidth(130.0f * _scaleIndex);

    if (imgui_wrapper->slider_float(_L("##Spacing"), &settings.distance, dist_min, 100.0f, "%5.2f") || dist_min > settings.distance) {
        settings.distance = std::max(dist_min, settings.distance);
        settings_out.distance = settings.distance;
        appcfg->set("arrange", dist_key.c_str(), float_to_string_decimal_point(settings_out.distance));
    }
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(2.0f * _scaleIndex, 0));
    ImGui::SameLine(); imgui_wrapper->text("mm");

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10 * _scaleIndex);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f * _scaleIndex, 2.0f * _scaleIndex));
    //ImGui::AlignTextToFramePadding();
    if (imgui_wrapper->checkbox(_L("Enable rotations (slow)"), settings.enable_rotation)) {
        settings_out.enable_rotation = settings.enable_rotation;
        appcfg->set("arrange", rot_key.c_str(), settings_out.enable_rotation? "1" : "0");
    }

    ImGui::PopStyleVar();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10 * _scaleIndex);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 50 * _scaleIndex);
    /*ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8);

    ImGui::PushStyleColor(ImGuiCol_Border, ImGuiWrapper::COL_AC_BLUE);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWrapper::COL_AC_BLUE);*/
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 44.0f * _scaleIndex);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.97f, 0.99f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGuiWrapper::COL_HOVER);
    imgui_wrapper->push_ac_button_style(ImGui::GetCursorPosY() + 5.0f * _scaleIndex, 30.0f * _scaleIndex, 6.0f * _scaleIndex,
                                        8.0f * _scaleIndex);

    wxString btText = _L("Apply");
    ImVec2 btTextSize = imgui_wrapper->calc_button_size(btText);
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - btTextSize.x)/2);
    if (imgui_wrapper->button(_L("Apply"))) {
        wxGetApp().plater()->arrange();
        UpdateAndShow(true);
    }
    ImGui::PopStyleColor(2);
    imgui_wrapper->pop_ac_button_style();
    /*ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(2);*/
    ImGuiWrapper::pop_ac_toolwin_style();
    ImGui::PopFont();
    imgui_wrapper->end();

    
}

} //namespace GUI
} //namespace Slic3r
