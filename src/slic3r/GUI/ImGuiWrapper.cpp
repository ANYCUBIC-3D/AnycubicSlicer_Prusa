#include "ImGuiWrapper.hpp"

#include <cstdio>
#include <vector>
#include <cmath>
#include <stdexcept>

#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/nowide/convert.hpp>

#include <wx/string.h>
#include <wx/event.h>
#include <wx/clipbrd.h>
#include <wx/debug.h>

#include <GL/glew.h>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui/imgui_internal.h>

#include "libslic3r/libslic3r.h"
#include "libslic3r/Utils.hpp"
#include "libslic3r/Color.hpp"

#include "3DScene.hpp"
#include "GUI.hpp"
#include "I18N.hpp"
#include "Search.hpp"
#include "BitmapCache.hpp"
#include "GUI_App.hpp"

#include "../Utils/MacDarkMode.hpp"
#include <nanosvg/nanosvg.h>
#include <nanosvg/nanosvgrast.h>

// suggest location
#include "libslic3r/ClipperUtils.hpp" // Slic3r::intersection
#include "nanosvg/nanosvg.h"
#include "nanosvg/nanosvgrast.h"
#include "ACDefines.h"

namespace Slic3r {
namespace GUI {


static const std::map<const wchar_t, std::string> font_icons = {
    {ImGui::PrintIconMarker       , "ACEmpty"                           },
    {ImGui::PrinterIconMarker     , "ACEmpty"                       },
    {ImGui::PrinterSlaIconMarker  , "ACEmpty"                   },
    {ImGui::FilamentIconMarker    , "ACEmpty"                         },
    {ImGui::MaterialIconMarker    , "ACEmpty"                         },
    {ImGui::MinimalizeButton      , "ACEmpty"       },
    {ImGui::MinimalizeHoverButton , "ACEmpty" },
    {ImGui::RightArrowButton      , "ACEmpty"            },
    {ImGui::RightArrowHoverButton , "ACEmpty"      },
    {ImGui::PreferencesButton     , "ACEmpty"      },
    {ImGui::PreferencesHoverButton, "ACEmpty"},
    {ImGui::SliderFloatEditBtnIcon, "edit-nor"                   },
    {ImGui::SliderFloatEditBtnPressedIcon, "edit-nor"    },
    {ImGui::ClipboardBtnIcon      , "ACEmpty"                     },
    {ImGui::ExpandBtn             , "arrow-up-hover"                    },
    {ImGui::CollapseBtn           , "arrow-down-hover"                  },
    {ImGui::RevertButton          , "ACEmpty"                          },
    {ImGui::WarningMarkerSmall    , "ACEmpty"          },
    {ImGui::InfoMarkerSmall       , "ACEmpty"             },
    {ImGui::PlugMarker            , "ACEmpty"                          },
    {ImGui::DowelMarker           , "ACEmpty"                         },
};

static const std::map<const wchar_t, std::string> font_icons_large = {
    //{ImGui::LegendTravel            , "legend_travel"                   },
    //{ImGui::LegendWipe              , "legend_wipe"                     },
    //{ImGui::LegendRetract           , "legend_retract"                  },
    //{ImGui::LegendDeretract         , "legend_deretract"                },
    //{ImGui::LegendSeams             , "legend_seams"                    },
    //{ImGui::LegendToolChanges       , "legend_toolchanges"              },
    //{ImGui::LegendColorChanges      , "legend_colorchanges"             },
    //{ImGui::LegendPausePrints       , "legend_pauseprints"              },
    //{ImGui::LegendCustomGCodes      , "legend_customgcodes"             },
    //{ImGui::LegendCOG               , "legend_cog"                      },
    //{ImGui::LegendShells            , "legend_shells"                   },
    //{ImGui::LegendToolMarker        , "legend_toolmarker"               },
    //{ImGui::InfoMarker              , "notification_info"               },
    //{ImGui::SlaViewOriginal         , "sla_view_original"               },
    //{ImGui::SlaViewProcessed        , "sla_view_processed"              },
    {ImGui::LegendNorTravel            , "legend_travel_nor"                   },
    //{ImGui::LegendNorWipe              , "legend_wipe_nor"                     },
    {ImGui::LegendNorRetract           , "legend_retract_nor"                  },
    {ImGui::LegendNorDeretract         , "legend_deretract_nor"                },
    {ImGui::LegendNorSeams             , "legend_seams_nor"                    },
    //{ImGui::LegendNorToolChanges       , "legend_toolchanges_nor"              },
    //{ImGui::LegendNorColorChanges      , "legend_colorchanges_nor"             },
    //{ImGui::LegendNorPausePrints       , "legend_pauseprints_nor"              },
    //{ImGui::LegendNorCustomGCodes      , "legend_customgcodes_nor"             },
    //{ImGui::LegendNorCOG               , "legend_cog_nor"                      },
    {ImGui::LegendNorShells            , "legend_shells_nor"                   },
    //{ImGui::LegendNorToolMarker        , "legend_toolmarker_nor"               },

    {ImGui::LegendHovTravel            , "legend_travel_hov"                   },
    //{ImGui::LegendHovWipe              , "legend_wipe_hov"                     },
    {ImGui::LegendHovRetract           , "legend_retract_hov"                  },
    {ImGui::LegendHovDeretract         , "legend_deretract_hov"                },
    {ImGui::LegendHovSeams             , "legend_seams_hov"                    },
    //{ImGui::LegendHovToolChanges       , "legend_toolchanges_hov"              },
    //{ImGui::LegendHovColorChanges      , "legend_colorchanges_hov"             },
    //{ImGui::LegendHovPausePrints       , "legend_pauseprints_hov"              },
    //{ImGui::LegendHovCustomGCodes      , "legend_customgcodes_hov"             },
    //{ImGui::LegendHovCOG               , "legend_cog_hov"                      },
    {ImGui::LegendHovShells            , "legend_shells_hov"                   },
    //{ImGui::LegendHovToolMarker        , "legend_toolmarker_hov"               },


    {ImGui::LegendSelTravel            , "legend_travel_sel"                   },
    //{ImGui::LegendSelWipe              , "legend_wipe_sel"                     },
    {ImGui::LegendSelRetract           , "legend_retract_sel"                  },
    {ImGui::LegendSelDeretract         , "legend_deretract_sel"                },
    {ImGui::LegendSelSeams             , "legend_seams_sel"                    },
    //{ImGui::LegendSelToolChanges       , "legend_toolchanges_sel"              },
    //{ImGui::LegendSelColorChanges      , "legend_colorchanges_sel"             },
    //{ImGui::LegendSelPausePrints       , "legend_pauseprints_sel"              },
    //{ImGui::LegendSelCustomGCodes      , "legend_customgcodes_sel"             },
    //{ImGui::LegendSelCOG               , "legend_cog_sel"                      },
    {ImGui::LegendSelShells            , "legend_shells_sel"                   },
    //{ImGui::LegendSelToolMarker        , "legend_toolmarker_sel"               },

    {ImGui::CloseNotifButton        , "ACEmpty"              },
    {ImGui::CloseNotifHoverButton   , "ACEmpty"        },
    {ImGui::EjectButton             , "ACEmpty"           },
    {ImGui::EjectHoverButton        , "ACEmpty"     },
    {ImGui::WarningMarker           , "ico-exclamation"            },
    {ImGui::ErrorMarker             , "ico-exclamation"              },
    {ImGui::CancelButton            , "ACEmpty"             },
    {ImGui::CancelHoverButton       , "ACEmpty"       },
//    {ImGui::SinkingObjectMarker     , "ACEmpty"                            },
//    {ImGui::CustomSupportsMarker    , "ACEmpty"                    },
//    {ImGui::CustomSeamMarker        , "ACEmpty"                            },
//    {ImGui::MmuSegmentationMarker   , "ACEmpty"                },
//    {ImGui::VarLayerHeightMarker    , "ACEmpty"                          },
    {ImGui::DocumentationButton     , "ACEmpty"      },
    {ImGui::DocumentationHoverButton, "ACEmpty"},
    {ImGui::InfoMarker              , "ACEmpty"               },
    {ImGui::PlayButton              , "ACEmpty"               },
    {ImGui::PlayHoverButton         , "ACEmpty"         },
    {ImGui::PauseButton             , "ACEmpty"              },
    {ImGui::PauseHoverButton        , "ACEmpty"        },
    {ImGui::OpenButton              , "ACEmpty"               },
    {ImGui::OpenHoverButton         , "ACEmpty"         },
};

static const std::map<const wchar_t, std::string> font_icons_extra_large = {
    {ImGui::ClippyMarker            , "ACEmpty"             },
};

const ImVec4 ImGuiWrapper::COL_AC_RED = {float(229.0 / 255.0), float(46.0 / 255.0), float(46.0 / 255.0),1.0f};
const ImVec4 ImGuiWrapper::COL_AC_GREEN = { float(47.0 / 255.0), float(229.0 / 255.0), float(47.0 / 255.0),1.0f};
const ImVec4 ImGuiWrapper::COL_AC_PURPLE = { float(61.0 / 255.0), float(61.0 / 255.0), float(204.0 / 255.0),1.0f};

const ImVec4 ImGuiWrapper::COL_AC_BLACK             = { float(20/255.0), float(28/255.0), float(41/255.0) , 1.0f };
const ImVec4 ImGuiWrapper::COL_AC_BLACK_GRAY        = { float(20.0 / 255.0), float(28.0 / 255.0), float(41.0 / 255.0), 0.6f};


const ImVec4 ImGuiWrapper::COL_AC_LIST_SCROLL = { float(142 / 255.0), float(153 / 255.0), float(173 / 255.0), 1.0f};
const ImVec4 ImGuiWrapper::COL_AC_LIST_CLICK = { float(128 / 255.0), float(176 / 255.0), float(255 / 255.0), 1.0f};
const ImVec4 ImGuiWrapper::COL_AC_LIST_HOVER        = { float(230 / 255.0), float(240 / 255.0), float(255 / 255.0), 1.0f};

const ImVec4 ImGuiWrapper::COL_AC_BLUE              = { float(57/255.0), float(134/255.0), float(255/255.0), 1.0f };
const ImVec4 ImGuiWrapper::COL_AC_BLUE_PROGRESSBAR  = { float(168 / 255.0), float(194 / 255.0), float(233 / 255.0), 1.0f};
const ImVec4 ImGuiWrapper::COL_AC_LIGHTBLUE         = { float(95/255.0), float(157/255.0), float(255/255.0), 1.0f };
const ImVec4 ImGuiWrapper::COL_AC_DARKBLUE          = { float(29/255.0), float(105/255.0), float(224/255.0), 1.0f };
const ImVec4 ImGuiWrapper::COL_AC_ITEMBLUE          = { float(231/255.0), float(240/255.0), float(255/255.0), 1.0f };

const ImVec4 ImGuiWrapper::COL_AC_GRAY              = { float(214/255.0), float(220/255.0), float(230/255.0), 1.0f };
const ImVec4 ImGuiWrapper::COL_AC_LIGHTGRAY         = { float(240/255.0), float(243/255.0), float(247/255.0), 1.0f };
const ImVec4 ImGuiWrapper::COL_AC_DARKGRAY          = { 0.4f, 0.4f, 0.4f, 1.0f };
const ImVec4 ImGuiWrapper::COL_AC_PANELGRAY         = COL_AC_LIGHTGRAY;
const ImVec4 ImGuiWrapper::COL_AC_BOARDGRAY         = COL_AC_GRAY;
const ImVec4 ImGuiWrapper::COL_AC_WHITE             = { 1.0, 1.0, 1.0, 1.0f };
const ImVec4 ImGuiWrapper::COL_AC_PANEL_BACKGROUND        = { float(240/255.0), float(243/255.0), float(247/255.0), 1.0f };

//const ImVec4 ImGuiWrapper::COL_AC_WINDOW_BACKGROUND = COL_AC_WHITE;
//const ImVec4 ImGuiWrapper::COL_AC_PANEL_BACKGROUND  = COL_AC_PANELGRAY;
//const ImVec4 ImGuiWrapper::COL_AC_BUTTON_BACKGROUND = COL_AC_PANELGRAY;
//
//const ImVec4 ImGuiWrapper::COL_AC_BUTTON_HOVERED    = COL_AC_MAINBLUE;
//const ImVec4 ImGuiWrapper::COL_AC_BUTTON_ACTIVE     = COL_BUTTON_AC_HOVERED;

const ImVec4 ImGuiWrapper::COL_GREY_DARK         = { 0.33f, 0.33f, 0.33f, 1.0f };
const ImVec4 ImGuiWrapper::COL_GREY_LIGHT        = { 0.4f, 0.4f, 0.4f, 1.0f };
const ImVec4 ImGuiWrapper::COL_BLUE_LIGHT        = ImVec4(230/255.0f, 240/255.0f, 255/255.0f, 1.0f); // 230, 240, 255
const ImVec4 ImGuiWrapper::COL_BLUE_DARK        = ImVec4(57/255.0f, 134/255.0f, 255/255.0f, 1.0f); //57, 134, 255
const ImVec4 ImGuiWrapper::COL_WINDOW_BACKGROUND = COL_AC_LIGHTGRAY;// { 0.13f, 0.13f, 0.13f, 0.8f };
const ImVec4 ImGuiWrapper::COL_BUTTON_BACKGROUND = COL_BLUE_DARK;
const ImVec4 ImGuiWrapper::COL_BUTTON_HOVERED    = COL_BLUE_LIGHT;
const ImVec4 ImGuiWrapper::COL_BUTTON_ACTIVE     = COL_BUTTON_HOVERED;

const ImVec4 ImGuiWrapper::COL_GREEN_LIGHT       = ImVec4(0.86f, 0.99f, 0.91f, 1.0f);
const ImVec4 ImGuiWrapper::COL_HOVER             = { 0.933f, 0.933f, 0.933f, 1.0f };
const ImVec4 ImGuiWrapper::COL_ACTIVE            = { 0.675f, 0.675f, 0.675f, 1.0f };
const ImVec4 ImGuiWrapper::COL_SEPARATOR         = { 0.93f, 0.93f, 0.93f, 1.0f };
const ImVec4 ImGuiWrapper::COL_SEPARATOR_DARK    = { 0.24f, 0.24f, 0.27f, 1.0f };
const ImVec4 ImGuiWrapper::COL_TITLE_BG          = { 0.745f, 0.745f, 0.745f, 1.0f };
const ImVec4 ImGuiWrapper::COL_WINDOW_BG         = { 1.000f, 1.000f, 1.000f, 1.0f };
const ImVec4 ImGuiWrapper::COL_WINDOW_BG_DARK    = { 45 / 255.f, 45 / 255.f, 49 / 255.f, 1.f };

const ImVec4 ImGuiWrapper::COL_WINDOW_BLUE      = {57 / 255.f, 134 / 255.f, 255 / 255.f, 1.f};
const ImVec4 ImGuiWrapper::COL_WINDOW_BLUE_DARK = {45 / 255.f, 45 / 255.f, 49 / 255.f, 1.f};

int ImGuiWrapper::TOOLBAR_WINDOW_FLAGS = ImGuiWindowFlags_AlwaysAutoResize
                                 | ImGuiWindowFlags_NoMove
                                 | ImGuiWindowFlags_NoResize
                                 | ImGuiWindowFlags_NoCollapse
                                 | ImGuiWindowFlags_NoTitleBar;

int ImGuiWrapper::TOOLBAR_WINDOW_FLAGS_AC = ImGuiWindowFlags_NoMove |
                                         ImGuiWindowFlags_NoResize |
                                         ImGuiWindowFlags_NoCollapse |
                                         ImGuiWindowFlags_NoTitleBar;

int ImGuiWrapper::TOOLBAR_WINDOW_FLAGS_AC_NEW = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                            ImGuiWindowFlags_NoCollapse;




static float accer = 1.f;

bool get_data_from_svg(const std::string &filename, unsigned int max_size_px, ThumbnailData &thumbnail_data)
{
    bool compression_enabled = false;

    NSVGimage *image = nsvgParseFromFile(filename.c_str(), "px", 96.0f);
    if (image == nullptr) { return false; }

    float scale = (float) max_size_px / std::max(image->width, image->height);

    thumbnail_data.width  = (int) (scale * image->width);
    thumbnail_data.height = (int) (scale * image->height);

    int n_pixels = thumbnail_data.width * thumbnail_data.height;

    if (n_pixels <= 0) {
        nsvgDelete(image);
        return false;
    }

    NSVGrasterizer *rast = nsvgCreateRasterizer();
    if (rast == nullptr) {
        nsvgDelete(image);
        return false;
    }

    // creates the temporary buffer only once, with max size, and reuse it for all the levels, if generating mipmaps
    std::vector<unsigned char> data(n_pixels * 4, 0);
    thumbnail_data.pixels = data;
    nsvgRasterize(rast, image, 0, 0, scale, thumbnail_data.pixels.data(), thumbnail_data.width, thumbnail_data.height, thumbnail_data.width * 4);

    // we manually generate mipmaps because glGenerateMipmap() function is not reliable on all graphics cards
    int   lod_w = thumbnail_data.width;
    int   lod_h = thumbnail_data.height;
    GLint level = 0;
    while (lod_w > 1 || lod_h > 1) {
        ++level;

        lod_w = std::max(lod_w / 2, 1);
        lod_h = std::max(lod_h / 2, 1);
        scale /= 2.0f;

        data.resize(lod_w * lod_h * 4);

        nsvgRasterize(rast, image, 0, 0, scale, data.data(), lod_w, lod_h, lod_w * 4);
    }

    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);

    return true;
}


bool slider_behavior(ImGuiID id, const ImRect& region, const ImS32 v_min, const ImS32 v_max, ImS32* out_value, ImRect* out_handle, ImGuiSliderFlags flags/* = 0*/, const int fixed_value/* = -1*/, const ImVec4& fixed_rect/* = ImRect()*/)
{
    ImGuiContext& context = *GImGui;
    ImGuiIO& io = ImGui::GetIO();

    const ImGuiAxis axis = (flags & ImGuiSliderFlags_Vertical) ? ImGuiAxis_Y : ImGuiAxis_X;

    const ImVec2 handle_sz = out_handle->GetSize();
    ImS32 v_range = (v_min < v_max ? v_max - v_min : v_min - v_max);
    const float region_usable_sz = (region.Max[axis] - region.Min[axis]);
    const float region_usable_pos_min = region.Min[axis];
    const float region_usable_pos_max = region.Max[axis];

    // Process interacting with the slider
    ImS32 v_new = *out_value;
    bool value_changed = false;
    // wheel behavior
    ImRect mouse_wheel_responsive_region;
    if (axis == ImGuiAxis_X)
        mouse_wheel_responsive_region = ImRect(region.Min - ImVec2(handle_sz.x / 2, 0), region.Max + ImVec2(handle_sz.x / 2, 0));
    if (axis == ImGuiAxis_Y)
        mouse_wheel_responsive_region = ImRect(region.Min - ImVec2(0, handle_sz.y), region.Max + ImVec2(0, handle_sz.y));
    if (ImGui::ItemHoverable(mouse_wheel_responsive_region, id)) {
        v_new = ImClamp(*out_value + (ImS32)(context.IO.MouseWheel * accer), v_min, v_max);
    }
    // drag behavior
    if (context.ActiveId == id)
    {
        float mouse_pos_ratio = 0.0f;
        if (context.ActiveIdSource == ImGuiInputSource_Mouse)
        {
            if (context.IO.MouseReleased[0])
            {
                ImGui::ClearActiveID();
            }
            if (context.IO.MouseDown[0])
            {
                const float mouse_abs_pos = context.IO.MousePos[axis];
                mouse_pos_ratio = (region_usable_sz > 0.0f) ? ImClamp((mouse_abs_pos - region_usable_pos_min) / region_usable_sz, 0.0f, 1.0f) : 0.0f;
                if (axis == ImGuiAxis_Y)
                    mouse_pos_ratio = 1.0f - mouse_pos_ratio;
                v_new = v_min + (ImS32)(v_range * mouse_pos_ratio + 0.5f);
            }
        }
    }
    // click in fixed_rect behavior
    if (ImGui::ItemHoverable(fixed_rect, id) && context.IO.MouseReleased[0])
    {
        v_new = fixed_value;
    }

	// apply result, output value
	if (*out_value != v_new)
	{
		*out_value = v_new;
		value_changed = true;
	}

    // Output handle position so it can be displayed by the caller
    const ImS32 v_clamped = (v_min < v_max) ? ImClamp(*out_value, v_min, v_max) : ImClamp(*out_value, v_max, v_min);
    float handle_pos_ratio = v_range != 0 ? ((float)(v_clamped - v_min) / (float)v_range) : 0.0f;
    handle_pos_ratio = axis == ImGuiAxis_Y ? 1.0f - handle_pos_ratio : handle_pos_ratio;
    const float handle_pos = region_usable_pos_min + (region_usable_pos_max - region_usable_pos_min) * handle_pos_ratio;

    ImVec2 new_handle_center = axis == ImGuiAxis_Y ? ImVec2(out_handle->GetCenter().x, handle_pos) : ImVec2(handle_pos, out_handle->GetCenter().y);
    *out_handle = ImRect(new_handle_center - handle_sz * 0.5f, new_handle_center + handle_sz * 0.5f);

    return value_changed;
}

bool button_with_pos(ImTextureID user_texture_id, const ImVec2 &size, const ImVec2 &pos, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col, const ImVec2 &margin)
{

    ImGuiContext &g      = *GImGui;
    ImGuiWindow * window = g.CurrentWindow;
    if (window->SkipItems) return false;

    // Default to using texture ID as ID. User can still push string/integer prefixes.
    ImGui::PushID((void *) (intptr_t) user_texture_id);
    const ImGuiID id = window->GetID("#image");
    ImGui::PopID();

    const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float) frame_padding, (float) frame_padding) : g.Style.FramePadding;

    const ImRect bb(pos, pos + size + padding * 2 + margin * 2);

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Render
    const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImGui::RenderNavHighlight(bb, id);

    const float border_size = g.Style.FrameBorderSize;
    if (border_size > 0.0f) {
        window->DrawList->AddRect(bb.Min + ImVec2(1, 1), bb.Max + ImVec2(1, 1), col, g.Style.FrameRounding, 0, border_size);
        window->DrawList->AddRect(bb.Min, bb.Max, col, g.Style.FrameRounding, 0, border_size);
    }

    if (bg_col.w > 0.0f) window->DrawList->AddRectFilled(bb.Min + padding, bb.Max - padding, ImGui::GetColorU32(bg_col));
    window->DrawList->AddImage(user_texture_id, bb.Min + padding + margin, bb.Max - padding - margin, uv0, uv1, ImGui::GetColorU32(tint_col));

    return pressed;
}

ImGuiWrapper::ImGuiWrapper()
{
    ImGui::CreateContext();

    init_input();
    init_style();

    ImGui::GetIO().IniFilename = nullptr;
}

ImGuiWrapper::~ImGuiWrapper()
{
    destroy_font();
    ImGui::DestroyContext();
}

