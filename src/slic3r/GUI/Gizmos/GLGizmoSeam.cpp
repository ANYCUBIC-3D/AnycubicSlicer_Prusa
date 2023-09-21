#include "GLGizmoSeam.hpp"

#include "libslic3r/Model.hpp"

//#include "slic3r/GUI/3DScene.hpp"
#include "slic3r/GUI/GLCanvas3D.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/ImGuiWrapper.hpp"
#include "slic3r/GUI/Plater.hpp"
#include "slic3r/GUI/GUI_ObjectList.hpp"
#include "slic3r/Utils/UndoRedo.hpp"

#include <GL/glew.h>


namespace Slic3r::GUI {



void GLGizmoSeam::on_shutdown()
{
    m_parent.toggle_model_objects_visibility(true);
}



bool GLGizmoSeam::on_init()
{
    m_shortcut_key = WXK_CONTROL_P;

    m_desc["clipping_of_view"] = _L("Clipping of view") + ": ";
    m_desc["reset_direction"]  = _L("Reset direction");
    m_desc["cursor_size"]      = _L("Brush size") + ": ";
    m_desc["cursor_type"]      = _L("Brush shape") + ": ";
    m_desc["enforce_caption"]  = _L("Left mouse button") + ": ";
    m_desc["enforce"]          = _L("Enforce seam");
    m_desc["block_caption"]    = _L("Right mouse button") + ": ";
    m_desc["block"]            = _L("Block seam");
    m_desc["remove_caption"]   = _L("Shift + Left mouse button") + ": ";
    m_desc["remove"]           = _L("Remove selection");
    m_desc["remove_all"]       = _L("Remove all selection");
    m_desc["circle"]           = _L("Circle");
    m_desc["sphere"]           = _L("Sphere");
    m_desc["tool_type"]        = _L("Tool type");

    return true;
}



std::string GLGizmoSeam::on_get_name() const
{
    return _u8L("Seam painting");
}

void GLGizmoSeam::render_painter_gizmo()
{
    const Selection& selection = m_parent.get_selection();

    glsafe(::glEnable(GL_BLEND));
    glsafe(::glEnable(GL_DEPTH_TEST));

    render_triangles(selection);

    m_c->object_clipper()->render_cut();
    m_c->instances_hider()->render_cut();
    render_cursor();

    glsafe(::glDisable(GL_BLEND));
}



//void GLGizmoSeam::on_render_input_window(float x, float y, float bottom_limit)
//{
//    if (! m_c->selection_info()->model_object())
//        return;
//
//    static float last_seam_win_h = 0.0f;
//
//    const float approx_height = last_seam_win_h ;
//    y = std::min(y, bottom_limit - approx_height);
//    m_imgui->set_next_window_pos(x, y, ImGuiCond_Always);
//
//    m_imgui->begin(get_name(), ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
//
//    // First calculate width of all the texts that are could possibly be shown. We will decide set the dialog width based on that:
//    const float clipping_slider_left = std::max(m_imgui->calc_text_size(m_desc.at("clipping_of_view")).x,
//                                                m_imgui->calc_text_size(m_desc.at("reset_direction")).x)
//                                           + m_imgui->scaled(1.5f);
//    const float cursor_size_slider_left = m_imgui->calc_text_size(m_desc.at("cursor_size")).x + m_imgui->scaled(1.f);
//
//    const float cursor_type_radio_left   = m_imgui->calc_text_size(m_desc["cursor_type"]).x + m_imgui->scaled(1.f);
//    const float cursor_type_radio_sphere = m_imgui->calc_text_size(m_desc["sphere"]).x + m_imgui->scaled(2.5f);
//    const float cursor_type_radio_circle = m_imgui->calc_text_size(m_desc["circle"]).x + m_imgui->scaled(2.5f);
//
//    const float button_width = m_imgui->calc_text_size(m_desc.at("remove_all")).x + m_imgui->scaled(1.f);
//    const float minimal_slider_width = m_imgui->scaled(4.f);
//
//    float caption_max    = 0.f;
//    float total_text_max = 0.f;
//    for (const auto &t : std::array<std::string, 3>{"enforce", "block", "remove"}) {
//        caption_max    = std::max(caption_max, m_imgui->calc_text_size(m_desc[t + "_caption"]).x);
//        total_text_max = std::max(total_text_max, m_imgui->calc_text_size(m_desc[t]).x);
//    }
//    total_text_max += caption_max + m_imgui->scaled(1.f);
//    caption_max    += m_imgui->scaled(1.f);
//
//    const float sliders_left_width = std::max(cursor_size_slider_left, clipping_slider_left);
//    const float slider_icon_width  = m_imgui->get_slider_icon_size().x;
//    float       window_width       = minimal_slider_width + sliders_left_width + slider_icon_width;
//    window_width = std::max(window_width, total_text_max);
//    window_width = std::max(window_width, button_width);
//    window_width = std::max(window_width, cursor_type_radio_left + cursor_type_radio_sphere + cursor_type_radio_circle);
//
//    auto draw_text_with_caption = [this, &caption_max](const wxString& caption, const wxString& text) {
//        m_imgui->text_colored(ImGuiWrapper::COL_BLUE_LIGHT, caption);
//        ImGui::SameLine(caption_max);
//        m_imgui->text(text);
//    };
//
//    for (const auto &t : std::array<std::string, 3>{"enforce", "block", "remove"})
//        draw_text_with_caption(m_desc.at(t + "_caption"), m_desc.at(t));
//
//    ImGui::Separator();
//
//    const float max_tooltip_width = ImGui::GetFontSize() * 20.0f;
//
//    ImGui::AlignTextToFramePadding();
//    m_imgui->text(m_desc.at("cursor_size"));
//    ImGui::SameLine(sliders_left_width);
//    ImGui::PushItemWidth(window_width - sliders_left_width - slider_icon_width);
//    m_imgui->slider_float("##cursor_radius", &m_cursor_radius, CursorRadiusMin, CursorRadiusMax, "%.2f", 1.0f, true, _L("Alt + Mouse wheel"));
//
//    ImGui::AlignTextToFramePadding();
//    m_imgui->text(m_desc.at("cursor_type"));
//
//    float cursor_type_offset = cursor_type_radio_left + (window_width - cursor_type_radio_left - cursor_type_radio_sphere - cursor_type_radio_circle + m_imgui->scaled(0.5f)) / 2.f;
//    ImGui::SameLine(cursor_type_offset);
//    ImGui::PushItemWidth(cursor_type_radio_sphere);
//    if (m_imgui->radio_button(m_desc["sphere"], m_cursor_type == TriangleSelector::CursorType::SPHERE))
//        m_cursor_type = TriangleSelector::CursorType::SPHERE;
//
//    if (ImGui::IsItemHovered())
//        m_imgui->tooltip(_L("Paints all facets inside, regardless of their orientation."), max_tooltip_width);
//
//    ImGui::SameLine(cursor_type_offset + cursor_type_radio_sphere);
//    ImGui::PushItemWidth(cursor_type_radio_circle);
//    if (m_imgui->radio_button(m_desc["circle"], m_cursor_type == TriangleSelector::CursorType::CIRCLE))
//        m_cursor_type = TriangleSelector::CursorType::CIRCLE;
//
//    if (ImGui::IsItemHovered())
//        m_imgui->tooltip(_L("Ignores facets facing away from the camera."), max_tooltip_width);
//
//    ImGui::Separator();
//    if (m_c->object_clipper()->get_position() == 0.f) {
//        ImGui::AlignTextToFramePadding();
//        m_imgui->text(m_desc.at("clipping_of_view"));
//    }
//    else {
//        if (m_imgui->button(m_desc.at("reset_direction"))) {
//            wxGetApp().CallAfter([this](){
//                    m_c->object_clipper()->set_position_by_ratio(-1., false);
//                });
//        }
//    }
//
//    auto clp_dist = float(m_c->object_clipper()->get_position());
//    ImGui::SameLine(sliders_left_width);
//    ImGui::PushItemWidth(window_width - sliders_left_width - slider_icon_width);
//    if (m_imgui->slider_float("##clp_dist", &clp_dist, 0.f, 1.f, "%.2f", 1.0f, true, _L("Ctrl + Mouse wheel")))
//        m_c->object_clipper()->set_position_by_ratio(clp_dist, true);
//
//    ImGui::Separator();
//    if (m_imgui->button(m_desc.at("remove_all"))) {
//        Plater::TakeSnapshot snapshot(wxGetApp().plater(), _L("Reset selection"), UndoRedo::SnapshotType::GizmoAction);
//        ModelObject         *mo  = m_c->selection_info()->model_object();
//        int                  idx = -1;
//        for (ModelVolume *mv : mo->volumes)
//            if (mv->is_model_part()) {
//                ++idx;
//                m_triangle_selectors[idx]->reset();
//                m_triangle_selectors[idx]->request_update_render_data();
//            }
//
//        update_model_object();
//        m_parent.set_as_dirty();
//    }
//    last_seam_win_h = ImGui::GetWindowHeight();
//    m_imgui->end();
//
//    //last_h = ImGui::GetWindowHeight();
//
//}
void GLGizmoSeam::on_render_input_window(float x, float y, float bottom_limit)
{
    if (!m_c->selection_info()->model_object())
        return;

    static float last_seam_win_h = 0.0f;
    float        _scaleIndex     = m_imgui->get_style_scaling();
    const float  approx_height   = last_seam_win_h;
    y                            = std::min(y, bottom_limit - approx_height);
    m_imgui->set_next_window_pos(x + 10.0f * _scaleIndex, y - 34.0f * _scaleIndex, ImGuiCond_Always);
    float win_w = 334.0f * _scaleIndex;
    float win_h = 370.0f * _scaleIndex;
    //m_imgui->set_next_window_size(win_w, win_h, ImGuiCond_None);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 10.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWrapper::COL_AC_WHITE);
    ImGui::PushFont(m_imgui->bold_font_14);
    m_imgui->begin(get_name(), ImGuiWrapper::TOOLBAR_WINDOW_FLAGS_AC_NEW);
    ImGui::PopFont();
    ImGui::PushFont(m_imgui->default_font_13);
    ImGui::PopStyleColor();
    // First calculate width of all the texts that are could possibly be shown. We will decide set the dialog width based on that:
    const float clipping_slider_left = std::max(m_imgui->calc_text_size(m_desc.at("clipping_of_view")).x,
                                                m_imgui->calc_text_size(m_desc.at("reset_direction")).x) +
                                       m_imgui->scaled(1.5f);
    const float cursor_size_slider_left = m_imgui->calc_text_size(m_desc.at("cursor_size")).x + m_imgui->scaled(1.f);

    const float cursor_type_radio_left   = m_imgui->calc_text_size(m_desc["cursor_type"]).x + m_imgui->scaled(1.f);
    const float cursor_type_radio_sphere = m_imgui->calc_text_size(m_desc["sphere"]).x + m_imgui->scaled(2.5f);
    const float cursor_type_radio_circle = m_imgui->calc_text_size(m_desc["circle"]).x + m_imgui->scaled(2.5f);

    const float button_width         = m_imgui->calc_text_size(m_desc.at("remove_all")).x + m_imgui->scaled(1.f);
    const float minimal_slider_width = m_imgui->scaled(4.f);

    float caption_max    = 0.f;
    float total_text_max = 0.f;
    for (const auto &t : std::array<std::string, 3>{"enforce", "block", "remove"}) {
        caption_max    = std::max(caption_max, m_imgui->calc_text_size(m_desc[t + "_caption"]).x);
        total_text_max = std::max(total_text_max, m_imgui->calc_text_size(m_desc[t]).x);
    }
    total_text_max += caption_max + m_imgui->scaled(1.f);
    caption_max += m_imgui->scaled(1.f);

    const float sliders_left_width = std::max(cursor_size_slider_left, clipping_slider_left);
    const float slider_icon_width  = m_imgui->get_slider_icon_size().x;
    float       window_width       = minimal_slider_width + sliders_left_width + slider_icon_width;
    window_width                   = std::max(window_width, total_text_max);
    window_width                   = std::max(window_width, button_width);
    window_width                   = std::max(window_width, cursor_type_radio_left + cursor_type_radio_sphere + cursor_type_radio_circle);

    auto draw_text_with_caption = [this, &caption_max](const float &_win_w, const wxString &caption, const wxString &text) {
        // ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.0f * m_imgui->get_style_scaling());
        m_imgui->text_colored(ImGuiWrapper::COL_AC_BLUE, caption);
        ImGui::SameLine(caption_max);
        //ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
        ImVec2 text_size = m_imgui->calc_text_size(text);
        ImGui::SetCursorPosX(_win_w - text_size.x - 25.0f * m_imgui->get_style_scaling());

        m_imgui->text_colored(ImGuiWrapper::COL_AC_BLACK, text);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f * m_imgui->get_style_scaling());
    };
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f * _scaleIndex);
    for (const auto &t : std::array<std::string, 3>{"enforce", "block", "remove"})
        draw_text_with_caption(win_w, m_desc.at(t + "_caption"), m_desc.at(t));

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f * _scaleIndex);
    ImGui::Separator();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f * _scaleIndex);

    ImTextureID normal_id = m_parent.get_gizmos_manager().get_icon_texture_id(GLGizmosManager::MENU_ICON_NAME::IC_TITLE_CIRCLE);

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2 * _scaleIndex);
    ImGui::Image(normal_id, ImVec2(6.0f * _scaleIndex, 6.0f * _scaleIndex));

    ImGui::PushFont(m_imgui->default_font_13);

    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * _scaleIndex);
    m_imgui->text(m_desc.at("tool_type"));
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 16.0f * _scaleIndex);
    ImGui::PopFont();

    const float max_tooltip_width = ImGui::GetFontSize() * 20.0f;

    // ImGui::AlignTextToFramePadding();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 8.0f * _scaleIndex);
    m_imgui->text(m_desc.at("cursor_size"));
    ImGui::SameLine(sliders_left_width);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 28.0f * _scaleIndex);
    // ImGui::PushItemWidth(window_width - sliders_left_width - slider_icon_width);
    ImGui::PushItemWidth(130.0f * _scaleIndex);
    //ImGui::PushFont(m_imgui->default_font_12);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGuiWrapper::COL_AC_LIGHTGRAY);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 4.0f);
    m_imgui->slider_float("##cursor_radius", &m_cursor_radius, CursorRadiusMin, CursorRadiusMax, "%.2f", 1.0f, true,
                          ""/*_L("Alt + Mouse wheel")*/);
    //ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f * _scaleIndex);
    // ImGui::AlignTextToFramePadding();
    m_imgui->text(m_desc.at("cursor_type"));

    float cursor_type_offset = cursor_type_radio_left + (window_width - cursor_type_radio_left - cursor_type_radio_sphere -
                                                         cursor_type_radio_circle + m_imgui->scaled(0.5f)) /
                                                            2.f;
    ImGui::SameLine(cursor_type_offset);
    ImGui::PushItemWidth(cursor_type_radio_sphere);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 36.0f * _scaleIndex);
    if (m_imgui->radio_button(m_desc["sphere"], m_cursor_type == TriangleSelector::CursorType::SPHERE))
        m_cursor_type = TriangleSelector::CursorType::SPHERE;

    if (ImGui::IsItemHovered())
        m_imgui->tooltip(_L("Paints all facets inside, regardless of their orientation."), max_tooltip_width);

    ImGui::SameLine(cursor_type_offset + cursor_type_radio_sphere);
    ImGui::PushItemWidth(cursor_type_radio_circle);
    ImGui::SetCursorPosX(win_w - cursor_type_radio_circle - 15.0f * _scaleIndex);
    if (m_imgui->radio_button(m_desc["circle"], m_cursor_type == TriangleSelector::CursorType::CIRCLE))
        m_cursor_type = TriangleSelector::CursorType::CIRCLE;

    if (ImGui::IsItemHovered())
        m_imgui->tooltip(_L("Ignores facets facing away from the camera."), max_tooltip_width);
    ImGui::PopStyleVar();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 13.0f * _scaleIndex);
    ImGui::Separator();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15.0f * _scaleIndex);
    if (m_c->object_clipper()->get_position() == 0.f) {
        // ImGui::AlignTextToFramePadding();
        m_imgui->text(m_desc.at("clipping_of_view"));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.97f, 0.99f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGuiWrapper::COL_HOVER);
        m_imgui->push_ac_button_style(ImGui::GetCursorPosY(), 10.0f * _scaleIndex, 3.0f * _scaleIndex, 6.0f * _scaleIndex);
        if (m_imgui->button(m_desc.at("reset_direction"))) {
            wxGetApp().CallAfter([this]() { m_c->object_clipper()->set_position_by_ratio(-1., false); });
        }


        ImGui::PopStyleColor(2);
        m_imgui->pop_ac_button_style();
        // ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4.0f * _scaleIndex);
    }

    auto clp_dist = float(m_c->object_clipper()->get_position());
    ImGui::SameLine(sliders_left_width);
    ImGui::PushItemWidth(130.0f * _scaleIndex);
    // ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f * _scaleIndex);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f * _scaleIndex);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGuiWrapper::COL_AC_LIGHTGRAY);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 4.0f);
    if (m_imgui->slider_float("##clp_dist", &clp_dist, 0.f, 1.f, "%.2f", 1.0f, true, ""/*_L("Ctrl + Mouse wheel")*/))
        m_c->object_clipper()->set_position_by_ratio(clp_dist, true);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f * _scaleIndex);

    wxString btText = m_desc.at("remove_all");
    ImVec2 btTextSize = m_imgui->calc_button_size(btText);
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - btTextSize.x)/2);


    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.97f, 0.99f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGuiWrapper::COL_HOVER);
    ImGui::PushStyleColor(ImGuiCol_Border, ImGuiWrapper::COL_AC_BLUE);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWrapper::COL_AC_BLUE);

    ImGui::PushFont(m_imgui->default_font_14);
    if (m_imgui->button(btText)) {
        Plater::TakeSnapshot snapshot(wxGetApp().plater(), _L("Reset selection"), UndoRedo::SnapshotType::GizmoAction);
        ModelObject *        mo  = m_c->selection_info()->model_object();
        int                  idx = -1;
        for (ModelVolume *mv : mo->volumes)
            if (mv->is_model_part()) {
                ++idx;
                m_triangle_selectors[idx]->reset();
                m_triangle_selectors[idx]->request_update_render_data();
            }

        update_model_object();
        m_parent.set_as_dirty();
    }
    ImGui::PopStyleColor(4);
    ImGui::PopFont();

    last_seam_win_h = ImGui::GetWindowHeight();
    ImGui::PopFont();
    ImGui::PopStyleVar(2);
    m_imgui->end();

    // last_h = ImGui::GetWindowHeight();
}


