#include <vector>
#include "windfall_objects.h"
#include "misc.h"

unsigned int wind_create_VAO() {

  float windVertices[] = {
      0.f, 0.f, 0.f,
  };

  unsigned int windVAO, windVBO;
  glGenVertexArrays(1, &windVAO);
  glGenBuffers(1, &windVBO);
  glBindVertexArray(windVAO);
  glBindBuffer(GL_ARRAY_BUFFER, windVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(windVertices), &windVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, (void *) 0);

  return windVAO;
}

unsigned int water_create_VAO(float heightf) {

 float waterVertices[] = {
      -20000.0f, heightf, -20000.0f,
      20000.0f, heightf, -20000.0f,
      20000.0f, heightf, 20000.0f,
      20000.0f, heightf, 20000.0f,
      -20000.0f, heightf, 20000.0f,
      -20000.0f, heightf, -20000.0f
      };

  unsigned int waterVAO, waterVBO;
  glGenVertexArrays(1, &waterVAO);
  glGenBuffers(1, &waterVBO);
  glBindVertexArray(waterVAO);
  glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), &waterVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);

  return waterVAO;
}

std::vector<unsigned int> loadSunTextures()
{
  std::vector<unsigned int> sun_textures{};

  std::vector<std::string> files {
      "images/sprites/sun_border.png",
      "images/sprites/sun_eclat.png",
      "images/sprites/sun_center.png",
      "images/sprites/sun_inner_ring.png",
      "images/sprites/sun_gradient.png"
  };

  for (const auto &file : files) {
    sun_textures.emplace_back(load_image(file));
  }
  return sun_textures;
}


unsigned int sun_create_VAO() {
  float x_start = -50.0f;
  float y_start = -50.0f;
  float x_end = 50.0f;
  float y_end = 50.0f;
  float height = 500.0f;
  float sunVertices[] = {
      x_end,  height, y_end,  0.0f, 0.0f,
      x_start, height, y_end,     1.0f, 0.0f,
      x_start, height,  y_start,      1.0f, 1.0f,

      x_start,  height, y_start,  1.0f, 1.0f,
      x_end, height,  y_start,      0.0f, 1.0f,
      x_end, height, y_end,     0.0f, 0.0f,
  };
  unsigned int sunVAO, sunVBO;
  glGenVertexArrays(1, &sunVAO);
  glGenBuffers(1, &sunVBO);
  glBindVertexArray(sunVAO);
  glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(sunVertices), &sunVertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  return sunVAO;

}

float get_eye_cancer(float alignment, float alignment_limit, float lowest_eye_cancer) {
  float eye_cancer = 1.0f; // Value to darken the scene if we look too directly into the sun

  if (alignment > alignment_limit) {
    float coeff_dir = (lowest_eye_cancer - 1.0f) / (1.0f - alignment_limit);
    eye_cancer = coeff_dir * alignment + (lowest_eye_cancer - coeff_dir);
  }
  return eye_cancer;
}

std::vector<unsigned int> loadShore()
{
  std::vector<unsigned int> shore{};

  std::vector<std::string> files {
      "images/sprites/shore_alpha_wave.png",
      "images/sprites/shore_black_wave.png",
      "images/sprites/shore_limit_wave.png",
      "images/sprites/shore_mask.png"};

  for (const auto &file : files)
  {
    shore.emplace_back(load_image(file));
  }
  return shore;
}