void ImGuiWrapper::set_language(const std::string &language)
{
    if (m_new_frame_open) {
        // ImGUI internally locks the font between NewFrame() and EndFrame()
        // NewFrame() might've been called here because of input from the 3D scene;
        // call EndFrame()
        ImGui::EndFrame();
        m_new_frame_open = false;
    }

    const ImWchar *ranges = nullptr;
    size_t idx = language.find('_');
    std::string lang = (idx == std::string::npos) ? language : language.substr(0, idx);
    static const ImWchar ranges_latin2[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0100, 0x017F, // Latin Extended-A
        0,
    };
	static const ImWchar ranges_turkish[] = {
		0x0020, 0x01FF, // Basic Latin + Latin Supplement
		0x0100, 0x017F, // Latin Extended-A
		0x0180, 0x01FF, // Turkish
		0,
	};
    static const ImWchar ranges_vietnamese[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x0102, 0x0103,
        0x0110, 0x0111,
        0x0128, 0x0129,
        0x0168, 0x0169,
        0x01A0, 0x01A1,
        0x01AF, 0x01B0,
        0x1EA0, 0x1EF9,
        0,
    };
    m_font_cjk = false;
    if (lang == "cs" || lang == "pl" || lang == "hu") {
        ranges = ranges_latin2;
    } else if (lang == "ru" || lang == "uk" || lang == "be") {
        ranges = ImGui::GetIO().Fonts->GetGlyphRangesCyrillic(); // Default + about 400 Cyrillic characters
    } else if (lang == "tr") {
        ranges = ranges_turkish;
    } else if (lang == "vi") {
        ranges = ranges_vietnamese;
    } else if (lang == "ja") {
        ranges = ImGui::GetIO().Fonts->GetGlyphRangesJapanese(); // Default + Hiragana, Katakana, Half-Width, Selection of 1946 Ideographs
        m_font_cjk = true;
    } else if (lang == "ko") {
        ranges = ImGui::GetIO().Fonts->GetGlyphRangesKorean(); // Default + Korean characters
        m_font_cjk = true;
    } else if (lang == "zh") {
        //ranges = (language == "zh_TW") ?
        //    // Traditional Chinese
        //    // Default + Half-Width + Japanese Hiragana/Katakana + full set of about 21000 CJK Unified Ideographs
        //    ImGui::GetIO().Fonts->GetGlyphRangesChineseFull() :
        //    // Simplified Chinese
        //    // Default + Half-Width + Japanese Hiragana/Katakana + set of 2500 CJK Unified Ideographs for common simplified Chinese
        //    ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon();
        ranges     = GetGlyphRangesChineseSimplifiedOfficial();

        //ranges     = ImGui::GetIO().Fonts->GetGlyphRangesChineseFull();
        m_font_cjk = true;
    } else if (lang == "th") {
        ranges = ImGui::GetIO().Fonts->GetGlyphRangesThai(); // Default + Thai characters
    } else {
        //ranges = ImGui::GetIO().Fonts->GetGlyphRangesDefault(); // Basic Latin, Extended Latin

        ranges = GetGlyphRangesChineseSimplifiedOfficial(); // Basic Latin, Extended Latin
        //ranges = ImGui::GetIO().Fonts->GetGlyphRangesChineseFull();
    }

    if (ranges != m_glyph_ranges) {
        m_glyph_ranges = ranges;
        destroy_font();
    }
}
static void UnpackAccumulativeOffsetsIntoRanges(int          base_codepoint,
                                                const short *accumulative_offsets,
                                                int          accumulative_offsets_count,
                                                ImWchar *    out_ranges)
{
    for (int n = 0; n < accumulative_offsets_count; n++, out_ranges += 2) {
        out_ranges[0] = out_ranges[1] = (ImWchar) (base_codepoint + accumulative_offsets[n]);
        base_codepoint += accumulative_offsets[n];
    }
    out_ranges[0] = 0;
}
const ImWchar *ImGuiWrapper::GetGlyphRangesChineseSimplifiedOfficial()
{
    // Store all official characters for Simplified Chinese.
    // Sourced from https://en.wikipedia.org/wiki/Table_of_General_Standard_Chinese_Characters
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source
    // code size.)
    static const short accumulative_offsets_from_0x4E00[] =
        {0,  1,  2,  4,  1,  1,  1,  1,  2,  1,  2,  1,  2,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  3,  2,  1,  1,  1,  3,  1,  2,  3,
         2,  2,  4,  1,  1,  1,  2,  1,  5,  2,  3,  1,  2,  1,  1,  1,  1,  1,  2,  1,  1,  2,  2,  1,  4,  1,  1,  1,  1,  5,  3,  7,  1,
         2,  11, 4,  4,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  4,  1,  1,  3,  1,  2,  1,  2,  1,  1,  1,  1,  1,  2,  1,  1,  1,  4,  1,
         7,  5,  1,  1,  2,  1,  1,  1,  1,  3,  1,  2,  1,  3,  2,  1,  1,  1,  1,  1,  1,  5,  1,  4,  1,  1,  5,  2,  4,  2,  1,  3,  1,
         4,  2,  2,  2,  8,  1,  1,  2,  1,  1,  1,  1,  4,  2,  1,  1,  1,  4,  1,  1,  4,  2,  4,  5,  1,  4,  2,  2,  2,  2,  1,  6,  3,
         1,  1,  5,  1,  1,  1,  1,  2,  1,  1,  2,  2,  1,  1,  1,  1,  1,  2,  3,  6,  3,  3,  1,  3,  2,  5,  1,  3,  1,  4,  1,  2,  2,
         3,  2,  2,  2,  5,  1,  4,  2,  3,  3,  2,  1,  1,  1,  1,  5,  1,  6,  1,  1,  8,  3,  1,  1,  6,  4,  1,  1,  1,  6,  1,  2,  3,
         1,  1,  1,  1,  8,  4,  1,  1,  2,  2,  5,  2,  4,  2,  6,  3,  2,  1,  1,  2,  2,  1,  2,  2,  2,  1,  1,  5,  2,  2,  2,  1,  2,
         1,  1,  1,  2,  13, 2,  2,  5,  4,  2,  3,  2,  1,  6,  5,  2,  9,  8,  2,  5,  1,  1,  1,  3,  3,  2,  1,  5,  3,  5,  4,  5,  2,
         1,  1,  8,  1,  5,  4,  1,  2,  3,  1,  2,  2,  4,  3,  7,  2,  7,  1,  2,  4,  1,  4,  11, 2,  2,  4,  1,  3,  2,  2,  4,  2,  5,
         2,  2,  1,  3,  1,  7,  2,  2,  1,  3,  7,  2,  9,  8,  1,  4,  1,  3,  1,  3,  1,  1,  2,  1,  1,  1,  1,  1,  1,  2,  1,  1,  4,
         1,  2,  1,  2,  3,  2,  6,  3,  2,  1,  1,  2,  1,  1,  1,  2,  1,  2,  1,  1,  1,  1,  1,  1,  2,  1,  1,  3,  5,  3,  1,  1,  2,
         1,  4,  1,  3,  2,  2,  2,  1,  4,  2,  2,  1,  5,  2,  3,  1,  2,  1,  2,  1,  1,  4,  2,  3,  4,  2,  3,  2,  1,  1,  2,  2,  10,
         1,  1,  3,  1,  3,  9,  2,  1,  1,  2,  3,  2,  1,  1,  1,  2,  2,  1,  1,  2,  3,  1,  1,  2,  4,  3,  1,  4,  1,  1,  1,  1,  1,
         2,  3,  4,  1,  3,  1,  1,  1,  3,  2,  6,  1,  1,  1,  1,  1,  2,  4,  1,  1,  4,  3,  1,  1,  1,  1,  2,  1,  3,  2,  5,  1,  1,
         8,  2,  2,  1,  5,  3,  2,  1,  2,  6,  2,  3,  1,  4,  1,  1,  1,  3,  4,  2,  8,  2,  1,  1,  1,  1,  2,  5,  1,  1,  1,  1,  5,
         1,  1,  8,  3,  1,  2,  2,  4,  2,  2,  7,  3,  2,  1,  1,  2,  2,  1,  1,  3,  1,  1,  2,  10, 3,  2,  3,  2,  1,  3,  1,  1,  5,
         1,  2,  5,  2,  1,  5,  1,  1,  2,  4,  3,  1,  2,  7,  5,  2,  8,  1,  1,  3,  1,  1,  1,  2,  2,  2,  1,  1,  1,  4,  1,  2,  1,
         1,  1,  1,  1,  1,  3,  2,  2,  2,  1,  1,  2,  2,  1,  4,  3,  1,  1,  1,  2,  1,  1,  2,  1,  1,  2,  4,  3,  2,  1,  1,  3,  2,
         1,  9,  3,  2,  3,  2,  3,  3,  1,  2,  1,  4,  5,  9,  4,  2,  1,  1,  5,  1,  1,  1,  1,  1,  4,  3,  2,  1,  1,  1,  2,  4,  1,
         2,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  2,  1,  5,  2,  1,  2,  2,  1,  1,  1,  1,
         1,  1,  1,  1,  1,  1,  1,  2,  2,  4,  2,  1,  1,  1,  6,  1,  1,  1,  2,  1,  1,  1,  1,  2,  2,  1,  1,  1,  2,  1,  2,  1,  2,
         2,  2,  1,  3,  2,  2,  4,  2,  5,  3,  3,  1,  6,  6,  9,  2,  2,  1,  1,  1,  3,  1,  1,  3,  4,  2,  5,  1,  2,  1,  1,  2,  3,
         1,  3,  1,  12, 1,  1,  1,  1,  1,  1,  3,  2,  2,  5,  3,  2,  2,  1,  1,  1,  2,  2,  1,  1,  1,  4,  1,  3,  6,  8,  2,  4,  1,
         2,  1,  1,  3,  1,  4,  8,  2,  1,  4,  5,  1,  2,  6,  1,  4,  16, 3,  5,  2,  1,  2,  2,  4,  1,  6,  3,  2,  1,  2,  4,  5,  6,
         1,  1,  5,  2,  1,  2,  2,  1,  1,  1,  4,  4,  10, 3,  1,  2,  2,  2,  1,  1,  3,  2,  1,  1,  9,  4,  1,  1,  2,  1,  2,  8,  3,
         1,  1,  2,  3,  1,  1,  4,  4,  10, 1,  1,  2,  5,  3,  2,  7,  2,  3,  2,  2,  1,  1,  1,  2,  7,  7,  1,  6,  1,  2,  6,  2,  1,
         4,  1,  2,  4,  2,  1,  1,  3,  4,  10, 5,  1,  1,  1,  1,  2,  2,  3,  2,  4,  15, 5,  4,  2,  9,  2,  1,  3,  1,  1,  1,  1,  3,
         2,  3,  2,  2,  1,  7,  5,  2,  7,  1,  1,  5,  14, 2,  3,  6,  6,  2,  1,  5,  4,  1,  1,  6,  1,  1,  2,  5,  9,  1,  2,  1,  2,
         2,  2,  6,  1,  2,  3,  1,  3,  6,  3,  1,  1,  4,  1,  2,  2,  1,  2,  2,  5,  1,  3,  2,  7,  4,  5,  1,  3,  1,  2,  1,  3,  7,
         1,  3,  2,  7,  3,  3,  1,  1,  1,  1,  6,  3,  1,  2,  1,  1,  1,  1,  3,  2,  3,  1,  5,  8,  5,  6,  1,  1,  7,  7,  1,  8,  5,
         2,  1,  3,  5,  3,  20, 1,  8,  3,  6,  11, 1,  4,  19, 2,  1,  8,  3,  1,  3,  1,  7,  4,  8,  3,  1,  3,  6,  5,  1,  1,  3,  1,
         21, 1,  1,  5,  1,  2,  3,  1,  2,  4,  4,  9,  2,  8,  4,  4,  1,  2,  3,  2,  5,  8,  1,  6,  3,  2,  1,  7,  2,  1,  5,  5,  11,
         3,  4,  2,  7,  3,  2,  2,  3,  1,  5,  1,  3,  2,  5,  2,  1,  2,  1,  1,  3,  3,  3,  1,  3,  7,  3,  6,  2,  5,  1,  1,  3,  1,
         2,  3,  1,  2,  2,  1,  2,  2,  1,  1,  2,  1,  1,  2,  3,  3,  1,  1,  1,  4,  6,  3,  1,  1,  2,  3,  1,  1,  1,  3,  1,  1,  1,
         1,  2,  6,  2,  3,  2,  2,  1,  3,  1,  5,  1,  2,  2,  1,  4,  4,  1,  1,  1,  2,  1,  1,  2,  3,  5,  1,  3,  3,  4,  1,  5,  1,
         1,  3,  6,  1,  4,  6,  2,  3,  7,  1,  4,  1,  2,  3,  1,  1,  1,  1,  4,  2,  2,  7,  2,  1,  2,  2,  2,  15, 4,  2,  2,  1,  3,
         2,  3,  5,  2,  5,  3,  1,  3,  1,  3,  2,  4,  8,  5,  6,  4,  1,  5,  3,  1,  16, 8,  4,  4,  10, 2,  1,  8,  19, 8,  1,  12, 11,
         1,  9,  1,  1,  3,  1,  7,  3,  10, 1,  1,  9,  2,  3,  3,  12, 6,  13, 1,  2,  1,  17, 7,  6,  6,  4,  4,  8,  3,  13, 1,  2,  1,
         1,  2,  1,  1,  1,  1,  1,  1,  2,  4,  1,  2,  3,  1,  1,  5,  1,  2,  2,  3,  2,  3,  2,  2,  2,  2,  2,  1,  1,  2,  1,  3,  8,
         1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  4,  1,  3,  2,  3,  1,  1,  1,  2,  1,  4,  1,  1,  3,  2,  1,  1,  1,  5,
         4,  2,  1,  10, 1,  1,  2,  1,  2,  1,  1,  1,  1,  1,  2,  1,  2,  2,  5,  1,  2,  1,  1,  1,  1,  3,  2,  3,  2,  1,  1,  1,  1,
         1,  2,  1,  1,  2,  3,  2,  2,  2,  3,  5,  2,  3,  5,  5,  6,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  3,  1,  2,  1,  1,  1,  2,
         1,  1,  1,  1,  4,  3,  4,  1,  1,  2,  1,  1,  2,  1,  3,  4,  3,  2,  8,  6,  2,  1,  10, 4,  1,  3,  3,  4,  6,  8,  2,  2,  4,
         2,  4,  1,  7,  19, 7,  8,  5,  2,  2,  3,  3,  4,  2,  5,  2,  4,  5,  1,  7,  3,  3,  2,  1,  2,  2,  7,  2,  3,  2,  4,  7,  19,
         5,  4,  25, 6,  21, 3,  13, 9,  27, 2,  2,  1,  12, 4,  5,  2,  2,  7,  1,  3,  1,  3,  1,  1,  1,  1,  2,  3,  3,  1,  1,  1,  3,
         6,  1,  3,  1,  1,  2,  1,  2,  4,  4,  1,  4,  1,  2,  2,  1,  1,  1,  2,  6,  1,  1,  4,  2,  1,  5,  3,  1,  1,  5,  3,  2,  1,
         2,  7,  8,  1,  2,  4,  4,  2,  1,  1,  8,  7,  1,  1,  2,  2,  1,  2,  1,  1,  1,  1,  5,  2,  1,  3,  5,  1,  3,  1,  1,  1,  1,
         2,  1,  2,  2,  1,  1,  6,  1,  4,  2,  8,  1,  1,  1,  6,  3,  1,  2,  4,  1,  1,  9,  3,  4,  3,  2,  1,  2,  1,  9,  7,  3,  1,
         3,  4,  1,  1,  1,  1,  1,  1,  4,  2,  1,  4,  3,  1,  1,  1,  2,  1,  3,  4,  1,  5,  1,  1,  2,  4,  2,  2,  4,  2,  2,  1,  2,
         10, 2,  2,  2,  2,  4,  1,  2,  2,  2,  4,  5,  2,  1,  1,  3,  1,  1,  1,  1,  3,  1,  6,  2,  2,  1,  3,  1,  1,  3,  1,  2,  1,
         1,  1,  1,  1,  4,  1,  1,  5,  1,  1,  3,  2,  2,  1,  7,  1,  1,  2,  2,  7,  2,  2,  4,  6,  2,  1,  6,  1,  9,  1,  1,  1,  4,
         3,  7,  3,  1,  6,  4,  3,  3,  2,  2,  1,  1,  1,  12, 1,  3,  2,  1,  1,  5,  1,  1,  3,  1,  4,  2,  1,  1,  1,  1,  4,  6,  6,
         8,  3,  5,  2,  3,  2,  3,  4,  9,  1,  1,  1,  3,  1,  1,  1,  1,  1,  2,  1,  3,  3,  6,  3,  5,  1,  4,  3,  1,  7,  2,  4,  5,
         1,  3,  3,  2,  4,  3,  3,  2,  1,  1,  2,  3,  1,  1,  8,  1,  1,  3,  1,  6,  4,  3,  2,  2,  3,  1,  1,  5,  1,  1,  1,  2,  2,
         1,  1,  1,  1,  2,  1,  2,  3,  1,  1,  5,  1,  5,  2,  1,  4,  1,  1,  3,  3,  5,  1,  1,  3,  4,  1,  3,  13, 3,  7,  1,  5,  3,
         1,  3,  1,  1,  1,  3,  4,  3,  2,  3,  2,  3,  1,  4,  1,  3,  3,  2,  5,  1,  1,  5,  2,  4,  8,  1,  3,  2,  1,  3,  6,  10, 3,
         1,  1,  2,  1,  2,  4,  4,  8,  4,  5,  1,  1,  1,  1,  2,  5,  17, 3,  12, 3,  1,  1,  1,  2,  2,  2,  1,  1,  8,  2,  2,  1,  1,
         1,  1,  1,  1,  3,  1,  2,  2,  1,  4,  2,  1,  8,  4,  2,  2,  1,  1,  2,  1,  7,  1,  1,  1,  6,  1,  1,  2,  2,  1,  3,  1,  1,
         1,  4,  3,  8,  3,  1,  2,  2,  1,  1,  1,  1,  1,  3,  3,  3,  3,  2,  1,  1,  4,  2,  3,  1,  7,  1,  1,  2,  1,  1,  1,  2,  1,
         5,  1,  1,  2,  1,  3,  3,  1,  5,  4,  4,  3,  1,  2,  3,  2,  1,  1,  1,  1,  1,  2,  1,  1,  1,  2,  2,  1,  1,  2,  1,  1,  1,
         1,  1,  1,  3,  3,  1,  2,  1,  1,  1,  1,  3,  1,  1,  1,  2,  2,  1,  3,  5,  1,  1,  1,  2,  1,  5,  1,  1,  5,  3,  5,  4,  1,
         2,  1,  1,  1,  1,  2,  1,  1,  3,  2,  1,  4,  11, 3,  1,  4,  3,  1,  3,  3,  1,  1,  1,  1,  5,  9,  1,  2,  1,  1,  4,  1,  1,
         2,  3,  3,  1,  4,  1,  3,  1,  5,  2,  1,  1,  3,  1,  1,  3,  3,  1,  2,  4,  2,  1,  2,  5,  1,  1,  1,  2,  2,  1,  1,  1,  2,
         7,  4,  1,  2,  6,  6,  3,  4,  2,  1,  2,  4,  4,  1,  6,  2,  6,  1,  3,  1,  6,  5,  4,  3,  1,  1,  3,  1,  7,  2,  1,  3,  1,
         2,  1,  5,  2,  12, 2,  1,  7,  2,  4,  3,  1,  6,  2,  1,  1,  2,  7,  1,  2,  4,  7,  8,  2,  4,  2,  9,  1,  1,  1,  7,  3,  2,
         1,  8,  1,  1,  1,  2,  4,  1,  4,  4,  2,  1,  4,  2,  1,  1,  1,  2,  2,  1,  2,  6,  1,  2,  1,  2,  1,  2,  1,  2,  3,  1,  2,
         1,  4,  2,  4,  2,  4,  2,  2,  4,  6,  1,  3,  1,  1,  2,  3,  3,  1,  3,  2,  6,  9,  3,  2,  4,  1,  3,  1,  6,  1,  6,  1,  1,
         3,  7,  2,  1,  2,  3,  1,  6,  3,  1,  3,  3,  2,  3,  1,  1,  1,  1,  1,  2,  2,  1,  4,  1,  3,  6,  4,  2,  2,  1,  2,  1,  10,
         1,  4,  4,  1,  4,  1,  4,  2,  2,  2,  1,  3,  2,  3,  1,  2,  2,  2,  1,  1,  2,  1,  7,  2,  3,  1,  4,  4,  6,  1,  2,  1,  7,
         1,  9,  2,  3,  1,  1,  1,  1,  3,  1,  4,  5,  1,  3,  6,  2,  4,  1,  3,  2,  2,  1,  4,  1,  1,  10, 1,  4,  1,  2,  1,  5,  2,
         2,  11, 2,  3,  1,  1,  2,  4,  2,  1,  3,  3,  1,  2,  5,  3,  1,  6,  2,  2,  6,  1,  1,  4,  2,  1,  3,  4,  4,  2,  3,  1,  1,
         4,  4,  5,  1,  1,  5,  5,  6,  3,  3,  3,  3,  4,  1,  11, 2,  1,  2,  13, 3,  3,  1,  3,  6,  3,  3,  4,  2,  1,  1,  3,  1,  1,
         3,  2,  1,  1,  3,  5,  1,  2,  2,  7,  1,  2,  4,  2,  2,  7,  1,  1,  2,  1,  1,  1,  1,  1,  2,  3,  1,  5,  3,  3,  2,  1,  3,
         3,  5,  1,  1,  1,  2,  3,  1,  5,  2,  1,  1,  1,  4,  3,  2,  3,  2,  1,  1,  1,  1,  2,  2,  5,  2,  1,  2,  3,  3,  2,  2,  5,
         3,  2,  2,  3,  2,  1,  5,  1,  7,  1,  4,  1,  4,  2,  1,  1,  12, 11, 1,  1,  1,  1,  1,  5,  1,  2,  1,  1,  2,  2,  3,  4,  3,
         3,  2,  2,  1,  1,  10, 6,  2,  1,  3,  4,  2,  2,  3,  1,  1,  9,  8,  1,  13, 1,  1,  3,  1,  3,  1,  1,  1,  2,  1,  2,  4,  4,
         1,  2,  1,  15, 2,  3,  1,  13, 9,  2,  1,  1,  2,  1,  13, 1,  3,  6,  2,  2,  1,  4,  1,  1,  5,  2,  1,  1,  2,  2,  11, 4,  5,
         2,  2,  5,  3,  2,  1,  2,  5,  1,  3,  4,  7,  3,  1,  3,  4,  1,  1,  10, 8,  1,  4,  27, 3,  13, 2,  11, 9,  1,  6,  1,  2,  1,
         2,  3,  5,  3,  2,  1,  7,  2,  2,  3,  6,  4,  14, 1,  6,  1,  7,  3,  2,  3,  2,  1,  6,  3,  4,  6,  2,  8,  1,  1,  3,  3,  8,
         13, 5,  6,  10, 4,  1,  3,  5,  7,  2,  6,  5,  1,  2,  2,  7,  7,  4,  3,  1,  1,  3,  7,  3,  1,  3,  13, 1,  6,  2,  1,  9,  6,
         13, 2,  4,  12, 4,  3,  5,  4,  2,  1,  9,  2,  1,  4,  5,  3,  7,  1,  16, 7,  1,  2,  8,  7,  14, 1,  9,  6,  6,  6,  10, 1,  1,
         1,  4,  11, 8,  3,  1,  9,  2,  3,  2,  2,  9,  3,  3,  2,  1,  1,  1,  1,  1,  1,  3,  8,  5,  1,  1,  2,  1,  3,  4,  3,  3,  1,
         1,  11, 2,  12, 10, 1,  3,  2,  1,  2,  3,  2,  3,  1,  1,  4,  1,  5,  2,  2,  3,  1,  1,  1,  1,  1,  2,  1,  1,  6,  10, 1,  3,
         14, 8,  3,  7,  1,  1,  2,  1,  2,  5,  4,  3,  1,  1,  2,  1,  1,  3,  1,  2,  1,  1,  3,  1,  4,  6,  2,  1,  1,  5,  2,  4,  1,
         2,  5,  2,  2,  2,  2,  1,  1,  1,  1,  3,  4,  2,  6,  2,  2,  2,  3,  1,  3,  1,  3,  1,  1,  2,  1,  2,  1,  3,  1,  2,  1,  2,
         2,  2,  2,  1,  2,  4,  2,  3,  1,  1,  1,  3,  1,  1,  2,  3,  2,  5,  1,  2,  1,  1,  1,  1,  2,  3,  1,  4,  1,  2,  7,  2,  1,
         1,  4,  1,  2,  2,  1,  1,  1,  2,  3,  2,  5,  1,  1,  2,  2,  6,  1,  1,  4,  10, 1,  2,  4,  5,  4,  3,  7,  2,  2,  1,  1,  3,
         3,  1,  3,  1,  2,  1,  2,  1,  1,  1,  3,  4,  1,  1,  3,  1,  3,  3,  2,  6,  1,  11, 1,  3,  1,  2,  2,  6,  3,  1,  1,  9,  3,
         2,  1,  1,  1,  2,  2,  5,  1,  1,  6,  2,  2,  2,  2,  1,  2,  1,  1,  1,  1,  4,  1,  3,  3,  3,  4,  3,  1,  4,  1,  1,  1,  4,
         1,  5,  1,  4,  2,  1,  1,  4,  3,  3,  2,  2,  2,  1,  1,  2,  3,  2,  2,  2,  2,  1,  1,  10, 5,  3,  3,  4,  3,  2,  1,  1,  2,
         3,  1,  2,  1,  1,  1,  3,  3,  1,  2,  3,  2,  4,  2,  4,  5,  1,  6,  3,  1,  6,  2,  2,  3,  4,  4,  4,  2,  5,  1,  3,  12, 1,
         4,  2,  4,  7,  6,  2,  4,  1,  5,  3,  2,  3,  1,  4,  5,  2,  4,  2,  5,  1,  2,  1,  2,  4,  1,  5,  2,  1,  1,  5,  4,  3,  3,
         1,  1,  2,  1,  3,  3,  3,  2,  3,  7,  1,  2,  1,  4,  9,  4,  1,  1,  11, 2,  1,  6,  1,  1,  1,  3,  2,  1,  1,  5,  6,  1,  14,
         4,  3,  4,  3,  1,  2,  1,  1,  4,  2,  7,  1,  1,  1,  8,  2,  2,  4,  4,  2,  2,  6,  9,  5,  4,  1,  3,  2,  1,  10, 2,  1,  2,
         3,  7,  1,  1,  1,  15, 3,  3,  3,  4,  1,  1,  3,  5,  2,  1,  2,  1,  1,  1,  9,  4,  8,  3,  2,  4,  2,  4,  3,  2,  1,  2,  2,
         9,  8,  2,  12, 14, 5,  7,  6,  5,  1,  7,  2,  2,  1,  5,  1,  2,  4,  1,  1,  1,  10, 1,  4,  4,  2,  1,  4,  11, 7,  1,  1,  1,
         1,  4,  5,  1,  1,  2,  1,  4,  1,  1,  5,  2,  5,  9,  1,  2,  4,  5,  2,  1,  2,  2,  1,  1,  2,  8,  2,  4,  12, 1,  11, 4,  1,
         2,  5,  5,  10, 6,  16, 3,  3,  2,  8,  3,  3,  2,  6,  1,  1,  1,  1,  1,  3,  2,  15, 7,  6,  5,  3,  2,  5,  6,  9,  4,  5,  8,
         5,  5,  1,  4,  1,  5,  2,  2,  3,  4,  3,  3,  1,  5,  1,  1,  5,  1,  1,  6,  4,  3,  1,  2,  8,  7,  3,  11, 13, 2,  2,  1,  3,
         1,  4,  1,  1,  1,  1,  1,  1,  2,  1,  8,  1,  1,  4,  6,  4,  2,  1,  2,  2,  2,  1,  1,  1,  5,  2,  6,  3,  2,  1,  4,  1,  3,
         3,  1,  3,  6,  8,  4,  5,  7,  5,  5,  3,  7,  3,  7,  2,  2,  4,  6,  2,  7,  2,  5,  2,  1,  8,  3,  1,  1,  2,  1,  6,  1,  1,
         3,  1,  17, 5,  3,  3,  2,  1,  9,  4,  1,  1,  3,  6,  2,  1,  7,  1,  5,  1,  9,  2,  7,  17, 5,  3,  2,  3,  2,  1,  2,  1,  5,
         4,  2,  1,  2,  2,  11, 5,  4,  6,  3,  1,  2,  4,  1,  2,  1,  4,  4,  5,  10, 3,  3,  14, 2,  10, 3,  1,  16, 5,  2,  1,  3,  1,
         3,  3,  11, 7,  3,  1,  2,  2,  5,  4,  1,  1,  1,  4,  2,  3,  2,  20, 4,  1,  1,  1,  2,  1,  4,  1,  5,  1,  3,  2,  1,  6,  8,
         2,  3,  8,  13, 4,  2,  1,  4,  3,  1,  1,  1,  7,  13, 2,  11, 5,  5,  2,  4,  2,  4,  2,  1,  1,  2,  8,  8,  1,  13, 8,  1,  8,
         3,  2,  2,  3,  3,  3,  1,  2,  1,  2,  1,  1,  3,  1,  1,  1,  2,  2,  1,  3,  2,  8,  6,  1,  2,  1,  5,  5,  1,  2,  1,  5,  3,
         1,  4,  1,  5,  4,  2,  2,  7,  7,  1,  3,  1,  4,  2,  6,  2,  1,  3,  2,  2,  1,  1,  1,  1,  6,  3,  1,  3,  1,  2,  3,  1,  3,
         1,  1,  4,  3,  2,  1,  1,  1,  3,  5,  2,  1,  3,  1,  2,  3,  2,  2,  1,  7,  6,  1,  1,  1,  1,  5,  1,  5,  1,  1,  8,  2,  2,
         6,  12, 1,  1,  1,  2,  2,  1,  2,  1,  1,  9,  4,  2,  8,  4,  6,  6,  4,  2,  7,  2,  1,  1,  2,  4,  3,  2,  3,  1,  6,  3,  1,
         1,  1,  4,  2,  2,  1,  1,  3,  3,  3,  2,  3,  4,  20, 2,  1,  3,  6,  5,  3,  1,  2,  1,  2,  2,  3,  1,  1,  1,  1,  1,  2,  2,
         1,  1,  3,  1,  2,  1,  2,  2,  1,  1,  2,  3,  4,  1,  3,  2,  4,  1,  3,  2,  3,  6,  2,  2,  20, 1,  8,  1,  6,  7,  1,  1,  2,
         2,  2,  2,  1,  14, 12, 1,  2,  3,  1,  1,  2,  1,  2,  2,  1,  1,  13, 4,  2,  5,  1,  2,  4,  1,  2,  3,  1,  12, 1,  1,  5,  2,
         2,  1,  2,  1,  3,  3,  8,  1,  1,  2,  8,  12, 4,  3,  1,  1,  6,  1,  2,  4,  2,  2,  1,  1,  4,  10, 1,  1,  2,  1,  10, 1,  5,
         2,  2,  4,  3,  2,  1,  5,  2,  6,  3,  2,  2,  3,  1,  6,  2,  3,  2,  9,  4,  3,  8,  14, 1,  2,  1,  8,  5,  3,  10, 3,  1,  1,
         3,  2,  4,  1,  7,  4,  4,  2,  7,  1,  2,  1,  6,  2,  2,  3,  4,  5,  1,  5,  5,  5,  13, 1,  4,  3,  5,  3,  3,  7,  13, 11, 13,
         4,  1,  1,  14, 2,  2,  2,  1,  5,  1,  1,  1,  7,  5,  1,  1,  3,  3,  1,  1,  1,  5,  3,  5,  10, 1,  2,  5,  2,  3,  9,  1,  1,
         23, 1,  3,  4,  3,  2,  6,  1,  1,  2,  1,  1,  1,  1,  2,  3,  2,  1,  2,  2,  4,  1,  6,  7,  4,  1,  2,  1,  2,  6,  1,  8,  3,
         2,  3,  5,  3,  3,  2,  1,  6,  3,  3,  2,  2,  1,  1,  14, 3,  2,  4,  4,  1,  1,  2,  1,  6,  5,  1,  1,  1,  9,  9,  1,  1,  7,
         2,  9,  2,  1,  2,  1,  5,  2,  2,  1,  4,  5,  4,  1,  3,  1,  1,  1,  4,  1,  2,  1,  5,  4,  1,  4,  1,  11, 5,  5,  1,  2,  3,
         1,  11, 3,  5,  1,  1,  3,  2,  8,  2,  7,  3,  1,  3,  2,  7,  5,  6,  3,  5,  2,  3,  2,  6,  2,  4,  2,  12, 2,  10, 3,  2,  2,
         2,  1,  1,  1,  2,  2,  5,  5,  7,  7,  7,  2,  2,  5,  2,  4,  3,  4,  2,  2,  5,  1,  2,  6,  3,  1,  8,  4,  4,  3,  4,  8,  1,
         3,  2,  1,  2,  3,  7,  6,  2,  5,  1,  3,  2,  3,  5,  3,  6,  7,  9,  5,  1,  4,  13, 3,  2,  4,  3,  1,  12, 1,  5,  1,  1,  1,
         3,  9,  1,  3,  15, 1,  3,  1,  1,  7,  2,  4,  1,  8,  2,  12, 9,  3,  2,  1,  7,  5,  1,  5,  4,  3,  2,  2,  4,  4,  1,  12, 11,
         1,  3,  3,  1,  1,  1,  1,  2,  5,  8,  1,  8,  3,  3,  2,  2,  2,  1,  1,  1,  2,  1,  1,  2,  3,  4,  1,  2,  2,  1,  1,  1,  4,
         1,  1,  5,  4,  3,  1,  1,  1,  1,  1,  6,  3,  4,  2,  1,  1,  2,  10, 5,  6,  3,  3,  1,  4,  2,  6,  2,  1,  1,  1,  6,  8,  14,
         2,  2,  9,  2,  2,  2,  10, 1,  3,  1,  1,  2,  1,  3,  2,  1,  2,  1,  1,  8,  3,  8,  6,  1,  1,  3,  1,  2,  3,  5,  1,  3,  3,
         3,  2,  5,  2,  13, 1,  1,  6,  1,  10, 2,  1,  5,  8,  3,  1,  2,  1,  2,  1,  2,  2,  4,  1,  1,  2,  2,  2,  1,  2,  10, 2,  1,
         4,  1,  4,  3,  8,  1,  1,  2,  6,  6,  3,  1,  5,  10, 3,  4,  1,  2,  6,  4,  2,  2,  2,  1,  3,  1,  1,  3,  2,  1,  1,  2,  1,
         1,  1,  1,  1,  3,  2,  4,  1,  1,  1,  1,  1,  2,  3,  1,  1,  2,  1,  1,  2,  3,  1,  1,  2,  1,  1,  1,  3,  1,  2,  1,  1,  2,
         1,  3,  3,  3,  1,  1,  2,  1,  4,  1,  2,  1,  4,  1,  1,  3,  5,  5,  1,  1,  1,  2,  2,  3,  3,  8,  1,  5,  6,  2,  2,  2,  3,
         7,  4,  4,  4,  1,  3,  1,  2,  3,  5,  3,  7,  1,  4,  2,  2,  3,  2,  2,  12, 3,  2,  1,  1,  2,  6,  6,  4,  3,  3,  4,  1,  6,
         3,  4,  2,  1,  3,  1,  2,  2,  4,  7,  2,  4,  2,  1,  3,  7,  1,  1,  8,  5,  2,  1,  2,  6,  2,  1,  4,  1,  2,  1,  1,  1,  5,
         1,  1,  3,  1,  1,  3,  2,  1,  1,  3,  10, 2,  2,  9,  3,  2,  3,  2,  2,  3,  4,  4,  1,  1,  6,  2,  2,  2,  2,  4,  12, 5,  1,
         1,  1,  1,  3,  1,  6,  1,  1,  4,  1,  1,  1,  1,  2,  2,  1,  7,  3,  2,  2,  1,  2,  3,  1,  1,  2,  1,  4,  1,  1,  3,  1,  2,
         1,  2,  6,  4,  1,  1,  1,  2,  1,  2,  6,  2,  8,  4,  1,  1,  1,  1,  3,  3,  1,  8,  2,  1,  1,  2,  1,  2,  2,  2,  2,  1,  5,
         3,  1,  2,  1,  1,  5,  1,  1,  2,  3,  4,  9,  1,  5,  2,  4,  1,  1,  7,  1,  5,  8,  4,  1,  1,  2,  2,  1,  2,  1,  1,  16, 1,
         5,  3,  2,  1,  2,  1,  4,  1,  1,  1,  3,  1,  1,  2,  6,  2,  5,  9,  2,  1,  1,  1,  3,  8,  1,  10, 7,  3,  1,  1,  1,  2,  1,
         3,  3,  4,  2,  9,  2,  5,  3,  1,  2,  2,  1,  2,  2,  1,  6,  1,  6,  1,  1,  2,  2,  2,  3,  1,  1,  1,  1,  1,  2,  1,  3,  2,
         2,  7,  1,  2,  1,  3,  6,  2,  1,  1,  4,  1,  4,  1,  1,  2,  14, 4,  13, 1,  13, 8,  1,  1,  1,  1,  1,  4,  3,  4,  4,  9,  2,
         5,  7,  3,  1,  2,  4,  2,  1,  6,  1,  2,  1,  1,  2,  4,  1,  1,  2,  2,  1,  9,  6,  2,  2,  1,  1,  1,  3,  4,  1,  2,  1,  4,
         2,  1,  1,  4,  4,  2,  8,  8,  1,  1,  1,  1,  1,  3,  5,  9,  16, 1,  3,  1,  5,  1,  2,  1,  1,  2,  1,  7,  3,  3,  3,  1,  5,
         1,  2,  2,  6,  7,  2,  1,  3,  4,  4,  8,  1,  3,  1,  10, 3,  1,  4,  4,  1,  3,  2,  6,  2,  2,  5,  4,  6,  1,  5,  2,  4,  4,
         4,  2,  3,  2,  3,  5,  8,  3,  1,  9,  6,  1,  1,  1,  2,  4,  5,  6,  1,  3,  3,  1,  1,  1,  1,  15, 7,  1,  2,  1,  4,  5,  4,
         3,  4,  1,  9,  3,  14, 4,  6,  2,  3,  3,  2,  1,  1,  4,  4,  2,  11, 9,  5,  9,  3,  2,  20, 6,  4,  3,  1,  4,  2,  3,  1,  1,
         6,  2,  3,  7,  2,  13, 2,  17, 7,  11, 2,  1,  3,  3,  7,  2,  2,  6,  1,  1,  10, 5,  1,  6,  5,  1,  1,  15, 5,  10, 1,  1,  3,
         7,  10, 6,  3,  3,  7,  3,  7,  1,  10, 2,  1,  1,  5,  1,  9,  1,  15, 4,  2,  1,  5,  8,  7,  1,  4,  1,  13, 2,  1,  1,  11, 1,
         3,  1,  6,  6,  3,  19, 11, 7,  15, 1,  1,  1,  3,  1,  5,  1,  1,  2,  1,  8,  4,  6,  8,  1,  1,  2,  1,  2,  1,  1,  8,  2,  7,
         2,  7,  7,  1,  5,  1,  5,  2,  4,  1,  10, 4,  2,  1,  2,  1,  1,  5,  4,  5,  2,  3,  1,  5,  9,  1,  2,  9,  2,  3,  2,  2,  1,
         3,  1,  1,  2,  8,  1,  2,  2,  1,  4,  5,  1,  3,  4,  11, 3,  4,  4,  8,  2,  3,  2,  5,  2,  2,  2,  7,  6,  2,  12, 2,  2,  10,
         1,  10, 17, 1,  3,  9,  8,  7,  1,  5,  6,  2,  3,  6,  1,  14, 12, 3,  3,  7,  2,  3,  7,  8,  7,  1,  10, 2,  1,  1,  14, 5,  3,
         2,  5,  5,  7,  1,  6,  1,  3,  2,  2,  2,  4,  1,  1,  2,  3,  3,  1,  4,  7,  2,  4,  4,  1,  2,  4,  3,  2,  5,  4,  6,  2,  13,
         2,  4,  16, 1,  3,  5,  8,  2,  1,  3,  1,  3,  1,  2,  2,  3,  4,  7,  4,  1,  4,  1,  4,  5,  5,  3,  6,  2,  1,  2,  5,  7,  4,
         5,  1,  7,  4,  2,  3,  2,  7,  18, 8,  1,  1,  4,  6,  2,  3,  3,  13, 2,  2,  3,  5,  4,  4,  3,  6,  10, 4,  2,  6,  8,  3,  3,
         1,  1,  2,  1,  1,  2,  1,  9,  2,  15, 3,  14, 4,  4,  4,  2,  1,  5,  2,  2,  2,  1,  1,  1,  2,  2,  1,  1,  1,  3,  2,  2,  3,
         1,  2,  5,  3,  4,  3,  2,  4,  1,  1,  2,  3,  5,  10, 4,  1,  1,  5,  3,  2,  2,  1,  3,  2,  1,  2,  1,  1,  1,  2,  1,  1,  1,
         6,  2,  1,  9,  2,  2,  1,  2,  1,  4,  2,  2,  3,  2,  4,  2,  1,  2,  1,  1,  1,  1,  1,  2,  6,  2,  4,  3,  3,  3,  3,  2,  3,
         2,  2,  2,  5,  4,  5,  1,  10, 1,  4,  2,  1,  3,  2,  3,  1,  1,  2,  2,  2,  2,  1,  1,  2,  6,  4,  2,  7,  2,  2,  2,  3,  8,
         3,  1,  13, 8,  5,  3,  5,  1,  9,  1,  1,  2,  3,  7,  5,  1,  3,  1,  4,  3,  6,  3,  10, 8,  1,  3,  4,  2,  5,  1,  1,  1,  1,
         3,  1,  2,  2,  1,  1,  1,  2,  1,  3,  2,  1,  1,  2,  1,  1,  1,  1,  1,  3,  1,  1,  2,  1,  1,  2,  1,  4,  4,  2,  3,  1,  2,
         1,  3,  1,  1,  2,  1,  1,  4,  1,  1,  2,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  3,  2,  2,  1,  2,  2,  1,  2,  1,  2,  3,  4,
         3,  2,  6,  1,  1,  2,  2,  1,  3,  1,  3,  1,  3,  3,  8,  2,  5,  2,  1,  3,  2,  2,  4,  1,  5,  7,  1,  8,  1,  1,  1,  6,  6,
         1,  8,  7,  1,  2,  1,  6,  10, 1,  2,  1,  1,  5,  1,  1,  1,  1,  1,  3,  3,  1,  1,  2,  1,  1,  3,  1,  1,  2,  1,  1,  1,  1,
         1,  1,  2,  1,  2,  7,  1,  1,  9,  2,  2,  1,  1,  1,  1,  2,  4,  2,  1,  7,  4,  2,  1,  1,  1,  8,  3,  2,  3,  2,  4,  3,  1,
         4,  2,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  1,  2,  1,  1,  1,  2,  2,  1,  2,  1,  1,
         1,  6,  1,  1,  2,  1,  1,  4,  2,  2,  2,  1,  3,  2,  1,  1,  1,  1,  2,  1,  4,  2,  3,  3,  1,  1,  1,  1,  10, 4,  5,  1,  10,
         4,  6,  4,  5,  11, 1,  6,  4,  5,  3,  5,  1,  2,  3,  9,  1,  2,  2,  2,  3,  2,  2,  2,  5,  4,  2,  5,  2,  14, 2,  3,  1,  13,
         1,  1,  2,  1,  7,  1,  1,  6,  3,  1,  4,  9,  2,  4,  1,  1,  1,  10, 4,  6,  4,  1,  2,  6,  1,  5,  2,  2,  4,  2,  3,  2,  1,
         3,  2,  2,  1,  1,  2,  4,  2,  14, 9,  1,  1,  3,  3,  8,  6,  10, 1,  1,  1,  2,  3,  2,  7,  4,  25, 3,  1,  2,  5,  6,  1,  8,
         1,  1,  1,  6,  1,  1,  1,  1,  6,  1,  3,  1,  5,  4,  5,  3,  3,  1,  3,  2,  1,  4,  4,  1,  6,  11, 2,  5,  1,  1,  1,  3,  2,
         1,  1,  4,  7,  2,  2,  4,  2,  1,  2,  1,  1,  6,  1,  2,  2,  1,  1,  2,  2,  1,  2,  1,  4,  3,  2,  1,  2,  2,  1,  1,  1,  1,
         1,  7,  4,  1,  3,  1,  2,  1,  1,  1,  1,  6,  2,  1,  3,  4,  1,  2,  1,  1,  1,  1,  3,  2,  1,  1,  2,  2,  1,  2,  3,  4,  3,
         1,  1,  1,  3,  3,  1,  1,  1,  2,  1,  3,  1,  3,  2,  1,  2,  2,  1,  1,  1,  2,  1,  1,  2,  1,  1,  1,  2,  1,  11, 3,  1,  3,
         1,  2,  3,  1,  2,  3,  1,  5,  3,  1,  2,  1,  1,  1,  1,  2,  1,  1,  2,  1,  1,  2,  1,  1,  1,  2,  2,  1,  2,  3,  1,  4,  1,
         4,  2,  1,  2,  1,  2,  2,  1,  2,  1,  1,  1,  1,  1,  3,  1,  2,  1,  4,  1,  1,  2,  2,  13, 1,  3,  4,  4,  1,  2,  4,  1,  2,
         1,  2,  6,  2,  7,  4,  3,  12, 4,  1,  5,  1,  5,  7,  1,  3,  5,  1,  4,  7,  9,  5,  1,  1,  5,  9,  6,  3,  1,  3,  5,  15, 1,
         1,  1,  1,  1,  5,  5,  3,  3,  1,  5,  2,  4,  1,  2,  2,  3,  3,  1,  1,  1,  1,  7,  4,  4,  2,  2,  7,  8,  1,  1,  2,  5,  8,
         1,  2,  3,  3,  6,  6,  1,  4,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  6,  1,  1,  3,  1,  6,  3,  1,  2,  4,  8,  18, 2,  4,
         3,  1,  3,  1,  9,  5,  17, 3,  1,  1,  4,  1,  1,  1,  5,  4,  2,  2,  4,  6,  3,  1,  9,  16, 4,  3,  1,  4,  5,  12, 2,  3,  2,
         4,  12, 4,  6,  1,  4,  4,  1,  5,  4,  2,  11, 18, 6,  6,  8,  4,  8,  12, 5,  3,  1,  1,  6,  1,  1,  4,  2,  3,  3,  1,  4,  3,
         20, 1,  2,  7,  6,  1,  10, 7,  4,  5,  9,  4,  8,  10, 2,  1,  1,  2,  8,  10, 2,  2,  3,  16, 5,  5,  4,  5,  1,  1,  4,  1,  2,
         2,  1,  2,  4,  21, 6,  21, 19, 3,  4,  1,  2,  3,  2,  10, 13, 1,  14, 3,  1,  6,  6,  2,  5,  5,  1,  1,  1,  9,  1,  5,  4,  1,
         6,  4,  1,  1,  1,  1,  1,  3,  1,  1,  1,  1,  5,  3,  2,  2,  6,  3,  2,  1,  1,  1,  2,  1,  22, 1,  4,  2,  9,  1,  3,  1,  1,
         4,  2,  2,  4,  2,  1,  3,  1,  3,  3,  1,  4,  1,  6,  2,  1,  7,  2,  1,  1,  1,  2,  5,  1,  1,  1,  9,  2,  5,  1,  6,  3,  3,
         3,  1,  11, 7,  10, 3,  6,  1,  8,  1,  2,  1,  6,  2,  3,  1,  2,  1,  1,  1,  4,  9,  2,  1,  1,  3,  1,  4,  4,  10, 11, 2,  2,
         1,  1,  4,  4,  1,  1,  6,  5,  1,  6,  2,  3,  1,  1,  1,  2,  2,  3,  1,  1,  1,  1,  2,  3,  1,  1,  3,  4,  5,  2,  1,  3,  8,
         3,  2,  9,  2,  2,  3,  1,  1,  1,  1,  1,  5,  4,  1,  2,  2,  1,  1,  1,  1,  3,  1,  1,  3,  5,  3,  3,  2,  1,  3,  1,  1,  1,
         4,  1,  1,  2,  4,  2,  1,  1,  1,  1,  1,  1,  5,  7,  1,  1,  4,  1,  2,  1,  1,  1,  4,  1,  3,  1,  2,  1,  11, 1,  2,  7,  3,
         2,  2,  1,  2,  3,  2,  1,  3,  1,  1,  1,  2,  1,  1,  1,  1,  2,  3,  3,  1,  3,  2,  1,  2,  4,  1,  5,  1,  1,  5,  3,  4,  1,
         2,  2,  3,  2,  4,  2,  1,  2,  4,  1,  1,  2,  2,  3,  6,  2,  6,  3,  3,  6,  2,  6,  1,  4,  1,  4,  2,  4,  10, 4,  3,  2,  1,
         2,  2,  1,  1,  4,  2,  1,  10, 1,  2,  6,  9,  6,  2,  2,  9,  8,  3,  3,  7,  20, 2,  3,  3,  2,  1,  7,  9,  10, 1,  2,  4,  6,
         2,  1,  3,  2,  4,  2,  1,  1,  2,  1,  2,  2,  2,  4,  1,  1,  1,  4,  1,  4,  5,  3,  7,  2,  3,  7,  1,  1,  2,  11, 6,  1,  1,
         1,  4,  5,  3,  3,  7,  2,  4,  4,  1,  1,  4,  1,  1,  2,  1,  1,  2,  1,  1,  2,  1,  2,  1,  1,  1,  1,  2,  6,  1,  2,  3,  1,
         1,  4,  1,  3,  4,  3,  1,  7,  2,  2,  3,  4,  2,  8,  10, 10, 3,  3,  1,  3,  5,  2,  1,  3,  2,  3,  2,  8,  1,  1,  5,  2,  4,
         1,  2,  4,  2,  1,  5,  2,  1,  3,  3,  7,  6,  7,  8,  1,  1,  6,  4,  8,  1,  3,  2,  5,  2,  7,  8,  1,  1,  1,  1,  1,  3,  1,
         4,  2,  3,  2,  4,  1,  2,  3,  4,  3,  4,  3,  12, 3,  4,  1,  1,  3,  2,  1,  13, 15, 1,  1,  3,  4,  2,  2,  2,  3,  11, 4,  14,
         2,  13, 8,  3,  18, 5,  5,  2,  7,  2,  3,  2,  8,  8,  2,  3,  1,  4,  3,  3,  5,  2,  1,  1,  1,  1,  1,  2,  1,  2,  3,  2,  1,
         2,  4,  2,  2,  1,  4,  1,  3,  2,  6,  3,  7,  3,  4,  7,  1,  7,  1,  4,  5,  14, 2,  1,  1,  1,  2,  2,  9,  5,  7,  1,  1,  12,
         8,  3,  7,  13, 8,  1,  2,  2,  1,  9,  1,  10, 5,  1,  2,  1,  8,  1,  1,  2,  3,  4,  2,  21, 8,  26, 25, 1,  2,  1,  18, 8,  1,
         1,  12, 5,  12, 1,  21, 16, 6,  18, 8,  4,  10, 3,  11, 11, 2,  1,  8,  24, 1,  4,  5,  12, 15, 8,  20, 11, 19, 8,  7,  4,  10, 1,
         2,  13, 3,  3,  9,  3,  6,  8,  3,  2,  19, 4,  18, 12, 3,  1,  9,  2,  6,  23, 38, 5,  4,  10, 17, 4,  14, 20, 1,  32, 1,  3,  1,
         2,  2,  2,  1,  7,  2,  4,  2,  8,  2,  4,  2,  5,  5,  21, 12, 5,  4,  3,  1,  2,  3,  9,  7,  1,  4,  6,  2,  6,  1,  3,  11, 1,
         6,  1,  2,  5,  1,  8,  1,  1,  1,  2,  3,  4,  3,  1,  1,  1,  2,  8,  7,  5,  3,  16, 7,  5,  2,  4,  7,  12, 3,  5,  7,  3,  12,
         1,  5,  2,  10, 3,  2,  2,  3,  1,  3,  2,  6,  5,  8,  5,  7,  2,  3,  4,  5,  1,  2,  3};
    static ImWchar base_ranges[] = // not zero-terminated
        {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x2000, 0x206F, // General Punctuation
            0x3000, 0x30FF, // Punctuations, Hiragana, Katakana
            0x31F0, 0x31FF, // Katakana Phonetic Extensions
            0xFF00, 0xFFEF, // Half-width characters
        };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00) * 2 + 1] = {0};
    if (!full_ranges[0]) {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00),
                                            full_ranges + IM_ARRAYSIZE(base_ranges));
    }

    return &full_ranges[0];
}
void ImGuiWrapper::set_display_size(float w, float h)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(w, h);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
}

