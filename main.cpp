// firmir-paint: draw with the mouse on a canvas (color palette, brush size),
// save the result to a PNG file.
//
//   firmir-paint [output.png]         (default: ./painting.png)
//   firmir-paint --selftest out.png   (draws two strokes and exits, no UI)
//
// The UI is declared in paint.fml (compiled by firmlc); the canvas and the
// save logic live in PaintCanvas.hpp / PaintApp.cpp.
//
// Backend selection via FIRMIR_GUI_DISPLAY / FIRMIR_GUI_INPUT /
// FIRMIR_GUI_PLUGIN_PATH, like every firmir-gui app.

#include "paint.hpp"

#include "PaintApp.hpp"
#include "PaintCanvas.hpp"

#include <firmir/gui/platform.hpp>
#include <firmir/gui/screen.hpp>
#include <firmir/gui/window.hpp>
#include <firmir/log.hpp>
#include <firmir/thread.hpp>

#include <cstdio>
#include <string>

int main(int argc, char** argv) {
    if (argc > 2 && std::string(argv[1]) == "--selftest") {
        PaintCanvas canvas;
        canvas.setWidth(200);
        canvas.setHeight(120);
        canvas.setBrushColor(Firmir::Gui::Color::rgb(0xE0, 0x30, 0x30));
        canvas.stroke({20, 20}, {150, 90});
        canvas.setBrushColor(Firmir::Gui::Color::rgb(0x30, 0x60, 0xE0));
        canvas.setBrushRadius(12);
        canvas.stroke({150, 20}, {40, 100});
        PaintApp app;
        app.attach(&canvas, argv[2]);
        app.save();
        const bool ok = app.property("statusText").has_value();
        std::printf("selftest: %dx%d -> %s\n", canvas.imageWidth(),
                    canvas.imageHeight(), argv[2]);
        return ok ? 0 : 1;
    }

    auto* platform = Firmir::Gui::Platform::instance();
    if (platform->screens().empty()) {
        std::fprintf(stderr, "no screens — check FIRMIR_GUI_DISPLAY / "
                             "FIRMIR_GUI_PLUGIN_PATH\n");
        return 1;
    }

    // Creates the window, the toolbar, the canvas and the PaintApp logic
    // object, then shows the window (see paint.fml).
    PaintObjects objects = createPaint(platform->screens().front());
    objects.paintapp->attach(objects.canvas, argc > 1 ? argv[1] : "");
    objects.root->connect<bool>(
        objects.root, Firmir::Gui::Window::signalVisibilityChanged,
        [](bool visible) {
            if (!visible)
                Firmir::Thread::mainThread()->quit();
        });

    const int rc = Firmir::Thread::mainThread()->exec();
    Firmir::logInfo() << "firmir-paint: bye";
    delete objects.root; // owns the whole tree
    return rc;
}
