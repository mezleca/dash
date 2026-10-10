#include "camera.hpp"
#include "core/math.hpp"

void CameraView::set_target(Vector2 target, Vector2 smoothing) {
    m_view.target = {d_math::lerp(m_view.target.x, target.x, smoothing.x), d_math::lerp(m_view.target.y, target.y, smoothing.y)};
}

void CameraView::center_viewport(Vector2 size) {
    m_view.offset = {size.x / 2.0f, size.y / 2.0f};
}
