//
// Created on July 1st 2021 by e-erdal.
//

#pragma once

namespace lr
{
    enum class Direction
    {
        NONE,
        FORWARD = 1 << 0,
        BACKWARD = 1 << 1,
        LEFT = 1 << 2,
        RIGHT = 1 << 3,
        UP = 1 << 4,
        DOWN = 1 << 5
    };
    BitFlags(Direction);

    struct CameraFrustum
    {
        XMFLOAT3 PointX;  // Top left
        float __p;
        XMFLOAT3 PointY;  // Top right
        float __p1;
        XMFLOAT3 PointZ;  // Bottom left
        float __p2;
        XMFLOAT3 PointW;  // Bottom right
        float __p3;
    };

    class Camera3D
    {
    public:
        void Init(const XMFLOAT3 &pos, const XMFLOAT2 &size, const XMFLOAT3 &direction, const XMFLOAT3 &up, float fov, float zNear, float zFar);

        void CalculateView();
        void CalculateProjection();

        void SetPosition(const XMFLOAT3 &pos);
        void SetSize(const XMFLOAT2 &size);

        void Stop();
        void Move(Direction direction, bool stop);
        void SetDirection(float offX, float offY);

        void Update(float deltaTime);

        void GetFrustum(CameraFrustum &frustumOut);

    public:
        const XMMATRIX &GetProjection() const
        {
            return m_Projection;
        }

        XMMATRIX &GetProjection()
        {
            return m_Projection;
        }

        const XMMATRIX &GetView() const
        {
            return m_View;
        }

        XMMATRIX &GetView()
        {
            return m_View;
        }

        const XMMATRIX &GetTransposedMatrix() const
        {
            return m_TransposedMatrix;
        }

        XMMATRIX &GetTransposedMatrix()
        {
            return m_TransposedMatrix;
        }

        const XMVECTOR &GetPosition() const
        {
            return m_Pos;
        }

        const XMVECTOR &GetDirection() const
        {
            return m_Direction;
        }

        float GetAspect()
        {
            return m_Aspect;
        }

        float GetFov()
        {
            return m_Fov;
        }

    private:
        XMMATRIX m_Projection = {};
        XMMATRIX m_View = {};
        XMMATRIX m_RotationMat = {};
        XMMATRIX m_TransposedMatrix = {};

        XMVECTOR m_Pos = {};
        XMVECTOR m_Direction = {};
        XMVECTOR m_Right = {};
        XMVECTOR m_Up = {};

        float m_Fov = 0.f;
        float m_Aspect = 0.f;
        float m_zNear = 0.f;
        float m_zFar = 0.f;
        float m_MovingSpeed = 50.f;
        float m_Pitch = 0, m_Yaw = 0;

        Direction m_MovingDirection = Direction::NONE;
    };
}  // namespace lr