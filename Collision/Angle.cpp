#include "Angle.h"

namespace sf
{
    namespace priv
    {
        constexpr float pi = 3.141592654f;

        constexpr float positiveRemainder(float a, float b)
        {
            assert(b > 0.0f);
            const float val = a - static_cast<float>(static_cast<int>(a / b)) * b;
            if (val >= 0.f)
                return val;
            else
                return val + b;
        }
    }

    constexpr Angle::Angle() :
        m_degrees(0.0f)
    {
    }

    constexpr Angle::Angle(float degrees) :
        m_degrees(degrees)
    {
    }

    float Angle::asDegrees() const
    {
        return m_degrees;
    }

    float Angle::asRadians() const
    {
        return m_degrees * (priv::pi / 180);
    }

    Angle Angle::wrapSigned() const
    {
        return degrees(priv::positiveRemainder(m_degrees + 180, 360) - 180);
    }

    Angle Angle::wrapUnsigned() const
    {
        return degrees(priv::positiveRemainder(m_degrees, 360));
    }


    Angle degrees(float angle)
    {
        return Angle(angle);
    }

    ////////////////////////////////////////////////////////////
    Angle radians(float angle)
    {
        return Angle(angle * (180 / priv::pi));
    }

    bool operator ==(Angle left, Angle right)
    {
        return left.asDegrees() == right.asDegrees();
    }


    ////////////////////////////////////////////////////////////
   bool operator !=(Angle left, Angle right)
    {
        return left.asDegrees() != right.asDegrees();
    }


    ////////////////////////////////////////////////////////////
    bool operator <(Angle left, Angle right)
    {
        return left.asDegrees() < right.asDegrees();
    }


    ////////////////////////////////////////////////////////////
     bool operator >(Angle left, Angle right)
    {
        return left.asDegrees() > right.asDegrees();
    }


    ////////////////////////////////////////////////////////////
    bool operator <=(Angle left, Angle right)
    {
        return left.asDegrees() <= right.asDegrees();
    }


    ////////////////////////////////////////////////////////////
     bool operator >=(Angle left, Angle right)
    {
        return left.asDegrees() >= right.asDegrees();
    }


    ////////////////////////////////////////////////////////////
    Angle operator -(Angle right)
    {
        return degrees(-right.asDegrees());
    }


    ////////////////////////////////////////////////////////////
     Angle operator +(Angle left, Angle right)
    {
        return degrees(left.asDegrees() + right.asDegrees());
    }


    ////////////////////////////////////////////////////////////
    Angle& operator +=(Angle& left, Angle right)
    {
        return left = left + right;
    }


    ////////////////////////////////////////////////////////////
    Angle operator -(Angle left, Angle right)
    {
        return degrees(left.asDegrees() - right.asDegrees());
    }


    ////////////////////////////////////////////////////////////
    Angle& operator -=(Angle& left, Angle right)
    {
        return left = left - right;
    }


    ////////////////////////////////////////////////////////////
    Angle operator *(Angle left, float right)
    {
        return degrees(left.asDegrees() * right);
    }


    ////////////////////////////////////////////////////////////
    Angle operator *(float left, Angle right)
    {
        return right * left;
    }


    ////////////////////////////////////////////////////////////
    Angle& operator *=(Angle& left, float right)
    {
        return left = left * right;
    }


    ////////////////////////////////////////////////////////////
    Angle operator /(Angle left, float right)
    {
        return degrees(left.asDegrees() / right);
    }


    ////////////////////////////////////////////////////////////
    Angle& operator /=(Angle& left, float right)
    {
        return left = left / right;
    }


    ////////////////////////////////////////////////////////////
    float operator /(Angle left, Angle right)
    {
        return left.asDegrees() / right.asDegrees();
    }


    ////////////////////////////////////////////////////////////
    Angle operator %(Angle left, Angle right)
    {
        return degrees(priv::positiveRemainder(left.asDegrees(), right.asDegrees()));
    }


    ////////////////////////////////////////////////////////////
    Angle& operator %=(Angle& left, Angle right)
    {
        return left = left % right;
    }

    constexpr Angle Angle::Zero;
}