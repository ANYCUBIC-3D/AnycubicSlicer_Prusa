#include "ACScaleBitmapHighQuality.hpp"

#include "libslic3r/Utils.hpp"
#include <boost/filesystem.hpp>
//#include "libslic3r/Preset.hpp"

#include "wx/string.h"


ACScaleBitmapHighQuality::ACScaleBitmapHighQuality(wxWindow* parent, wxWindowID id, const std::string& name)
	: wxStaticBitmap(parent, id, wxNullBitmap)
{
	wxInitAllImageHandlers();
	LoadPngImage(name);
}
void ACScaleBitmapHighQuality::LoadPngImage(const std::string& name)
{
	std::string filePath;
	if (boost::filesystem::exists(name + ".png"))
		filePath = name + ".png";
	else
		filePath = Slic3r::var(name + ".png");
	wxImage image(wxString::FromUTF8(filePath.c_str()), wxBITMAP_TYPE_ANY);
	if (image.IsOk())
	{
		originImage = image;
		UpdateBitmap();
	}
}
void ACScaleBitmapHighQuality::UpdateBitmap()
{
	if (scaleBitmap.IsOk())
		SetBitmap(scaleBitmap);
	else
		SetBitmap(wxNullBitmap);
}
void ACScaleBitmapHighQuality::ScaleImage(int width, int height)
{
	if (originImage.IsOk())
	{
		wxImage image = originImage;
		wxImage scaledImage = image.Rescale(width, height, wxIMAGE_QUALITY_HIGH);
		if (scaledImage.IsOk())
		{
			scaleBitmap = wxBitmap(scaledImage);
			UpdateBitmap();
		}
	}
}