void GLGizmoSeam::update_model_object() const
{
    bool updated = false;
    ModelObject* mo = m_c->selection_info()->model_object();
    int idx = -1;
    for (ModelVolume* mv : mo->volumes) {
        if (! mv->is_model_part())
            continue;
        ++idx;
        updated |= mv->seam_facets.set(*m_triangle_selectors[idx].get());
    }

    if (updated) {
        const ModelObjectPtrs& mos = wxGetApp().model().objects;
        wxGetApp().obj_list()->update_info_items(std::find(mos.begin(), mos.end(), mo) - mos.begin());

        m_parent.post_event(SimpleEvent(EVT_GLCANVAS_SCHEDULE_BACKGROUND_PROCESS));
    }
}



void GLGizmoSeam::update_from_model_object()
{
    wxBusyCursor wait;

    const ModelObject* mo = m_c->selection_info()->model_object();
    m_triangle_selectors.clear();

    int volume_id = -1;
    for (const ModelVolume* mv : mo->volumes) {
        if (! mv->is_model_part())
            continue;

        ++volume_id;

        // This mesh does not account for the possible Z up SLA offset.
        const TriangleMesh* mesh = &mv->mesh();

        m_triangle_selectors.emplace_back(std::make_unique<TriangleSelectorGUI>(*mesh));
        // Reset of TriangleSelector is done inside TriangleSelectorGUI's constructor, so we don't need it to perform it again in deserialize().
        m_triangle_selectors.back()->deserialize(mv->seam_facets.get_data(), false);
        m_triangle_selectors.back()->request_update_render_data();
    }
}


PainterGizmoType GLGizmoSeam::get_painter_type() const
{
    return PainterGizmoType::SEAM;
}

wxString GLGizmoSeam::handle_snapshot_action_name(bool shift_down, GLGizmoPainterBase::Button button_down) const
{
    wxString action_name;
    if (shift_down)
        action_name = _L("Remove selection");
    else {
        if (button_down == Button::Left)
            action_name = _L("Enforce seam");
        else
            action_name = _L("Block seam");
    }
    return action_name;
}

} // namespace Slic3r::GUI