void ImGuiWrapper::set_scaling(float font_size, float scale_style, float scale_both)
{
    font_size *= scale_both;
    scale_style *= scale_both;

    if (m_font_size == font_size && m_style_scaling == scale_style) {
        return;
    }

    m_font_size = font_size;

    ImGui::GetStyle().ScaleAllSizes(scale_style / m_style_scaling);
    m_style_scaling = scale_style;

    destroy_font();
}

bool ImGuiWrapper::update_mouse_data(wxMouseEvent& evt)
{
    if (! display_initialized()) {
        return false;
    }

    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2((float)evt.GetX(), (float)evt.GetY());
    io.MouseDown[0] = evt.LeftIsDown();
    io.MouseDown[1] = evt.RightIsDown();
    io.MouseDown[2] = evt.MiddleIsDown();
    io.MouseDoubleClicked[0] = evt.LeftDClick();
    io.MouseDoubleClicked[1] = evt.RightDClick();
    io.MouseDoubleClicked[2] = evt.MiddleDClick();
    float wheel_delta = static_cast<float>(evt.GetWheelDelta());
    if (wheel_delta != 0.0f)
        io.MouseWheel = static_cast<float>(evt.GetWheelRotation()) / wheel_delta;

    unsigned buttons = (evt.LeftIsDown() ? 1 : 0) | (evt.RightIsDown() ? 2 : 0) | (evt.MiddleIsDown() ? 4 : 0);
    m_mouse_buttons = buttons;

    if (want_mouse())
        new_frame();
    return want_mouse();
}

void ImGuiWrapper::releaseCtrlKey()
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.KeyCtrl) {
        io.KeyCtrl = false;
    }
}

void ImGuiWrapper::SetKeyEvent(int key,bool index)
{
    ImGuiIO &io = ImGui::GetIO();
    io.KeysDown[key] = index;
    
}
bool ImGuiWrapper::update_key_data(wxKeyEvent &evt)
{
    if (! display_initialized()) {
        return false;
    }

    auto to_string = [](wxEventType type) -> std::string {
        if (type == wxEVT_CHAR) return "Char";
        if (type == wxEVT_KEY_DOWN) return "KeyDown";
        if (type == wxEVT_KEY_UP) return "KeyUp";
        return "Other";
    };

    wxEventType type = evt.GetEventType();
    ImGuiIO& io = ImGui::GetIO();
    BOOST_LOG_TRIVIAL(debug) << "ImGui - key event(" << to_string(type) << "):"
                             //<< " Unicode(" << evt.GetUnicodeKey() << ")"
                             << " KeyCode(" << evt.GetKeyCode() << ")";

    if (type == wxEVT_CHAR) {
        // Char event
        const auto   key   = evt.GetUnicodeKey();

        // Release BackSpace, Delete, ... when miss wxEVT_KEY_UP event
        // Already Fixed at begining of new frame
        // unsigned int key_u = static_cast<unsigned int>(key);
        //if (key_u >= 0 && key_u < IM_ARRAYSIZE(io.KeysDown) && io.KeysDown[key_u]) {
        //    io.KeysDown[key_u] = false;
        //}

        if (key != 0) {
            io.AddInputCharacter(key);
        }
    } else if (type == wxEVT_KEY_DOWN || type == wxEVT_KEY_UP) {
        // Key up/down event
        int key = evt.GetKeyCode();
        wxCHECK_MSG(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown), false, "Received invalid key code");

        io.KeysDown[key] = (type == wxEVT_KEY_DOWN);
        io.KeyShift = evt.ShiftDown();
        io.KeyCtrl = evt.ControlDown();
        io.KeyAlt = evt.AltDown();
        io.KeySuper = evt.MetaDown();
    }
    bool ret = want_keyboard() || want_text_input();
    if (ret)
        new_frame();
    return ret;
}

#include <array>
void ImGuiWrapper::new_frame()
{
    if (m_new_frame_open) {
        return;
    }

    if (m_font_texture == 0) {
        init_font(true);
    }

    ImGuiIO& io = ImGui::GetIO();
    // synchronize key states
    // when the application loses the focus it may happen that the key up event is not processed

    // synchronize modifier keys
    constexpr std::array<std::pair<ImGuiKeyModFlags_, wxKeyCode>, 3> imgui_mod_keys{
        std::make_pair(ImGuiKeyModFlags_Ctrl, WXK_CONTROL),
        std::make_pair(ImGuiKeyModFlags_Shift, WXK_SHIFT),
        std::make_pair(ImGuiKeyModFlags_Alt, WXK_ALT)};
    for (const std::pair<ImGuiKeyModFlags_, wxKeyCode>& key : imgui_mod_keys) {
        if ((io.KeyMods & key.first) != 0 && !wxGetKeyState(key.second))
            io.KeyMods &= ~key.first;
    }

    // Not sure if it is neccessary
    // values from 33 to 126 are reserved for the standard ASCII characters
    //for (size_t i = 33; i <= 126; ++i) {
    //    wxKeyCode keycode = static_cast<wxKeyCode>(i);
    //    if (io.KeysDown[i] && keycode != WXK_NONE && !wxGetKeyState(keycode))
    //        io.KeysDown[i] = false;
    //}

    //// special keys: delete, backspace, ...
    //for (int key: io.KeyMap) {
    //    wxKeyCode keycode = static_cast<wxKeyCode>(key);
    //    if (io.KeysDown[key] && keycode != WXK_NONE && !wxGetKeyState(keycode))
    //        io.KeysDown[key] = false;
    //}

    ImGui::NewFrame();
    m_new_frame_open = true;
}

void ImGuiWrapper::render()
{
    ImGui::Render();
    render_draw_data(ImGui::GetDrawData());
    m_new_frame_open = false;
}

ImVec2 ImGuiWrapper::calc_text_size(std::string_view text,
                                    bool  hide_text_after_double_hash,
                                    float wrap_width)
{
    return ImGui::CalcTextSize(text.data(), text.data() + text.length(),
                               hide_text_after_double_hash, wrap_width);
}

ImVec2 ImGuiWrapper::calc_text_size(const std::string& text,
                                    bool  hide_text_after_double_hash,
                                    float wrap_width)
{
    return ImGui::CalcTextSize(text.c_str(), NULL, hide_text_after_double_hash, wrap_width);
}

ImVec2 ImGuiWrapper::calc_text_size(const wxString &text,
                                    bool  hide_text_after_double_hash,
                                    float wrap_width)
{
    auto text_utf8 = into_u8(text);
    ImVec2 size = ImGui::CalcTextSize(text_utf8.c_str(), NULL, hide_text_after_double_hash, wrap_width);

/*#ifdef __linux__
    size.x *= m_style_scaling;
    size.y *= m_style_scaling;
#endif*/

    return size;
}

ImVec2 ImGuiWrapper::calc_button_size(const wxString &text, const ImVec2 &button_size) const
{
    const ImVec2        text_size = this->calc_text_size(text);
    const ImGuiContext &g         = *GImGui;
    const ImGuiStyle   &style     = g.Style;

    return ImGui::CalcItemSize(button_size, text_size.x + style.FramePadding.x * 2.0f, text_size.y + style.FramePadding.y * 2.0f);
}

ImVec2 ImGuiWrapper::get_item_spacing() const
{
    const ImGuiContext &g     = *GImGui;
    const ImGuiStyle   &style = g.Style;
    return style.ItemSpacing;
}

float ImGuiWrapper::get_slider_float_height() const
{
    const ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    return g.FontSize + style.FramePadding.y * 2.0f + style.ItemSpacing.y;
}

void ImGuiWrapper::set_next_window_pos(float x, float y, int flag, float pivot_x, float pivot_y)
{
    ImGui::SetNextWindowPos(ImVec2(x, y), (ImGuiCond)flag, ImVec2(pivot_x, pivot_y));
    ImGui::SetNextWindowSize(ImVec2(0.0, 0.0));
}

void ImGuiWrapper::set_next_window_bg_alpha(float alpha)
{
    ImGui::SetNextWindowBgAlpha(alpha);
}

void ImGuiWrapper::set_next_window_size(float x, float y, ImGuiCond cond)
{
	ImGui::SetNextWindowSize(ImVec2(x, y), cond);
}

/* BBL style widgets */
bool ImGuiWrapper::bbl_combo_with_filter(const char* label, const std::string& preview_value, const std::vector<std::string>& all_items, std::vector<int>* filtered_items_idx, bool* is_filtered, float item_height)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    static char pattern_buffer[256] = { 0 };
    auto simple_match = [](const char* pattern, const char* str) {
        wxString sub_str = wxString(pattern).Lower();
        wxString main_str = wxString(str).Lower();
        return main_str.Find(sub_str);
    };

    bool is_filtering = false;
    bool is_new_open = false;

    float sz = ImGui::GetFrameHeight();
    ImVec2 arrow_size(sz, sz);
    ImVec2 CursorPos = window->DC.CursorPos;
    const ImRect arrow_bb(CursorPos, CursorPos + arrow_size);

    float ButtonTextAlignX = g.Style.ButtonTextAlign.x;
    g.Style.ButtonTextAlign.x = 0;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { sz, style.FramePadding.y});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
    if (button(preview_value + label, ImGui::CalcItemWidth(), 0))
    {
        ImGui::OpenPopup(label);
        is_new_open = true;
    }
    g.Style.ButtonTextAlign.x = ButtonTextAlignX;
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::BBLRenderArrow(window->DrawList, arrow_bb.Min + ImVec2(ImMax(0.0f, (arrow_size.x - g.FontSize) * 0.5f), ImMax(0.0f, (arrow_size.y - g.FontSize) * 0.5f)), ImGui::GetColorU32(ImGuiCol_Text), ImGuiDir_Down);

    if (is_new_open)
        memset(pattern_buffer, 0, IM_ARRAYSIZE(pattern_buffer));

    float item_rect_width = ImGui::GetItemRectSize().x;
    float item_rect_height = item_height ? item_height : ImGui::GetItemRectSize().y;
    ImGui::SetNextWindowPos({ CursorPos.x, ImGui::GetItemRectMax().y + 4 * m_style_scaling });
    ImGui::SetNextWindowSize({ item_rect_width, 0 });
    if (ImGui::BeginPopup(label))
    {
        ImGuiWindow* popup_window = ImGui::GetCurrentWindow();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f * m_style_scaling, item_rect_height - g.FontSize) * 0.5f);
        wchar_t ICON_SEARCH = *pattern_buffer != '\0' ? ImGui::TextSearchCloseIcon : ImGui::TextSearchIcon;
        const ImVec2 label_size = ImGui::CalcTextSize(into_u8(ICON_SEARCH).c_str(), nullptr, true);
        const ImVec2 search_icon_pos(ImGui::GetItemRectMax().x - label_size.x, popup_window->DC.CursorPos.y + style.FramePadding.y);
        ImGui::RenderText(search_icon_pos, into_u8(ICON_SEARCH).c_str());

        auto temp = popup_window->DC.CursorPos;
        popup_window->DC.CursorPos = search_icon_pos;
        ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_Button));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_Button));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_Button));
        ImGui::PushStyleColor(ImGuiCol_Border, { 0, 0, 0, 0 });
        if (button("##invisible_clear_button", label_size.x, label_size.y))
        {
            if (*pattern_buffer != '\0')
                memset(pattern_buffer, 0, IM_ARRAYSIZE(pattern_buffer));
        }
        ImGui::PopStyleColor(5);
        popup_window->DC.CursorPos = temp;


        ImGui::PushItemWidth(item_rect_width);
        if (is_new_open)
            ImGui::SetKeyboardFocusHere();
        ImGui::InputText("##bbl_combo_with_filter_inputText", pattern_buffer, sizeof(pattern_buffer));
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();

        if (*pattern_buffer != '\0')
            is_filtering = true;

        if (is_filtering)
        {
            std::vector<std::pair<int, int> > filtered_items_with_priority;// std::pair<index, priority>
            for (int i = 0; i < all_items.size(); i++)
            {
                int priority = simple_match(pattern_buffer, all_items[i].c_str());
                if (priority != wxNOT_FOUND)
                    filtered_items_with_priority.push_back({ i, priority });
            }
            std::sort(filtered_items_with_priority.begin(), filtered_items_with_priority.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {return (b.second > a.second); });
            for (auto item : filtered_items_with_priority)
            {
                filtered_items_idx->push_back(item.first);
            }
        }

        *is_filtered = is_filtering;

        popup_window->DC.CursorPos.y -= 1 * m_style_scaling;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f, 1.0f) * m_style_scaling);
        if (ImGui::BeginListBox("##bbl_combo_with_filter_listBox", { item_rect_width, item_rect_height * 7.75f})) {
            ImGui::PopStyleVar(2);
            return true;
        }
        else
        {
            ImGui::PopStyleVar(2);
            ImGui::EndPopup();
            return false;
        }
    }
    else
        return false;
}

bool ImGuiWrapper::bbl_input_double(const wxString& label, const double& value, const std::string& format)
{
    //return ImGui::InputDouble(label.c_str(), const_cast<double *>(&value), 0.0f, 0.0f, format.c_str(), ImGuiInputTextFlags_CharsDecimal);
    return ImGui::InputDouble(label.c_str(), const_cast<double *>(&value), 0.0f, 0.0f, format.c_str(), ImGuiInputTextFlags_CharsDecimal);
}

bool ImGuiWrapper::bbl_slider_float_style(const std::string &label, float *v, float v_min, float v_max, const char *format, float power, bool clamp, const wxString &tooltip)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(238 / 255.0f, 238 / 255.0f, 238 / 255.0f, 0.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(238 / 255.0f, 238 / 255.0f, 238 / 255.0f, 0.00f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.81f, 0.81f, 0.81f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.00f, 0.68f, 0.26f, 1.00f));

    bool ret = bbl_slider_float(label, v, v_min,v_max, format, power, clamp,tooltip);

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(1);

    return ret;
}

bool ImGuiWrapper::bbl_slider_float(const std::string& label, float* v, float v_min, float v_max, const char* format, float power, bool clamp, const wxString& tooltip)
{

    const float max_tooltip_width = ImGui::GetFontSize() * 20.0f;

    // let the label string start with "##" to hide the automatic label from ImGui::SliderFloat()
    bool label_visible = !boost::algorithm::istarts_with(label, "##");
    std::string str_label = label_visible ? std::string("##") + std::string(label) : std::string(label);

    // removes 2nd evenience of "##", if present
    std::string::size_type pos = str_label.find("##", 2);
    if (pos != std::string::npos)
        str_label = str_label.substr(0, pos) + str_label.substr(pos + 2);

    bool ret = ImGui::BBLSliderFloat(str_label.c_str(), v, v_min, v_max, format, power);

    m_last_slider_status.hovered = ImGui::IsItemHovered();
    m_last_slider_status.clicked = ImGui::IsItemClicked();
    m_last_slider_status.deactivated_after_edit = ImGui::IsItemDeactivatedAfterEdit();

    if (!tooltip.empty() && ImGui::IsItemHovered())
        this->tooltip(into_u8(tooltip).c_str(), max_tooltip_width);

    if (clamp)
        *v = std::clamp(*v, v_min, v_max);

    const ImGuiStyle& style = ImGui::GetStyle();

    if (label_visible) {
        // if the label is visible, hide the part of it that should be hidden
        std::string out_label = std::string(label);
        std::string::size_type pos = out_label.find("##");
        if (pos != std::string::npos)
            out_label = out_label.substr(0, pos);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 1, style.ItemSpacing.y });
        ImGui::SameLine();
        this->text(out_label.c_str());
        ImGui::PopStyleVar();
    }


    return ret;
}

bool ImGuiWrapper::begin(const std::string &name, int flags)
{
    return ImGui::Begin(name.c_str(), nullptr, (ImGuiWindowFlags)flags);
}

bool ImGuiWrapper::begin(const wxString &name, int flags)
{
    return begin(into_u8(name), flags);
}

bool ImGuiWrapper::begin(const std::string& name, bool* close, int flags)
{
    return ImGui::Begin(name.c_str(), close, (ImGuiWindowFlags)flags);
}

bool ImGuiWrapper::begin(const wxString& name, bool* close, int flags)
{
    return begin(into_u8(name), close, flags);
}

void ImGuiWrapper::end()
{
    ImGui::End();
}

bool ImGuiWrapper::button(const wxString &label, const wxString& tooltip)
{
    auto label_utf8 = into_u8(label);
    //return ImGui::Button(label_utf8.c_str());
    return ACIMButton(label_utf8.c_str(), ImVec2(0, 0));
}

