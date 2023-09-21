#ifndef slic3r_GUI_ACSCALE_BITMAP_HIGH_QUALITY_HPP
#define slic3r_GUI_ACSCALE_BITMAP_HIGH_QUALITY_HPP

#include <wx/bitmap.h>
#include <wx/window.h>
#include <wx/image.h>
#include <wx/statbmp.h>

#include <string>

class ACScaleBitmapHighQuality : public wxStaticBitmap
{
public:
	ACScaleBitmapHighQuality(wxWindow* parent = nullptr, wxWindowID id = wxID_ANY, const std::string& name = "");


public:
	void LoadPngImage(const std::string& name);
	void UpdateBitmap();
	void ScaleImage(int width, int height);
protected:

private:
	wxImage originImage;
	wxBitmap scaleBitmap;
};

#endif // !slic3r_GUI_ACSCALE_BITMAP_HIGH_QUALITY_HPP
