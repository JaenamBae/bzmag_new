#ifndef BZMAG_CORE_TYPE_VECTOR2_H
#define BZMAG_CORE_TYPE_VECTOR2_H
/**
    @ingroup bzmagCoreType
    @class bzmag::Vector2
    @brief
*/

#include "platform.h"
#include "tuple2.h"
#include "primitivetype.h"
#include <cmath> // for std::sqrt

namespace bzmag
{
    class Vector2 : public Tuple2<float64>
    {
    public:
        using Tuple2<float64>::Tuple2; // Tuple2의 생성자 사용

        // 벡터 크기
        float64 size() const
        {
            return std::sqrt(x_ * x_ + y_ * y_);
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
        }

        // 두 벡터의 내적
        float64 dotprod(const Vector2& v) const
        {
            return (x_ * v.x_ + y_ * v.y_);
        }

        // 음수 벡터 반환
        Vector2 operator - () const
        {
            return Vector2(-x_, -y_);
        }

        // 벡터 덧셈 (자기 자신에 더함)
        const Vector2& operator += (const Vector2& rhs)
        {
            x_ += rhs.x_;
            y_ += rhs.y_;
            return *this;
        }

        // 벡터 뺄셈 (자기 자신에서 뺌)
        const Vector2& operator -= (const Vector2& rhs)
        {
            x_ -= rhs.x_;
            y_ -= rhs.y_;
            return *this;
        }

        // 스칼라 곱셈 (자기 자신에 곱함)
        const Vector2& operator *= (float64 v)
        {
            x_ *= v;
            y_ *= v;
            return *this;
        }

        // 벡터 덧셈
        Vector2 operator + (const Vector2& v) const
        {
            return Vector2(x_ + v.x_, y_ + v.y_);
        }

        // 벡터 뺄셈
        Vector2 operator - (const Vector2& v) const
        {
            return Vector2(x_ - v.x_, y_ - v.y_);
        }

        // 스칼라 곱셈
        Vector2 operator * (float64 value) const
        {
            return Vector2(x_ * value, y_ * value);
        }
    };
}

#endif // BZMAG_CORE_TYPE_VECTOR2_H
