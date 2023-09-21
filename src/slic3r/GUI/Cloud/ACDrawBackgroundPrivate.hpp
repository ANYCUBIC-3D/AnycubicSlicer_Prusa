#ifndef AC_DRAW_BACKGROUND_PRIVATE_HPP
#define AC_DRAW_BACKGROUND_PRIVATE_HPP
/*---------------------------------------------------------------------------------
								用于创建通用背景界面绘制
-----------------------------------------------------------------------------------*/

#include "../GUI_Utils.hpp"
#include <string>

namespace Slic3r {

namespace GUI {

class ACDrawBackgroundPrivate : public DPIPanelEX
{
public:
	ACDrawBackgroundPrivate(wxWindow* parent, const std::string& fileName, int w, int h);
	ACDrawBackgroundPrivate(const ACDrawBackgroundPrivate& obj) = delete;
	int state{ 1 };//此特例用于绘制云登录界面的icon，100放弃
protected:
	void msw_rescale();
	void on_dpi_changed(const wxRect& suggested_rect) { msw_rescale(); };
	void OnPaint(wxPaintEvent& event);
private:
	int w{ 10 };//image width
	int h{ 10 };//image height
	wxBitmap bitmap;
	wxBitmap logoIcon;//特例 显示AC FDM logo
	wxImage imageHigh;
	wxImage image;
	std::string filePath;
};

} // GUI
} // Slic3r

#endif //AC_DRAW_BACKGROUND_PRIVATE_HPP