bool ImGuiWrapper::button(const wxString& label, float width, float height)
{
	auto label_utf8 = into_u8(label);
	//return ImGui::Button(label_utf8.c_str(), ImVec2(width, height));
    return ACIMButton(label_utf8.c_str(), ImVec2(width, height));
}

bool ImGuiWrapper::button(const wxString& label, const ImVec2 &size, bool enable)
{
    disabled_begin(!enable);

    auto label_utf8 = into_u8(label);
    bool res = ImGui::Button(label_utf8.c_str(), size);

    disabled_end();
    return (enable) ? res : false;
}


bool ImGuiWrapper::radio_button(const wxString &label, bool active)
{
    auto label_utf8 = into_u8(label);
    return ImGui::RadioButton(label_utf8.c_str(), active);
}

void ImGuiWrapper::draw_icon(ImGuiWindow& window, const ImVec2& pos, float size, wchar_t icon_id)
{
    ImGuiIO& io = ImGui::GetIO();
    const ImTextureID tex_id = io.Fonts->TexID;
    const float tex_w = static_cast<float>(io.Fonts->TexWidth);
    const float tex_h = static_cast<float>(io.Fonts->TexHeight);
    const ImFontAtlas::CustomRect* const rect = GetTextureCustomRect(icon_id);
    const ImVec2 uv0 = { static_cast<float>(rect->X) / tex_w, static_cast<float>(rect->Y) / tex_h };
    const ImVec2 uv1 = { static_cast<float>(rect->X + rect->Width) / tex_w, static_cast<float>(rect->Y + rect->Height) / tex_h };
    window.DrawList->AddImage(tex_id, pos, { pos.x + size, pos.y + size }, uv0, uv1, ImGuiWrapper::to_ImU32({ 1.0f, 1.0f, 1.0f, 1.0f }));
}

bool ImGuiWrapper::draw_radio_button(const std::string& name, float size, bool active,
    std::function<void(ImGuiWindow& window, const ImVec2& pos, float size)> draw_callback, bool showHover)
{
    ImGuiWindow& window = *ImGui::GetCurrentWindow();
    if (window.SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window.GetID(name.c_str());

    const ImVec2 pos = window.DC.CursorPos;
    const ImRect total_bb(pos, pos + ImVec2(size, size + style.FramePadding.y * 2.0f));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
        ImGui::MarkItemEdited(id);

    if (showHover && hovered)
        window.DrawList->AddRect({ pos.x - 1.0f, pos.y - 1.0f }, { pos.x + size + 1.0f, pos.y + size + 1.0f }, ImGui::GetColorU32(ImGuiCol_CheckMark));

    if (active)
        window.DrawList->AddRect(pos, { pos.x + size, pos.y + size }, ImGui::GetColorU32(ImGuiCol_CheckMark));

    draw_callback(window, pos, size);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window.DC.LastItemStatusFlags);
    return pressed;
}

bool ImGuiWrapper::checkbox(const wxString &label, bool &value)
{
    //auto label_utf8 = into_u8(label);
    //return ImGui::Checkbox(label_utf8.c_str(), &value);
    return bbl_ac_checkbox(label, value);
}

bool ImGuiWrapper::bbl_checkbox(const wxString &label, bool &value)
{
    bool result;
    bool b_value = value;
    if (b_value) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, COL_AC_PANELGRAY);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, COL_AC_LIGHTBLUE);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, COL_AC_LIGHTBLUE);
        ImGui::PushStyleColor(ImGuiCol_CheckMark, COL_AC_BLUE);
    }
    auto label_utf8 = into_u8(label);
    result          = ImGui::BBLCheckbox(label_utf8.c_str(), &value);

    if (b_value) { ImGui::PopStyleColor(4);}
    return result;
}


bool ImGuiWrapper::bbl_ac_checkbox(const wxString &label, bool &value)
{
    return bbl_checkbox(label, value);
}

bool ImGuiWrapper::bbl_radio_button(const char *label, bool active)
{
    bool result;
    bool b_value = active;
    if (b_value) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, COL_AC_PANELGRAY);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, COL_AC_LIGHTBLUE);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, COL_AC_LIGHTBLUE);
    }
    result = ImGui::BBLRadioButton(label,active);
    if (b_value) { ImGui::PopStyleColor(3); }
    return result;
}

bool ImGuiWrapper::bbl_sliderin(const char *label, int *v, int v_min, int v_max, const char *format, ImGuiSliderFlags flags)
{
    return ImGui::BBLSliderScalarIn(label, ImGuiDataType_S32, v, &v_min, &v_max, format, flags);
}

void ImGuiWrapper::text(const char *label)
{
    ImGui::Text("%s", label);
}

void ImGuiWrapper::text(const std::string &label)
{
    ImGuiWrapper::text(label.c_str());
}

void ImGuiWrapper::text(const wxString &label)
{
    auto label_utf8 = into_u8(label);
    ImGuiWrapper::text(label_utf8.c_str());
}

void ImGuiWrapper::text_colored(const ImVec4& color, const char* label)
{
    ImGui::TextColored(color, "%s", label);
}

void ImGuiWrapper::text_colored(const ImVec4& color, const std::string& label)
{
    ImGuiWrapper::text_colored(color, label.c_str());
}

void ImGuiWrapper::text_colored(const ImVec4& color, const wxString& label)
{
    auto label_utf8 = into_u8(label);
    ImGuiWrapper::text_colored(color, label_utf8.c_str());
}

void ImGuiWrapper::text_wrapped(const char *label, float wrap_width)
{
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrap_width);
    this->text(label);
    ImGui::PopTextWrapPos();
}

void ImGuiWrapper::text_wrapped(const std::string &label, float wrap_width)
{
    this->text_wrapped(label.c_str(), wrap_width);
}

void ImGuiWrapper::text_wrapped(const wxString &label, float wrap_width)
{
    auto label_utf8 = into_u8(label);
    this->text_wrapped(label_utf8.c_str(), wrap_width);
}

void ImGuiWrapper::tooltip(const char *label, float wrap_width)
{
    ImGui::PushStyleColor(ImGuiCol_Text, COL_AC_ITEMBLUE);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 8.0f, 8.0f });

    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(wrap_width);
    ImGui::TextUnformatted(label);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void ImGuiWrapper::tooltip(const wxString &label, float wrap_width)
{
    tooltip(label.ToUTF8().data(), wrap_width);
}

ImVec2 ImGuiWrapper::get_slider_icon_size() const
{
    return this->calc_button_size(std::wstring(&ImGui::SliderFloatEditBtnIcon, 1));
}

static const char *PatchFormatStringFloatToInt(const char *fmt)
{
    if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '0' && fmt[3] == 'f' &&
        fmt[4] == 0) // Fast legacy path for "%.0f" which is expected to be the most common case.
        return "%d";
    const char *fmt_start = ImParseFormatFindStart(fmt);     // Find % (if any, and ignore %%)
    const char *fmt_end   = ImParseFormatFindEnd(fmt_start); // Find end of format specifier, which itself is an exercise of
                                                             // confidence/recklessness (because snprintf is dependent on libc or user).
    if (fmt_end > fmt_start && fmt_end[-1] == 'f') {
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        if (fmt_start == fmt && fmt_end[0] == 0)
            return "%d";
        ImGuiContext &g = *GImGui;
        ImFormatString(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), "%.*s%%d%s", (int) (fmt_start - fmt), fmt,
                       fmt_end); // Honor leading and trailing decorations, but lose alignment/precision.
        return g.TempBuffer;
#else
        IM_ASSERT(0 &&"DragInt(): Invalid format string!"); // Old versions used a default parameter of "%.0f", please replace with e.g. "%d"
#endif
    }
    return fmt;
}
bool ImGuiWrapper::ACSliderScalar(const char *     label,
                                  ImGuiDataType    data_type,
                                  void *           p_data,
                                  const void *     p_min,
                                  const void *     p_max,
                                  const char *     format,
                                  ImGuiSliderFlags flags)
{
    ImGuiSliderFlags slider_flags    = ImGuiSliderFlags_None;
    float            _lineWidth      = 2.0f * get_style_scaling();
    float            _smallPosRadius = 7.0f * get_style_scaling();
    float            _bigPosRadius   = 15.0f * get_style_scaling();
    static float     _lefLineMaxX    = 0.0f;
    if (flags != 1.0f) {
        IM_ASSERT(flags == 1.0f &&
                  "Call function with ImGuiSliderFlags_Logarithmic flags instead of using the old 'float power' function!");
        slider_flags |= ImGuiSliderFlags_Logarithmic; // Fallback for non-asserting paths
    }
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &    g     = *GImGui;
    const ImGuiStyle &style = g.Style;
    const ImGuiID     id    = window->GetID(label);
    const float       w     = ImGui::CalcItemWidth();

    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

    const bool temp_input_allowed = (slider_flags & ImGuiSliderFlags_NoInput) == 0;
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id, &frame_bb, temp_input_allowed ? ImGuiItemAddFlags_Focusable : 0))
        return false;

    // Default format string when passing NULL
    if (format == NULL)
        format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;
    else if (data_type == ImGuiDataType_S32 &&
             strcmp(format, "%d") != 0) // (FIXME-LEGACY: Patch old "%.0f" format string to use "%d", read function more details.)
        format = PatchFormatStringFloatToInt(format);

    // Tabbing or CTRL-clicking on Slider turns it into an input box
    const bool hovered              = ImGui::ItemHoverable(frame_bb, id);
    bool       temp_input_is_active = temp_input_allowed && ImGui::TempInputIsActive(id);
    if (!temp_input_is_active) {
        const bool focus_requested = temp_input_allowed && (window->DC.LastItemStatusFlags & ImGuiItemStatusFlags_Focused) != 0;
        const bool clicked         = (hovered && g.IO.MouseClicked[0]);
        if (focus_requested || clicked || g.NavActivateId == id || g.NavInputId == id) {
            ImGui::SetActiveID(id, window);
            ImGui::SetFocusID(id, window);
            ImGui::FocusWindow(window);
            g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
            /*if (temp_input_allowed && (focus_requested || (clicked && g.IO.KeyCtrl) || g.NavInputId == id))
                temp_input_is_active = true;*/
        }
    }

    // if (temp_input_is_active) {
    //    // Only clamp CTRL+Click input when ImGuiSliderFlags_AlwaysClamp is set
    //    const bool is_clamp_input = (slider_flags & ImGuiSliderFlags_AlwaysClamp) != 0;
    //    return ImGui::TempInputScalar(frame_bb, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL,
    //                                  is_clamp_input ? p_max : NULL);
    //}

    // Draw frame
    /*const ImU32 frame_col = ImGui::GetColorU32(g.ActiveId == id  ? ImGuiCol_FrameBgActive :
                                        g.HoveredId == id ? ImGuiCol_FrameBgHovered :
                                                            ImGuiCol_FrameBg);*/
    // ImGui::RenderNavHighlight(frame_bb, id);
    window->DrawList->AddLine(ImVec2(frame_bb.Min.x, frame_bb.Min.y + _bigPosRadius), ImVec2(_lefLineMaxX, frame_bb.Min.y + _bigPosRadius),
                              ImGui::GetColorU32(COL_AC_BLUE));

    // Slider behavior
    ImRect     grab_bb;
    const bool value_changed = ImGui::SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, slider_flags, &grab_bb);
    if (value_changed)
        ImGui::MarkItemEdited(id);
    _lefLineMaxX = grab_bb.Min.x;
    // Render grab
    ImVec2 circlePos(grab_bb.Min.x - 2.0f + _smallPosRadius, grab_bb.Min.y + _bigPosRadius - 1.0f);

    window->DrawList->AddCircleFilled(circlePos, _smallPosRadius, ImGui::GetColorU32(COL_AC_BLUE), 12);
    if (hovered || g.IO.MouseClicked[0]) {
        ImVec4 c = COL_AC_BLUE;
        c.w      = 0.14f;
        window->DrawList->AddCircleFilled(circlePos, _bigPosRadius, ImGui::ColorConvertFloat4ToU32(c), 12);
    }
    //}

    window->DrawList->AddLine(ImVec2(grab_bb.Min.x - 2.0f + _smallPosRadius * 2, frame_bb.Min.y + _bigPosRadius),
                              ImVec2(frame_bb.Max.x, frame_bb.Min.y + _bigPosRadius),
                              ImGui::GetColorU32(ImVec4(195.0f / 255.0f, 204.0f / 255.0f, 217.0f / 255.0f, 1.0f)));

    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    /*char        value_buf[64];
    const char *value_buf_end = value_buf + ImGui::DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
    if (g.LogEnabled)
        ImGui::LogSetNextTextDecoration("{", "}");
    ImGui::RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));*/

    /*if (label_size.x > 0.0f)
        ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);*/

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
    return value_changed;
}

bool  ImGuiWrapper::ACSliderScalar(const char *     label,
                         ImGuiDataType    data_type,
                         void *           p_data,
                         void *           slider_data,
                         int              old_data,
                         const void *     p_min,
                         const void *     p_max,
                         const char *     format,
                         ImGuiSliderFlags flags)
{

    ImGuiSliderFlags slider_flags = ImGuiSliderFlags_None;
    float            _lineWidth    = 2.0f * get_style_scaling();
    float            _smallPosRadius = 7.0f * get_style_scaling();
    float            _oldPosRadius = 4.0f * get_style_scaling();
    float            _bigPosRadius   = 15.0f * get_style_scaling();
    static float     _lefLineMaxX  = 0.0f;
    if (flags != 1.0f) {
        IM_ASSERT(flags == 1.0f &&
                  "Call function with ImGuiSliderFlags_Logarithmic flags instead of using the old 'float power' function!");
        slider_flags |= ImGuiSliderFlags_Logarithmic; // Fallback for non-asserting paths
    }
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &    g     = *GImGui;
    const ImGuiStyle &style = g.Style;
    const ImGuiID     id    = window->GetID(label);
    const float       w     = ImGui::CalcItemWidth();

    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

    const bool temp_input_allowed = (slider_flags & ImGuiSliderFlags_NoInput) == 0;
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id, &frame_bb, temp_input_allowed ? ImGuiItemAddFlags_Focusable : 0))
        return false;

    // Default format string when passing NULL
    if (format == NULL)
        format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;
    else if (data_type == ImGuiDataType_S32 &&
             strcmp(format, "%d") != 0) // (FIXME-LEGACY: Patch old "%.0f" format string to use "%d", read function more details.)
        format = PatchFormatStringFloatToInt(format);

    // Tabbing or CTRL-clicking on Slider turns it into an input box
    const bool hovered              = ImGui::ItemHoverable(frame_bb, id);
    bool       temp_input_is_active = temp_input_allowed && ImGui::TempInputIsActive(id);
    if (!temp_input_is_active) {
        const bool focus_requested = temp_input_allowed && (window->DC.LastItemStatusFlags & ImGuiItemStatusFlags_Focused) != 0;
        const bool clicked         = (hovered && g.IO.MouseClicked[0]);
        if (focus_requested || clicked || g.NavActivateId == id || g.NavInputId == id) {
            ImGui::SetActiveID(id, window);
            ImGui::SetFocusID(id, window);
            ImGui::FocusWindow(window);
            g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
            /*if (temp_input_allowed && (focus_requested || (clicked && g.IO.KeyCtrl) || g.NavInputId == id))
                temp_input_is_active = true;*/
        }
    }

    //if (temp_input_is_active) {
    //    // Only clamp CTRL+Click input when ImGuiSliderFlags_AlwaysClamp is set
    //    const bool is_clamp_input = (slider_flags & ImGuiSliderFlags_AlwaysClamp) != 0;
    //    return ImGui::TempInputScalar(frame_bb, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL,
    //                                  is_clamp_input ? p_max : NULL);
    //}

    // Draw frame
    /*const ImU32 frame_col = ImGui::GetColorU32(g.ActiveId == id  ? ImGuiCol_FrameBgActive :
                                        g.HoveredId == id ? ImGuiCol_FrameBgHovered :
                                                            ImGuiCol_FrameBg);*/
    //ImGui::RenderNavHighlight(frame_bb, id);

    ImVec4 bgLineColor(195.0f / 255.0f, 204.0f / 255.0f, 217.0f / 255.0f, 1.0f);
    window->DrawList->AddLine(ImVec2(frame_bb.Min.x, frame_bb.Min.y + _bigPosRadius),
                              ImVec2(frame_bb.Max.x, frame_bb.Min.y + _bigPosRadius), ImGui::GetColorU32(bgLineColor));

    window->DrawList->AddLine(ImVec2(frame_bb.Min.x, frame_bb.Min.y + _bigPosRadius), ImVec2(_lefLineMaxX, frame_bb.Min.y + _bigPosRadius),
                              ImGui::GetColorU32(COL_AC_BLUE));



    ImVec2 old_circlePos(frame_bb.Min.x + ((frame_bb.Max.x - frame_bb.Min.x) * old_data / 100),
                         frame_bb.Min.y + _bigPosRadius);
    window->DrawList->AddCircleFilled(old_circlePos, _oldPosRadius, ImGui::GetColorU32(COL_AC_BLACK), 12);

    // Slider behavior
    ImRect     grab_bb;
    //bool       result;
    const int  s_previousValue = *reinterpret_cast<int *>(p_data);
    const bool value_changed = ImGui::SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, slider_flags, &grab_bb);
    //result                     = value_changed;
    int        value         = *reinterpret_cast<int *>(p_data);
    int        _valueGap       = int(value % 10);
    int        _upIndex      = value + (10 - _valueGap);
    if (old_data + _valueGap > _upIndex && _upIndex > old_data)
        _upIndex = old_data;
    int        _lowIndex       = value - _valueGap;
    if (_lowIndex > old_data - _valueGap && _lowIndex < old_data)
        _lowIndex = old_data;
    int        newValue        = _valueGap > 5 ? _upIndex : _lowIndex;


    // Render grab
    ImVec2 circlePos(frame_bb.Min.x + ((frame_bb.Max.x - frame_bb.Min.x) * s_previousValue / 100),
                     frame_bb.Min.y + _bigPosRadius);
    if (value_changed)
        ImGui::MarkItemEdited(id);
    if (s_previousValue != newValue && value_changed) {
        circlePos = ImVec2(frame_bb.Min.x + ((frame_bb.Max.x - frame_bb.Min.x) * newValue / 100),
                           frame_bb.Min.y + _bigPosRadius);
        *(ImS32 *) slider_data = newValue;
    } else {
        //result = false;
        //void *_p_data = reinterpret_cast<void *>(s_previousValue);
        *(ImS32 *) slider_data = s_previousValue;
    }

    _lefLineMaxX = circlePos.x;
    if (newValue == 100)
        circlePos.x -= (_smallPosRadius - 1.0f * get_style_scaling());
    if (newValue == 0)
        circlePos.x += _smallPosRadius;

    window->DrawList->AddCircleFilled(circlePos, _smallPosRadius, ImGui::GetColorU32(COL_AC_BLUE), 12);
    if (hovered || g.IO.MouseClicked[0]) {
        ImVec4 c = COL_AC_BLUE;
        c.w      = 0.14f;
        window->DrawList->AddCircleFilled(circlePos, _bigPosRadius, ImGui::ColorConvertFloat4ToU32(c), 12);
    }


    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    /*char        value_buf[64];
    const char *value_buf_end = value_buf + ImGui::DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
    if (g.LogEnabled)
        ImGui::LogSetNextTextDecoration("{", "}");
    ImGui::RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));*/

    /*if (label_size.x > 0.0f)
        ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);*/

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
    return value_changed;
}

bool ImGuiWrapper::ACInputInt(const char *label, int *v, int step, int step_fast, ImGuiInputTextFlags flags)
{

    const char *format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
    return ImGui::InputScalar(label, ImGuiDataType_S32, (void *) v, (void *) (step > 0 ? &step : NULL),
                       (void *) (step_fast > 0 ? &step_fast : NULL), format, flags);
}

bool ImGuiWrapper::slider_float_line(const char *    label,
                                int *         v,
                                int           v_min,
                                     int             v_max,
                                     bool *          changeType,
                                     ImTextureID imId,
                                     int old_value,
                                const char *    format /* = "%.3f"*/,
                                     float             power /* = 1.0f*/,
                                bool            clamp /*= true*/,
                                const wxString &tooltip /*= ""*/,
                                     bool            show_edit_input /*= true*/)
{
    const float max_tooltip_width = ImGui::GetFontSize() * 20.0f;
    // let the label string start with "##" to hide the automatic label from ImGui::SliderFloat()
    bool        label_visible = !boost::algorithm::starts_with(label, "##");
    std::string str_label     = label_visible ? std::string("##") + std::string(label) : std::string(label);
    // removes 2nd evenience of "##", if present
    std::string::size_type pos = str_label.find("##", 2);
    if (pos != std::string::npos)
        str_label = str_label.substr(0, pos) + str_label.substr(pos + 2);

    // the current slider edit state needs to be detected here before calling SliderFloat()
    bool slider_editing = ImGui::GetCurrentWindow()->GetID(str_label.c_str()) == ImGui::GetActiveID();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(51.0f, 3.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 4.0f);
    int *sliderResult = new int(*v);
    bool ret          = ACSliderScalar(str_label.c_str(), ImGuiDataType_S32, v, sliderResult,old_value, &v_min, &v_max, format, power);
    if (ret) {
        *changeType = ret;
    }
    ImGui::PopStyleVar(3);

    m_last_slider_status.hovered                = ImGui::IsItemHovered();
    m_last_slider_status.edited                 = ImGui::IsItemEdited();
    m_last_slider_status.clicked                = ImGui::IsItemClicked();
    m_last_slider_status.deactivated_after_edit = ImGui::IsItemDeactivatedAfterEdit();

    ImGui::PushStyleColor(ImGuiCol_PopupBg, {.0f, .0f, .0f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 1.0f, 1.0f, 1.0f});
    if (!tooltip.empty() && ImGui::IsItemHovered())
        this->tooltip(into_u8(tooltip).c_str(), max_tooltip_width);
    ImGui::PopStyleColor(2);

    if (clamp)
        *v = std::clamp(*v, v_min, v_max);

    const ImGuiStyle &style = ImGui::GetStyle();

    if (show_edit_input) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {1, style.ItemSpacing.y});
        ImGui::SameLine();
        ImGui::PushItemWidth(60 * get_style_scaling());
        ImGuiIO &io = ImGui::GetIO();
        assert(io.Fonts->TexWidth > 0 && io.Fonts->TexHeight > 0);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 9.0f * get_style_scaling());
        push_ac_button_style(ImGui::GetCursorPosY() + 5.0f * get_style_scaling(), 6.0f, 3.0f, 6.0f);
        std::string _name               = label + wxString::Format("%d", 1).ToStdString();
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(195.0f/255.0f, 204.0f/255.0f, 217.0f/255.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, COL_AC_WHITE);
        ImGui::PushStyleColor(ImGuiCol_Text, COL_AC_BLACK);
        int *inputResult         = new int(ret? *sliderResult:*v);
        bool value_input_changed = ACInputInt(_name.c_str(), inputResult, NULL, 100,
                                              ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue);


        if (value_input_changed) {
            ret = value_input_changed;
            *v  = *inputResult;
        } else {
            if (ImGui::IsItemDeactivated()) {
                ImGuiID              input_id = ImGui::GetID(_name.c_str());
                ImGuiInputTextState *state    = ImGui::GetInputTextState(input_id);
                if (state->TextA.Data != NULL) {
                    std::string _value(state->TextA.Data);
                    if (_value.length() > 0) {
                    *v = std::stoi(_value);
                    ret = true;
                    }
                }
            } else {
                *v = *sliderResult;
            }
        }
        delete sliderResult, inputResult;
        ImGui::PopStyleColor(3);
        pop_ac_button_style();

        ImGui::PopStyleVar();
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 25.0f * get_style_scaling());
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(129.0f / 255.0f, 133.0f / 255.0f, 140.0f / 255.0f, 1.0f));
        text(L("%"));
        ImGui::PopStyleColor();
        /*if (old_value != *v) {
            ImGui::SameLine();
            const ImVec2      img_size = {22.0f * get_style_scaling(), 22.0f * get_style_scaling()};
            const ImTextureID          tex_id   = io.Fonts->TexID;
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2.0f * get_style_scaling());
            if (image_button(imId, img_size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
                -1, ImVec4(0.0, 0.0, 0.0, 0.0),ImVec4(1.0, 1.0, 1.0, 1.0),ImGuiButtonFlags_PressedOnClick)) {
                *v = old_value;
                ret = true;
            }
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
        }*/
    }

    if (label_visible) {
        // if the label is visible, hide the part of it that should be hidden
        std::string            out_label = std::string(label);
        std::string::size_type pos       = out_label.find("##");
        if (pos != std::string::npos)
            out_label = out_label.substr(0, pos);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {1, style.ItemSpacing.y});
        ImGui::SameLine();
        this->text(out_label.c_str());
        ImGui::PopStyleVar();
    }

    return ret;
}

bool ImGuiWrapper::slider_float(const char* label, float* v, float v_min, float v_max, const char* format/* = "%.3f"*/, float power/* = 1.0f*/, bool clamp /*= true*/, const wxString& tooltip /*= ""*/, bool show_edit_btn /*= true*/)
{
    const float max_tooltip_width = ImGui::GetFontSize() * 20.0f;

    // let the label string start with "##" to hide the automatic label from ImGui::SliderFloat()
    bool label_visible = !boost::algorithm::starts_with(label, "##");
    std::string str_label = label_visible ? std::string("##") + std::string(label) : std::string(label);

    // removes 2nd evenience of "##", if present
    std::string::size_type pos = str_label.find("##", 2);
    if (pos != std::string::npos)
        str_label = str_label.substr(0, pos) + str_label.substr(pos + 2);

    // the current slider edit state needs to be detected here before calling SliderFloat()
    bool slider_editing = ImGui::GetCurrentWindow()->GetID(str_label.c_str()) == ImGui::GetActiveID();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(51.0f, 3.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 4.0f);
    bool ret = ImGui::SliderFloat(str_label.c_str(), v, v_min, v_max, format, power);
    ImGui::PopStyleVar(3);

    m_last_slider_status.hovered = ImGui::IsItemHovered();
    m_last_slider_status.edited = ImGui::IsItemEdited();
    m_last_slider_status.clicked = ImGui::IsItemClicked();
    m_last_slider_status.deactivated_after_edit = ImGui::IsItemDeactivatedAfterEdit();

    ImGui::PushStyleColor(ImGuiCol_PopupBg, {.0f, .0f, .0f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 1.0f, 1.0f, 1.0f });
    if (!tooltip.empty() && ImGui::IsItemHovered())
        this->tooltip(into_u8(tooltip).c_str(), max_tooltip_width);
    ImGui::PopStyleColor(2);

    if (clamp)
        *v = std::clamp(*v, v_min, v_max);

    const ImGuiStyle& style = ImGui::GetStyle();
    if (show_edit_btn) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 1, style.ItemSpacing.y });
        ImGui::SameLine();

        ImGuiIO& io = ImGui::GetIO();
        assert(io.Fonts->TexWidth > 0 && io.Fonts->TexHeight > 0);
        float inv_tex_w = 1.0f / float(io.Fonts->TexWidth);
        float inv_tex_h = 1.0f / float(io.Fonts->TexHeight);

        const ImFontAtlasCustomRect* const rect = GetTextureCustomRect(slider_editing ? ImGui::SliderFloatEditBtnPressedIcon : ImGui::SliderFloatEditBtnIcon);
        const ImVec2 size = { float(rect->Width), float(rect->Height) };
        const ImVec2 uv0 = ImVec2(float(rect->X) * inv_tex_w, float(rect->Y) * inv_tex_h);
        const ImVec2 uv1 = ImVec2(float(rect->X + rect->Width) * inv_tex_w, float(rect->Y + rect->Height) * inv_tex_h);

        ImGui::PushStyleColor(ImGuiCol_Button, { 1.0f, 1.0f, 1.0f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.96f, 0.97f, 1.0f, 1.0f }); // 244, 248, 254, 1 rgba(95, 157, 255, 1)
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.92f, 0.94f, 0.96f, 1.0f }); // 235, 239, 245, 1

        const ImTextureID tex_id = io.Fonts->TexID;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 9.0f);
        push_ac_button_style(ImGui::GetCursorPosY(),6.0f, 3.0f,6.0f);
        if (image_button(tex_id, size, uv0, uv1, -1, ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImGuiButtonFlags_PressedOnClick)) {
            if (!slider_editing)
                ImGui::SetKeyboardFocusHere(-1);
            else
                ImGui::ClearActiveID();
            this->set_requires_extra_frame();
        }
        pop_ac_button_style();
        ImGui::PopStyleColor(3);

        ImGui::PushStyleColor(ImGuiCol_PopupBg, {.0f, .0f, .0f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 1.0f, 1.0f, 1.0f });
        if (ImGui::IsItemHovered())
            this->tooltip(into_u8(_L("Edit")).c_str(), max_tooltip_width);
        ImGui::PopStyleColor(2);

        ImGui::PopStyleVar();
    }

    if (label_visible) {
        // if the label is visible, hide the part of it that should be hidden
        std::string out_label = std::string(label);
        std::string::size_type pos = out_label.find("##");
        if (pos != std::string::npos)
            out_label = out_label.substr(0, pos);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 1, style.ItemSpacing.y });
        ImGui::SameLine();
        this->text(out_label.c_str());
        ImGui::PopStyleVar();
    }

    return ret;
}

bool ImGuiWrapper::slider_float(const std::string& label, float* v, float v_min, float v_max, const char* format/* = "%.3f"*/, float power/* = 1.0f*/, bool clamp /*= true*/, const wxString& tooltip /*= ""*/, bool show_edit_btn /*= true*/)
{
    return this->slider_float(label.c_str(), v, v_min, v_max, format, power, clamp, tooltip, show_edit_btn);
}

bool ImGuiWrapper::slider_float(const wxString& label, float* v, float v_min, float v_max, const char* format/* = "%.3f"*/, float power/* = 1.0f*/, bool clamp /*= true*/, const wxString& tooltip /*= ""*/, bool show_edit_btn /*= true*/)
{
    auto label_utf8 = into_u8(label);
    return this->slider_float(label_utf8.c_str(), v, v_min, v_max, format, power, clamp, tooltip, show_edit_btn);
}

static bool image_button_ex(ImGuiID id, ImTextureID texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec2& padding, const ImVec4& bg_col, const ImVec4& tint_col, ImGuiButtonFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImGui::RenderNavHighlight(bb, id);
    ImGui::RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, g.Style.FrameRounding));
    if (bg_col.w > 0.0f)
        window->DrawList->AddRectFilled(bb.Min + padding, bb.Max - padding, ImGui::GetColorU32(bg_col));
    window->DrawList->AddImage(texture_id, bb.Min + padding, bb.Max - padding, uv0, uv1, ImGui::GetColorU32(tint_col));

    return pressed;
}

