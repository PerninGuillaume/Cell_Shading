#include <iostream>
#include "matrix4.h"
#include "Vector3.h"

mygl::matrix4::matrix4()
{
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        matrix_[i][j] = 0;
      }
    }
}

void mygl::matrix4::operator*=(const mygl::matrix4 &rhs) {
  matrix4 res = matrix4();
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      float sum = 0.0f;
      for (int k = 0; k < 4; ++k) {
        sum += this->matrix_[i][k] * rhs.matrix_[k][j];
      }
      res.matrix_[i][j] = sum;
    }
  }
  std::cout << res << '\n';
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      this->matrix_[i][j] = res.matrix_[i][j];
    }
  }
}

mygl::matrix4 mygl::matrix4::identity() {
  mygl::matrix4 mat =  mygl::matrix4();
  mat.matrix_[0][0] = 1;
  mat.matrix_[1][1] = 1;
  mat.matrix_[2][2] = 1;
  mat.matrix_[3][3] = 1;
  return mat;
}

mygl::matrix4 frustrum(const float &left, const float &right,
              const float &bottom, const float &top, const float &z_near, const float &z_far
)
{
  float A = (right + left) / (right - left);
  float B = (top + bottom) / (top - bottom);
  float C = -(z_far + z_near) / (z_far - z_near);
  float D = -(2 * z_far * z_near) / (z_far - z_near);
  float E = (2 * z_near) / (right - left);
  float F = (2 * z_near) / (top - bottom);
  std::array<std::array<float, 4>, 4> projection_mat {
          {             {E, 0, A, 0},
                  {0, F, B, 0},
                  {0, 0, C, D},
                  {0,0, -1,0}
          }};
  mygl::matrix4 projection;
  projection.matrix_ = projection_mat;
  return projection;
}

mygl::matrix4 look_at(const float &eyeX, const float &eyeY, const float &eyeZ,
             const float &centerX, const float &centerY, const float &centerZ,
             float upX, float upY, float upZ
)
{
  Vector3 f(centerX - eyeX, centerY - eyeY, centerZ - eyeZ);
  f.normalize();
  Vector3 up(upX, upY, upZ);
  up.normalize();
  Vector3 s = f.vector_product(up);
  Vector3 u = (s / s.norm()).vector_product(f);
  std::array<std::array<float, 4>, 4> M_matrix{
          {             {s.x, s.y, s.z, 0},
                  {u.x, u.y, u.z, 0},
                  {-f.x, -f.y, -f.z, 0},
                  {0,0, 0,1}
          }};
  mygl::matrix4 M;
  M.matrix_ = M_matrix;
  mygl::matrix4 translate = mygl::matrix4::identity();
  translate.matrix_[0][3] = -eyeX;
  translate.matrix_[1][3] = -eyeY;
  translate.matrix_[2][3] = -eyeZ;
  M *= translate;
  return M;
  };

std::ostream &operator<<(std::ostream &out, const mygl::matrix4 &m){
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      out << m.matrix_[i][j] << ' ';
    }
    out << '\n';
  }
  return out;
}