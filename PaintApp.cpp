#include "PaintApp.hpp"

#include "PaintCanvas.hpp"

#ifdef FIRMIR_PAINT_HAVE_PNG
#include <png.h>
#endif

#include <cstdio>
#include <vector>

namespace {

#ifdef FIRMIR_PAINT_HAVE_PNG
bool savePng(const std::string& path, const std::vector<std::uint8_t>& rgba,
             int w, int h) {
    FILE* file = std::fopen(path.c_str(), "wb");
    if (!file)
        return false;
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr,
                                              nullptr, nullptr);
    png_infop info = png ? png_create_info_struct(png) : nullptr;
    if (!info || setjmp(png_jmpbuf(png))) {
        if (png)
            png_destroy_write_struct(&png, info ? &info : nullptr);
        std::fclose(file);
        return false;
    }
    png_init_io(png, file);
    png_set_IHDR(png, info, w, h, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    std::vector<png_bytep> rows(h);
    for (int y = 0; y < h; ++y)
        rows[y] = const_cast<std::uint8_t*>(rgba.data() +
                                            static_cast<size_t>(y) * w * 4);
    png_write_image(png, rows.data());
    png_write_end(png, nullptr);
    png_destroy_write_struct(&png, &info);
    std::fclose(file);
    return true;
}
#endif

} // namespace

PaintApp::PaintApp(Object* parent) : Object(parent) {
    setProperty("statusText", Firmir::String(""));
}

void PaintApp::attach(PaintCanvas* canvas, const std::string& outPath) {
    m_canvas = canvas;
    if (!outPath.empty())
        m_outPath = outPath;
}

void PaintApp::save() {
#ifdef FIRMIR_PAINT_HAVE_PNG
    const bool ok =
        m_canvas && savePng(m_outPath, m_canvas->pixels(),
                            m_canvas->imageWidth(), m_canvas->imageHeight());
    setProperty("statusText",
                Firmir::String(
                    (ok ? "saved to " + m_outPath : "save failed: " + m_outPath)
                        .c_str()));
#else
    setProperty("statusText",
                Firmir::String("PNG support not built (libpng missing)"));
#endif
}