bool ImGuiWrapper::image_button(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col, ImGuiButtonFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return false;

    // Default to using texture ID as ID. User can still push string/integer prefixes.
    ImGui::PushID((void*)(intptr_t)user_texture_id);
    const ImGuiID id = window->GetID("#image");
    ImGui::PopID();

    const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : g.Style.FramePadding;
    return image_button_ex(id, user_texture_id, size, uv0, uv1, padding, bg_col, tint_col, flags);
}

bool ImGuiWrapper::image_button(const wchar_t icon, const wxString& tooltip)
{
    const ImGuiIO& io = ImGui::GetIO();
    const ImTextureID tex_id = io.Fonts->TexID;
    assert(io.Fonts->TexWidth > 0 && io.Fonts->TexHeight > 0);
    const float inv_tex_w = 1.0f / float(io.Fonts->TexWidth);
    const float inv_tex_h = 1.0f / float(io.Fonts->TexHeight);
    const ImFontAtlasCustomRect* const rect = GetTextureCustomRect(icon);
    const ImVec2 size = { float(rect->Width), float(rect->Height) };
    const ImVec2 uv0 = ImVec2(float(rect->X) * inv_tex_w, float(rect->Y) * inv_tex_h);
    const ImVec2 uv1 = ImVec2(float(rect->X + rect->Width) * inv_tex_w, float(rect->Y + rect->Height) * inv_tex_h);
    ImGui::PushStyleColor(ImGuiCol_Button, { 0.25f, 0.25f, 0.25f, 0.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.4f, 0.4f, 0.4f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.25f, 0.25f, 0.25f, 1.0f });
    const bool res = image_button(tex_id, size, uv0, uv1);
    ImGui::PopStyleColor(3);

    if (!tooltip.empty() && ImGui::IsItemHovered())
        this->tooltip(tooltip, ImGui::GetFontSize() * 20.0f);

    return res;
}
bool ImGuiWrapper::ACIMButton(const char *label, const ImVec2 &size_arg, ImGuiButtonFlags flags)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &    g          = *GImGui;
    const ImGuiStyle &style      = g.Style;
    const ImGuiID     id         = window->GetID(label);
    const ImVec2      label_size = ImGui::CalcTextSize(label, NULL, true);

    // if size_arg is not empty, use size_arg calc padding
    bool   isSizeArgEmpty = size_arg.x == 0 && size_arg.y == 0;
    ImVec2 buttonPadding  = !isSizeArgEmpty ? ImVec2((size_arg.x - label_size.x) / 2, (size_arg.y - label_size.y) / 2) : style.FramePadding;

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) &&
        buttonPadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that
                                                             // text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - buttonPadding.y;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + buttonPadding.x * 2.0f, label_size.y + buttonPadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, buttonPadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImGui::RenderNavHighlight(bb, id);
    ImGui::RenderFrame(bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), col, true, m_button_radius);

    if (g.LogEnabled)
        ImGui::LogSetNextTextDecoration("[", "]");
    ImGui::RenderTextClipped(bb.Min + buttonPadding, bb.Max - buttonPadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

    // Automatically close popups
    // if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
    //    CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
    return pressed;
}

bool ImGuiWrapper::ACIMButton(bool isLeft,const char *label, const ImVec2 &size_arg, ImGuiButtonFlags flags)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &    g          = *GImGui;
    const ImGuiStyle &style      = g.Style;
    const ImGuiID     id         = window->GetID(label);
    const ImVec2      label_size = ImGui::CalcTextSize(label, NULL, true);

    // if size_arg is not empty, use size_arg calc padding
    bool   isSizeArgEmpty = size_arg.x == 0 && size_arg.y == 0;
    ImVec2 buttonPadding  = !isSizeArgEmpty ? ImVec2((size_arg.x - label_size.x) / 2, (size_arg.y - label_size.y) / 2) : style.FramePadding;

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) &&
        buttonPadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that
                                                             // text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - buttonPadding.y;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + buttonPadding.x * 2.0f, label_size.y + buttonPadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, buttonPadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImGui::RenderNavHighlight(bb, id);
    ImGui::RenderFrame(bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), col, true, m_button_radius);

    if (g.LogEnabled)
        ImGui::LogSetNextTextDecoration("[", "]");
    if (isLeft) {
        ImGui::RenderTextClipped(bb.Min + buttonPadding, bb.Max - buttonPadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    } else {
        ImGui::RenderTextClipped(bb.Min + buttonPadding, bb.Max - buttonPadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    }

    // Automatically close popups
    // if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
    //    CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
    return pressed;
}



bool ImGuiWrapper::ACIMButton(ImTextureID &_id,const char *label, const ImVec2 &size_arg, ImGuiButtonFlags flags)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    // if size_arg is not empty, use size_arg calc padding
    bool isSizeArgEmpty = size_arg.x == 0 && size_arg.y == 0;
    ImVec2 buttonPadding = !isSizeArgEmpty ? ImVec2((size_arg.x-label_size.x)/2, (size_arg.y-label_size.y)/2) : style.FramePadding;

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && buttonPadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - buttonPadding.y;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + buttonPadding.x * 2.0f, label_size.y + buttonPadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, buttonPadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImGui::RenderNavHighlight(bb, id);
    ImGui::RenderFrame(bb.Min+ImVec2(1,1), bb.Max-ImVec2(1,1), col, true, m_button_radius);

    if (g.LogEnabled)
        ImGui::LogSetNextTextDecoration("[", "]");
    ImGui::RenderTextClipped(bb.Min + buttonPadding, bb.Max - buttonPadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

    ImVec2 _padding = style.FramePadding;
    ImVec2 _spacing = style.ItemInnerSpacing;
    ImVec2  icoSize(20.0f * get_style_scaling(), 14.0f * get_style_scaling());
    float   frame_height = ImGui::GetFrameHeight();
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - _padding.x - icoSize.x - _spacing.x);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (frame_height - icoSize.y + _padding.y / 2) / 2);
    ImGui::Image(ImTextureID(_id), icoSize);

    // Automatically close popups
    //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
    //    CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
    return pressed;
}

//
//bool ImGuiWrapper::ACIMCheckbox(const char *label,bool *v, const float label_offset_x,const float label_offset_y)
//{
//    ImGuiWindow* window = ImGui::GetCurrentWindow();
//    if (window->SkipItems)
//        return false;
//
//    ImGuiContext& g = *GImGui;
//    const ImGuiStyle& style = g.Style;
//    const ImGuiID id = window->GetID(label);
//    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
//
//    const float square_sz = 16; // ImGui::GetFrameHeight();
//    const ImVec2 pos = window->DC.CursorPos;
//    const ImRect total_bb(pos, pos + ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
//    ImGui::ItemSize(total_bb, style.FramePadding.y);
//    if (!ImGui::ItemAdd(total_bb, id))
//    {
//        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
//        return false;
//    }
//
//    bool hovered, held;
//    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
//    if (pressed)
//    {
//        *v = !(*v);
//        ImGui::MarkItemEdited(id);
//    }
//
//    const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));
//    ImGui::RenderNavHighlight(total_bb, id);
//    ImGui::RenderFrame(check_bb.Min, check_bb.Max, ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), false, m_checkbox_radius);
//    ImU32 check_col = ImGui::GetColorU32(ImGuiCol_CheckMark);
//    bool mixed_value = (g.CurrentItemFlags & ImGuiItemFlags_MixedValue) != 0;
//    if (mixed_value)
//    {
//        // Undocumented tristate/mixed/indeterminate checkbox (#2644)
//        // This may seem awkwardly designed because the aim is to make ImGuiItemFlags_MixedValue supported by all widgets (not just checkbox)
//        ImVec2 pad(ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)), ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)));
//        window->DrawList->AddRectFilled(check_bb.Min + pad, check_bb.Max - pad, check_col, style.FrameRounding);
//    }
//    else if (*v)
//    {
//        const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
//        ImGui::RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_col, square_sz - pad * 2.0f);
//    }
//
//    ImVec2 label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x +label_offset_x,check_bb.Min.y + style.FramePadding.y + label_offset_y);
//    if (g.LogEnabled)
//        ImGui::LogRenderedText(&label_pos, mixed_value ? "[~]" : *v ? "[x]" : "[ ]");
//    if (label_size.x > 0.0f)
//        ImGui::RenderText(label_pos, label);
//
//    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
//    return pressed;
//}

bool ImGuiWrapper::combo( ImTextureID &_id, const wxString &label, const std::vector<std::string> &options, int &selection, float itemWidth, ImGuiComboFlags flags)
{
    // this is to force the label to the left of the widget:
    if (!label.empty() && !label.StartsWith("##")) {
        text(label);
        ImGui::SameLine();
    }

    ImGuiContext &    g        = *GImGui;
    const ImGuiStyle &style    = g.Style;
    ImVec2            _padding = style.FramePadding;
    ImVec2            _spacing = style.ItemInnerSpacing;

    int  selection_out = selection;
    bool res           = false;
    if (!(flags & ImGuiComboFlags_NoArrowButton)) {
        flags |= ImGuiComboFlags_NoArrowButton;
    }
    const char *selection_str = selection < int(options.size()) && selection >= 0 ? options[selection].c_str() : "";
    ImFont *    font          = ImGui::GetFont();
    float       font_size     = font->FontSize;
    ImVec2      icoSize(1.34f * font_size, font_size);
    ImVec2      text_size       = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, selection_str);
    float       last_item_width = ImGui::CalcItemWidth();

    if (ACBeginCombo(label.c_str(), selection_str, flags, icoSize.x + _padding.x + _spacing.x, itemWidth)) {
        for (int i = 0; i < (int) options.size(); i++) {
            if (ImGui::Selectable(options[i].c_str(), i == selection, 0, ImVec2(itemWidth > 0.0f ? itemWidth:0.0f,0.0f))) {
                selection_out = i;
            }
            if (ImGui::IsItemHovered() && itemWidth > 0) {
                ImGui::PushStyleColor(ImGuiCol_PopupBg, {.0f, .0f, .0f, 1.0f});
                ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 1.0f, 1.0f, 1.0f});
                this->tooltip(options[i].c_str(), itemWidth);
                ImGui::PopStyleColor(2);
            }
        }
        ImGui::EndCombo();
        res = true;
    }

    float frame_height = ImGui::GetFrameHeight();
    // ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 20);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - _padding.x - icoSize.x - _spacing.x);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (frame_height - icoSize.y) / 2);
    ImGui::Image(ImTextureID(_id), icoSize);
    selection = selection_out;
    return res;
}

//bool ImGuiWrapper::combo(const wxString& label, const std::vector<std::string>& options, int& selection, ImGuiComboFlags flags/* = 0*/, float label_width/* = 0.0f*/, float item_width/* = 0.0f*/)
//{
//    return combo(into_u8(label), options, selection, flags, label_width, item_width);
//}

bool ImGuiWrapper::combo(const std::string& label, const std::vector<std::string>& options, int& selection, ImGuiComboFlags flags/* = 0*/, float label_width/* = 0.0f*/, float item_width/* = 0.0f*/)
{
    // this is to force the label to the left of the widget:
    if (!label.empty()) {
        text(label);
        ImGui::SameLine(label_width);
    }
    ImGui::PushItemWidth(item_width);

    int selection_out = selection;
    bool res = false;

    const char *selection_str = selection < int(options.size()) && selection >= 0 ? options[selection].c_str() : "";
    if (ImGui::BeginCombo(("##" + label).c_str(), selection_str, flags)) {
        for (int i = 0; i < (int)options.size(); i++) {
            if (ImGui::Selectable(options[i].c_str(), i == selection)) {
                selection_out = i;
                res = true;
            }
        }

        ImGui::EndCombo();
    }

    selection = selection_out;
    return res;
}


bool ImGuiWrapper::combo(const wxString& label, const std::vector<std::string>& options, int& selection, ImGuiComboFlags flags)
{
    // this is to force the label to the left of the widget:
    if (!label.empty()&& !label.StartsWith("##")) {
        text(label);
        ImGui::SameLine();
    }

    int selection_out = selection;
    bool res = false;

    const char *selection_str = selection < int(options.size()) && selection >= 0 ? options[selection].c_str() : "";
    if (ImGui::BeginCombo(into_u8(label).c_str(), selection_str, flags)) {
        for (int i = 0; i < (int)options.size(); i++) {
            if (ImGui::Selectable(options[i].c_str(), i == selection)) {
                selection_out = i;
            }
        }

        ImGui::EndCombo();
        res = true;
    }

    selection = selection_out;
    return res;
}

static float CalcMaxPopupHeightFromItemCount(int items_count)
{
    ImGuiContext &g = *GImGui;
    if (items_count <= 0)
        return FLT_MAX;
    return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
}

bool ImGuiWrapper::ACBeginCombo(const char *label, const char *preview_value, ImGuiComboFlags flags, float arrowWidth, float itemWidth)
{
    // Always consume the SetNextWindowSizeConstraint() call in our early return paths
    ImGuiContext &g                          = *GImGui;
    bool          has_window_size_constraint = (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint) != 0;
    g.NextWindowData.Flags &= ~ImGuiNextWindowDataFlags_HasSizeConstraint;

    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) !=
              (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together

    const ImGuiStyle &style = g.Style;
    const ImGuiID     id    = window->GetID(label);

    const float  arrow_size = (flags & ImGuiComboFlags_NoArrowButton) ? arrowWidth : ImGui::GetFrameHeight();
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    const float  expected_w = ImGui::CalcItemWidth();
    const float  w          = (flags & ImGuiComboFlags_NoPreview) ? arrow_size : expected_w;
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id, &frame_bb))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(frame_bb, id, &hovered, &held);

    if (hovered && itemWidth > 0) {
        ImGui::PushStyleColor(ImGuiCol_PopupBg, {.0f, .0f, .0f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 1.0f, 1.0f, 1.0f});
        this->tooltip(preview_value, itemWidth);
        ImGui::PopStyleColor(2);
    }


    const ImGuiID popup_id   = ImHashStr("##ComboPopup", 0, id);
    bool          popup_open = ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None);

    const ImU32 frame_col = ImGui::GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    const float value_x2  = ImMax(frame_bb.Min.x, frame_bb.Max.x - arrow_size);
    ImGui::RenderNavHighlight(frame_bb, id);
    if (!(flags & ImGuiComboFlags_NoPreview))
        window->DrawList->AddRectFilled(frame_bb.Min, ImVec2(value_x2, frame_bb.Max.y), frame_col, style.FrameRounding,
                                        (flags & ImGuiComboFlags_NoArrowButton) ? ImDrawFlags_RoundCornersAll :
                                                                                  ImDrawFlags_RoundCornersLeft);
    if (!(flags & ImGuiComboFlags_NoArrowButton)) {
        ImU32 bg_col   = ImGui::GetColorU32((popup_open || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);
        window->DrawList->AddRectFilled(ImVec2(value_x2, frame_bb.Min.y), frame_bb.Max, bg_col, style.FrameRounding,
                                        (w <= arrow_size) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersRight);
        if (value_x2 + arrow_size - style.FramePadding.x <= frame_bb.Max.x)
            ImGui::RenderArrow(window->DrawList, ImVec2(value_x2 + style.FramePadding.y, frame_bb.Min.y + style.FramePadding.y), text_col,
                        ImGuiDir_Down, 1.0f);
    }
    ImGui::RenderFrameBorder(frame_bb.Min, frame_bb.Max, style.FrameRounding);
    if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview)) {
        ImVec2 preview_pos = frame_bb.Min + style.FramePadding;
        if (g.LogEnabled)
            ImGui::LogSetNextTextDecoration("{", "}");
        ImGui::RenderTextClipped(preview_pos, ImVec2(value_x2, frame_bb.Max.y), preview_value, NULL, NULL, ImVec2(0.0f, 0.0f));
    }
    if (label_size.x > 0)
        ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

    if ((pressed || g.NavActivateId == id) && !popup_open) {
        if (window->DC.NavLayerCurrent == 0)
            window->NavLastIds[0] = id;
        ImGui::OpenPopupEx(popup_id, ImGuiPopupFlags_None);
        popup_open = true;
    }

    if (!popup_open)
        return false;

    if (has_window_size_constraint) {
        g.NextWindowData.Flags |= ImGuiNextWindowDataFlags_HasSizeConstraint;
        g.NextWindowData.SizeConstraintRect.Min.x = ImMax(g.NextWindowData.SizeConstraintRect.Min.x, w);
    } else {
        if ((flags & ImGuiComboFlags_HeightMask_) == 0)
            flags |= ImGuiComboFlags_HeightRegular;
        IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiComboFlags_HeightMask_)); // Only one
        int popup_max_height_in_items = -1;
        if (flags & ImGuiComboFlags_HeightRegular)
            popup_max_height_in_items = 8;
        else if (flags & ImGuiComboFlags_HeightSmall)
            popup_max_height_in_items = 4;
        else if (flags & ImGuiComboFlags_HeightLarge)
            popup_max_height_in_items = 20;
        ImGui::SetNextWindowSizeConstraints(ImVec2(w, 0.0f), ImVec2(FLT_MAX,CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));
    }

    char name[16];
    ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size); // Recycle windows based on depth

    // Position the window given a custom constraint (peak into expected window size so we can position it)
    // This might be easier to express with an hypothetical SetNextWindowPosConstraints() function.
    if (ImGuiWindow *popup_window = ImGui::FindWindowByName(name))
        if (popup_window->WasActive) {
            // Always override 'AutoPosLastDirection' to not leave a chance for a past value to affect us.
            ImVec2 size_expected = ImGui::CalcWindowNextAutoFitSize(popup_window);
            if (flags & ImGuiComboFlags_PopupAlignLeft)
                popup_window->AutoPosLastDirection = ImGuiDir_Left; // "Below, Toward Left"
            else
                popup_window->AutoPosLastDirection = ImGuiDir_Down; // "Below, Toward Right (default)"
            ImRect r_outer = ImGui::GetWindowAllowedExtentRect(popup_window);
            ImVec2 pos = ImGui::FindBestWindowPosForPopupEx(frame_bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer,
                                                     frame_bb, ImGuiPopupPositionPolicy_ComboBox);
            ImGui::SetNextWindowPos(pos);
        }

    // We don't use BeginPopupEx() solely because we have a custom name string, which we could make an argument to BeginPopupEx()
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

    // Horizontally align ourselves with the framed text
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.FramePadding.x, style.WindowPadding.y));
    bool ret = ImGui::Begin(name, NULL, window_flags);
    ImGui::PopStyleVar();
    if (!ret) {
        ImGui::EndPopup();
        IM_ASSERT(0); // This should never happen as we tested for IsPopupOpen() above
        return false;
    }
    return true;
}


// Scroll up for one item
static void scroll_up()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    float item_size_y = window->DC.PrevLineSize.y + g.Style.ItemSpacing.y;
    float win_top = window->Scroll.y;

    ImGui::SetScrollY(win_top - item_size_y);
}

// Scroll down for one item 
static void scroll_down()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    float item_size_y = window->DC.PrevLineSize.y + g.Style.ItemSpacing.y;
    float win_top = window->Scroll.y;

    ImGui::SetScrollY(win_top + item_size_y);
}

static void process_mouse_wheel(int& mouse_wheel)
{
    if (mouse_wheel > 0)
        scroll_up();
    else if (mouse_wheel < 0)
        scroll_down();
    mouse_wheel = 0;
}

bool ImGuiWrapper::undo_redo_list(const ImVec2& size, const bool is_undo, bool (*items_getter)(const bool , int , const char**), int& hovered, int& selected, int& mouse_wheel)
{
    bool is_hovered = false;
    ImGui::ListBoxHeader("", size);

    int i=0;
    const char* item_text;
    while (items_getter(is_undo, i, &item_text)) {
        ImGui::Selectable(item_text, i < hovered);

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", item_text);
            hovered = i;
            is_hovered = true;
        }

        if (ImGui::IsItemClicked())
            selected = i;
        i++;
    }

    if (is_hovered)
        process_mouse_wheel(mouse_wheel);

    ImGui::ListBoxFooter();
    return is_hovered;
}

// It's a copy of IMGui::Selactable function.
// But a little beat modified to change a label text.
// If item is hovered we should use another color for highlighted letters.
// To do that we push a ColorMarkerHovered symbol at the very beginning of the label
// This symbol will be used to a color selection for the highlighted letters.
// see imgui_draw.cpp, void ImFont::RenderText()
static bool selectable(const char* label, bool selected, ImGuiSelectableFlags flags = 0, const ImVec2& size_arg = ImVec2(0, 0))
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
    ImGuiID id = window->GetID(label);
    ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ImGui::ItemSize(size, 0.0f);

    // Fill horizontal space
    // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not visibly match other widgets.
    const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
    const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
    const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
        size.x = ImMax(label_size.x, max_x - min_x);

    // Text stays at the submission position, but bounding box may be extended on both sides
    const ImVec2 text_min = pos;
    const ImVec2 text_max(min_x + size.x, pos.y + size.y);

    // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
    ImRect bb(min_x, pos.y, text_max.x, text_max.y);
    if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
    {
        const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
        const float spacing_y = style.ItemSpacing.y;
        const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
        const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
        bb.Min.x -= spacing_L;
        bb.Min.y -= spacing_U;
        bb.Max.x += (spacing_x - spacing_L);
        bb.Max.y += (spacing_y - spacing_U);
    }
    //if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

    // Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackground for every Selectable..
    const float backup_clip_rect_min_x = window->ClipRect.Min.x;
    const float backup_clip_rect_max_x = window->ClipRect.Max.x;
    if (span_all_columns)
    {
        window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
        window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
    }

    bool item_add;
    if (flags & ImGuiSelectableFlags_Disabled)
    {
        ImGuiItemFlags backup_item_flags = g.CurrentItemFlags;
        g.CurrentItemFlags |= ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNavDefaultFocus;
        item_add = ImGui::ItemAdd(bb, id);
        g.CurrentItemFlags = backup_item_flags;
    }
    else
    {
        item_add = ImGui::ItemAdd(bb, id);
    }

    if (span_all_columns)
    {
        window->ClipRect.Min.x = backup_clip_rect_min_x;
        window->ClipRect.Max.x = backup_clip_rect_max_x;
    }

    if (!item_add)
        return false;

    // FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render only,
    // which would be advantageous since most selectable are not selected.
    if (span_all_columns && window->DC.CurrentColumns)
        ImGui::PushColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        ImGui::TablePushBackgroundChannel();

    // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
    if (flags & ImGuiSelectableFlags_SelectOnClick)     { button_flags |= ImGuiButtonFlags_PressedOnClick; }
    if (flags & ImGuiSelectableFlags_SelectOnRelease)   { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
    if (flags & ImGuiSelectableFlags_Disabled)          { button_flags |= ImGuiButtonFlags_Disabled; }
    if (flags & ImGuiSelectableFlags_AllowDoubleClick)  { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
    if (flags & ImGuiSelectableFlags_AllowItemOverlap)  { button_flags |= ImGuiButtonFlags_AllowItemOverlap; }

    if (flags & ImGuiSelectableFlags_Disabled)
        selected = false;

    const bool was_selected = selected;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, button_flags);

    // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
    {
        if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
        {
            ImGui::SetNavID(id, window->DC.NavLayerCurrent, window->DC.NavFocusScopeIdCurrent, ImRect(bb.Min - window->Pos, bb.Max - window->Pos));
            g.NavDisableHighlight = true;
        }
    }
    if (pressed)
        ImGui::MarkItemEdited(id);

    if (flags & ImGuiSelectableFlags_AllowItemOverlap)
        ImGui::SetItemAllowOverlap();

    // In this branch, Selectable() cannot toggle the selection so this will never trigger.
    if (selected != was_selected) //-V547
        window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    // Render
    if (held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld))
        hovered = true;
    if (hovered || selected)
    {
        const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
        ImGui::RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
        ImGui::RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
    }

    if (span_all_columns && window->DC.CurrentColumns)
        ImGui::PopColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        ImGui::TablePopBackgroundChannel();

    // mark a label with a ColorMarkerHovered, if item is hovered
    char marked_label[512]; //255 symbols is not enough for translated string (e.t. to Russian)
    if (hovered)
        sprintf(marked_label, "%c%s", ImGui::ColorMarkerHovered, label);
    else
        strcpy(marked_label, label);

    if (flags & ImGuiSelectableFlags_Disabled) ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
    ImGui::RenderTextClipped(text_min, text_max, marked_label, NULL, &label_size, style.SelectableTextAlign, &bb);
    if (flags & ImGuiSelectableFlags_Disabled) ImGui::PopStyleColor();

    // Automatically close popups
    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(g.CurrentItemFlags & ImGuiItemFlags_SelectableDontClosePopup))
        ImGui::CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
    return pressed;
}

bool begin_menu(const char *label, bool enabled)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext &    g            = *GImGui;
    const ImGuiStyle &style        = g.Style;
    const ImGuiID     id           = window->GetID(label);
    bool              menu_is_open = ImGui::IsPopupOpen(id, ImGuiPopupFlags_None);

    // Sub-menus are ChildWindow so that mouse can be hovering across them (otherwise top-most popup menu would steal focus and not allow hovering on parent menu)
    ImGuiWindowFlags flags = ImGuiWindowFlags_ChildMenu | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus;
    if (window->Flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_ChildMenu)) flags |= ImGuiWindowFlags_ChildWindow;

    // If a menu with same the ID was already submitted, we will append to it, matching the behavior of Begin().
    // We are relying on a O(N) search - so O(N log N) over the frame - which seems like the most efficient for the expected small amount of BeginMenu() calls per frame.
    // If somehow this is ever becoming a problem we can switch to use e.g. ImGuiStorage mapping key to last frame used.
    if (g.MenusIdSubmittedThisFrame.contains(id)) {
        if (menu_is_open)
            menu_is_open = ImGui::BeginPopupEx(id, flags); // menu_is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
        else
            g.NextWindowData.ClearFlags(); // we behave like Begin() and need to consume those values
        return menu_is_open;
    }

    // Tag menu as used. Next time BeginMenu() with same ID is called it will append to existing menu
    g.MenusIdSubmittedThisFrame.push_back(id);

    ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    bool   pressed;
    bool   menuset_is_open = !(window->Flags & ImGuiWindowFlags_Popup) &&
                           (g.OpenPopupStack.Size > g.BeginPopupStack.Size && g.OpenPopupStack[g.BeginPopupStack.Size].OpenParentId == window->IDStack.back());
    ImGuiWindow *backed_nav_window = g.NavWindow;
    if (menuset_is_open) g.NavWindow = window; // Odd hack to allow hovering across menus of a same menu-set (otherwise we wouldn't be able to hover parent)

    // The reference position stored in popup_pos will be used by Begin() to find a suitable position for the child menu,
    // However the final position is going to be different! It is chosen by FindBestWindowPosForPopup().
    // e.g. Menus tend to overlap each other horizontally to amplify relative Z-ordering.
    ImVec2 popup_pos, pos = window->DC.CursorPos;
    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal) {
        // Menu inside an horizontal menu bar
        // Selectable extend their highlight by half ItemSpacing in each direction.
        // For ChildMenu, the popup position will be overwritten by the call to FindBestWindowPosForPopup() in Begin()
        popup_pos = ImVec2(pos.x - 1.0f - IM_FLOOR(style.ItemSpacing.x * 0.5f), pos.y - style.FramePadding.y + window->MenuBarHeight());
        window->DC.CursorPos.x += IM_FLOOR(style.ItemSpacing.x * 0.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x * 2.0f, style.ItemSpacing.y));
        float w = label_size.x;
        pressed = selectable(label, menu_is_open,
                             ImGuiSelectableFlags_NoHoldingActiveID | ImGuiSelectableFlags_SelectOnClick | ImGuiSelectableFlags_DontClosePopups |
                                 (!enabled ? ImGuiSelectableFlags_Disabled : 0),
                             ImVec2(w, 0.0f));
        ImGui::PopStyleVar();
        window->DC.CursorPos.x += IM_FLOOR(
            style.ItemSpacing.x *
            (-1.0f +
             0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
    } else {
        // Menu inside a menu
        // (In a typical menu window where all items are BeginMenu() or MenuItem() calls, extra_w will always be 0.0f.
        //  Only when they are other items sticking out we're going to add spacing, yet only register minimum width into the layout system.
        popup_pos      = ImVec2(pos.x, pos.y - style.WindowPadding.y);
        float min_w    = window->DC.MenuColumns.DeclColumns(label_size.x, 0.0f, IM_FLOOR(g.FontSize * 1.20f)); // Feedback to next frame
        float extra_w  = ImMax(0.0f, ImGui::GetContentRegionAvail().x - min_w);
        pressed        = selectable(label, menu_is_open,
                             ImGuiSelectableFlags_NoHoldingActiveID | ImGuiSelectableFlags_SelectOnClick | ImGuiSelectableFlags_DontClosePopups |
                                 ImGuiSelectableFlags_SpanAvailWidth | (!enabled ? ImGuiSelectableFlags_Disabled : 0),
                             ImVec2(min_w, 0.0f));
        ImU32 text_col = ImGui::GetColorU32(enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled);
        ImGui::RenderArrow(window->DrawList, pos + ImVec2(window->DC.MenuColumns.Pos[2] + extra_w + g.FontSize * 0.30f, 0.0f), text_col, ImGuiDir_Right);
    }

    const bool hovered = enabled && ImGui::ItemHoverable(window->DC.LastItemRect, id);
    if (menuset_is_open) g.NavWindow = backed_nav_window;

    bool want_open  = false;
    bool want_close = false;
    if (window->DC.LayoutType == ImGuiLayoutType_Vertical) // (window->Flags & (ImGuiWindowFlags_Popup|ImGuiWindowFlags_ChildMenu))
    {
        // Close menu when not hovering it anymore unless we are moving roughly in the direction of the menu
        // Implement http://bjk5.com/post/44698559168/breaking-down-amazons-mega-dropdown to avoid using timers, so menus feels more reactive.
        bool moving_toward_other_child_menu = false;

        ImGuiWindow *child_menu_window = (g.BeginPopupStack.Size < g.OpenPopupStack.Size && g.OpenPopupStack[g.BeginPopupStack.Size].SourceWindow == window) ?
                                             g.OpenPopupStack[g.BeginPopupStack.Size].Window :
                                             NULL;
        if (g.HoveredWindow == window && child_menu_window != NULL && !(window->Flags & ImGuiWindowFlags_MenuBar)) {
            // FIXME-DPI: Values should be derived from a master "scale" factor.
            ImRect next_window_rect = child_menu_window->Rect();
            ImVec2 ta               = g.IO.MousePos - g.IO.MouseDelta;
            ImVec2 tb               = (window->Pos.x < child_menu_window->Pos.x) ? next_window_rect.GetTL() : next_window_rect.GetTR();
            ImVec2 tc               = (window->Pos.x < child_menu_window->Pos.x) ? next_window_rect.GetBL() : next_window_rect.GetBR();
            float  extra            = ImClamp(ImFabs(ta.x - tb.x) * 0.30f, 5.0f, 30.0f); // add a bit of extra slack.
            ta.x += (window->Pos.x < child_menu_window->Pos.x) ? -0.5f : +0.5f;          // to avoid numerical issues
            tb.y = ta.y +
                   ImMax((tb.y - extra) - ta.y, -100.0f); // triangle is maximum 200 high to limit the slope and the bias toward large sub-menus // FIXME: Multiply by fb_scale?
            tc.y                           = ta.y + ImMin((tc.y + extra) - ta.y, +100.0f);
            moving_toward_other_child_menu = ImTriangleContainsPoint(ta, tb, tc, g.IO.MousePos);
            // GetForegroundDrawList()->AddTriangleFilled(ta, tb, tc, moving_within_opened_triangle ? IM_COL32(0,128,0,128) : IM_COL32(128,0,0,128)); // [DEBUG]
        }
        if (menu_is_open && !hovered && g.HoveredWindow == window && g.HoveredIdPreviousFrame != 0 && g.HoveredIdPreviousFrame != id && !moving_toward_other_child_menu)
            want_close = true;

        if (!menu_is_open && hovered && pressed) // Click to open
            want_open = true;
        else if (!menu_is_open && hovered && !moving_toward_other_child_menu) // Hover to open
            want_open = true;

        if (g.NavActivateId == id) {
            want_close = menu_is_open;
            want_open  = !menu_is_open;
        }
        if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Right) // Nav-Right to open
        {
            want_open = true;
            ImGui::NavMoveRequestCancel();
        }
    } else {
        // Menu bar
        if (menu_is_open && pressed && menuset_is_open) // Click an open menu again to close it
        {
            want_close = true;
            want_open = menu_is_open = false;
        } else if (pressed || (hovered && menuset_is_open && !menu_is_open)) // First click to open, then hover to open others
        {
            want_open = true;
        } else if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Down) // Nav-Down to open
        {
            want_open = true;
            ImGui::NavMoveRequestCancel();
        }
    }

    if (!enabled) // explicitly close if an open menu becomes disabled, facilitate users code a lot in pattern such as 'if (BeginMenu("options", has_object)) { ..use object.. }'
        want_close = true;
    if (want_close && ImGui::IsPopupOpen(id, ImGuiPopupFlags_None)) ImGui::ClosePopupToLevel(g.BeginPopupStack.Size, true);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags | ImGuiItemStatusFlags_Openable | (menu_is_open ? ImGuiItemStatusFlags_Opened : 0));

    if (!menu_is_open && want_open && g.OpenPopupStack.Size > g.BeginPopupStack.Size) {
        // Don't recycle same menu level in the same frame, first close the other menu and yield for a frame.
        ImGui::OpenPopup(label);
        return false;
    }

    menu_is_open |= want_open;
    if (want_open) ImGui::OpenPopup(label);

    if (menu_is_open) {
        ImGui::SetNextWindowPos(popup_pos,
                                ImGuiCond_Always);     // Note: this is super misleading! The value will serve as reference for FindBestWindowPosForPopup(), not actual pos.
        menu_is_open = ImGui::BeginPopupEx(id, flags); // menu_is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
    } else {
        g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
    }

    return menu_is_open;
}

