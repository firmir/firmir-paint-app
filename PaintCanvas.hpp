#pragma once

// firmir-paint canvas: an RGBA8 bitmap edited with a round brush and blitted
// through the painter's texture cache. Declared in paint.fml as an opaque
// Firml type; brushColor/brushRadius are registered properties.

#include <firmir/gui/item.hpp>
#include <firmir/gui/painter.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

class PaintCanvas : public Firmir::Gui::Item {
public:
    explicit PaintCanvas(Firmir::Object* parent = nullptr) : Item(parent) {
        setClip(true);
        registerProperty<Firmir::Gui::Color>("brushColor",
                                             &PaintCanvas::brushColor,
                                             &PaintCanvas::setBrushColor);
        registerProperty<float>("brushRadius", &PaintCanvas::brushRadius,
                                &PaintCanvas::setBrushRadius);
    }

    Firmir::Gui::Color brushColor() const { return m_brush; }
    void setBrushColor(Firmir::Gui::Color color) {
        m_brush = color;
        notifyPropertyChanged("brushColor");
    }
    float brushRadius() const { return m_brushRadius; }
    void setBrushRadius(float radius) {
        radius = std::max(1.0f, radius);
        if (m_brushRadius == radius)
            return;
        m_brushRadius = radius;
        notifyPropertyChanged("brushRadius");
    }

    void clear() {
        std::fill(m_pixels.begin(), m_pixels.end(), 255);
        ++m_generation;
        update();
    }

    /// Current bitmap (empty before the first paint).
    const std::vector<std::uint8_t>& pixels() const { return m_pixels; }
    int imageWidth() const { return m_imgW; }
    int imageHeight() const { return m_imgH; }

    /// Draws a stroke programmatically (used by --selftest).
    void stroke(Firmir::Gui::Point from, Firmir::Gui::Point to) {
        stamp(from);
        m_last = from;
        lineTo(to);
    }

protected:
    void mouseEvent(Firmir::Gui::MouseEvent& event) override {
        using Action = Firmir::Gui::MouseEvent::Action;
        if (event.action == Action::Press && event.button == 1) {
            m_drawing = true;
            m_last = event.pos;
            stamp(event.pos);
            event.accepted = true;
        } else if (event.action == Action::Move && m_drawing) {
            lineTo(event.pos);
            event.accepted = true;
        } else if (event.action == Action::Release && m_drawing) {
            m_drawing = false;
            event.accepted = true;
        }
    }

    void paint(Firmir::Gui::Painter& painter) override {
        ensureBuffer();
        if (m_pixels.empty())
            return;
        painter.drawRgba(Firmir::Gui::RectF{0, 0, width(), height()},
                         m_pixels.data(), m_imgW, m_imgH, this, m_generation);
    }

private:
    void ensureBuffer() {
        const int w = std::max(1, static_cast<int>(std::lround(width())));
        const int h = std::max(1, static_cast<int>(std::lround(height())));
        if (w == m_imgW && h == m_imgH)
            return;
        std::vector<std::uint8_t> next(static_cast<size_t>(w) * h * 4, 255);
        const int cw = std::min(w, m_imgW), ch = std::min(h, m_imgH);
        for (int y = 0; y < ch; ++y)
            std::memcpy(next.data() + static_cast<size_t>(y) * w * 4,
                        m_pixels.data() + static_cast<size_t>(y) * m_imgW * 4,
                        static_cast<size_t>(cw) * 4);
        m_pixels = std::move(next);
        m_imgW = w;
        m_imgH = h;
        ++m_generation;
    }

    void stamp(Firmir::Gui::Point p) {
        ensureBuffer();
        const int cx = static_cast<int>(p.x + 0.5f);
        const int cy = static_cast<int>(p.y + 0.5f);
        const int r = static_cast<int>(m_brushRadius + 0.5f);
        for (int dy = -r; dy <= r; ++dy) {
            const int span = static_cast<int>(
                std::sqrt(static_cast<double>(r * r - dy * dy)));
            for (int dx = -span; dx <= span; ++dx) {
                const int x = cx + dx, y = cy + dy;
                if (x < 0 || y < 0 || x >= m_imgW || y >= m_imgH)
                    continue;
                std::uint8_t* px =
                    m_pixels.data() + (static_cast<size_t>(y) * m_imgW + x) * 4;
                px[0] = m_brush.r;
                px[1] = m_brush.g;
                px[2] = m_brush.b;
                px[3] = 255;
            }
        }
        ++m_generation;
        update();
    }

    void lineTo(Firmir::Gui::Point p) {
        const float dx = p.x - m_last.x, dy = p.y - m_last.y;
        const float dist = std::sqrt(dx * dx + dy * dy);
        const int steps =
            std::max(1, static_cast<int>(dist / (m_brushRadius * 0.5f)));
        for (int i = 1; i <= steps; ++i)
            stamp({m_last.x + dx * i / steps, m_last.y + dy * i / steps});
        m_last = p;
    }

    std::vector<std::uint8_t> m_pixels; // RGBA8, white background
    int m_imgW = 0;
    int m_imgH = 0;
    std::uint64_t m_generation = 0;
    bool m_drawing = false;
    Firmir::Gui::Point m_last;
    Firmir::Gui::Color m_brush = Firmir::Gui::Color::rgb(0x20, 0x20, 0x20);
    float m_brushRadius = 6.0f;
};
