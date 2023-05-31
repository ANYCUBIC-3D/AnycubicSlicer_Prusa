#include "GLGizmosCommon.hpp"

#include <cassert>

#include "slic3r/GUI/GLCanvas3D.hpp"
#include "libslic3r/SLAPrint.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/Camera.hpp"
#include "slic3r/GUI/Plater.hpp"

#include "libslic3r/PresetBundle.hpp"

#include <GL/glew.h>

namespace Slic3r {
namespace GUI {

using namespace CommonGizmosDataObjects;

CommonGizmosDataPool::CommonGizmosDataPool(GLCanvas3D* canvas)
    : m_canvas(canvas)
{
    using c = CommonGizmosDataID;
    m_data[c::SelectionInfo].reset(   new SelectionInfo(this));
    m_data[c::InstancesHider].reset(  new InstancesHider(this));
//    m_data[c::HollowedMesh].reset(    new HollowedMesh(this));
    m_data[c::Raycaster].reset(       new Raycaster(this));
    m_data[c::ObjectClipper].reset(   new ObjectClipper(this));
    m_data[c::SupportsClipper].reset( new SupportsClipper(this));

}

void CommonGizmosDataPool::update(CommonGizmosDataID required)
{
    assert(check_dependencies(required));
    for (auto& [id, data] : m_data) {
        if (int(required) & int(CommonGizmosDataID(id)))
            data->update();
        else
            if (data->is_valid())
                data->release();

    }
}


SelectionInfo* CommonGizmosDataPool::selection_info() const
{
    SelectionInfo* sel_info = dynamic_cast<SelectionInfo*>(m_data.at(CommonGizmosDataID::SelectionInfo).get());
    assert(sel_info);
    return sel_info->is_valid() ? sel_info : nullptr;
}


InstancesHider* CommonGizmosDataPool::instances_hider() const
{
    InstancesHider* inst_hider = dynamic_cast<InstancesHider*>(m_data.at(CommonGizmosDataID::InstancesHider).get());
    assert(inst_hider);
    return inst_hider->is_valid() ? inst_hider : nullptr;
}

Raycaster* CommonGizmosDataPool::raycaster() const
{
    Raycaster* rc = dynamic_cast<Raycaster*>(m_data.at(CommonGizmosDataID::Raycaster).get());
    assert(rc);
    return rc->is_valid() ? rc : nullptr;
}

ObjectClipper* CommonGizmosDataPool::object_clipper() const
{
    ObjectClipper* oc = dynamic_cast<ObjectClipper*>(m_data.at(CommonGizmosDataID::ObjectClipper).get());
    // ObjectClipper is used from outside the gizmos to report current clipping plane.
    // This function can be called when oc is nullptr.
    return (oc && oc->is_valid()) ? oc : nullptr;
}

SupportsClipper* CommonGizmosDataPool::supports_clipper() const
{
    SupportsClipper* sc = dynamic_cast<SupportsClipper*>(m_data.at(CommonGizmosDataID::SupportsClipper).get());
    assert(sc);
    return sc->is_valid() ? sc : nullptr;
}

#ifndef NDEBUG
// Check the required resources one by one and return true if all
// dependencies are met.
bool CommonGizmosDataPool::check_dependencies(CommonGizmosDataID required) const
{
    // This should iterate over currently required data. Each of them should
    // be asked about its dependencies and it must check that all dependencies
    // are also in required and before the current one.
    for (auto& [id, data] : m_data) {
        // in case we don't use this, the deps are irrelevant
        if (! (int(required) & int(CommonGizmosDataID(id))))
            continue;


        CommonGizmosDataID deps = data->get_dependencies();
        assert(int(deps) == (int(deps) & int(required)));
    }


    return true;
}
#endif // NDEBUG




void SelectionInfo::on_update()
{
    const Selection& selection = get_pool()->get_canvas()->get_selection();

    m_model_object = nullptr;
    m_print_object = nullptr;

    if (selection.is_single_full_instance()) {
        m_model_object = selection.get_model()->objects[selection.get_object_idx()];
        if (m_model_object)
            m_print_object = get_pool()->get_canvas()->sla_print()->get_print_object_by_model_object_id(m_model_object->id());

        m_z_shift = m_print_object ? m_print_object->get_current_elevation() : selection.get_first_volume()->get_sla_shift_z();
    }
}

void SelectionInfo::on_release()
{
    m_model_object = nullptr;
    m_model_volume = nullptr;
}

ModelInstance *SelectionInfo::model_instance() const
{
    int inst_idx = get_active_instance();
    return inst_idx < int(m_model_object->instances.size()) ?
               m_model_object->instances[get_active_instance()] : nullptr;
}

int SelectionInfo::get_active_instance() const
{
    return get_pool()->get_canvas()->get_selection().get_instance_idx();
}





void InstancesHider::on_update()
{
    const ModelObject* mo = get_pool()->selection_info()->model_object();
    int active_inst = get_pool()->selection_info()->get_active_instance();
    GLCanvas3D* canvas = get_pool()->get_canvas();

    if (mo && active_inst != -1) {
        canvas->toggle_model_objects_visibility(false);
        if (!m_hide_full_scene) {
            canvas->toggle_model_objects_visibility(true, mo, active_inst);
            canvas->toggle_sla_auxiliaries_visibility(false, mo, active_inst);
        }
        canvas->set_use_clipping_planes(true);
        // Some objects may be sinking, do not show whatever is below the bed.
        canvas->set_clipping_plane(0, ClippingPlane(Vec3d::UnitZ(), -SINKING_Z_THRESHOLD));
        canvas->set_clipping_plane(1, ClippingPlane(-Vec3d::UnitZ(), std::numeric_limits<double>::max()));


        std::vector<const TriangleMesh*> meshes;
        for (const ModelVolume* mv : mo->volumes)
            meshes.push_back(&mv->mesh());

        if (meshes != m_old_meshes) {
            m_clippers.clear();
            for (const TriangleMesh* mesh : meshes) {
                m_clippers.emplace_back(new MeshClipper);
                m_clippers.back()->set_plane(ClippingPlane(-Vec3d::UnitZ(), -SINKING_Z_THRESHOLD));
                m_clippers.back()->set_mesh(mesh->its);
            }
            m_old_meshes = meshes;
        }
    }
    else
        canvas->toggle_model_objects_visibility(true);
}

void InstancesHider::on_release()
{
    get_pool()->get_canvas()->toggle_model_objects_visibility(true);
    get_pool()->get_canvas()->set_use_clipping_planes(false);
    m_old_meshes.clear();
    m_clippers.clear();
}

void InstancesHider::set_hide_full_scene(bool hide)
{
    if (m_hide_full_scene != hide) {
        m_hide_full_scene = hide;
        on_update();
    }
}

void InstancesHider::render_cut() const
{
    const SelectionInfo* sel_info = get_pool()->selection_info();
    const ModelObject* mo = sel_info->model_object();
    Geometry::Transformation inst_trafo = mo->instances[sel_info->get_active_instance()]->get_transformation();

    size_t clipper_id = 0;
    for (const ModelVolume* mv : mo->volumes) {
        Geometry::Transformation vol_trafo  = mv->get_transformation();
        Geometry::Transformation trafo = inst_trafo * vol_trafo;
        trafo.set_offset(trafo.get_offset() + Vec3d(0., 0., sel_info->get_sla_shift()));

        auto& clipper = m_clippers[clipper_id];
        clipper->set_transformation(trafo);
        const ObjectClipper* obj_clipper = get_pool()->object_clipper();
        if (obj_clipper->is_valid() && obj_clipper->get_clipping_plane()
         && obj_clipper->get_position() != 0.) {
            ClippingPlane clp = *get_pool()->object_clipper()->get_clipping_plane();
            clp.set_normal(-clp.get_normal());
            clipper->set_limiting_plane(clp);
        }
        else
            clipper->set_limiting_plane(ClippingPlane::ClipsNothing());

#if ENABLE_GL_CORE_PROFILE || ENABLE_OPENGL_ES
        bool depth_test_enabled = ::glIsEnabled(GL_DEPTH_TEST);
#else
        glsafe(::glPushAttrib(GL_DEPTH_TEST));
#endif // ENABLE_GL_CORE_PROFILE || ENABLE_OPENGL_ES
        glsafe(::glDisable(GL_DEPTH_TEST));
        clipper->render_cut(mv->is_model_part() ? ColorRGBA(0.8f, 0.3f, 0.0f, 1.0f) : color_from_model_volume(*mv));
#if ENABLE_GL_CORE_PROFILE || ENABLE_OPENGL_ES
        if (depth_test_enabled)
            glsafe(::glEnable(GL_DEPTH_TEST));
#else
        glsafe(::glPopAttrib());
#endif // ENABLE_GL_CORE_PROFILE || ENABLE_OPENGL_ES

        ++clipper_id;
    }
}


void Raycaster::on_update()
{
    wxBusyCursor wait;
    const ModelObject* mo = get_pool()->selection_info()->model_object();
    const ModelVolume* mv = get_pool()->selection_info()->model_volume();

    if (mo == nullptr && mv == nullptr)
        return;

    std::vector<ModelVolume*> mvs;
    if (mv != nullptr)
        mvs.push_back(const_cast<ModelVolume*>(mv));
    else
        mvs = mo->volumes;

    std::vector<const TriangleMesh*> meshes;
    bool force_raycaster_regeneration = false;
    if (wxGetApp().preset_bundle->printers.get_selected_preset().printer_technology() == ptSLA) {
        // For sla printers we use the mesh generated by the backend
        std::shared_ptr<const indexed_triangle_set> preview_mesh_ptr;
        const SLAPrintObject* po = get_pool()->selection_info()->print_object();
        if (po)
            preview_mesh_ptr = po->get_mesh_to_print();

        if (preview_mesh_ptr)
            m_sla_mesh_cache = TriangleMesh{*preview_mesh_ptr};

        if (!m_sla_mesh_cache.empty()) {
            m_sla_mesh_cache.transform(po->trafo().inverse());
            meshes.emplace_back(&m_sla_mesh_cache);
            force_raycaster_regeneration = true;
        }
    }

    if (meshes.empty()) {
        const std::vector<ModelVolume*>& mvs = mo->volumes;
        for (const ModelVolume* mv : mvs) {
            if (mv->is_model_part())
                meshes.push_back(&mv->mesh());
        }
    }

    if (force_raycaster_regeneration || meshes != m_old_meshes) {
        m_raycasters.clear();
        for (const TriangleMesh* mesh : meshes)
            m_raycasters.emplace_back(new MeshRaycaster(std::make_shared<const TriangleMesh>(*mesh)));
        m_old_meshes = meshes;
    }
}

void Raycaster::on_release()
{
    m_raycasters.clear();
    m_old_meshes.clear();
}

std::vector<const MeshRaycaster*> Raycaster::raycasters() const
{
    std::vector<const MeshRaycaster*> mrcs;
    for (const auto& raycaster_unique_ptr : m_raycasters)
        mrcs.push_back(raycaster_unique_ptr.get());
    return mrcs;
}

} // namespace GUI

namespace GUI {


void ObjectClipper::on_update()
{
    const ModelObject* mo = get_pool()->selection_info()->model_object();
    if (! mo)
        return;

    // which mesh should be cut?
    std::vector<const TriangleMesh*> meshes;
    std::vector<Geometry::Transformation> trafos;
    bool force_clipper_regeneration = false;

    std::unique_ptr<MeshClipper> mc;
    Geometry::Transformation     mc_tr;
    if (wxGetApp().preset_bundle->printers.get_selected_preset().printer_technology() == ptSLA) {
        // For sla printers we use the mesh generated by the backend
        const SLAPrintObject* po = get_pool()->selection_info()->print_object();
        if (po) {
            auto partstoslice = po->get_parts_to_slice();
            if (! partstoslice.empty()) {
                mc = std::make_unique<MeshClipper>();
                mc->set_mesh(range(partstoslice));
                mc_tr = Geometry::Transformation{po->trafo().inverse().cast<double>()};
            }
        }
    }

    if (!mc && meshes.empty()) {
        for (const ModelVolume* mv : mo->volumes) {
            meshes.emplace_back(&mv->mesh());
            trafos.emplace_back(mv->get_transformation());
        }
    }

    if (mc || force_clipper_regeneration || meshes != m_old_meshes) {
        m_clippers.clear();
        for (size_t i = 0; i < meshes.size(); ++i) {
            m_clippers.emplace_back(new MeshClipper, trafos[i]);
            m_clippers.back().first->set_mesh(meshes[i]->its);
        }
        m_old_meshes = std::move(meshes);

        if (mc) {
            m_clippers.emplace_back(std::move(mc), mc_tr);
        }

        m_active_inst_bb_radius =
            mo->instance_bounding_box(get_pool()->selection_info()->get_active_instance()).radius();
    }
}


void ObjectClipper::on_release()
{
    m_clippers.clear();
    m_old_meshes.clear();
    m_clp.reset();
    m_clp_ratio = 0.;

}

void ObjectClipper::render_cut() const
{
    if (m_clp_ratio == 0.)
        return;
    const SelectionInfo* sel_info = get_pool()->selection_info();
    const Geometry::Transformation inst_trafo = sel_info->model_object()->instances[sel_info->get_active_instance()]->get_transformation();

    for (auto& clipper : m_clippers) {
      Geometry::Transformation trafo = inst_trafo * clipper.second;
      trafo.set_offset(trafo.get_offset() + Vec3d(0., 0., sel_info->get_sla_shift()));
      clipper.first->set_plane(*m_clp);
      clipper.first->set_transformation(trafo);
      clipper.first->set_limiting_plane(ClippingPlane(Vec3d::UnitZ(), -SINKING_Z_THRESHOLD));
      clipper.first->render_cut({ 1.0f, 0.37f, 0.0f, 1.0f });
      clipper.first->render_contour({ 1.f, 1.f, 1.f, 1.f });
    }
}

bool ObjectClipper::is_projection_inside_cut(const Vec3d& point) const
{
    return m_clp_ratio != 0. && std::any_of(m_clippers.begin(), m_clippers.end(), [point](const auto& cl) { return cl.first->is_projection_inside_cut(point); });
}

bool ObjectClipper::has_valid_contour() const
{
    return m_clp_ratio != 0. && std::any_of(m_clippers.begin(), m_clippers.end(), [](const auto& cl) { return cl.first->has_valid_contour(); });
}

void ObjectClipper::set_position_by_ratio(double pos, bool keep_normal)
{
    const ModelObject* mo = get_pool()->selection_info()->model_object();
    int active_inst = get_pool()->selection_info()->get_active_instance();
    double z_shift = get_pool()->selection_info()->get_sla_shift();

    Vec3d normal = (keep_normal && m_clp) ? m_clp->get_normal() : -wxGetApp().plater()->get_camera().get_dir_forward();
    const Vec3d& center = mo->instances[active_inst]->get_offset() + Vec3d(0., 0., z_shift);
    float dist = normal.dot(center);

    if (pos < 0.)
        pos = m_clp_ratio;

    m_clp_ratio = pos;
    m_clp.reset(new ClippingPlane(normal, (dist - (-m_active_inst_bb_radius) - m_clp_ratio * 2*m_active_inst_bb_radius)));
    get_pool()->get_canvas()->set_as_dirty();
}

void ObjectClipper::set_range_and_pos(const Vec3d& cpl_normal, double cpl_offset, double pos)
{
    m_clp.reset(new ClippingPlane(cpl_normal, cpl_offset));
    m_clp_ratio = pos;
    get_pool()->get_canvas()->set_as_dirty();
}

const ClippingPlane* ObjectClipper::get_clipping_plane(bool ignore_hide_clipped) const
{
    static const ClippingPlane no_clip = ClippingPlane::ClipsNothing();
    return (ignore_hide_clipped || m_hide_clipped) ? m_clp.get() : &no_clip;
}

void ObjectClipper::set_behavior(bool hide_clipped, bool fill_cut, double contour_width)
{
    m_hide_clipped = hide_clipped;
    for (auto& clipper : m_clippers)
        clipper.first->set_behaviour(fill_cut, contour_width);
}

void ObjectClipper::pass_mouse_click(const Vec3d& pt)
{
    for (auto& clipper : m_clippers)
        clipper.first->pass_mouse_click(pt);
}

std::vector<Vec3d> ObjectClipper::get_disabled_contours() const
{
    return std::vector<Vec3d>();
}

void SupportsClipper::on_update()
{
    const ModelObject* mo = get_pool()->selection_info()->model_object();
    bool is_sla = wxGetApp().preset_bundle->printers.get_selected_preset().printer_technology() == ptSLA;
    if (! mo || ! is_sla)
        return;

    const GLCanvas3D* canvas = get_pool()->get_canvas();
    const PrintObjects& print_objects = canvas->sla_print()->objects();
    const SLAPrintObject* print_object = (m_print_object_idx >= 0 && m_print_object_idx < int(print_objects.size()))
            ? print_objects[m_print_object_idx]
            : nullptr;

    // Find the respective SLAPrintObject.
    if (m_print_object_idx < 0 || m_print_objects_count != int(print_objects.size())) {
        m_print_objects_count = print_objects.size();
        m_print_object_idx = -1;
        for (const SLAPrintObject* po : print_objects) {
            ++m_print_object_idx;
            if (po->model_object()->id() == mo->id()) {
                print_object = po;
                break;
            }
        }
    }

    if (print_object
     && print_object->is_step_done(slaposSupportTree)
     && ! print_object->support_mesh().empty())
    {
        // If the supports are already calculated, save the timestamp of the respective step
        // so we can later tell they were recalculated.
        size_t timestamp = print_object->step_state_with_timestamp(slaposSupportTree).timestamp;
        if (! m_clipper || timestamp != m_old_timestamp) {
            // The timestamp has changed.
            m_clipper.reset(new MeshClipper);
            // The mesh should already have the shared vertices calculated.
            m_clipper->set_mesh(print_object->support_mesh().its);
            m_old_timestamp = timestamp;
        }
    }
    else
        // The supports are not valid. We better dump the cached data.
        m_clipper.reset();
}


void SupportsClipper::on_release()
{
    m_clipper.reset();
    m_old_timestamp = 0;
    m_print_object_idx = -1;
}

void SupportsClipper::render_cut() const
{
    const CommonGizmosDataObjects::ObjectClipper* ocl = get_pool()->object_clipper();
    if (ocl->get_position() == 0.
     || ! m_clipper)
        return;

    const SelectionInfo* sel_info = get_pool()->selection_info();
    const ModelObject* mo = sel_info->model_object();
    const Geometry::Transformation inst_trafo = mo->instances[sel_info->get_active_instance()]->get_transformation();
    //Geometry::Transformation vol_trafo  = mo->volumes.front()->get_transformation();
    Geometry::Transformation trafo = inst_trafo;// * vol_trafo;
    trafo.set_offset(trafo.get_offset() + Vec3d(0., 0., sel_info->get_sla_shift()));


    // Get transformation of supports
    Geometry::Transformation supports_trafo = trafo;
    supports_trafo.set_scaling_factor(Vec3d::Ones());
    supports_trafo.set_offset(Vec3d(trafo.get_offset()(0), trafo.get_offset()(1), sel_info->get_sla_shift()));
    supports_trafo.set_rotation(Vec3d(0., 0., trafo.get_rotation()(2)));
    // I don't know why, but following seems to be correct.
    supports_trafo.set_mirror(Vec3d(trafo.get_mirror()(0) * trafo.get_mirror()(1) * trafo.get_mirror()(2),
                                    1,
                                    1.));

    m_clipper->set_plane(*ocl->get_clipping_plane());
    m_clipper->set_transformation(supports_trafo);

    m_clipper->render_cut({ 1.0f, 0.f, 0.37f, 1.0f });
    m_clipper->render_contour({ 1.f, 1.f, 1.f, 1.f });
}


} // namespace GUI
} // namespace Slic3r