void end_menu()
{
    ImGui::EndMenu();
}

bool menu_item_with_icon(const char *label, const char *shortcut, ImVec2 icon_size /* = ImVec2(0, 0)*/, ImU32 icon_color /* = 0*/, bool selected /* = false*/, bool enabled /* = true*/)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext &g          = *GImGui;
    ImGuiStyle &  style      = g.Style;
    ImVec2        pos        = window->DC.CursorPos;
    ImVec2        label_size = ImGui::CalcTextSize(label, NULL, true);

    // We've been using the equivalent of ImGuiSelectableFlags_SetNavIdOnHover on all Selectable() since early Nav system days (commit 43ee5d73),
    // but I am unsure whether this should be kept at all. For now moved it to be an opt-in feature used by menus only.
    ImGuiSelectableFlags flags = ImGuiSelectableFlags_SelectOnRelease | ImGuiSelectableFlags_SetNavIdOnHover | (enabled ? 0 : ImGuiSelectableFlags_Disabled);
    bool                 pressed;
    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal) {
        // Mimic the exact layout spacing of BeginMenu() to allow MenuItem() inside a menu bar, which is a little misleading but may be useful
        // Note that in this situation: we don't render the shortcut, we render a highlight instead of the selected tick mark.
        float w = label_size.x;
        window->DC.CursorPos.x += IM_FLOOR(style.ItemSpacing.x * 0.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x * 2.0f, style.ItemSpacing.y));
        pressed = ImGui::Selectable(label, selected, flags, ImVec2(w, 0.0f));
        ImGui::PopStyleVar();
        window->DC.CursorPos.x += IM_FLOOR(
            style.ItemSpacing.x *
            (-1.0f +
             0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
    } else {
        // Menu item inside a vertical menu
        // (In a typical menu window where all items are BeginMenu() or MenuItem() calls, extra_w will always be 0.0f.
        //  Only when they are other items sticking out we're going to add spacing, yet only register minimum width into the layout system.
        float shortcut_w = shortcut ? ImGui::CalcTextSize(shortcut, NULL).x : 0.0f;
        float min_w      = window->DC.MenuColumns.DeclColumns(label_size.x, shortcut_w, IM_FLOOR(g.FontSize * 1.20f)); // Feedback for next frame
        float extra_w    = std::max(0.0f, ImGui::GetContentRegionAvail().x - min_w);
        pressed          = selectable(label, false, flags | ImGuiSelectableFlags_SpanAvailWidth, ImVec2(min_w, 0.0f));

        if (icon_size.x != 0 && icon_size.y != 0) {
            float selectable_pos_y = pos.y + -0.5f * style.ItemSpacing.y;
            float icon_pos_y = selectable_pos_y + (label_size.y + style.ItemSpacing.y - icon_size.y) / 2;
            float icon_pos_x = pos.x + window->DC.MenuColumns.Pos[2] + extra_w + g.FontSize * 0.40f;
            ImVec2 icon_pos = ImVec2(icon_pos_x, icon_pos_y);
            ImGui::RenderFrame(icon_pos, icon_pos + icon_size, icon_color);
        }

        if (shortcut_w > 0.0f) {
            ImGui::PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_TextDisabled]);
            ImGui::RenderText(pos + ImVec2(window->DC.MenuColumns.Pos[1] + extra_w, 0.0f), shortcut, NULL, false);
            ImGui::PopStyleColor();
        }
        if (selected) {
            //ImGui::RenderCheckMark(window->DrawList, pos + ImVec2(window->DC.MenuColumns.Pos[2] + extra_w + g.FontSize * 0.40f, g.FontSize * 0.134f * 0.5f),
            //                       ImGui::GetColorU32(enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled), g.FontSize * 0.866f);
        }
    }

    IMGUI_TEST_ENGINE_ITEM_INFO(window->DC.LastItemId, label, window->DC.LastItemStatusFlags | ImGuiItemStatusFlags_Checkable | (selected ? ImGuiItemStatusFlags_Checked : 0));
    return pressed;
}

// Scroll so that the hovered item is at the top of the window
static void scroll_y(int hover_id)
{
    if (hover_id < 0)
        return;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    float item_size_y = window->DC.PrevLineSize.y + g.Style.ItemSpacing.y;
    float item_delta = 0.5 * item_size_y;

    float item_top = item_size_y * hover_id;
    float item_bottom = item_top + item_size_y;

    float win_top = window->Scroll.y;
    float win_bottom = window->Scroll.y + window->Size.y;

    if (item_bottom + item_delta >= win_bottom)
        ImGui::SetScrollY(win_top + item_size_y);
    else if (item_top - item_delta <= win_top)
        ImGui::SetScrollY(win_top - item_size_y);
}

// Use this function instead of ImGui::IsKeyPressed.
// ImGui::IsKeyPressed is related for *GImGui.IO.KeysDownDuration[user_key_index]
// And after first key pressing IsKeyPressed() return "true" always even if key wasn't pressed
static void process_key_down(ImGuiKey imgui_key, std::function<void()> f)
{
    if (ImGui::IsKeyDown(ImGui::GetKeyIndex(imgui_key)))
    {
        f();
        // set KeysDown to false to avoid redundant key down processing
        ImGuiContext& g = *GImGui;
        g.IO.KeysDown[ImGui::GetKeyIndex(imgui_key)] = false;
    }
}

void ImGuiWrapper::search_list(const ImVec2& size_, bool (*items_getter)(int, const char** label, const char** tooltip), char* search_str,
                               Search::OptionViewParameters& view_params, int& selected, bool& edited, int& mouse_wheel, bool is_localized)
{
    int& hovered_id = view_params.hovered_id;
    // ImGui::ListBoxHeader("", size);
    {   
        // rewrote part of function to add a TextInput instead of label Text
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return ;

        const ImGuiStyle& style = g.Style;

        // Size default to hold ~7 items. Fractional number of items helps seeing that we can scroll down/up without looking at scrollbar.
        ImVec2 size = ImGui::CalcItemSize(size_, ImGui::CalcItemWidth(), ImGui::GetTextLineHeightWithSpacing() * 7.4f + style.ItemSpacing.y);
        ImRect frame_bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + size.x, window->DC.CursorPos.y + size.y));

        ImRect bb(frame_bb.Min, frame_bb.Max);
        window->DC.LastItemRect = bb; // Forward storage for ListBoxFooter.. dodgy.
        g.NextItemData.ClearFlags();

        if (!ImGui::IsRectVisible(bb.Min, bb.Max))
        {
            ImGui::ItemSize(bb.GetSize(), style.FramePadding.y);
            ImGui::ItemAdd(bb, 0, &frame_bb);
            return ;
        }

        ImGui::BeginGroup();

        const ImGuiID id = ImGui::GetID(search_str);
        ImVec2 search_size = ImVec2(size.x, ImGui::GetTextLineHeightWithSpacing() + style.ItemSpacing.y);

        if (!ImGui::IsAnyItemFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
            ImGui::SetKeyboardFocusHere(0);

        // The press on Esc key invokes editing of InputText (removes last changes)
        // So we should save previous value...
        std::string str = search_str;
        ImGui::InputTextEx("", NULL, search_str, 240, search_size, ImGuiInputTextFlags_AutoSelectAll, NULL, NULL);
        edited = ImGui::IsItemEdited();
        if (edited)
            hovered_id = 0;

        process_key_down(ImGuiKey_Escape, [&selected, search_str, str]() {
            // use 9999 to mark selection as a Esc key
            selected = 9999;
            // ... and when Esc key was pressed, than revert search_str value
            strcpy(search_str, str.c_str());
        });

        ImGui::BeginChildFrame(id, frame_bb.GetSize());
    }

    int i = 0;
    const char* item_text;
    const char* tooltip;
    int mouse_hovered = -1;

    while (items_getter(i, &item_text, &tooltip))
    {
        selectable(item_text, i == hovered_id);

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", /*item_text*/tooltip);
                hovered_id = -1;
            mouse_hovered = i;
        }

        if (ImGui::IsItemClicked())
            selected = i;
        i++;
    }

    // Process mouse wheel
    if (mouse_hovered > 0)
        process_mouse_wheel(mouse_wheel);

    // process Up/DownArrows and Enter
    process_key_down(ImGuiKey_UpArrow, [&hovered_id, mouse_hovered]() {
        if (mouse_hovered > 0)
            scroll_up();
        else {
            if (hovered_id > 0)
                --hovered_id;
            scroll_y(hovered_id);
        }
    });

    process_key_down(ImGuiKey_DownArrow, [&hovered_id, mouse_hovered, i]() {
        if (mouse_hovered > 0)
            scroll_down();
        else {
            if (hovered_id < 0)
                hovered_id = 0;
            else if (hovered_id < i - 1)
                ++hovered_id;
            scroll_y(hovered_id);
        }
    });

    process_key_down(ImGuiKey_Enter, [&selected, hovered_id]() {
        selected = hovered_id;
    });

    ImGui::ListBoxFooter();

    auto check_box = [&edited, this](const wxString& label, bool& check) {
        ImGui::SameLine();
        bool ch = check;
        checkbox(label, ch);
        if (ImGui::IsItemClicked()) {
            check = !check;
            edited = true;
        }
    };

    ImGui::AlignTextToFramePadding();

    // add checkboxes for show/hide Categories and Groups
    text(_L("Use for search")+":");
    check_box(_L("Category"),   view_params.category);
    if (is_localized)
        check_box(_L("Search in English"), view_params.english);
}

void ImGuiWrapper::bold_text(const std::string& str)
{
    if (bold_font){
        ImGui::PushFont(bold_font);
        text(str);
        ImGui::PopFont();
    } else {
        text(str);
    }
}

bool ImGuiWrapper::push_font_by_name(std::string font_name)
{
    auto sys_font = im_fonts_map.find(font_name);
    if (sys_font != im_fonts_map.end()) {
        ImFont* font = sys_font->second;
        if (font && font->ContainerAtlas && font->Glyphs.Size > 4)
            ImGui::PushFont(font);
        else {
            ImGui::PushFont(default_font);
        }
        return true;
    }
    return false;
}

bool ImGuiWrapper::pop_font_by_name(std::string font_name)
{
    auto sys_font = im_fonts_map.find(font_name);
    if (sys_font != im_fonts_map.end()) {
        ImGui::PopFont();
        return true;
    }
    return false;
}

void ImGuiWrapper::load_fonts_texture()
{
    //if (m_font_another_texture == 0) {
    //    ImGuiIO& io = ImGui::GetIO();
    //    io.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;
    //    ImFontConfig cfg = ImFontConfig();
    //    cfg.OversampleH = cfg.OversampleV = 1;
    //    std::map<std::string, std::string> sys_fonts_map = get_occt_fonts_maps(); // map<font name, font path>
    //    im_fonts_map.clear();                                                     // map<font name, ImFont*>
    //    BOOST_LOG_TRIVIAL(info) << "init_im_font start";
    //    for (auto sys_font : sys_fonts_map) {
    //        boost::filesystem::path font_path(sys_font.second);
    //        if (!boost::filesystem::exists(font_path)) {
    //            BOOST_LOG_TRIVIAL(trace) << "load font = " << sys_font.first << ", path = " << font_path << " is not exists";
    //            continue;
    //        }
    //        ImFont* im_font = io.Fonts->AddFontFromFileTTF(sys_font.second.c_str(), m_font_size, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesBasic());
    //        if (im_font == nullptr) {
    //            BOOST_LOG_TRIVIAL(trace) << "load font = " << sys_font.first << " failed, path = " << font_path << " is not exists";
    //            continue;
    //        }
    //        im_fonts_map.insert({ sys_font.first, im_font });
    //    }
    //    BOOST_LOG_TRIVIAL(info) << "init_im_font end";

    //    unsigned char* pixels;
    //    int            width, height;
    //    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    //    BOOST_LOG_TRIVIAL(trace) << "Build system fonts texture done. width: " << width << ", height: " << height;

    //    if (m_fonts_names.size() == 0) {
    //        std::vector<std::string> to_delete_fonts;
    //        for (auto im_font : im_fonts_map) {
    //            if (im_font.second->Glyphs.Size < 4) { to_delete_fonts.push_back(im_font.first); }
    //        }
    //        for (auto to_delete_font : to_delete_fonts) {
    //            sys_fonts_map.erase(to_delete_font);
    //            im_fonts_map.erase(to_delete_font);
    //        }
    //        for (auto im_font : im_fonts_map) m_fonts_names.push_back(im_font.first);
    //    }

    //    GLint last_texture;
    //    glsafe(::glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture));
    //    glsafe(::glGenTextures(1, &(m_font_another_texture)));
    //    glsafe(::glBindTexture(GL_TEXTURE_2D, m_font_another_texture));
    //    glsafe(::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    //    glsafe(::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    //    glsafe(::glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));

    //    glsafe(::glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

    //    // Store our identifier
    //    io.Fonts->TexID = (ImTextureID)(intptr_t)m_font_another_texture;

    //    // Restore state
    //    glsafe(::glBindTexture(GL_TEXTURE_2D, last_texture));
    //}
}

void ImGuiWrapper::destroy_fonts_texture() {
    //if (m_font_another_texture != 0) {
    //    if (m_new_frame_open) {
    //        render();
    //    }
    //    init_font(true);
    //    glsafe(::glDeleteTextures(1, &m_font_another_texture));
    //    m_font_another_texture = 0;
    //    if (!m_new_frame_open) {
    //        new_frame();
    //    }
    //}
}

void ImGuiWrapper::title(const std::string& str)
{
    text(str);
    ImGui::Separator();
}

void ImGuiWrapper::disabled_begin(bool disabled)
{
    wxCHECK_RET(!m_disabled, "ImGUI: Unbalanced disabled_begin() call");

    if (disabled) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        m_disabled = true;
    }
}

void ImGuiWrapper::disabled_end()
{
    if (m_disabled) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
        m_disabled = false;
    }
}

bool ImGuiWrapper::want_mouse() const
{
    return ImGui::GetIO().WantCaptureMouse;
}

bool ImGuiWrapper::want_keyboard() const
{
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool ImGuiWrapper::want_text_input() const
{
    return ImGui::GetIO().WantTextInput;
}

bool ImGuiWrapper::want_any_input() const
{
    const auto io = ImGui::GetIO();
    return io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput;
}

ImFontAtlasCustomRect* ImGuiWrapper::GetTextureCustomRect(const wchar_t& tex_id)
{
    auto item = m_custom_glyph_rects_ids.find(tex_id);
    return (item != m_custom_glyph_rects_ids.end()) ? ImGui::GetIO().Fonts->GetCustomRectByIndex(m_custom_glyph_rects_ids[tex_id]) : nullptr;
}

void ImGuiWrapper::disable_background_fadeout_animation()
{
    GImGui->DimBgRatio = 1.0f;
}

ImU32 ImGuiWrapper::to_ImU32(const ColorRGBA& color)
{
    return ImGui::GetColorU32({ color.r(), color.g(), color.b(), color.a() });
}

ImVec4 ImGuiWrapper::to_ImVec4(const ColorRGBA& color)
{
    return { color.r(), color.g(), color.b(), color.a() };
}

ColorRGBA ImGuiWrapper::from_ImU32(const ImU32& color)
{
    return from_ImVec4(ImGui::ColorConvertU32ToFloat4(color));
}

ColorRGBA ImGuiWrapper::from_ImVec4(const ImVec4& color)
{
    return { color.x, color.y, color.z, color.w };
}

template <typename T, typename Func>
static bool input_optional(std::optional<T> &v, Func& f, std::function<bool(const T&)> is_default, const T& def_val)
{
    if (v.has_value()) {
        if (f(*v)) {
            if (is_default(*v)) v.reset();
            return true;
        }
    } else {
        T val = def_val;
        if (f(val)) {
            if (!is_default(val)) v = val;
            return true;
        }
    }
    return false;
}

bool ImGuiWrapper::input_optional_int(const char *        label,
                                      std::optional<int>& v,
                                      int                 step,
                                      int                 step_fast,
                                      ImGuiInputTextFlags flags,
                                      int                 def_val)
{
    auto func = [&](int &value) {
        return ImGui::InputInt(label, &value, step, step_fast, flags);
    };
    std::function<bool(const int &)> is_default =
        [def_val](const int &value) -> bool { return value == def_val; };
    return input_optional(v, func, is_default, def_val);
}

bool ImGuiWrapper::input_optional_float(const char *          label,
                                        std::optional<float> &v,
                                        float                 step,
                                        float                 step_fast,
                                        const char *          format,
                                        ImGuiInputTextFlags   flags,
                                        float                 def_val)
{
    auto func = [&](float &value) {
        return ImGui::InputFloat(label, &value, step, step_fast, format, flags);
    };
    std::function<bool(const float &)> is_default =
        [def_val](const float &value) -> bool {
        return std::fabs(value-def_val) <= std::numeric_limits<float>::epsilon();
    };
    return input_optional(v, func, is_default, def_val);
}

bool ImGuiWrapper::drag_optional_float(const char *          label,
                                       std::optional<float> &v,
                                       float                 v_speed,
                                       float                 v_min,
                                       float                 v_max,
                                       const char *          format,
                                       float                 power,
                                       float                 def_val)
{
    auto func = [&](float &value) {
        return ImGui::DragFloat(label, &value, v_speed, v_min, v_max, format, power);
    };
    std::function<bool(const float &)> is_default =
        [def_val](const float &value) -> bool {
        return std::fabs(value-def_val) <= std::numeric_limits<float>::epsilon();
    };
    return input_optional(v, func, is_default, def_val);
}

bool ImGuiWrapper::slider_optional_float(const char           *label,
                                         std::optional<float> &v,
                                         float                 v_min,
                                         float                 v_max,
                                         const char           *format,
                                         float                 power,
                                         bool                  clamp,
                                         const wxString       &tooltip,
                                         bool                  show_edit_btn,
                                         float                 def_val)
{
    auto func = [&](float &value) {
        return slider_float(label, &value, v_min, v_max, format, power, clamp, tooltip, show_edit_btn);
    };
    std::function<bool(const float &)> is_default =
        [def_val](const float &value) -> bool {
        return std::fabs(value - def_val) <= std::numeric_limits<float>::epsilon();
    };
    return input_optional(v, func, is_default, def_val);
}

bool ImGuiWrapper::slider_optional_int(const char         *label,
                                       std::optional<int> &v,
                                       int                 v_min,
                                       int                 v_max,
                                       const char         *format,
                                       float               power,
                                       bool                clamp,
                                       const wxString     &tooltip,
                                       bool                show_edit_btn,
                                       int                 def_val)
{
    std::optional<float> val;
    if (v.has_value()) val = static_cast<float>(*v);
    auto func = [&](float &value) {
        return slider_float(label, &value, v_min, v_max, format, power, clamp, tooltip, show_edit_btn);
    };
    std::function<bool(const float &)> is_default =
        [def_val](const float &value) -> bool {
        return std::fabs(value - def_val) < 0.9f;
    };

    float default_value = static_cast<float>(def_val);
    if (input_optional(val, func, is_default, default_value)) {
        if (val.has_value())
            v = static_cast<int>(std::round(*val));
        else
            v.reset();
        return true;
    } else return false;
}

void ImGuiWrapper::left_inputs() {
    ImGui::ClearActiveID();
}

std::string ImGuiWrapper::trunc(const std::string &text,
                                float              width,
                                const char *       tail)
{
    float text_width = ImGui::CalcTextSize(text.c_str()).x;
    if (text_width < width) return text;
    float tail_width = ImGui::CalcTextSize(tail).x;
    assert(width > tail_width);
    if (width <= tail_width) return "Error: Can't add tail and not be under wanted width.";
    float allowed_width = width - tail_width;

    // guess approx count of letter
    float average_letter_width = calc_text_size(std::string_view("n")).x; // average letter width
    unsigned count_letter  = static_cast<unsigned>(allowed_width / average_letter_width);

    std::string_view text_ = text;
    std::string_view result_text = text_.substr(0, count_letter);
    text_width = calc_text_size(result_text).x;
    if (text_width < allowed_width) {
        // increase letter count
        while (count_letter < text.length()) {
            ++count_letter;
            std::string_view act_text = text_.substr(0, count_letter);
            text_width = calc_text_size(act_text).x;
            if (text_width > allowed_width) break;
            result_text = act_text;
        }
    } else {
        // decrease letter count
        while (count_letter > 1) {
            --count_letter;
            result_text = text_.substr(0, count_letter);
            text_width  = calc_text_size(result_text).x;
            if (text_width < allowed_width) break;
        }
    }
    return std::string(result_text) + tail;
}

void ImGuiWrapper::escape_double_hash(std::string &text)
{
    // add space between hashes
    const std::string search  = "##";
    const std::string replace = "# #";
    size_t pos = 0;
    while ((pos = text.find(search, pos)) != std::string::npos)
        text.replace(pos, search.length(), replace);
}

ImVec2 ImGuiWrapper::suggest_location(const ImVec2 &dialog_size,
                                      const Slic3r::Polygon &interest,
                                      const ImVec2 &canvas_size)
{
    // IMPROVE 1: do not select place over menu
    // BoundingBox top_menu;
    // GLGizmosManager &gizmo_mng = canvas->get_gizmos_manager();
    // BoundingBox      side_menu; // gizmo_mng.get_size();
    // BoundingBox left_bottom_menu; // is permanent?
    // NotificationManager *notify_mng = plater->get_notification_manager();
    // BoundingBox          notifications; // notify_mng->get_size();
    // m_window_width, m_window_height + position

    // IMPROVE 2: use polygon of interest not only bounding box
    BoundingBox bb(interest.points);
    Point       center = bb.center(); // interest.centroid();

    // area size
    Point window_center(canvas_size.x / 2, canvas_size.y / 2);

    // mov on side
    Point bb_half_size = (bb.max - bb.min) / 2 + Point(1,1);
    Point diff_center  = window_center - center;
    Vec2d diff_norm(diff_center.x() / (double) bb_half_size.x(),
                    diff_center.y() / (double) bb_half_size.y());
    if (diff_norm.x() > 1.) diff_norm.x() = 1.;
    if (diff_norm.x() < -1.) diff_norm.x() = -1.;
    if (diff_norm.y() > 1.) diff_norm.y() = 1.;
    if (diff_norm.y() < -1.) diff_norm.y() = -1.;

    Vec2d abs_diff(abs(diff_norm.x()), abs(diff_norm.y()));
    if (abs_diff.x() < 1. && abs_diff.y() < 1.) {
        if (abs_diff.x() > abs_diff.y())
            diff_norm.x() = (diff_norm.x() < 0.) ? (-1.) : 1.;
        else
            diff_norm.y() = (diff_norm.y() < 0.) ? (-1.) : 1.;
    }

    Point half_dialog_size(dialog_size.x / 2., dialog_size.y / 2.);
    Point move_size       = bb_half_size + half_dialog_size;
    Point offseted_center = center - half_dialog_size;
    Vec2d offset(offseted_center.x() + diff_norm.x() * move_size.x(),
                 offseted_center.y() + diff_norm.y() * move_size.y());

    // move offset close to center
    Points window_polygon = {offset.cast<int>(),
                             Point(offset.x(), offset.y() + dialog_size.y),
                             Point(offset.x() + dialog_size.x,
                                   offset.y() + dialog_size.y),
                             Point(offset.x() + dialog_size.x, offset.y())};
    // check that position by Bounding box is not intersecting
    assert(Slic3r::intersection(interest, Polygon(window_polygon)).empty());

    double allowed_space = 10; // in px
    double allowed_space_sq = allowed_space * allowed_space;
    Vec2d  move_vec         = (center - (offset.cast<int>() + half_dialog_size))
                         .cast<double>();
    Vec2d result_move(0, 0);
    do {
        move_vec             = move_vec / 2.;
        Point  move_point    = (move_vec + result_move).cast<int>();
        Points moved_polygon = window_polygon; // copy
        for (Point &p : moved_polygon) p += move_point;
        if (Slic3r::intersection(interest, Polygon(moved_polygon)).empty())
            result_move += move_vec;

    } while (move_vec.squaredNorm() >= allowed_space_sq);
    offset += result_move;

    return ImVec2(offset.x(), offset.y());
}

void ImGuiWrapper::draw(
    const Polygon &polygon,
    ImDrawList *   draw_list /* = ImGui::GetOverlayDrawList()*/,
    ImU32          color     /* = ImGui::GetColorU32(COL_BLUE_LIGHT)*/,
    float          thickness /* = 3.f*/)
{
    // minimal one line consist of 2 points
    if (polygon.size() < 2) return;
    // need a place to draw
    if (draw_list == nullptr) return;

    const Point *prev_point = &polygon.points.back();
    for (const Point &point : polygon.points) {
        ImVec2 p1(prev_point->x(), prev_point->y());
        ImVec2 p2(point.x(), point.y());
        draw_list->AddLine(p1, p2, color, thickness);
        prev_point = &point;
    }
}

void ImGuiWrapper::draw_cross_hair(const ImVec2 &position, float radius, ImU32 color, int num_segments, float thickness) {
    auto draw_list = ImGui::GetOverlayDrawList();
    draw_list->AddCircle(position, radius, color, num_segments, thickness);
    auto dirs = {ImVec2{0, 1}, ImVec2{1, 0}, ImVec2{0, -1}, ImVec2{-1, 0}};
    for (const ImVec2 &dir : dirs) {
        ImVec2 start(position.x + dir.x * 0.5 * radius, position.y + dir.y * 0.5 * radius);
        ImVec2 end(position.x + dir.x * 1.5 * radius, position.y + dir.y * 1.5 * radius);
        draw_list->AddLine(start, end, color, thickness);
    }
}

bool ImGuiWrapper::contain_all_glyphs(const ImFont      *font,
                                     const std::string &text)
{
    if (font == nullptr) return false;
    if (!font->IsLoaded()) return false;
    const ImFontConfig *fc = font->ConfigData;
    if (fc == nullptr) return false;
    if (text.empty()) return true;
    return is_chars_in_ranges(fc->GlyphRanges, text.c_str());
}

bool ImGuiWrapper::is_char_in_ranges(const ImWchar *ranges,
                                     unsigned int   letter)
{
    for (const ImWchar *range = ranges; range[0] && range[1]; range += 2) {
        ImWchar from = range[0];
        ImWchar to   = range[1];
        if (from <= letter && letter <= to) return true;
        if (letter < to) return false; // ranges should be sorted
    }
    return false;
};

bool ImGuiWrapper::is_chars_in_ranges(const ImWchar *ranges,
                                     const char    *chars_ptr)
{
    while (*chars_ptr) {
        unsigned int c = 0;
        // UTF-8 to 32-bit character need imgui_internal
        int c_len = ImTextCharFromUtf8(&c, chars_ptr, NULL);
        chars_ptr += c_len;
        if (c_len == 0) break;
        if (!is_char_in_ranges(ranges, c)) return false;
    }
    return true;
}


#ifdef __APPLE__
static const ImWchar ranges_keyboard_shortcuts[] =
{
    0x21E7, 0x21E7, // OSX Shift Key symbol
    0x2318, 0x2318, // OSX Command Key symbol
    0x2325, 0x2325, // OSX Option Key symbol
    0,
};
#endif // __APPLE__


std::vector<unsigned char> ImGuiWrapper::load_svg(const std::string& bitmap_name, unsigned target_width, unsigned target_height)
{
    std::vector<unsigned char> empty_vector;

    NSVGimage* image = BitmapCache::nsvgParseFromFileWithReplace(Slic3r::var(bitmap_name + ".svg").c_str(), "px", 96.0f, { { "\"#808080\"", "\"#FFFFFF\"" } });
    if (image == nullptr)
        return empty_vector;

    float svg_scale = target_height != 0 ?
        (float)target_height / image->height : target_width != 0 ?
        (float)target_width / image->width : 1;

    int   width = (int)(svg_scale * image->width + 0.5f);
    int   height = (int)(svg_scale * image->height + 0.5f);
    int   n_pixels = width * height;
    if (n_pixels <= 0) {
        ::nsvgDelete(image);
        return empty_vector;
    }

    NSVGrasterizer* rast = ::nsvgCreateRasterizer();
    if (rast == nullptr) {
        ::nsvgDelete(image);
        return empty_vector;
    }

    std::vector<unsigned char> data(n_pixels * 4, 0);
    ::nsvgRasterize(rast, image, 0, 0, svg_scale, data.data(), width, height, width * 4);
    ::nsvgDeleteRasterizer(rast);
    ::nsvgDelete(image);

    return data;
}


//BBS
static bool m_is_dark_mode = false;

void ImGuiWrapper::on_change_color_mode(bool is_dark)
{
    m_is_dark_mode = is_dark;
}

void ImGuiWrapper::push_toolbar_style(const float scale)
{
    //if (m_is_dark_mode) {
    //    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f * scale);
    //    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 10.0f) * scale);
    //    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f * scale);
    //    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    //    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f * scale);
    //    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f) * scale);
    //    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.88f));                                        // 1
    //    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGuiWrapper::COL_WINDOW_BG_DARK);                                   // 2
    //    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGuiWrapper::COL_TITLE_BG);                                          // 3
    //    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImGuiWrapper::COL_TITLE_BG);                                    // 4
    //    ImGui::PushStyleColor(ImGuiCol_Separator, ImGuiWrapper::COL_SEPARATOR_DARK);                                  // 5
    //    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(62 / 255.0f, 62 / 255.0f, 69 / 255.0f, 1.00f));                 // 6
    //    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(73 / 255.0f, 73 / 255.0f, 78 / 255.0f, 1.00f));          // 7
    //    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(73 / 255.0f, 73 / 255.0f, 78 / 255.0f, 1.00f));           // 8
    //    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(84 / 255.0f, 84 / 255.0f, 90 / 255.0f, 1.00f));         // 9
    //    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(62 / 255.0f, 62 / 255.0f, 69 / 255.0f, 1.00f));          // 10
    //    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(238 / 255.0f, 238 / 255.0f, 238 / 255.0f, 0.00f));             // 11
    //    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(43 / 255.0f, 64 / 255.0f, 54 / 255.0f, 1.00f));         // 12
    //    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));                                // 13
    //    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.42f, 0.42f, 0.42f, 1.00f));                            // 14
    //    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.93f, 0.93f, 0.93f, 1.00f));                     // 15
    //    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0.93f, 0.93f, 0.93f, 1.00f));                      // 16
    //}
    //else {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 10.0f) * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f) * scale);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(50 / 255.0f, 58 / 255.0f, 61 / 255.0f, 1.00f));       // 1
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGuiWrapper::COL_WINDOW_BG);          // 2
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGuiWrapper::COL_TITLE_BG);            // 3
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImGuiWrapper::COL_TITLE_BG);      // 4
        ImGui::PushStyleColor(ImGuiCol_Separator, ImGuiWrapper::COL_SEPARATOR);         // 5
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));     // 6
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGuiWrapper::COL_HOVER);         // 7
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(238 / 255.0f, 238 / 255.0f, 238 / 255.0f, 1.00f)); // 8
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(172 / 255.0f, 172 / 255.0f, 172 / 255.0f, 1.00f));                        // 9
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(238 / 255.0f, 238 / 255.0f, 238 / 255.0f, 1.00f));  // 10
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(238 / 255.0f, 238 / 255.0f, 238 / 255.0f, 0.00f));        // 11
        ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, COL_AC_LIGHTBLUE);                                        // 12
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));//13
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.42f, 0.42f, 0.42f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.93f, 0.93f, 0.93f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0.93f, 0.93f, 0.93f, 1.00f));
    //}
}

