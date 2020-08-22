
#ifndef GENERIC_VERTEX_HH
#define GENERIC_VERTEX_HH

// #define GLM_ENABLE_EXPERIMENTAL // needed?
#include <glm/glm.hpp>

// simple vertex
class s_generic_vertex {
public:
   glm::vec3 pos;
   glm::vec3 normal; // normalized on input
   glm::vec4 color;  // make this "colour"
   s_generic_vertex(const glm::vec3 &pos_in,
                    const glm::vec3 &norm_in,
                    const glm::vec4 &col_in) : pos(pos_in), normal(norm_in), color(col_in)  {}
   s_generic_vertex() {}
};

// for instanced objects (those animated/moving) the position in molecular space
// and the colour is dictacted by the instancing matrices (and colours)
// We don't need to create them here.
class position_normal_vertex {
public:
   glm::vec3 pos; // based around the origin
   glm::vec3 normal; // normalized on input
   position_normal_vertex(const glm::vec3 &pos_in, const glm::vec3 &norm_in) : pos(pos_in), normal(norm_in) {}
   position_normal_vertex() {}
};

class vertex_with_rotation_translation {
public:
   glm::mat3 model_rotation_matrix; // orientation
   glm::vec3 model_translation; // the coordinates of the first atom of the bond
   glm::vec3 pos;
   glm::vec3 normal; // normalized when set
   glm::vec4 colour;
   vertex_with_rotation_translation(const glm::vec3 &p, const glm::vec3 &n, const glm::vec4 &c) : pos(p), normal(n), colour(c) {}
   vertex_with_rotation_translation(const s_generic_vertex &v, float scale) : pos(v.pos * scale), normal(v.normal), colour(v.color) {}
   vertex_with_rotation_translation() {}
};



#endif
