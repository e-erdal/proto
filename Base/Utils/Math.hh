//
// Created on July 1st 2021 by e-erdal.
//

#pragma once

namespace lr::Math
{
    constexpr float PI = 3.14159265358979323f;
    constexpr float PI2 = 6.283185307f;
    constexpr float PIDIV2 = 1.570796327f;
    constexpr float E = 2.71828182846f;

    inline constexpr float Approach(float target, float current, float delta)
    {
        float deltaVel = target - current;
        if (deltaVel > delta) return current + delta;
        if (deltaVel < -delta) return current - delta;

        return target;
    }

    // DXDMATH function
    inline void ScalarSinCos(float *pSin, float *pCos, float fVal)
    {
        float quotient = 0.15915494309189533577f * fVal;
        if (fVal >= 0.0f)
        {
            quotient = (float)((int)(quotient + 0.5f));
        }
        else
        {
            quotient = (float)((int)(quotient - 0.5f));
        }

        float y = fVal - PI2 * quotient;
        float sign;

        if (y > PIDIV2)
        {
            y = PI - y;
            sign = -1.0f;
        }
        else if (y < -PIDIV2)
        {
            y = -PI - y;
            sign = -1.0f;
        }
        else
        {
            sign = +1.0f;
        }

        float y2 = y * y;

        *pSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;
        float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
        *pCos = sign * p;
    }

    constexpr XMFLOAT3 ToFloat3(const XMVECTOR &v)
    {
        return XMFLOAT3(XMVectorGetX(v), XMVectorGetY(v), XMVectorGetZ(v));
    }

}  // namespace lr::Math