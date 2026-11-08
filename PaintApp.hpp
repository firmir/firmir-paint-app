#pragma once

// firmir-paint application logic, exposed to the Firml UI (paint.fml):
// the save button calls save(), the status label binds to "statusText".

#include <firmir/object.hpp>
#include <firmir/string.hpp>

#include <string>

class PaintCanvas;

class PaintApp : public Firmir::Object {
public:
    explicit PaintApp(Object* parent = nullptr);

    /// Output path (argv[1], default ./painting.png) + the canvas to save
    /// (wired from main() once the FML tree exists).
    void attach(PaintCanvas* canvas, const std::string& outPath);

    /// Saves the canvas as a PNG; reports through the "statusText" property.
    void save();

private:
    PaintCanvas* m_canvas = nullptr; // not owned
    std::string m_outPath = "painting.png";
};
