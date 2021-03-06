#pragma once

#include "Application/InputEvent.h"
#include "Render/Camera.h"

class CameraController
{
public:
    enum class Mode
    {
        FirsetPerson,
        Orbiter,
        SixDOF,
    };

    CameraController(const SPtr<Camera> &camera)
        : camera(camera)
    {
    }

    virtual ~CameraController() = default;

    virtual Mode GetMode() const = 0;
    virtual bool Update() = 0;

    virtual void OnTouchDown(TouchDownEvent &event)
    {
    }

    virtual void OnTouchUp(TouchUpEvent &event)
    {
    }

    virtual void OnMouseMoved(MouseMovedEvent &event)
    {
    }

    virtual void OnMouseScrolled(MouseScrolledEvent &event)
    {
    }

    virtual void OnKeyDown(KeyDownEvent &event)
    {
    }

    virtual void OnKeyUp(KeyUpEvent &event)
    {
    }

    void SetSpeed(float inSpeed)
    {
        speed = inSpeed;
    }

    float GetSpeed() const
    {
        return speed;
    }

    void SetViewport(float w, float h)
    {
        viewportWidth = w;
        viewportHeight = h;
        if (camera)
            camera->SetAspectRatio(w / h);
    }

    const SPtr<Camera> &GetCamera() const
    {
        return camera;
    }

protected:
    SPtr<Camera> camera;
    float speed = 0.1f;
    float viewportWidth = 1.0f;
    float viewportHeight = 1.0f;
};

class OrbiterCameraController : public CameraController
{
public:
    OrbiterCameraController(const SPtr<Camera> &camera)
        : CameraController(camera)
    {
    }

    void SetModelParams(const Vector3 &center, float radius, float distance);

    virtual bool Update() override;
    virtual void OnTouchDown(TouchDownEvent &event) override;
    virtual void OnTouchUp(TouchUpEvent &event) override;
    virtual void OnMouseMoved(MouseMovedEvent &event) override;
    virtual void OnMouseScrolled(MouseScrolledEvent &event) override;

    virtual Mode GetMode() const override
    {
        return CameraController::Mode::Orbiter;
    }

    static SPtr<OrbiterCameraController> Create(const SPtr<Camera> &camera)
    {
        return Memory::MakeShared<OrbiterCameraController>(camera);
    }

private:
    Vector3 modelCenter;
    float modelRadius;
    float cameraDistance;
    Quat rotation;
    Vector3 lastVector;
    bool dragging = false;

    bool dirty = false;
};

template <bool b6DOF>
class TFirstPersonCameraController : public CameraController
{
public:
    TFirstPersonCameraController(const SPtr<Camera> &camera)
        : CameraController(camera)
    {
    }

    virtual bool Update() override;
    virtual void OnTouchDown(TouchDownEvent &event) override;
    virtual void OnTouchUp(TouchUpEvent &event) override;
    virtual void OnMouseMoved(MouseMovedEvent &event) override;
    virtual void OnMouseScrolled(MouseScrolledEvent &event) override;
    virtual void OnKeyDown(KeyDownEvent &event) override;
    virtual void OnKeyUp(KeyUpEvent &event) override;

    virtual Mode GetMode() const override
    {
        return b6DOF ? CameraController::Mode::SixDOF : CameraController::Mode::FirsetPerson;
    }

    static SPtr<TFirstPersonCameraController> Create(const SPtr<Camera> &camera)
    {
        return Memory::MakeShared<TFirstPersonCameraController>(camera);
    }

protected:
    static constexpr int32 DIR_FORWARD = 0;
    static constexpr int32 DIR_BACKWARD = 1;
    static constexpr int32 DIR_RIGHT = 2;
    static constexpr int32 DIR_LEFT = 3;
    static constexpr int32 DIR_UP = 4;
    static constexpr int32 DIR_DOWN = 5;

    bool keyPressed[6] = { 0 };
    bool leftBtnDown = false;
    bool rightBtnDown = false;
    bool shouldRotate = false;
    Vector2 lastPos;
    Vector2 delta;
};

using FirstPersonCameraController = TFirstPersonCameraController<false>;
using SixDOFCameraController = TFirstPersonCameraController<true>;