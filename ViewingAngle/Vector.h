#ifndef __VECTOR_H_
#define __VECTOR_H_
#include <math.h>

/*
class Vector
{
	public:
	float x,y,z;
	
	Vector(float ax = 0, float ay = 0, float az = 0) : x(ax), y(ay), z(az) {;}
	
	Vector operator -() const
	{
		return Vector(x, -y, -z);
	}

	Vector operator +(const float value) const
	{
		return Vector(x + value, y + value, z + value);
	}

	Vector& operator +=(const float value)
	{
		x += value;
		y += value;
		z += value;
		
		return *this;
	}

	Vector operator *(const float value) const
	{
		return Vector(x * value, y * value, z * value);
	}

	Vector& operator *=(const float value)
	{
		x *= value;
		y *= value;
		z *= value;
		
		return *this;
	}


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

	Vector operator -(const float value) const
	{
		return Vector(x - value, y - value, z - value);
	}

	Vector& operator -=(const float value)
	{
		x -= value;
		y -= value;
		z -= value;

		return *this;
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

	Vector operator /(const float value) const
	{
		return Vector(x / value, y / value, z / value);
	}

	Vector& operator /=(const float value)
	{
		x /= value;
		y /= value;
		z /= value;

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

	float Squared() const
	{
		return x * x + y * y + z * z;
	}

	float Length() const
	{
		return sqrt(Squared());
	}

	float Dot(const Vector& Other) const
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

	Vector GetNormalize() const
	{
		float squared = Squared();
		// cos(theta) = 0.f;
		if(squared == 0.f)
		{
			// 서로 직교한다고 볼 수 있다.
			// 90, 270
			return Vector(0,0,0);
		}

		// cos(theta) = 1.f;
		if(squared == 1.f)
		{
			return *this;
		}

		float length = sqrt(squared);
		return Vector(x,y,z) / length;
	}

	void Normalize()
	{
		*this = GetNormalize();
	}
};
*/

template <typename _TYPE>
class Vector
{
	public:
	_TYPE x,y,z;
	
	Vector(_TYPE ax = 0, _TYPE ay = 0, _TYPE az = 0) : x(ax), y(ay), z(az) {;}
	
	Vector<_TYPE> operator -() const
	{
		return Vector<_TYPE>(-x, -y, -z);
	}

	Vector<_TYPE> operator +(const _TYPE value) const
	{
		return Vector<_TYPE>(x + value, y + value, z + value);
	}

	Vector<_TYPE>& operator +=(const _TYPE value)
	{
		x += value;
		y += value;
		z += value;
		
		return *this;
	}

	Vector<_TYPE> operator *(const _TYPE value) const
	{
		return Vector<_TYPE>(x * value, y * value, z * value);
	}

	Vector<_TYPE>& operator *=(const _TYPE value)
	{
		x *= value;
		y *= value;
		z *= value;
		
		return *this;
	}


	Vector<_TYPE> operator +(const Vector& Other) const
	{
		return Vector<_TYPE>(x + Other.x, y + Other.y, z + Other.z);
	}

	Vector<_TYPE>& operator +=(const Vector& Other)
	{
		x += Other.x;
		y += Other.y;
		z += Other.z;
		
		return *this;
	}

	Vector<_TYPE> operator *(const Vector& Other) const
	{
		return Vector<_TYPE>(x * Other.x, y * Other.y, z * Other.z);
	}

	Vector<_TYPE>& operator *=(const Vector& Other)
	{
		x *= Other.x;
		y *= Other.y;
		z *= Other.z;
		
		return *this;
	}

	Vector<_TYPE> operator -(const _TYPE value) const
	{
		return Vector<_TYPE>(x - value, y - value, z - value);
	}

	Vector<_TYPE>& operator -=(const _TYPE value)
	{
		x -= value;
		y -= value;
		z -= value;

		return *this;
	}

	Vector<_TYPE> operator -(const Vector& Other) const
	{
		return Vector<_TYPE>(x - Other.x, y - Other.y, z - Other.z);
	}

	Vector<_TYPE>& operator -=(const Vector& Other)
	{
		x -= Other.x;
		y -= Other.y;
		z -= Other.z;

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

	_TYPE Squared() const
	{
		return x * x + y * y + z * z;
	}

	_TYPE Length() const
	{
		return sqrt(Squared());
	}

	_TYPE Dot(const Vector& Other) const
	{
		return x * Other.x + y * Other.y + z * Other.z;
	}

	Vector<_TYPE> Cross(const Vector& Other) const
	{
		return Vector<_TYPE>(
					y * Other.z - Other.y * z,
					z * Other.x - Other.z * x,
					x * Other.y - Other.x * y
				);
	}

	Vector<_TYPE> GetNormalize() const
	{
		_TYPE squared = Squared();
		// cos(theta) = 0.f;
		if(squared == 0.f)
		{
			// 서로 직교한다고 볼 수 있다.
			// 90, 270
			return Vector<_TYPE>(0,0,0);
		}

		// cos(theta) = 1.f;
		if(squared == 1.f)
		{
			return *this;
		}

		_TYPE length = sqrt(squared);
		return Vector<_TYPE>(x,y,z) / length;
	}

	void Normalize()
	{
		*this = GetNormalize();
	}
};
#endif
