#ifndef BZMAG_CORE_TYPE_VECTOR3_H
#define BZMAG_CORE_TYPE_VECTOR3_H
/**
@ingroup bzmagCoreType
@class bzmag::Vector3
@brief
*/

#include "platform.h"
#include "tuple3.h"
#include "primitivetype.h"
#include <cmath> // for std::sqrt

namespace bzmag
{
    class Vector3 : public Tuple3<float64>
    {
    public:
        using Tuple3<float64>::Tuple3; // Tuple3의 생성자 사용

        // 두 벡터가 tol 내에서 같은지 확인
        bool isEqual(const Vector3& v, float64 tol) const
        {
            for (int i = 0; i < 3; ++i)
            {
                if (a_[i] - tol >= v.a_[i] || v.a_[i] <= a_[i] + tol)
                    return false;
            }
            return true;
        }

        // 벡터 크기
        float64 size() const
        {
            return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);
        }

        // 벡터 길이 (size()와 동일)
        float64 length() const
        {
            return size();
        }

        // 벡터 정규화
        void normalize()
        {
            float64 len = length();
            if (len == 0)
                return;
            x_ /= len;
            y_ /= len;
            z_ /= len;
        }

        // 두 벡터의 내적
        float64 dotprod(const Vector3& v) const
        {
            return (x_ * v.x_ + y_ * v.y_ + z_ * v.z_);
        }

        // 두 벡터의 외적
        Vector3 cross(const Vector3& v2) const
        {
            return Vector3(
                y_ * v2.z_ - z_ * v2.y_,
                z_ * v2.x_ - x_ * v2.z_,
                x_ * v2.y_ - y_ * v2.x_
            );
        }

        // 음수 벡터 반환
        Vector3 operator - () const
        {
            return Vector3(-x_, -y_, -z_);
        }

        // 벡터 덧셈 (자기 자신에 더함)
        const Vector3& operator += (const Vector3& rhs)
        {
            x_ += rhs.x_;
            y_ += rhs.y_;
            z_ += rhs.z_;
            return *this;
        }

        // 벡터 뺄셈 (자기 자신에서 뺌)
        const Vector3& operator -= (const Vector3& rhs)
        {
            x_ -= rhs.x_;
            y_ -= rhs.y_;
            z_ -= rhs.z_;
            return *this;
        }

        // 스칼라 곱셈 (자기 자신에 곱함)
        const Vector3& operator *= (float64 v)
        {
            x_ *= v;
            y_ *= v;
            z_ *= v;
            return *this;
        }

        // 벡터 덧셈
        Vector3 operator + (const Vector3& v) const
        {
            return Vector3(x_ + v.x_, y_ + v.y_, z_ + v.z_);
        }

        // 벡터 뺄셈
        Vector3 operator - (const Vector3& v) const
        {
            return Vector3(x_ - v.x_, y_ - v.y_, z_ - v.z_);
        }

        // 스칼라 곱셈
        Vector3 operator * (float64 value) const
        {
            return Vector3(x_ * value, y_ * value, z_ * value);
        }
    };
}

#endif // BZMAG_CORE_TYPE_VECTOR3_H