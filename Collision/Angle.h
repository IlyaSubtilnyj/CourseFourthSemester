#pragma once
#include <assert.h>

namespace sf
{

	class Angle
	{
    public:

        ////////////////////////////////////////////////////////////
        /// \brief Default constructor
        ///
        /// Sets the angle value to zero.
        ///
        ////////////////////////////////////////////////////////////
        constexpr Angle();

        ////////////////////////////////////////////////////////////
        /// \brief Return the angle's value in degrees
        ///
        /// \return Angle in degrees
        ///
        /// \see asRadians
        ///
        ////////////////////////////////////////////////////////////
        float asDegrees() const;

        ////////////////////////////////////////////////////////////
        /// \brief Return the angle's value in radians
        ///
        /// \return Angle in radians
        ///
        /// \see asDegrees
        ///
        ////////////////////////////////////////////////////////////
        float asRadians() const;

        ////////////////////////////////////////////////////////////
        /// \brief Wrap to a range such that -180° <= angle < 180°
        ///
        /// Similar to a modulo operation, this returns a copy of the angle
        /// constrained to the range [-180°, 180°) == [-Pi, Pi).
        /// The resulting angle represents a rotation which is equivalent to *this.
        ///
        /// The name "signed" originates from the similarity to signed integers:
        /// <table>
        /// <tr>
        ///   <th></th>
        ///   <th>signed</th>
        ///   <th>unsigned</th>
        /// </tr>
        /// <tr>
        ///   <td>char</td>
        ///   <td>[-128, 128)</td>
        ///   <td>[0, 256)</td>
        /// </tr>
        /// <tr>
        ///   <td>Angle</td>
        ///   <td>[-180°, 180°)</td>
        ///   <td>[0°, 360°)</td>
        /// </tr>
        /// </table>
        ///
        /// \return Signed angle, wrapped to [-180°, 180°)
        ///
        /// \see wrapUnsigned
        ///
        ////////////////////////////////////////////////////////////
        Angle wrapSigned() const;

        ////////////////////////////////////////////////////////////
        /// \brief Wrap to a range such that 0° <= angle < 360°
        ///
        /// Similar to a modulo operation, this returns a copy of the angle
        /// constrained to the range [0°, 360°) == [0, Tau) == [0, 2*Pi).
        /// The resulting angle represents a rotation which is equivalent to *this.
        ///
        /// The name "unsigned" originates from the similarity to unsigned integers:
        /// <table>
        /// <tr>
        ///   <th></th>
        ///   <th>signed</th>
        ///   <th>unsigned</th>
        /// </tr>
        /// <tr>
        ///   <td>char</td>
        ///   <td>[-128, 128)</td>
        ///   <td>[0, 256)</td>
        /// </tr>
        /// <tr>
        ///   <td>Angle</td>
        ///   <td>[-180°, 180°)</td>
        ///   <td>[0°, 360°)</td>
        /// </tr>
        /// </table>
        ///
        /// \return Unsigned angle, wrapped to [0°, 360°)
        ///
        /// \see wrapSigned
        ///
        ////////////////////////////////////////////////////////////
        Angle wrapUnsigned() const;

        ////////////////////////////////////////////////////////////
        // Static member data
        ////////////////////////////////////////////////////////////
        static const Angle Zero; //!< Predefined 0 degree angle value

    private:

        friend Angle degrees(float angle);
        friend Angle radians(float angle);

    public:
        ////////////////////////////////////////////////////////////
        /// \brief Construct from a number of degrees
        ///
        /// This function is internal. To construct angle values,
        /// use sf::radians or sf::degrees instead.
        ///
        /// \param degrees Angle in degrees
        ///
        ////////////////////////////////////////////////////////////
        constexpr Angle(float degrees);

    private:
		float m_degrees; //!< Angle value stored as degrees
	};


    Angle degrees(float angle);

    Angle radians(float angle);

    bool operator ==(Angle left, Angle right);

    bool operator !=(Angle left, Angle right);

    bool operator <(Angle left, Angle right);

    bool operator >(Angle left, Angle right);

    bool operator <=(Angle left, Angle right);

    bool operator >=(Angle left, Angle right);

    Angle operator -(Angle right);

    Angle operator +(Angle left, Angle right);

    Angle& operator +=(Angle& left, Angle right);

    Angle operator -(Angle left, Angle right);

    Angle& operator -=(Angle& left, Angle right);

    Angle operator *(Angle left, float right);

    Angle operator *(float left, Angle right);

    Angle& operator *=(Angle& left, float right);

    Angle operator /(Angle left, float right);

    Angle& operator /=(Angle& left, float right);

    float operator /(Angle left, Angle right);

    Angle operator %(Angle left, Angle right);

    Angle& operator %=(Angle& left, Angle right);

}


