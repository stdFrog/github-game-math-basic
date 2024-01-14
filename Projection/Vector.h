#ifndef __VECTOR_H_
#define __VECTOR_H_
#include <math.h>

template <typename _TYPE>
class Vector
{
	public:
		_TYPE x,y,z;

		Vector(_TYPE ax = 0, _TYPE ay = 0, _TYPE az = 0) : x(ax), y(ay), z(az) {;}
		
		Vector operator +(const Vector& Other) const
		{
			return Vector(x + Other.x, y + Other.y, z + Other.z);
		}

		Vector& operator +=(const Vector& Other)
		{
			x += Other.x;
			y += Other.y;
			z += Other.z;

			return *this;
		}

		Vector operator *(const Vector& Other) const
		{
			return Vector(x * Other.x, y * Other.y, z * Other.z);
		}

		Vector& operator *=(const Vector& Other)
		{
			x *= Other.x;
			y *= Other.y;
			z *= Other.z;

			return *this;
		}

		Vector operator -() const
		{
			return Vector(-x, -y, -z);
		}

		Vector operator -(const Vector& Other) const
		{
			return Vector(x - Other.x, y - Other.y, z - Other.z);
		}

		Vector& operator -=(const Vector& Other)
		{
			x -= Other.x;
			y -= Other.y;
			z -= Other.z;

			return *this;
		}

		Vector operator /(const Vector& Other) const
		{
			return Vector(x / Other.x, y / Other.y, z / Other.z);
		}

		Vector& operator /=(const Vector& Other)
		{
			x /= Other.x;
			y /= Other.y;
			z /= Other.z;

			return *this;
		}

		Vector operator +(const _TYPE value) const
		{
			return Vector(x + value, y + value, z + value);
		}

		Vector& operator +=(const _TYPE value)
		{
			x += value;
			y += value;
			z += value;

			return *this;
		}

		Vector operator *(const _TYPE value) const
		{
			return Vector(x * value, y * value, z * value);
		}

		Vector& operator *=(const _TYPE value)
		{
			x *= value;
			y *= value;
			z *= value;

			return *this;
		}

		Vector operator -(const _TYPE value) const
		{
			return Vector(x - value, y - value, z - value);
		}

		Vector& operator -=(const _TYPE value)
		{
			x -= value;
			y -= value;
			z -= value;

			return *this;
		}

		Vector operator /(const _TYPE value) const
		{
			return Vector(x / value, y / value, z / value);
		}

		Vector& operator /=(const _TYPE value)
		{
			x /= value;
			y /= value;
			z /= value;

			return *this;
		}
		
		_TYPE Squared()
		{
			return x * x + y * y + z * z;
		}

		_TYPE Norm()
		{
			return sqrt(Squared());
		}

		_TYPE Dot(const Vector& Other) const
		{
			return x * Other.x + y * Other.y + z * Other.z;
		}

		Vector Cross(const Vector& Other) const
		{
			return Vector(
						y * Other.z - Other.y * z,
						z * Other.x - Other.z * x,
						x * Other.y - Other.x * y
					);
		}

		Vector GetNormalize()
		{
			_TYPE sqr = Squared();

			if(sqr == 0.f)
			{
				return Vector(0,0,0);
			}

			if(sqr == 1.f)
			{
				return *this;
			}

			_TYPE length = sqrt(sqr);
			return Vector(x, y, z) / length;
		}

		void Normalize()
		{
			*this = GetNormalize();
		}
};

#endif
