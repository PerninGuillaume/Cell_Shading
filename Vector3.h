#pragma once
#include <optional>
#include <ostream>

class Vector3
{
 public:
  Vector3();
  Vector3(float x, float y, float z);

  float x;
  float y;
  float z;

  Vector3 operator*(const float &mul) const;
  Vector3 operator*(const Vector3 &v) const;
  Vector3& operator*=(float t);
  Vector3& operator*=(const Vector3 &v);

  Vector3 operator+(const Vector3 &v) const;
  Vector3 operator+(float t) const;
  Vector3& operator+=(const Vector3& v);

  Vector3 operator-(const Vector3 &v) const;
  Vector3 operator-() const;

  Vector3 operator/(float t);
  Vector3& operator/=(float t);
  bool operator==(const Vector3& v);

  [[nodiscard]] float norm() const;
  Vector3& normalize();
  float scalar_product(const Vector3 &v) const;
  [[nodiscard]] Vector3 vector_product(const Vector3 &v) const;

};

std::ostream& operator<<(std::ostream &out, const Vector3& v);
Vector3 operator*(const float &mul, const Vector3 &v);