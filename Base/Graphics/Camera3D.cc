#include "Camera3D.hh"

#include <DirectXCollision.h>

namespace lr
{
    void Camera3D::Init(const XMFLOAT3 &pos, const XMFLOAT2 &size, const XMFLOAT3 &direction, const XMFLOAT3 &up, float fov, float zNear, float zFar)
    {
        m_Pos = XMVectorSet(pos.x, pos.y, pos.z, 0.0);
        m_Direction = XMVectorSet(direction.x, direction.y, direction.z, 0.0);
        m_Up = XMVectorSet(up.x, up.y, up.z, 0.0);
        m_Fov = fov;
        m_zNear = zNear;
        m_zFar = zFar;
        m_Aspect = size.x / size.y;
        m_Right = XMVector3Normalize(XMVector3Cross(m_Direction, XMVectorSet(1.f, 0.f, 0.f, 0.f)));

        CalculateProjection();
        CalculateView();
    }

    void Camera3D::CalculateView()
    {
        XMVECTOR posDir = m_Pos + m_Direction;
        m_View = XMMatrixLookAtLH(m_Pos, posDir, m_Up);
        m_TransposedMatrix = XMMatrixMultiplyTranspose(m_View, m_Projection);
    }

    void Camera3D::CalculateProjection()
    {
        m_Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_Fov), m_Aspect, m_zNear, m_zFar);
    }

    void Camera3D::SetPosition(const XMFLOAT3 &pos)
    {
        m_Pos = XMVectorSet(pos.x, pos.y, pos.z, 0.0);

        CalculateView();
    }

    void Camera3D::SetSize(const XMFLOAT2 &size)
    {
        m_Aspect = size.x / size.y;

        CalculateProjection();
    }

    void Camera3D::Stop()
    {
        m_MovingDirection = Direction::NONE;
    }

    void Camera3D::Move(Direction direction, bool stop)
    {
        if (!stop)
            m_MovingDirection |= direction;
        else
            m_MovingDirection &= ~direction;
    }

    void Camera3D::SetDirection(float offX, float offY)
    {
        offX *= 0.3f;
        offY *= 0.3f;

        m_Yaw += offX;
        m_Pitch += offY;

        // Block camera direction
        m_Pitch = bx::clamp(m_Pitch, -89.f, 89.f);

        float radPitch = XMConvertToRadians(m_Pitch);
        float radYaw = XMConvertToRadians(m_Yaw);

        m_RotationMat = XMMatrixRotationRollPitchYaw(radPitch, radYaw, 0);
        m_Direction = XMVector3Normalize(XMVector3TransformCoord(XMVectorSet(0.f, 0.f, 1.f, 0.f), m_RotationMat));

        XMMATRIX rotationYMat;
        rotationYMat = XMMatrixRotationY(radYaw);

        // m_Pos = XMVectorMultiply(m_Direction, XMVectorSet(-50.f, -50.f, -50.f, 0.f));  // third person

        m_Right = XMVector3TransformCoord(XMVectorSet(1.f, 0.f, 0.f, 0.f), rotationYMat);
        m_Up = XMVector3TransformCoord(m_Up, rotationYMat);

        CalculateView();
    }

    void Camera3D::Update(float deltaTime)
    {
        XMVECTOR lastPos = m_Pos;

        if (m_MovingDirection & Direction::FORWARD) m_Pos = XMVectorAdd(m_Pos, m_Direction * m_MovingSpeed * deltaTime);
        if (m_MovingDirection & Direction::BACKWARD) m_Pos = XMVectorSubtract(m_Pos, m_Direction * m_MovingSpeed * deltaTime);

        if (m_MovingDirection & Direction::RIGHT) m_Pos = XMVectorAdd(m_Pos, m_Right * m_MovingSpeed * deltaTime);
        if (m_MovingDirection & Direction::LEFT) m_Pos = XMVectorSubtract(m_Pos, m_Right * m_MovingSpeed * deltaTime);

        if (m_MovingDirection & Direction::UP) m_Pos = XMVectorSetY(m_Pos, XMVectorGetY(m_Pos) + (m_MovingSpeed * deltaTime));
        if (m_MovingDirection & Direction::DOWN) m_Pos = XMVectorSetY(m_Pos, XMVectorGetY(m_Pos) - (m_MovingSpeed * deltaTime));

        if (XMVector3NotEqual(lastPos, m_Pos)) CalculateView();
    }

    void Camera3D::GetFrustum(CameraFrustum &frustumOut)
    {
        const XMMATRIX &view = XMMatrixInverse(nullptr, XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 1), m_Direction, m_Up));

        BoundingFrustum frustum;
        BoundingFrustum::CreateFromMatrix(frustum, m_Projection);
        frustum.Transform(frustum, (view));

        XMFLOAT3 pCorners[8];
        frustum.GetCorners(pCorners);

        frustumOut.PointX = pCorners[4];
        frustumOut.PointY = pCorners[5];
        frustumOut.PointZ = pCorners[7];
        frustumOut.PointW = pCorners[6];
    }

}  // namespace lr