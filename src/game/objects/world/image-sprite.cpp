#include "image-sprite.hpp"

#include "core/components/rigid-body.hpp"
#include "core/object.hpp"

#include <raymath.h>
#include <rlgl.h>

ImageSprite::ImageSprite(GameObject& object, ImageSpace space, bool fill_viewport)
    : Sprite(object), m_space(space), m_fill_viewport(fill_viewport) {}

void ImageSprite::render(Rectangle bounds, const Camera2D& camera, Vector2 viewport_size) const {
    if (m_space == ImageSpace::World) {
        Sprite::render(bounds, camera, viewport_size);
        return;
    }

    bounds = render_bounds(bounds, camera, viewport_size);

    // cancel the camera transform for static images
    rlPushMatrix();
    rlTranslatef(camera.target.x, camera.target.y, 0.0F);
    rlRotatef(-camera.rotation, 0.0F, 0.0F, 1.0F);
    rlScalef(1.0F / camera.zoom, 1.0F / camera.zoom, 1.0F);
    Sprite::render(bounds, camera, viewport_size);
    rlPopMatrix();
}

Camera2D ImageSprite::render_camera(const Camera2D& camera) const {
    return m_space == ImageSpace::Static ? Camera2D{camera.offset, {}, 0.0F, 1.0F} : camera;
}

Rectangle ImageSprite::render_bounds(Rectangle bounds, const Camera2D& camera, Vector2 viewport_size) const {
    if (fill_viewport()) {
        return {-camera.offset.x, -camera.offset.y, viewport_size.x, viewport_size.y};
    }
    return bounds;
}

void ImageSprite::set_space(ImageSpace space, const Camera2D& camera, Vector2 viewport_size) {
    if (m_space == space) {
        return;
    }

    auto& body = *object().get_component<RigidBody>();

    const Camera2D source = render_camera(camera);
    const Rectangle bounds = render_bounds(body.get_bounding_box(), camera, viewport_size);
    const Vector2 screen_center = GetWorldToScreen2D({bounds.x + bounds.width * 0.5F, bounds.y + bounds.height * 0.5F}, source);
    const Vector2 screen_size = {bounds.width * source.zoom, bounds.height * source.zoom};
    const float screen_rotation = rotation + source.rotation;

    m_space = space;

    const Camera2D destination = render_camera(camera);
    const Vector2 center = GetScreenToWorld2D(screen_center, destination);

    body.set_size(Vector2Scale(screen_size, 1.0F / destination.zoom), {0.5F, 0.5F});
    const Rectangle resized = body.get_bounding_box();
    const Vector2 position = body.get_position();

    body.set_position(
        position.x + center.x - resized.x - resized.width * 0.5F, position.y + center.y - resized.y - resized.height * 0.5F
    );
    rotation = screen_rotation - destination.rotation;
    body.set_rotation(rotation);
}

void ImageSprite::serialize(Serializer& serializer) const {
    Sprite::serialize(serializer);
    serializer.field("space", std::string_view(m_space == ImageSpace::Static ? "static" : "world"));

    if (fill_viewport()) {
        serializer.field("fill_viewport", true);
    }
}

void ImageSprite::deserialize(Deserializer& deserializer, const std::filesystem::path& directory) {
    Sprite::deserialize(deserializer, directory);
    std::string space = "world";
    deserializer.field("space", space);
    m_space = space == "static" ? ImageSpace::Static : ImageSpace::World;
    m_fill_viewport = false;
    deserializer.field("fill_viewport", m_fill_viewport);
}
