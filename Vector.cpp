#include "Vector3.h"
#include <cmath>
#include <optional>
#include <iostream>

Vector3::Vector3()
    : x(0)
    , y(0)
    , z(0)
{}

Vector3::Vector3(float x, float y, float z)
    : x(x)
    , y(y)
    , z(z)
{}

Vector3 Vector3::operator*(const float &mul) const {
    return Vector3(x * mul, y * mul, z * mul);
}

Vector3 Vector3::operator*(const Vector3 &v) const {
  return Vector3(this->x * v.x, this->y * v.y, this->z * v.z);
}

Vector3& Vector3::operator*=(const float t) {
  *this = *this * t;
  return *this;
}

Vector3& Vector3::operator*=(const Vector3 &v) {
  *this = *this * v;
  return *this;
}

Vector3 operator*(const float &mul, const Vector3 &v) {
  return v * mul;
}

Vector3 Vector3::operator+(const Vector3 &v) const {
    return Vector3(this->x + v.x, this->y + v.y, this->z + v.z);
}

Vector3 Vector3::operator+(float t) const {
  return Vector3(this->x + t, this->y + t, this->z + t);
}

Vector3 Vector3::operator-(const Vector3 &v) const {
    return Vector3(this->x - v.x, this->y - v.y, this->z - v.z);
}

Vector3 Vector3::operator-() const {
  return Vector3(-this->x, -this->y, -this->z);
}

Vector3& Vector3::operator+=(const Vector3& v) {
  *this = *this + v;
  return *this;
}

Vector3 Vector3::operator/(float t) {
  return (1/t) * *this;
}

Vector3& Vector3::operator/=(const float t) {
  return *this *= 1/t;
}

bool Vector3::operator==(const Vector3& v) {
  return this->x == v.x && this->y == v.y && this->z == v.z;
}

float Vector3::norm() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vector3& Vector3::normalize() {
  float norm = this->norm();
  x /= norm;
  y /= norm;
  z /= norm;
  return *this;
}

float Vector3::scalar_product(const Vector3 &v) const {
    return this->x * v.x + this->y * v.y + this->z * v.z;
}

Vector3 Vector3::vector_product(const Vector3 &v) const {
    return Vector3(this->y * v.z - v.y * this->z, v.x * this->z - this->x * v.z, this->x * v.y - v.x * this->y);
}

std::ostream& operator<<(std::ostream &out, const Vector3& v) {
  return out << "[" << v.x << ", " << v.y << ", " << v.z << "]";
}