void ImGuiWrapper::pop_toolbar_style()
{
    // size in push toolbar style
    ImGui::PopStyleColor(16);
    ImGui::PopStyleVar(6);
}

void ImGuiWrapper::push_ac_toolwin_style(const float scale)
{
    // AC TODO
    //if (m_is_dark_mode) {
    //}
    //else {
    //}
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 11.0f) * scale);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f * scale);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.4f * scale);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f * scale);

    //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 0.0f) * scale);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGuiWrapper::COL_WINDOW_BG);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(238 / 255.0f, 238 / 255.0f, 238 / 255.0f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(238 / 255.0f, 238 / 255.0f, 238 / 255.0f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(238 / 255.0f, 238 / 255.0f, 238 / 255.0f, 0.00f));

    //ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGuiWrapper::COL_AC_BLUE);
    //ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImGuiWrapper::COL_AC_BLUE);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.97f, 0.99f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGuiWrapper::COL_HOVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(172 / 255.0f, 172 / 255.0f, 172 / 255.0f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.42f, 0.42f, 0.42f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.93f, 0.93f, 0.93f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0.93f, 0.93f, 0.93f, 1.00f));

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(50 / 255.0f, 58 / 255.0f, 61 / 255.0f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, COL_AC_LIGHTBLUE);

    ImGui::PushStyleColor(ImGuiCol_Separator, ImGuiWrapper::COL_SEPARATOR);

    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));

}

void ImGuiWrapper::pop_ac_toolwin_style()
{
    ImGui::PopStyleColor(14);
    ImGui::PopStyleVar(5);

}

void ImGuiWrapper::push_menu_style(const float scale)
{
    if (m_is_dark_mode) {
        ImGuiWrapper::push_toolbar_style(scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f) * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 4.0f * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGuiWrapper::COL_WINDOW_BG_DARK);
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.00f, 0.68f, 0.26f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.00f, 0.68f, 0.26f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.00f, 0.68f, 0.26f, 1.0f));
    }
    else {
        ImGuiWrapper::push_toolbar_style(scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f) * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 4.0f * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGuiWrapper::COL_WINDOW_BG);
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.00f, 0.68f, 0.26f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.00f, 0.68f, 0.26f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.00f, 0.68f, 0.26f, 1.0f));
    }
}

void ImGuiWrapper::pop_menu_style()
{
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(3);
    ImGuiWrapper::pop_toolbar_style();
}

void ImGuiWrapper::push_ac_inputdouble_style(const float pos_x,const float _x,const float _y,
                                             ImFont *_font)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(_x, _y));
    ImGui::SetCursorPosX(pos_x);
    ImGui::PushFont(_font);
}

void ImGuiWrapper::pop_ac_inputdouble_style()
{
    ImGui::PopFont();
    ImGui::PopStyleVar();
}


void ImGuiWrapper::push_ac_button_style(const float pos_y,
                                        const float _x,
                                        const float _y,
                                        const float _r)
{
    ImGui::SetCursorPosY(pos_y);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(_x, _y));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, _r);

    ImGui::PushStyleColor(ImGuiCol_Border, ImGuiWrapper::COL_AC_BLUE);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWrapper::COL_AC_BLUE);
}

void ImGuiWrapper::pop_ac_button_style()
{
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}


void ImGuiWrapper::push_ac_combo_style(const float pos_y, const float _x, const float _y, const float _r)
{
    ImGui::SetCursorPosY(pos_y);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(_x, _y));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, _r);


    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGuiWrapper::COL_AC_BLACK_GRAY);
    ImGui::PushStyleColor(ImGuiCol_Border, ImGuiWrapper::COL_AC_BLUE);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWrapper::COL_AC_WHITE);
}

void ImGuiWrapper::pop_ac_combo_style()
{
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

void ImGuiWrapper::push_common_window_style(const float scale)
{
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 10.0f) * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.05f, 0.50f) * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 4.0f);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(38 / 255.0f, 46 / 255.0f, 48 / 255.0f, 1.00f));              // 1
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));                            // 2
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(245 / 255.0f, 245 / 255.0f, 245 / 255.0f, 1.00f));        // 3
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(245 / 255.0f, 245 / 255.0f, 245 / 255.0f, 1.00f));  // 4
        ImGui::PushStyleColor(ImGuiCol_Separator, ImGuiWrapper::COL_SEPARATOR);                                  // 5
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));                              // 6
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COL_AC_LIGHTBLUE);                                         // 7
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, COL_AC_BLUE);                                               // 8
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(238 / 255.0f, 238 / 255.0f, 238 / 255.0f, 1.00f)); // 9
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(238 / 255.0f, 238 / 255.0f, 238 / 255.0f, 1.00f));  // 10
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(238 / 255.0f, 238 / 255.0f, 238 / 255.0f, 0.00f));        // 11
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));                           // 12
        ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, COL_AC_LIGHTBLUE);                                        // 13
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.00f, 0.68f, 0.26f, 1.00f));                       // 14
        ImGui::PushStyleColor(ImGuiCol_ChildBg, COL_AC_PANEL_BACKGROUND);                                        // 15
        ImGui::PushStyleColor(ImGuiCol_PopupBg, COL_AC_PANEL_BACKGROUND);                                        // 16
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, COL_AC_BLUE);                                               // 17
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, COL_AC_LIST_HOVER);//18
        ImGui::PushStyleColor(ImGuiCol_Header, COL_AC_BLUE);              // 19
}

void ImGuiWrapper::pop_common_window_style() {
    ImGui::PopStyleColor(19);
    ImGui::PopStyleVar(6);
}

void ImGuiWrapper::push_confirm_button_style() {
    if (m_is_dark_mode) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f / 255.f, 174.f / 255.f, 66.f / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f / 255.f, 174.f / 255.f, 66.f / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(61.f / 255.f, 203.f / 255.f, 115.f / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(27.f / 255.f, 136.f / 255.f, 68.f / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 1.f, 1.f, 0.88f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.88f));
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f / 255.f, 174.f / 255.f, 66.f / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f / 255.f, 174.f / 255.f, 66.f / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(61.f / 255.f, 203.f / 255.f, 115.f / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(27.f / 255.f, 136.f / 255.f, 68.f / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    }
}

void ImGuiWrapper::pop_confirm_button_style() {
    ImGui::PopStyleColor(6);
}

void ImGuiWrapper::push_cancel_button_style() {
    if (m_is_dark_mode) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.f, 1.f, 1.f, 0.64f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(73 / 255.f, 73 / 255.f, 78 / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(129 / 255.f, 129 / 255.f, 131 / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 1.f, 1.f, 0.64f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.64f));
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(38 / 255.f, 46 / 255.f, 48 / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(238.f / 255.f, 238.f / 255.f, 238.f / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(206.f / 255.f, 206.f / 255.f, 206.f / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.f, 0.f, 0.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(38.f / 255.0f, 46.f / 255.0f, 48.f / 255.0f, 1.00f));
    }
}

void ImGuiWrapper::pop_cancel_button_style() {
    ImGui::PopStyleColor(6);
}

void ImGuiWrapper::push_button_disable_style() {
    if (m_is_dark_mode) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(54 / 255.f, 54 / 255.f, 60 / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(54 / 255.f, 54 / 255.f, 60 / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.4f));
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(206.f / 255.f, 206.f / 255.f, 206.f / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(206.f / 255.f, 206.f / 255.f, 206.f / 255.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    }
}

void ImGuiWrapper::pop_button_disable_style() {
    ImGui::PopStyleColor(3);
}

void ImGuiWrapper::push_ac_panel_style(const float scale) {
    push_common_window_style(scale);
}

void ImGuiWrapper::push_ac_listBox_style(const float scale)
{
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 8.0f * scale);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 8.0f * scale);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 8.0f * scale);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f * scale);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, COL_AC_LIST_CLICK);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, COL_AC_LIST_HOVER);
    ImGui::PushStyleColor(ImGuiCol_Header, COL_AC_LIST_CLICK);

}

void ImGuiWrapper::pop_ac_listBox_style() {
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(4);

}

void ImGuiWrapper::pop_ac_panel_style() {
    //ImGui::PopStyleColor(3);
    pop_common_window_style();
}

void ImGuiWrapper::init_font(bool compress)
{
    destroy_font();

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    // Create ranges of characters from m_glyph_ranges, possibly adding some OS specific special characters.
	ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
	builder.AddRanges(m_glyph_ranges);

    builder.AddChar(ImWchar(0x2026)); // …

    if (m_font_cjk) {
        // This is a temporary fix of https://github.com/prusa3d/PrusaSlicer/issues/8171. The translation
        // contains characters not in the ImGui ranges for simplified Chinese. For now, just add them manually.
        // In future, it might be worth to parse the dictionary and add all the necessary characters.
        builder.AddChar(ImWchar(0x5ED3));
        builder.AddChar(ImWchar(0x8F91));
    }

#ifdef __APPLE__
	if (m_font_cjk)
		// Apple keyboard shortcuts are only contained in the CJK fonts.
		builder.AddRanges(ranges_keyboard_shortcuts);
#endif
	builder.BuildRanges(&ranges); // Build the final result (ordered ranges with all the unique characters submitted)

    io.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;
    ImFontConfig cfg = ImFontConfig();
    cfg.OversampleH = cfg.OversampleV = 1;

    //FIXME replace with io.Fonts->AddFontFromMemoryTTF(buf_decompressed_data, (int)buf_decompressed_size, m_font_size, nullptr, ranges.Data);
    //https://github.com/ocornut/imgui/issues/220
	default_font = io.Fonts->AddFontFromFileTTF((Slic3r::resources_dir() + "/fonts/" + (m_font_cjk ? "HONORSansCN-Regular.ttf" : "HONORSansCN-Regular.ttf")).c_str(), m_font_size, nullptr, ranges.Data);
    if (default_font == nullptr) {
        default_font = io.Fonts->AddFontDefault();
        if (default_font == nullptr) {
            throw Slic3r::RuntimeError("ImGui: Could not load deafult font");
        }
    }
    default_font_12 = io.Fonts->AddFontFromFileTTF(
        (Slic3r::resources_dir() + "/fonts/" +
         (m_font_cjk ? "HONORSansCN-Regular.ttf" : "HONORSansCN-Regular.ttf"))
            .c_str(),
                                                   m_font_size, nullptr, ranges.Data);
    if (default_font_12 == nullptr) {
        default_font_12 = io.Fonts->AddFontDefault();
        if (default_font_12 == nullptr) {
            throw Slic3r::RuntimeError(
                "ImGui: Could not load deafult_12 font");
        }
    }

    default_font_13 = io.Fonts->AddFontFromFileTTF(
        (Slic3r::resources_dir() + "/fonts/" +
         (m_font_cjk ? "HONORSansCN-Regular.ttf" : "HONORSansCN-Regular.ttf"))
            .c_str(),
                                                   1.12 * m_font_size, nullptr, ranges.Data);
    if (default_font_13 == nullptr) {
        default_font_13 = io.Fonts->AddFontDefault();
        if (default_font_13 == nullptr) {
            throw Slic3r::RuntimeError("ImGui: Could not load deafult_13 font");
        }
    }

    default_font_14 = io.Fonts->AddFontFromFileTTF(
        (Slic3r::resources_dir() + "/fonts/" +
         (m_font_cjk ? "HONORSansCN-Regular.ttf" : "HONORSansCN-Regular.ttf"))
            .c_str(),
                                                   1.12 * m_font_size, nullptr, ranges.Data);
    if (default_font_14 == nullptr) {
        default_font_14 = io.Fonts->AddFontDefault();
        if (default_font_14 == nullptr) {
            throw Slic3r::RuntimeError("ImGui: Could not load deafult_14 font");
        }
    }

    bold_font        = io.Fonts->AddFontFromFileTTF((Slic3r::resources_dir() + "/fonts/" + "HONORSansCN-Bold.ttf").c_str(), m_font_size, &cfg, ranges.Data);
    if (bold_font == nullptr) {
        bold_font = io.Fonts->AddFontDefault();
        if (bold_font == nullptr) { throw Slic3r::RuntimeError("ImGui: Could not load deafult font"); }
    }
    bold_font_14 = io.Fonts->AddFontFromFileTTF((Slic3r::resources_dir() + "/fonts/" + "HONORSansCN-Bold.ttf").c_str(), m_font_size*1.12, &cfg,
                                             ranges.Data);
    if (bold_font == nullptr) {
        bold_font = io.Fonts->AddFontDefault();
        if (bold_font == nullptr) {
            throw Slic3r::RuntimeError("ImGui: Could not load deafult font");
        }
    }

#ifdef __APPLE__
    ImFontConfig config;
    config.MergeMode = true;
    if (! m_font_cjk) {
		// Apple keyboard shortcuts are only contained in the CJK fonts.
        [[maybe_unused]]ImFont *font_cjk = io.Fonts->AddFontFromFileTTF((Slic3r::resources_dir() + "/fonts/HONORSansCN-Regular.ttf").c_str(), m_font_size, &config, ranges_keyboard_shortcuts);
        assert(font_cjk != nullptr);
    }
#endif

    float font_scale = m_font_size/15;
    int icon_sz = lround(16 * font_scale); // default size of icon is 16 px

    int   gcode_icon_sz    = lround(22 * font_scale);
    int rect_id = io.Fonts->CustomRects.Size;  // id of the rectangle added next
    // add rectangles for the icons to the font atlas
    for (auto& icon : font_icons) {
        m_custom_glyph_rects_ids[icon.first] =
            io.Fonts->AddCustomRectFontGlyph(default_font, icon.first, icon_sz, icon_sz, 3.0 * font_scale + icon_sz);
    }
    for (auto& icon : font_icons_large) {
        m_custom_glyph_rects_ids[icon.first] =
            io.Fonts->AddCustomRectFontGlyph(default_font, icon.first, gcode_icon_sz * 2, gcode_icon_sz * 2, 3.0 * font_scale + gcode_icon_sz * 2);
    }
    for (auto& icon : font_icons_extra_large) {
        m_custom_glyph_rects_ids[icon.first] =
            io.Fonts->AddCustomRectFontGlyph(default_font, icon.first, icon_sz * 4, icon_sz * 4, 3.0 * font_scale + icon_sz * 4);
    }

    // Build texture atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

    auto load_icon_from_svg = [this, &io, pixels, width, &rect_id](const std::pair<const wchar_t, std::string> icon, int icon_sz) {
        if (const ImFontAtlas::CustomRect* rect = io.Fonts->GetCustomRectByIndex(rect_id)) {
            assert(rect->Width == icon_sz);
            assert(rect->Height == icon_sz);
            std::vector<unsigned char> raw_data = load_svg(icon.second, icon_sz, icon_sz);
            if (!raw_data.empty()) {
                const ImU32* pIn = (ImU32*)raw_data.data();
                for (int y = 0; y < icon_sz; y++) {
                    ImU32* pOut = (ImU32*)pixels + (rect->Y + y) * width + (rect->X);
                    for (int x = 0; x < icon_sz; x++)
                        *pOut++ = *pIn++;
                }
            }
        }
        rect_id++;
    };

    // Fill rectangles from the SVG-icons
    for (auto icon : font_icons) {
        load_icon_from_svg(icon, icon_sz);
    }

    icon_sz *= 2; // default size of large icon is 32 px
    gcode_icon_sz *= 2;
    for (auto icon : font_icons_large) {
        if (const ImFontAtlas::CustomRect* rect = io.Fonts->GetCustomRectByIndex(rect_id)) {
            assert(rect->Width == gcode_icon_sz);
            assert(rect->Height == gcode_icon_sz);
            std::vector<unsigned char> raw_data = load_svg(icon.second, gcode_icon_sz, gcode_icon_sz);
            const ImU32* pIn = (ImU32*)raw_data.data();
            for (int y = 0; y < gcode_icon_sz; y++) {
                ImU32* pOut = (ImU32*)pixels + (rect->Y + y) * width + (rect->X);
                for (int x = 0; x < gcode_icon_sz; x++)
                    *pOut++ = *pIn++;
            }
        }
        rect_id++;
    }

    icon_sz *= 2; // default size of extra large icon is 64 px
    for (auto icon : font_icons_extra_large) {
        load_icon_from_svg(icon, icon_sz);
    }

    // Upload texture to graphics system
    GLint last_texture;
    glsafe(::glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture));
    glsafe(::glGenTextures(1, &m_font_texture));
    glsafe(::glBindTexture(GL_TEXTURE_2D, m_font_texture));
    glsafe(::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    glsafe(::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    glsafe(::glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
    if (compress && OpenGLManager::are_compressed_textures_supported())
        glsafe(::glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
    else
        glsafe(::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)(intptr_t)m_font_texture;

    // Restore state
    glsafe(::glBindTexture(GL_TEXTURE_2D, last_texture));
}

void ImGuiWrapper::init_input()
{
    ImGuiIO& io = ImGui::GetIO();

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = WXK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = WXK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = WXK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = WXK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = WXK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = WXK_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = WXK_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = WXK_HOME;
    io.KeyMap[ImGuiKey_End] = WXK_END;
    io.KeyMap[ImGuiKey_Insert] = WXK_INSERT;
    io.KeyMap[ImGuiKey_Delete] = WXK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = WXK_BACK;
    io.KeyMap[ImGuiKey_Space] = WXK_SPACE;
    io.KeyMap[ImGuiKey_Enter] = WXK_RETURN;
    io.KeyMap[ImGuiKey_KeyPadEnter] = WXK_NUMPAD_ENTER;
    io.KeyMap[ImGuiKey_Escape] = WXK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

    // Don't let imgui special-case Mac, wxWidgets already do that
    io.ConfigMacOSXBehaviors = false;

    // Setup clipboard interaction callbacks
    io.SetClipboardTextFn = clipboard_set;
    io.GetClipboardTextFn = clipboard_get;
    io.ClipboardUserData = this;
}

void ImGuiWrapper::init_style()
{
    ImGuiStyle &style = ImGui::GetStyle();

    auto set_color = [&](ImGuiCol_ entity, ImVec4 color) {
        style.Colors[entity] = color;
    };
    // Text
    set_color(ImGuiCol_Text, COL_AC_BLACK);
    // Window
    style.WindowRounding = 14.0f;
    style.FrameBorderSize = 1.0f;
    style.FrameRounding = 8.0f;
    style.FramePadding = ImVec2(16, 8);

    set_color(ImGuiCol_WindowBg, COL_AC_WHITE);
    // Title
    set_color(ImGuiCol_TitleBg         , COL_AC_LIGHTBLUE);
    set_color(ImGuiCol_TitleBgActive   , COL_AC_LIGHTBLUE);
    set_color(ImGuiCol_TitleBgCollapsed, COL_AC_LIGHTBLUE);

    // Generics
    set_color(ImGuiCol_FrameBg       , COL_AC_LIGHTGRAY);
    set_color(ImGuiCol_FrameBgHovered, COL_AC_ITEMBLUE);
    set_color(ImGuiCol_FrameBgActive , COL_AC_ITEMBLUE);

    // Text selection
    set_color(ImGuiCol_TextSelectedBg, COL_AC_LIGHTBLUE);

    // Buttons
    set_color(ImGuiCol_Button, COL_AC_BLUE);
    set_color(ImGuiCol_ButtonHovered, COL_AC_LIGHTBLUE);
    set_color(ImGuiCol_ButtonActive, COL_AC_LIGHTBLUE);

    // Checkbox
    set_color(ImGuiCol_CheckMark, COL_AC_BLUE);

    // ComboBox items
    set_color(ImGuiCol_Header, COL_AC_BLUE);
    set_color(ImGuiCol_HeaderHovered, COL_AC_LIGHTBLUE);
    set_color(ImGuiCol_HeaderActive, COL_AC_DARKBLUE);

    // Slider
    set_color(ImGuiCol_SliderGrab, COL_AC_BLUE);
    set_color(ImGuiCol_SliderGrabActive, COL_AC_DARKBLUE);

    // Separator
    set_color(ImGuiCol_Separator, COL_AC_BOARDGRAY);

    // Tabs
    set_color(ImGuiCol_Tab, COL_AC_BLUE);
    set_color(ImGuiCol_TabHovered, COL_AC_LIGHTBLUE);
    set_color(ImGuiCol_TabActive, COL_AC_DARKBLUE);
    set_color(ImGuiCol_TabUnfocused, COL_GREY_DARK);
    set_color(ImGuiCol_TabUnfocusedActive, COL_GREY_LIGHT);

    // Scrollbars
    set_color(ImGuiCol_ScrollbarGrab, COL_AC_LIST_SCROLL);
    set_color(ImGuiCol_ScrollbarGrabHovered, COL_AC_LIST_SCROLL);
    set_color(ImGuiCol_ScrollbarGrabActive, COL_AC_LIST_SCROLL);

    //ImGuiCol_Text,
    //ImGuiCol_TextDisabled,
    //ImGuiCol_TextSelectedBg,
    //ImGuiCol_WindowBg,              // Background of normal windows
    //ImGuiCol_ChildBg,               // Background of child windows
    //ImGuiCol_PopupBg,               // Background of popups, menus, tooltips windows
    //ImGuiCol_Border,
    //ImGuiCol_BorderActive,
    //ImGuiCol_BorderShadow,
    //ImGuiCol_FrameBg,               // Background of checkbox, radio button, plot, slider, text input
    //ImGuiCol_FrameBgHovered,
    //ImGuiCol_FrameBgActive,
    //ImGuiCol_TitleBg,
    //ImGuiCol_TitleBgActive,
    //ImGuiCol_TitleBgCollapsed,
    //ImGuiCol_MenuBarBg,
    //ImGuiCol_ScrollbarBg,
    //ImGuiCol_ScrollbarGrab,
    //ImGuiCol_ScrollbarGrabHovered,
    //ImGuiCol_ScrollbarGrabActive,
    //ImGuiCol_CheckMark,
    //ImGuiCol_SliderGrab,
    //ImGuiCol_SliderGrabActive,
    //ImGuiCol_Button,
    //ImGuiCol_ButtonHovered,
    //ImGuiCol_ButtonActive,
    //ImGuiCol_Header,                // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
    //ImGuiCol_HeaderHovered,
    //ImGuiCol_HeaderActive,
    //ImGuiCol_Separator,
    //ImGuiCol_SeparatorHovered,
    //ImGuiCol_SeparatorActive,
    //ImGuiCol_ResizeGrip,
    //ImGuiCol_ResizeGripHovered,
    //ImGuiCol_ResizeGripActive,
    //ImGuiCol_Tab,
    //ImGuiCol_TabHovered,
    //ImGuiCol_TabActive,
    //ImGuiCol_TabUnfocused,
    //ImGuiCol_TabUnfocusedActive,
    //ImGuiCol_PlotLines,
    //ImGuiCol_PlotLinesHovered,
    //ImGuiCol_PlotHistogram,
    //ImGuiCol_PlotHistogramHovered,
    //ImGuiCol_TableHeaderBg,         // Table header background
    //ImGuiCol_TableBorderStrong,     // Table outer and header borders (prefer using Alpha=1.0 here)
    //ImGuiCol_TableBorderLight,      // Table inner borders (prefer using Alpha=1.0 here)
    //ImGuiCol_TableRowBg,            // Table row background (even rows)
    //ImGuiCol_TableRowBgAlt,         // Table row background (odd rows)
    //ImGuiCol_DragDropTarget,
    //ImGuiCol_NavHighlight,          // Gamepad/keyboard: current highlighted item
    //ImGuiCol_NavWindowingHighlight, // Highlight window when using CTRL+TAB
    //ImGuiCol_NavWindowingDimBg,     // Darken/colorize entire screen behind the CTRL+TAB window list, when active
    //ImGuiCol_ModalWindowDimBg,      // Darken/colorize entire screen behind a modal window, when one is active
    //ImGuiCol_COUNT
}

void ImGuiWrapper::render_draw_data(ImDrawData *draw_data)
{
    if (draw_data == nullptr || draw_data->CmdListsCount == 0)
        return;

    GLShaderProgram* shader = wxGetApp().get_shader("imgui");
    if (shader == nullptr)
        return;

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    const int fb_width  = (int)(draw_data->DisplaySize.x * io.DisplayFramebufferScale.x);
    const int fb_height = (int)(draw_data->DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;

    GLShaderProgram* curr_shader = wxGetApp().get_current_shader();
    if (curr_shader != nullptr)
        curr_shader->stop_using();

    shader->start_using();

#if ENABLE_GL_CORE_PROFILE || ENABLE_OPENGL_ES
    // Backup GL state
    GLenum last_active_texture;       glsafe(::glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture));
    GLuint last_program;              glsafe(::glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&last_program));
    GLuint last_texture;              glsafe(::glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&last_texture));
    GLuint last_array_buffer;         glsafe(::glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&last_array_buffer));
    GLuint last_vertex_array_object = 0;
    if (OpenGLManager::get_gl_info().is_version_greater_or_equal_to(3, 0))
        glsafe(::glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&last_vertex_array_object));
    GLint last_viewport[4];           glsafe(::glGetIntegerv(GL_VIEWPORT, last_viewport));
    GLint last_scissor_box[4];        glsafe(::glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box));
    GLenum last_blend_src_rgb;        glsafe(::glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb));
    GLenum last_blend_dst_rgb;        glsafe(::glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb));
    GLenum last_blend_src_alpha;      glsafe(::glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha));
    GLenum last_blend_dst_alpha;      glsafe(::glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha));
    GLenum last_blend_equation_rgb;   glsafe(::glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb));
    GLenum last_blend_equation_alpha; glsafe(::glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha));
    GLboolean last_enable_blend        = ::glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face    = ::glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test   = ::glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_stencil_test = ::glIsEnabled(GL_STENCIL_TEST);
    GLboolean last_enable_scissor_test = ::glIsEnabled(GL_SCISSOR_TEST);

    // set new GL state
    glsafe(::glActiveTexture(GL_TEXTURE0));
    glsafe(::glEnable(GL_BLEND));
    glsafe(::glBlendEquation(GL_FUNC_ADD));
    glsafe(::glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
    glsafe(::glDisable(GL_CULL_FACE));
    glsafe(::glDisable(GL_DEPTH_TEST));
    glsafe(::glDisable(GL_STENCIL_TEST));
    glsafe(::glEnable(GL_SCISSOR_TEST));
#else
    // We are using the OpenGL fixed pipeline to make the example code simpler to read!
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers, polygon fill.
    GLint last_texture;          glsafe(::glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture));
    GLint last_polygon_mode[2];  glsafe(::glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode));
    GLint last_viewport[4];      glsafe(::glGetIntegerv(GL_VIEWPORT, last_viewport));
    GLint last_scissor_box[4];   glsafe(::glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box));
    GLint last_texture_env_mode; glsafe(::glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &last_texture_env_mode));
    glsafe(::glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT));
    glsafe(::glEnable(GL_BLEND));
    glsafe(::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    glsafe(::glDisable(GL_CULL_FACE));
    glsafe(::glDisable(GL_DEPTH_TEST));
    glsafe(::glDisable(GL_STENCIL_TEST));
    glsafe(::glEnable(GL_SCISSOR_TEST));
    glsafe(::glEnable(GL_TEXTURE_2D));
    glsafe(::glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    glsafe(::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE));
#endif // ENABLE_GL_CORE_PROFILE || ENABLE_OPENGL_ES

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    glsafe(::glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height));
    const float L = draw_data->DisplayPos.x;
    const float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    const float T = draw_data->DisplayPos.y;
    const float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

    Matrix4f ortho_projection;
    ortho_projection <<
        2.0f / (R - L), 0.0f,           0.0f,  (R + L) / (L - R),
        0.0f,           2.0f / (T - B), 0.0f,  (T + B) / (B - T),
        0.0f,           0.0f,           -1.0f, 0.0f,
        0.0f,           0.0f,           0.0f,  1.0f;

    shader->set_uniform("Texture", 0);
    shader->set_uniform("ProjMtx", ortho_projection);

    // Will project scissor/clipping rectangles into framebuffer space
    const ImVec2 clip_off   = draw_data->DisplayPos;       // (0,0) unless using multi-viewports
    const ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; ++n) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
        const ImDrawIdx* idx_buffer  = cmd_list->IdxBuffer.Data;
        const GLsizeiptr vtx_buffer_size = (GLsizeiptr)cmd_list->VtxBuffer.Size * (int)sizeof(ImDrawVert);
        const GLsizeiptr idx_buffer_size = (GLsizeiptr)cmd_list->IdxBuffer.Size * (int)sizeof(ImDrawIdx);

