#include "ThumbnailData.hpp"

namespace Slic3r {

void ThumbnailData::set(unsigned int w, unsigned int h)
{
    if ((w == 0) || (h == 0))
        return;

    if ((width != w) || (height != h))
    {
        width = w;
        height = h;
        if (!precoding) {
            // defaults to white texture
            pixels = std::vector<unsigned char>(width * height * 4, 255);
        }
    }
}

void ThumbnailData::reset()
{
    width = 0;
    height = 0;
    precoding = false;
    pixels.clear();
}

bool ThumbnailData::is_valid() const
{
    bool result = (width != 0) && (height != 0);
    if (!precoding) {
        result = result && ((unsigned int)pixels.size() == 4 * width * height);
    }
    return result;
}

} // namespace Slic3r