unsigned int shore_create_VAO(int &nb_of_points, float heightf) {

  // You don't want to hear the story behind this
  std::vector<glm::vec3> points = {
      glm::vec3 (-2822.0f, 0.0f, 2894.0f),
      glm::vec3 (-2822.0f, 0.0f, 3107.0f),

      glm::vec3 (-2042.0f, 0.0f, 2800.0f),
      glm::vec3 (-2042.0f, 0.0f, 3012.0f),

      glm::vec3(-1347.0f, 0.0f, 2318.0f),
      glm::vec3(-1347.0f, 0.0f, 2529.0f),

      glm::vec3(-1186.0f, 0.0f, 2406.0f),
      glm::vec3(-1187.0f, 0.0f, 2618.0f),

      glm::vec3(-969.0f, 0.0f, 2471.0f),
      glm::vec3(-969.0f, 0.0f, 2683.0f),

      glm::vec3(-703.0f, 0.0f, 2503.0f),
      glm::vec3(-703.0f, 0.0f, 2713.0f),

      glm::vec3(-483.0f, 0.0f, 2417.0f),
      glm::vec3(-483.0f, 0.0f, 2629.0f),

      glm::vec3(584.0f, 0.0f, 1704.0f),
      glm::vec3(584.0f, 0.0f, 2000.0f),

      glm::vec3(884.0f, 0.0f, 2151.0f),
      glm::vec3(884.0f, 0.0f, 2362.0f),

      glm::vec3(1420.0f, 0.0f, 1711.0f),
      glm::vec3(1420.0f, 0.0f, 1975.0f),

      glm::vec3(1532.0f, 0.0f, 1953.0f),
      glm::vec3(1532.0f, 0.0f, 2216.0f),

      glm::vec3(1758.0f, 0.0f, 1954.0f),
      glm::vec3(1758.0f, 0.0f, 2196.0f),

      glm::vec3(1977.0f, 0.0f, 1747.0f),
      glm::vec3(1977.0f, 0.0f, 2013.0f),

      glm::vec3(2086.0f, 0.0f, 1380.0f),
      glm::vec3(2347.0f, 0.0f, 1380.0f),

      glm::vec3(3156.0f, 0.0f, -157.0f),
      glm::vec3(3156.0f, 0.0f, 337.0f),

      glm::vec3(3711.0f, 0.0f, -1712.0f),
      glm::vec3(4075.0f, 0.0f, -1712.0f),

      glm::vec3(4118.0f, 0.0f, -2054.0f),
      glm::vec3(4280.0f, 0.0f, -1899.0f),

      glm::vec3(3736.0f, 0.0f, -2918.0f),
      glm::vec3(4035.0f, 0.0f, -2918.0f),

      glm::vec3(3194.0f, 0.0f, -3731.0f),
      glm::vec3(3366.0f, 0.0f, -3985.0f),

      glm::vec3(3166.0f, 0.0f, -3983.0f),
      glm::vec3(3344.0f, 0.0f, -4331.0f),

      glm::vec3(2600.0f, 0.0f, -4128.0f),
      glm::vec3(2641.0f, 0.0f, -4227.0f),

      glm::vec3(2270.0f, 0.0f, -4547.0f),
      glm::vec3(2392.0f, 0.0f, -4618.0f),

      glm::vec3(1903.0f, 0.0f, -4739.0f),
      glm::vec3(1984.0f, 0.0f, -4857.0f),

      glm::vec3(1629.0f, 0.0f, -5083.0f),
      glm::vec3(1694.0f, 0.0f, -5236.0f),

      glm::vec3(885.0f, 0.0f, -5535.0f),
      glm::vec3(917.0f, 0.0f, -5640.0f),

      glm::vec3(615.0f, 0.0f, -5622.0f),
      glm::vec3(572.0f, 0.0f, -5673.0f),

      glm::vec3(288.0f, 0.0f, -5345.0f),
      glm::vec3(199.0f, 0.0f, -5405.0f),

      glm::vec3(-663.0f, 0.0f, -4879.0f),
      glm::vec3(-716.0f, 0.0f, -4933.0f),

      glm::vec3(-964.0f, 0.0f, -4576.0f),
      glm::vec3(-1011.0f, 0.0f, -4594.0f),

      glm::vec3(-918.0f, 0.0f, -4354.0f),
      glm::vec3(-971.0f, 0.0f, -4359.0f),

      glm::vec3(-1102.0f, 0.0f, -4033.0f),
      glm::vec3(-1189.0f, 0.0f, -4023.0f),

      glm::vec3(-843.0f, 0.0f, -3585.0f),
      glm::vec3(-925.0f, 0.0f, -3611.0f),

      glm::vec3(-1103.0f, 0.0f, -3357.0f),
      glm::vec3(-1135.0f, 0.0f, -3388.0f),

      glm::vec3(-1189.0f, 0.0f, -3218.0f),
      glm::vec3(-1219.0f, 0.0f, -3264.0f),

      glm::vec3(-1390.0f, 0.0f, -3175.0f),
      glm::vec3(-1409.0f, 0.0f, -3235.0f),

      glm::vec3(-1657.0f, 0.0f, -3035.0f),
      glm::vec3(-1665.0f, 0.0f, -3101.0f),

      glm::vec3(-1766.0f, 0.0f, -3057.0f),
      glm::vec3(-1768.0f, 0.0f, -3098.0f),

      glm::vec3(-1906.0f, 0.0f, -3026.0f),
      glm::vec3(-1946.0f, 0.0f, -3065.0f),

      glm::vec3(-1960.0f, 0.0f, -2852.0f),
      glm::vec3(-2049.0f, 0.0f, -2868.0f),

      glm::vec3(-1918.0f, 0.0f, -2678.0f),
      glm::vec3(-1966.0f, 0.0f, -2641.0f),

      glm::vec3(-1680.0f, 0.0f, -2479.0f),
      glm::vec3(-1746.0f, 0.0f, -2408.0f),

      glm::vec3(-1547.0f, 0.0f, -2041.0f),
      glm::vec3(-1617.0f, 0.0f, -2083.0f),

      glm::vec3(-1819.0f, 0.0f, -1954.0f),
      glm::vec3(-1816.0f, 0.0f, -2052.0f),

      glm::vec3(-2133.0f, 0.0f, -2273.0f),
      glm::vec3(-2082.0f, 0.0f, -2317.0f),

      glm::vec3(-2232.0f, 0.0f, -2469.0f),
      glm::vec3(-2196.0f, 0.0f, -2520.0f),

      glm::vec3(-2404.0f, 0.0f, -2501.0f),
      glm::vec3(-2403.0f, 0.0f, -2563.0f),

      glm::vec3(-2657.0f, 0.0f, -2441.0f),
      glm::vec3(-2702.0f, 0.0f, -2484.0f),

      glm::vec3(-2771.0f, 0.0f, -2142.0f),
      glm::vec3(-2860.0f, 0.0f, -2147.0f),

      glm::vec3(-2767.0f, 0.0f, -1829.0f),
      glm::vec3(-2815.0f, 0.0f, -1880.0f),

      glm::vec3(-2934.0f, 0.0f, -1646.0f),
      glm::vec3(-2967.0f, 0.0f, -1719.0f),

      glm::vec3(-3365.0f, 0.0f, -1463.0f),
      glm::vec3(-3390.0f, 0.0f, -1572.0f),

      glm::vec3(-3858.0f, 0.0f, -1089.0f),
      glm::vec3(-3927.0f, 0.0f, -1187.0f),

      glm::vec3(-4240.0f, 0.0f, -649.0f),
      glm::vec3(-4324.0f, 0.0f, -684.0f),

      glm::vec3(-4558.0f, 0.0f, -8.0f),
      glm::vec3(-4758.0f, 0.0f, 15.0f),

      glm::vec3(-4612.0f, 0.0f, 641.0f),
      glm::vec3(-4689.0f, 0.0f, 636.0f),

      glm::vec3(-4418.0f, 0.0f, 1273.0f),
      glm::vec3(-4513.0f, 0.0f, 1264.0f),

      glm::vec3(-4729.0f, 0.0f, 1719.0f),
      glm::vec3(-4769.0f, 0.0f, 1709.0f),

      glm::vec3(-4718.0f, 0.0f, 2232.0f),
      glm::vec3(-4754.0f, 0.0f, 2239.0f),

      glm::vec3(-4427.0f, 0.0f, 2208.0f),
      glm::vec3(-4408.0f, 0.0f, 2245.0f),

      glm::vec3(-4020.0f, 0.0f, 2213.0f),
      glm::vec3(-4048.0f, 0.0f, 2276.0f),

      glm::vec3 (-2822.0f, 0.0f, 2894.0f),
      glm::vec3 (-2822.0f, 0.0f, 3107.0f)


  };

  std::vector<glm::vec3> points_2 {
      glm::vec3(-2824.0f, 0.0f, -3399.0f),
      glm::vec3(-2879.0f, 0.0f, -3401.0f),

      glm::vec3(-2836.0f, 0.0f, -3218.0f),
      glm::vec3(-2948.0f, 0.0f, -3081.0f),

      glm::vec3(-2717.0f, 0.0f, -3168.0f),
      glm::vec3(-2727.0f, 0.0f, -3118.0f),

      glm::vec3(-2430.0f, 0.0f, -3247.0f),
      glm::vec3(-2360.0f, 0.0f, -3215.0f),

      glm::vec3(-2413.0f, 0.0f, -3386.0f),
      glm::vec3(-2333.0f, 0.0f, -3379.0f),

      glm::vec3(-2583.0f, 0.0f, -3556.0f),
      glm::vec3(-2590.0f, 0.0f, -3607.0f),

      glm::vec3(-2824.0f, 0.0f, -3399.0f),
      glm::vec3(-2879.0f, 0.0f, -3401.0f)

  };

  std::vector<glm::vec3> points_3 = {
      glm::vec3(-1357.0f, 0.0f, -5879.0f),
      glm::vec3(-1445.0f, 0.0f, -5955.0f),

      glm::vec3(-1416.0f, 0.0f, -5687.0f),
      glm::vec3(-1461.0f, 0.0f, -5676.0f),

      glm::vec3(-1379.0f, 0.0f, -5610.0f),
      glm::vec3(-1400.0f, 0.0f, -5548.0f),

      glm::vec3(-1055.0f, 0.0f, -5675.0f),
      glm::vec3(-1016.0f, 0.0f, -5669.0f),

      glm::vec3(-1177.0f, 0.0f, -5847.0f),
      glm::vec3(-1156.0f, 0.0f, -5887.0f),

      glm::vec3(-1357.0f, 0.0f, -5879.0f),
      glm::vec3(-1445.0f, 0.0f, -5955.0f)
  };

  std::vector<glm::vec3> points_4 = {
      glm::vec3(2321.0f, 0.0f, 2586.0f),
      glm::vec3(2188.0f, 0.0f, 2636.0f),

      glm::vec3(2639.0f, 0.0f, 2628.0f),
      glm::vec3(2756.0f, 0.0f, 2671.0f),

      glm::vec3(2598.0f, 0.0f, 2254.0f),
      glm::vec3(2655.0f, 0.0f, 2200.0f),

      glm::vec3(2370.0f, 0.0f, 2258.0f),
      glm::vec3(2259.0f, 0.0f, 2199.0f),

      glm::vec3(2321.0f, 0.0f, 2586.0f),
      glm::vec3(2188.0f, 0.0f, 2636.0f)
  };

  glm::mat4 model_mat_windfall = glm::mat4(1.0f);
  model_mat_windfall = glm::translate(model_mat_windfall, glm::vec3(1.0f, -10.0f, -25.0f));
  model_mat_windfall = glm::scale(model_mat_windfall, glm::vec3(0.01f, 0.01f, 0.01f));

  nb_of_points = 0;
  std::vector<std::vector<glm::vec3>> points_ensemble = {points, points_2, points_3, points_4};
  for (auto& point_ensemble : points_ensemble) {
    nb_of_points += (point_ensemble.size() / 2) - 1;
    for (auto & point : point_ensemble) {
      point = model_mat_windfall * glm::vec4(point, 1.0f);
    }
  }

  std::vector<float> shoreVertices = {};

  for (auto& point_ensemble : points_ensemble) {
    for (size_t i = 0; i < point_ensemble.size() - 2; i += 2) {
      std::vector<float> shoreVertices_quad = {
          point_ensemble[i].x, heightf, point_ensemble[i].z, 0.0f, 0.0f,
          point_ensemble[i + 1].x, heightf, point_ensemble[i + 1].z, 0.0f, 1.0f,
          point_ensemble[i + 2].x, heightf, point_ensemble[i + 2].z, 1.0f, 0.0f,

          point_ensemble[i + 3].x, heightf, point_ensemble[i + 3].z, 1.0f, 1.0f,
          point_ensemble[i + 2].x, heightf, point_ensemble[i + 2].z, 1.0f, 0.0f,
          point_ensemble[i + 1].x, heightf, point_ensemble[i + 1].z, 0.0f, 1.0f
      };
      shoreVertices.insert(shoreVertices.end(), shoreVertices_quad.begin(), shoreVertices_quad.end());
    }
  }

  unsigned int shoreVAO, shoreVBO;
  glGenVertexArrays(1, &shoreVAO);
  glGenBuffers(1, &shoreVBO);
  glBindVertexArray(shoreVAO);
  glBindBuffer(GL_ARRAY_BUFFER, shoreVBO);
  glBufferData(GL_ARRAY_BUFFER, shoreVertices.size() * sizeof(float), shoreVertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  return shoreVAO;
}