#if ENABLE_GL_CORE_PROFILE
        GLuint vao_id = 0;
        if (OpenGLManager::get_gl_info().is_version_greater_or_equal_to(3, 0)) {
            glsafe(::glGenVertexArrays(1, &vao_id));
            glsafe(::glBindVertexArray(vao_id));
        }
#endif // ENABLE_GL_CORE_PROFILE

        GLuint vbo_id;
        glsafe(::glGenBuffers(1, &vbo_id));
        glsafe(::glBindBuffer(GL_ARRAY_BUFFER, vbo_id));
        glsafe(::glBufferData(GL_ARRAY_BUFFER, vtx_buffer_size, vtx_buffer, GL_STATIC_DRAW));

        const int position_id = shader->get_attrib_location("Position");
        if (position_id != -1) {
            glsafe(::glVertexAttribPointer(position_id, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (const void*)IM_OFFSETOF(ImDrawVert, pos)));
            glsafe(::glEnableVertexAttribArray(position_id));
        }
        const int uv_id = shader->get_attrib_location("UV");
        if (uv_id != -1) {
            glsafe(::glVertexAttribPointer(uv_id, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (const void*)IM_OFFSETOF(ImDrawVert, uv)));
            glsafe(::glEnableVertexAttribArray(uv_id));
        }
        const int color_id = shader->get_attrib_location("Color");
        if (color_id != -1) {
            glsafe(::glVertexAttribPointer(color_id, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (const void*)IM_OFFSETOF(ImDrawVert, col)));
            glsafe(::glEnableVertexAttribArray(color_id));
        }

        GLuint ibo_id;
        glsafe(::glGenBuffers(1, &ibo_id));
        glsafe(::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id));
        glsafe(::glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx_buffer_size, idx_buffer, GL_STATIC_DRAW));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
                // User callback (registered via ImDrawList::AddCallback)
                pcmd->UserCallback(cmd_list, pcmd);
            else {
                // Project scissor/clipping rectangles into framebuffer space
                const ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                const ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
                glsafe(::glScissor((int)clip_min.x, (int)(fb_height - clip_max.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y)));

                // Bind texture, Draw
                glsafe(::glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID()));
                glsafe(::glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx))));
            }
        }

        glsafe(::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        if (color_id != -1)
            glsafe(::glDisableVertexAttribArray(color_id));
        if (uv_id != -1)
            glsafe(::glDisableVertexAttribArray(uv_id));
        if (position_id != -1)
            glsafe(::glDisableVertexAttribArray(position_id));

        glsafe(::glBindBuffer(GL_ARRAY_BUFFER, 0));

        glsafe(::glDeleteBuffers(1, &ibo_id));
        glsafe(::glDeleteBuffers(1, &vbo_id));
#if ENABLE_GL_CORE_PROFILE
        if (vao_id > 0)
        glsafe(::glDeleteVertexArrays(1, &vao_id));
#endif // ENABLE_GL_CORE_PROFILE
    }

#if ENABLE_GL_CORE_PROFILE || ENABLE_OPENGL_ES
    // Restore modified GL state
    glsafe(::glBindTexture(GL_TEXTURE_2D, last_texture));
    glsafe(::glActiveTexture(last_active_texture));
    if (OpenGLManager::get_gl_info().is_version_greater_or_equal_to(3, 0))
        glsafe(::glBindVertexArray(last_vertex_array_object));
    glsafe(::glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer));
    glsafe(::glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha));
    glsafe(::glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha));
    if (last_enable_blend) glsafe(::glEnable(GL_BLEND)); else glsafe(::glDisable(GL_BLEND));
    if (last_enable_cull_face) glsafe(::glEnable(GL_CULL_FACE)); else glsafe(::glDisable(GL_CULL_FACE));
    if (last_enable_depth_test) glsafe(::glEnable(GL_DEPTH_TEST)); else glsafe(::glDisable(GL_DEPTH_TEST));
    if (last_enable_stencil_test) glsafe(::glEnable(GL_STENCIL_TEST)); else glsafe(::glDisable(GL_STENCIL_TEST));
    if (last_enable_scissor_test) glsafe(::glEnable(GL_SCISSOR_TEST)); else glsafe(::glDisable(GL_SCISSOR_TEST));
    glsafe(::glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]));
    glsafe(::glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]));
#else
    // Restore modified state
    glsafe(::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, last_texture_env_mode));
    glsafe(::glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture));
    glsafe(::glPopAttrib());
    glsafe(::glPolygonMode(GL_FRONT, (GLenum)last_polygon_mode[0]);
    glsafe(::glPolygonMode(GL_BACK, (GLenum)last_polygon_mode[1])));
    glsafe(::glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]));
    glsafe(::glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]));
#endif // ENABLE_GL_CORE_PROFILE || ENABLE_OPENGL_ES

    shader->stop_using();

    if (curr_shader != nullptr)
        curr_shader->start_using();
}

bool ImGuiWrapper::display_initialized() const
{
    const ImGuiIO& io = ImGui::GetIO();
    return io.DisplaySize.x >= 0.0f && io.DisplaySize.y >= 0.0f;
}

void ImGuiWrapper::destroy_font()
{
    if (m_font_texture != 0) {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->TexID = 0;
        glsafe(::glDeleteTextures(1, &m_font_texture));
        m_font_texture = 0;
    }
}

const char* ImGuiWrapper::clipboard_get(void* user_data)
{
    ImGuiWrapper *self = reinterpret_cast<ImGuiWrapper*>(user_data);

    const char* res = "";

    if (wxTheClipboard->Open()) {
        if (wxTheClipboard->IsSupported(wxDF_TEXT)
#if wxUSE_UNICODE
        || wxTheClipboard->IsSupported(wxDF_UNICODETEXT)
#endif // wxUSE_UNICODE
            ) {
            wxTextDataObject data;
            wxTheClipboard->GetData(data);

            if (data.GetTextLength() > 0) {
                self->m_clipboard_text = into_u8(data.GetText());
                res = self->m_clipboard_text.c_str();
            }
        }

        wxTheClipboard->Close();
    }

    return res;
}

void ImGuiWrapper::clipboard_set(void* /* user_data */, const char* text)
{
    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(wxString::FromUTF8(text)));   // object owned by the clipboard
        wxTheClipboard->Close();
    }
}
bool ImGuiWrapper::ACSelectable(const int &              rounding,
                                const char *             label,
                                std::vector<ImTextureID> imgIds,
                                bool *                   p_selected,
                                ImGuiSelectableFlags     flags,
                                const ImVec2 &           size_arg)
{
    bool         selected = *p_selected;
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    ImTextureID  showId;
    ImVec2       imgObjSize;

    if (window->SkipItems)
        return false;

    ImGuiContext &    g     = *GImGui;
    const ImGuiStyle &style = g.Style;

    // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
    ImGuiID id         = window->GetID(label);
    ImVec2  label_size = ImGui::CalcTextSize(label, NULL, true);
    if (imgIds.size() > 0) {
        showId     = imgIds[0];
        float imgSize  = label_size.y + style.ItemSpacing.y;
        float _imgSize = imgSize - 2 * (imgSize * 0.3 - style.ItemSpacing.y / 2);
        imgObjSize   = {_imgSize, _imgSize};
    }
    ImVec2  size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
    ImVec2  pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ImGui::ItemSize(size, 0.0f);

    // Fill horizontal space
    // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not
    // visibly match other widgets.
    const bool  span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
    const float min_x            = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
    const float max_x            = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
    if (label_size.x > max_x - min_x) {
        label_size.x = max_x - min_x;
    }
    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
        size.x = ImMax(label_size.x, max_x - min_x);

    // Text stays at the submission position, but bounding box may be extended on both sides
    const ImVec2 text_min = ImVec2(pos.x + imgObjSize.x * 1.5, pos.y);
    const ImVec2 text_max(min_x + size.x - imgObjSize.x * 1.5, pos.y + size.y);

    // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
    ImRect bb(min_x, pos.y, text_max.x + imgObjSize.x * 1.5, text_max.y);
    if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0) {
        const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
        const float spacing_y = style.ItemSpacing.y;
        const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
        const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
        bb.Min.x -= spacing_L;
        bb.Min.y -= spacing_U;
        bb.Max.x += (spacing_x - spacing_L);
        bb.Max.y += (spacing_y - spacing_U);
    }
    // if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

    // Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackground for every Selectable..
    const float backup_clip_rect_min_x = window->ClipRect.Min.x;
    const float backup_clip_rect_max_x = window->ClipRect.Max.x;
    if (span_all_columns) {
        window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
        window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
    }

    bool item_add;
    if (flags & ImGuiSelectableFlags_Disabled) {
        ImGuiItemFlags backup_item_flags = g.CurrentItemFlags;
        g.CurrentItemFlags |= ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNavDefaultFocus;
        item_add           = ImGui::ItemAdd(bb, id);
        g.CurrentItemFlags = backup_item_flags;
    } else {
        item_add = ImGui::ItemAdd(bb, id);
    }

    if (span_all_columns) {
        window->ClipRect.Min.x = backup_clip_rect_min_x;
        window->ClipRect.Max.x = backup_clip_rect_max_x;
    }

    if (!item_add)
        return false;

    // FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render
    // only, which would be advantageous since most selectable are not selected.
    if (span_all_columns && window->DC.CurrentColumns)
        ImGui::PushColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        ImGui::TablePushBackgroundChannel();

    // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) {
        button_flags |= ImGuiButtonFlags_NoHoldingActiveId;
    }
    if (flags & ImGuiSelectableFlags_SelectOnClick) {
        button_flags |= ImGuiButtonFlags_PressedOnClick;
    }
    if (flags & ImGuiSelectableFlags_SelectOnRelease) {
        button_flags |= ImGuiButtonFlags_PressedOnRelease;
    }
    if (flags & ImGuiSelectableFlags_Disabled) {
        button_flags |= ImGuiButtonFlags_Disabled;
    }
    if (flags & ImGuiSelectableFlags_AllowDoubleClick) {
        button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
    }
    if (flags & ImGuiSelectableFlags_AllowItemOverlap) {
        button_flags |= ImGuiButtonFlags_AllowItemOverlap;
    }

    if (flags & ImGuiSelectableFlags_Disabled)
        selected = false;

    const bool was_selected = selected;
    bool       hovered, held;
    bool       pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, button_flags);

	if (hovered)//hover item tooltips
	{
		ImGui::PushStyleColor(ImGuiCol_BorderShadow, { 0.0f, 0.0f, 0.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_Border,       { 0.0f, 0.0f, 0.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_PopupBg,      { .0f, .0f, .0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_Text,         { 1.0f, 1.0f, 1.0f, 1.0f });
		tooltip(label, bb.GetWidth());
		ImGui::PopStyleColor(4);
	}
    // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover))) {
        if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent) {
            ImGui::SetNavID(id, window->DC.NavLayerCurrent, window->DC.NavFocusScopeIdCurrent,
                            ImRect(bb.Min - window->Pos, bb.Max - window->Pos));
            g.NavDisableHighlight = true;
        }
    }
    if (pressed)
        ImGui::MarkItemEdited(id);

    if (flags & ImGuiSelectableFlags_AllowItemOverlap)
        ImGui::SetItemAllowOverlap();

    // In this branch, Selectable() cannot toggle the selection so this will never trigger.
    if (selected != was_selected) //-V547
        window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    // Render
    if (held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld))
        hovered = true;
    if (hovered || selected) {
        if (selected && imgIds.size() > 0) {
            showId = imgIds[1];
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.00f));
            if (hovered) {
                showId = imgIds[0];
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(38 / 255.0f, 46 / 255.0f, 48 / 255.0f, 1.00f));
            }
        }
        const ImU32 col   = ImGui::GetColorU32((held && hovered) ? ImGuiCol_HeaderActive :
                                               hovered           ? ImGuiCol_HeaderHovered :
                                                                   ImGuiCol_Header);
        int         x_gap = 2;
        ImGui::RenderFrame(x_gap, bb.Min, bb.Max, col, true, rounding);
        ImGui::RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
    }

    if (span_all_columns && window->DC.CurrentColumns)
        ImGui::PopColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        ImGui::TablePopBackgroundChannel();

    if (flags & ImGuiSelectableFlags_Disabled)
        ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
    if (imgIds.size() > 0) {
        window->DrawList->AddImage(showId, ImVec2(pos.x, pos.y - style.FramePadding.y),
                                   ImVec2(pos.x, pos.y - style.FramePadding.y) + imgObjSize, ImVec2(0, 0), ImVec2(1, 1),
                                   ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
    }
    ImGui::RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
    if (flags & ImGuiSelectableFlags_Disabled)
        ImGui::PopStyleColor();
    if (selected && imgIds.size() > 0) {
        ImGui::PopStyleColor();
        if (hovered) {
            ImGui::PopStyleColor();
        }
    }
    // Automatically close popups
    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) &&
        !(g.CurrentItemFlags & ImGuiItemFlags_SelectableDontClosePopup))
        ImGui::CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
    if (pressed){
        *p_selected = !*p_selected;
        return true;
    }
    return pressed;
}

bool ImGuiWrapper::ACSelectable(const int &              rounding,
                                const char *             label,
                                const char *             tipLabel,
                                std::vector<ImTextureID> imgIds,
                                bool *                   p_selected,
                                ImGuiSelectableFlags     flags,
                                const ImVec2 &           size_arg)
{
    bool         selected = *p_selected;
    ImGuiWindow *window   = ImGui::GetCurrentWindow();
    ImTextureID  showId;
    ImVec2       imgObjSize;

    if (window->SkipItems)
        return false;

    ImGuiContext &    g     = *GImGui;
    const ImGuiStyle &style = g.Style;

    // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
    ImGuiID id         = window->GetID(label);
    ImVec2  label_size = ImGui::CalcTextSize(label, NULL, true);
    if (imgIds.size() > 0) {
        showId         = imgIds[0];
        float imgSize  = label_size.y + style.ItemSpacing.y;
        float _imgSize = imgSize - 2 * (imgSize * 0.3 - style.ItemSpacing.y / 2);
        imgObjSize     = {_imgSize, _imgSize};
    }
    ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ImGui::ItemSize(size, 0.0f);

    // Fill horizontal space
    // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not
    // visibly match other widgets.
    const bool  span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
    const float min_x            = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
    const float max_x            = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
    if (label_size.x > max_x - min_x) {
        label_size.x = max_x - min_x;
    }
    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
        size.x = ImMax(label_size.x, max_x - min_x);

    // Text stays at the submission position, but bounding box may be extended on both sides
    const ImVec2 text_min = ImVec2(pos.x + imgObjSize.x * 1.5, pos.y);
    const ImVec2 text_max(min_x + size.x - imgObjSize.x * 1.5, pos.y + size.y);

    // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
    ImRect bb(min_x, pos.y, text_max.x + imgObjSize.x * 1.5, text_max.y);
    if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0) {
        const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
        const float spacing_y = style.ItemSpacing.y;
        const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
        const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
        bb.Min.x -= spacing_L;
        bb.Min.y -= spacing_U;
        bb.Max.x += (spacing_x - spacing_L);
        bb.Max.y += (spacing_y - spacing_U);
    }
    // if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

    // Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackground for every Selectable..
    const float backup_clip_rect_min_x = window->ClipRect.Min.x;
    const float backup_clip_rect_max_x = window->ClipRect.Max.x;
    if (span_all_columns) {
        window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
        window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
    }

    bool item_add;
    if (flags & ImGuiSelectableFlags_Disabled) {
        ImGuiItemFlags backup_item_flags = g.CurrentItemFlags;
        g.CurrentItemFlags |= ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNavDefaultFocus;
        item_add           = ImGui::ItemAdd(bb, id);
        g.CurrentItemFlags = backup_item_flags;
    } else {
        item_add = ImGui::ItemAdd(bb, id);
    }

    if (span_all_columns) {
        window->ClipRect.Min.x = backup_clip_rect_min_x;
        window->ClipRect.Max.x = backup_clip_rect_max_x;
    }

    if (!item_add)
        return false;

    // FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render
    // only, which would be advantageous since most selectable are not selected.
    if (span_all_columns && window->DC.CurrentColumns)
        ImGui::PushColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        ImGui::TablePushBackgroundChannel();

    // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) {
        button_flags |= ImGuiButtonFlags_NoHoldingActiveId;
    }
    if (flags & ImGuiSelectableFlags_SelectOnClick) {
        button_flags |= ImGuiButtonFlags_PressedOnClick;
    }
    if (flags & ImGuiSelectableFlags_SelectOnRelease) {
        button_flags |= ImGuiButtonFlags_PressedOnRelease;
    }
    if (flags & ImGuiSelectableFlags_Disabled) {
        button_flags |= ImGuiButtonFlags_Disabled;
    }
    if (flags & ImGuiSelectableFlags_AllowDoubleClick) {
        button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
    }
    if (flags & ImGuiSelectableFlags_AllowItemOverlap) {
        button_flags |= ImGuiButtonFlags_AllowItemOverlap;
    }

    if (flags & ImGuiSelectableFlags_Disabled)
        selected = false;

    const bool was_selected = selected;
    bool       hovered, held;
    bool       pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, button_flags);

    if (hovered) // hover item tooltips
    {
        ImGui::PushStyleColor(ImGuiCol_BorderShadow, {0.0f, 0.0f, 0.0f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_Border, {0.0f, 0.0f, 0.0f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_PopupBg, {.0f, .0f, .0f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 1.0f, 1.0f, 1.0f});
        tooltip(tipLabel, bb.GetWidth());
        ImGui::PopStyleColor(4);
    }
    // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover))) {
        if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent) {
            ImGui::SetNavID(id, window->DC.NavLayerCurrent, window->DC.NavFocusScopeIdCurrent,
                            ImRect(bb.Min - window->Pos, bb.Max - window->Pos));
            g.NavDisableHighlight = true;
        }
    }
    if (pressed)
        ImGui::MarkItemEdited(id);

    if (flags & ImGuiSelectableFlags_AllowItemOverlap)
        ImGui::SetItemAllowOverlap();

    // In this branch, Selectable() cannot toggle the selection so this will never trigger.
    if (selected != was_selected) //-V547
        window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    // Render
    if (held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld))
        hovered = true;
    if (hovered || selected) {
        if (selected && imgIds.size() > 0) {
            showId = imgIds[1];
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.00f));
            if (hovered) {
                showId = imgIds[0];
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(38 / 255.0f, 46 / 255.0f, 48 / 255.0f, 1.00f));
            }
        }
        const ImU32 col   = ImGui::GetColorU32((held && hovered) ? ImGuiCol_HeaderActive :
                                               hovered           ? ImGuiCol_HeaderHovered :
                                                                   ImGuiCol_Header);
        int         x_gap = 2;
        ImGui::RenderFrame(x_gap, bb.Min, bb.Max, col, true, rounding);
        ImGui::RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
    }

    if (span_all_columns && window->DC.CurrentColumns)
        ImGui::PopColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        ImGui::TablePopBackgroundChannel();

    if (flags & ImGuiSelectableFlags_Disabled)
        ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
    if (imgIds.size() > 0) {
        window->DrawList->AddImage(showId, ImVec2(pos.x, pos.y - style.FramePadding.y),
                                   ImVec2(pos.x, pos.y - style.FramePadding.y) + imgObjSize, ImVec2(0, 0), ImVec2(1, 1),
                                   ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
    }
    ImGui::RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
    if (flags & ImGuiSelectableFlags_Disabled)
        ImGui::PopStyleColor();
    if (selected && imgIds.size() > 0) {
        ImGui::PopStyleColor();
        if (hovered) {
            ImGui::PopStyleColor();
        }
    }
    // Automatically close popups
    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) &&
        !(g.CurrentItemFlags & ImGuiItemFlags_SelectableDontClosePopup))
        ImGui::CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
    if (pressed) {
        *p_selected = !*p_selected;
        return true;
    }
    return pressed;
}

bool ImGuiWrapper::ACSelectable(const int &              rounding,
                                const char *             label,
                                std::vector<ImTextureID> imgIds,
                                bool                     selected,
                                ImGuiSelectableFlags     flags,
                                const ImVec2 &           size_arg)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    ImTextureID  showId;
    ImVec2       imgObjSize;
    if (window->SkipItems)
        return false;

    ImGuiContext &    g     = *GImGui;
    const ImGuiStyle &style = g.Style;

    // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
    ImGuiID id         = window->GetID(label);
    ImVec2  label_size = ImGui::CalcTextSize(label, NULL, true);
    if (imgIds.size() > 0) {
        showId     = imgIds[0];
        float imgSize  = label_size.y + style.ItemSpacing.y;
        float _imgSize = imgSize - 2 * (imgSize * 0.3 - style.ItemSpacing.y / 2);
        imgObjSize  = {_imgSize,_imgSize};
    }
    ImVec2  size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
    ImVec2  pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ImGui::ItemSize(size, 0.0f);

    // Fill horizontal space
    // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not
    // visibly match other widgets.
    const bool  span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
    const float min_x            = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
    const float max_x            = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
    if (label_size.x > max_x - min_x) {
        label_size.x = max_x - min_x;
    }
    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
        size.x = ImMax(label_size.x, max_x - min_x);

    // Text stays at the submission position, but bounding box may be extended on both sides
    const ImVec2 text_min = ImVec2(pos.x + imgObjSize.x * 1.5, pos.y);
    const ImVec2 text_max(min_x + size.x - imgObjSize.x * 1.5, pos.y + size.y);

    // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
    ImRect bb(min_x, pos.y, text_max.x + imgObjSize.x * 1.5, text_max.y);
    if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0) {
        const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
        const float spacing_y = style.ItemSpacing.y;
        const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
        const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
        bb.Min.x -= spacing_L;
        bb.Min.y -= spacing_U;
        bb.Max.x += (spacing_x - spacing_L);
        bb.Max.y += (spacing_y - spacing_U);
    }
    // if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

    // Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackground for every Selectable..
    const float backup_clip_rect_min_x = window->ClipRect.Min.x;
    const float backup_clip_rect_max_x = window->ClipRect.Max.x;
    if (span_all_columns) {
        window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
        window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
    }

    bool item_add;
    if (flags & ImGuiSelectableFlags_Disabled) {
        ImGuiItemFlags backup_item_flags = g.CurrentItemFlags;
        g.CurrentItemFlags |= ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNavDefaultFocus;
        item_add           = ImGui::ItemAdd(bb, id);
        g.CurrentItemFlags = backup_item_flags;
    } else {
        item_add = ImGui::ItemAdd(bb, id);
    }

    if (span_all_columns) {
        window->ClipRect.Min.x = backup_clip_rect_min_x;
        window->ClipRect.Max.x = backup_clip_rect_max_x;
    }

    if (!item_add)
        return false;

    // FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render
    // only, which would be advantageous since most selectable are not selected.
    if (span_all_columns && window->DC.CurrentColumns)
        ImGui::PushColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        ImGui::TablePushBackgroundChannel();

    // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) {
        button_flags |= ImGuiButtonFlags_NoHoldingActiveId;
    }
    if (flags & ImGuiSelectableFlags_SelectOnClick) {
        button_flags |= ImGuiButtonFlags_PressedOnClick;
    }
    if (flags & ImGuiSelectableFlags_SelectOnRelease) {
        button_flags |= ImGuiButtonFlags_PressedOnRelease;
    }
    if (flags & ImGuiSelectableFlags_Disabled) {
        button_flags |= ImGuiButtonFlags_Disabled;
    }
    if (flags & ImGuiSelectableFlags_AllowDoubleClick) {
        button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
    }
    if (flags & ImGuiSelectableFlags_AllowItemOverlap) {
        button_flags |= ImGuiButtonFlags_AllowItemOverlap;
    }

    if (flags & ImGuiSelectableFlags_Disabled)
        selected = false;

    const bool was_selected = selected;
    bool       hovered, held;
    bool       pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, button_flags);

	if (hovered)//hover item tooltips
	{
		ImGui::PushStyleColor(ImGuiCol_BorderShadow, { 0.0f, 0.0f, 0.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_Border,       { 0.0f, 0.0f, 0.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_PopupBg,      { .0f, .0f, .0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_Text,         { 1.0f, 1.0f, 1.0f, 1.0f });
		tooltip(label, bb.GetWidth());
		ImGui::PopStyleColor(4);
	}
    // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover))) {
        if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent) {
            ImGui::SetNavID(id, window->DC.NavLayerCurrent, window->DC.NavFocusScopeIdCurrent,
                            ImRect(bb.Min - window->Pos, bb.Max - window->Pos));
            g.NavDisableHighlight = true;
        }
    }
    if (pressed)
        ImGui::MarkItemEdited(id);

    if (flags & ImGuiSelectableFlags_AllowItemOverlap)
        ImGui::SetItemAllowOverlap();

    // In this branch, Selectable() cannot toggle the selection so this will never trigger.
    if (selected != was_selected) //-V547
        window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    // Render
    if (held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld))
        hovered = true;
    if (hovered || selected) {
        if (selected && imgIds.size() > 0) {
            showId = imgIds[1];
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.00f));
            if (hovered) {
                showId = imgIds[0];
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(38 / 255.0f, 46 / 255.0f, 48 / 255.0f, 1.00f));
            }
        }

        const ImU32 col   = ImGui::GetColorU32((held && hovered) ? ImGuiCol_HeaderActive :
                                               hovered           ? ImGuiCol_HeaderHovered :
                                                                   ImGuiCol_Header);
        int         x_gap = 2;
        ImGui::RenderFrame(x_gap, bb.Min, bb.Max, col, true, rounding);
        ImGui::RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
    }

    if (span_all_columns && window->DC.CurrentColumns)
        ImGui::PopColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        ImGui::TablePopBackgroundChannel();

    if (flags & ImGuiSelectableFlags_Disabled)
        ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
    if (imgIds.size() > 0) {
        window->DrawList->AddImage(showId, ImVec2(pos.x, pos.y - style.FramePadding.y),
            ImVec2(pos.x, pos.y - style.FramePadding.y) + imgObjSize,
            ImVec2(0, 0), ImVec2(1, 1),ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
    }
    ImGui::RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb);

    if (flags & ImGuiSelectableFlags_Disabled)
        ImGui::PopStyleColor();
    if (selected && imgIds.size() > 0) {
        ImGui::PopStyleColor();
        if (hovered) {
            ImGui::PopStyleColor();
        }
    }
    // Automatically close popups
    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) &&
        !(g.CurrentItemFlags & ImGuiItemFlags_SelectableDontClosePopup))
        ImGui::CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
    return pressed;
}

bool ImGuiWrapper::ACListBox(const int & rounding,
                             const char *label,
                             int *       current_item,
                             bool (*items_getter)(void *, int, const char **),
                             void *                                          data,
                             std::map<std::string, std::vector<ImTextureID>> itemAndImgNames,
                             int                                             items_count,
                             int                                             height_in_items,
    float listWidth)
{
    ImGuiContext &g = *GImGui;

    // Calculate size from "height_in_items"
    if (height_in_items < 0)
        height_in_items = ImMin(items_count, 7);
    float  height_in_items_f = height_in_items + 0.25f;
    ImVec2 size(listWidth, ImFloor(ImGui::GetTextLineHeightWithSpacing() * height_in_items_f + g.Style.FramePadding.y * 2.0f));

    if (!ImGui::BeginListBox(label, size))
        return false;

    // Assume all items have even height (= 1 line of text). If you need items of different height,
    // you can create a custom version of ListBox() in your code without using the clipper.
    bool             value_changed = false;
    ImGuiListClipper clipper;
    clipper.Begin(items_count, ImGui::GetTextLineHeightWithSpacing()); // We know exactly our line height here so we pass it as a minor
                                                                       // optimization, but generally you don't need to.
    while (clipper.Step())
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
            const char *item_text;
            if (!items_getter(data, i, &item_text))
                item_text = "*Unknown item*";

            ImGui::PushID(i);
            const bool               item_selected = (i == *current_item);
            std::string              keyImg_str(item_text);
            std::vector<ImTextureID> imgIds;
            if (itemAndImgNames.count(keyImg_str)) {
                imgIds = itemAndImgNames[keyImg_str];
            }
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
            if (ACSelectable(rounding, item_text, imgIds, item_selected)) {
                *current_item = i;
                value_changed = true;
            }
            if (item_selected)
                ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }
    ImGui::EndListBox();
    if (value_changed)
        ImGui::MarkItemEdited(g.CurrentWindow->DC.LastItemId);

    return value_changed;
}

bool IMTexture::load_from_svg_file(const std::string& filename, unsigned width, unsigned height, ImTextureID& texture_id)
{
    std::string nowidePath = boost::filesystem::path(filename).make_preferred().string();

    NSVGimage* image = BitmapCache::nsvgParseFromFileWithReplace(nowidePath.c_str(), "px", 96.0f, { { "\"#808080\"", "\"#FFFFFF\"" } });

    if (image == nullptr) {
        return false;
    }

    float scale = (float)width / std::max(image->width, image->height);

    int n_pixels = width * height;

    if (n_pixels <= 0) {
        nsvgDelete(image);
        return false;
    }

    NSVGrasterizer* rast = nsvgCreateRasterizer();
    if (rast == nullptr) {
        nsvgDelete(image);
        return false;
    }
    std::vector<unsigned char> data(n_pixels * 4, 0);
    nsvgRasterize(rast, image, 0, 0, scale, data.data(), width, height, width * 4);

    bool compress = false;
    GLint last_texture;
    unsigned m_image_texture{ 0 };
    unsigned char* pixels = (unsigned char*)(&data[0]);

    glsafe(::glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture));
    glsafe(::glGenTextures(1, &m_image_texture));
    glsafe(::glBindTexture(GL_TEXTURE_2D, m_image_texture));
    glsafe(::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    glsafe(::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    glsafe(::glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
    glsafe(::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

    // Store our identifier
    texture_id = (ImTextureID)(intptr_t)m_image_texture;

    // Restore state
    glsafe(::glBindTexture(GL_TEXTURE_2D, last_texture));

    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);

    return true;
}

} // namespace GUI
} // namespace Slic3r
