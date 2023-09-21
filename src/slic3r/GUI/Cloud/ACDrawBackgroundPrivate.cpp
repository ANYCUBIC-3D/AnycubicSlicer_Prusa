#include "ACDrawBackgroundPrivate.hpp"

#include "../GUI.hpp"
#include "../GUI_Utils.hpp"
#include "../ACLabel.hpp"
#include "../wxExtensions.hpp"
#include "../I18N.hpp"
#include "libslic3r/Utils.hpp"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <boost/filesystem.hpp>


namespace Slic3r {

namespace GUI {

ACDrawBackgroundPrivate::ACDrawBackgroundPrivate(wxWindow* parent, const std::string& fileName, int width, int height) :
	DPIPanelEX(parent, wxID_ANY, wxDefaultPosition, wxSize(width, height), wxNO_BORDER),
	w { width },
	h { height }
{
	wxInitAllImageHandlers();
	//wxPNGHandler();
	std::string name = fileName;
	if (boost::filesystem::exists(name + ".png"))
		filePath = name + ".png";
	else
		filePath = Slic3r::var(name + ".png");
	//filePath = ((boost::filesystem::path(resources_dir()) / "icons").make_preferred() / name).string() + ".png";

	wxFileInputStream imageStream(from_u8(filePath));//用于加载大文件
	imageHigh = wxImage(imageStream, wxBITMAP_TYPE_ANY /*wxBITMAP_TYPE_PNG*/);//这里不能指定为PNG，否则加载会失败

	image = imageHigh;
	wxSize size = FromDIP(wxSize(w, h));
	SetMinSize(size);
	image.Rescale(size.GetWidth(), size.GetHeight(), wxIMAGE_QUALITY_HIGH);
	bitmap = wxBitmap(image);

	wxBitmapBundle b = *get_bmp_bundle("logo_192px", FromDIP(86));
	logoIcon = b.GetBitmap(FromDIP(wxSize(86, 86)));
	
	Bind(wxEVT_PAINT, &ACDrawBackgroundPrivate::OnPaint, this);
}

void ACDrawBackgroundPrivate::OnPaint(wxPaintEvent& event)
{
	wxAutoBufferedPaintDC paintDC(this);

	std::unique_ptr<wxGraphicsContext> gc{ wxGraphicsContext::Create(paintDC) };
	//wxGraphicsContext* gc = wxGraphicsContext::Create(paintDC);
	
	if (gc && bitmap.IsOk())
	{
		//gc->ClearRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight());
		gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
		//gc->DrawRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight());
		gc->DrawBitmap(bitmap, 0, 0, bitmap.GetWidth(), bitmap.GetHeight());
		gc->DrawRoundedRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight(), 16);

		//特殊定制
		if (100 == state)//绘制额外的logo图标跟 "For Freedom to Make" 文本
		{
			gc->DrawBitmap(logoIcon, FromDIP(109), FromDIP(268), FromDIP(86), FromDIP(86));
			gc->SetPen(wxPen(wxColour(*wxWHITE), 2));
			wxFont font = ACLabel::sysFont(37, true);
			gc->SetFont(font, *wxWHITE);
			gc->DrawText(_L("For Freedom to Make"), FromDIP(109), FromDIP(388));
		}

		//delete gc; //析构
	}
}
void ACDrawBackgroundPrivate::msw_rescale()
{
	wxBitmapBundle b = *get_bmp_bundle("logo_192px", FromDIP(86));
	logoIcon = b.GetBitmap(FromDIP(wxSize(86, 86)));

	//为了获取高清图案
	image = imageHigh;
	wxSize size = FromDIP(wxSize(w, h));
	image.Rescale(size.GetWidth(), size.GetHeight(), wxIMAGE_QUALITY_HIGH);
	bitmap = wxBitmap(image);

	SetMinSize(size);
	Fit();
	Refresh();
}


} // GUI
} // Slic3r


