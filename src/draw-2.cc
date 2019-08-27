#ifdef USE_PYTHON
#include <Python.h>
#endif // USE_PYTHON

#define GLM_ENABLE_EXPERIMENTAL // # for norm things
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>  // to_string()

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <gtk/gtk.h>
#include <epoxy/gl.h>

#include "globjects.h"
#include "trackball.h"
#include "graphics-info.h"

#include "draw.hh"
#include "draw-2.hh"
#include "framebuffer.hh"

#include "text-rendering-utils.hh"

enum {VIEW_CENTRAL_CUBE, ORIGIN_CUBE};

gint idle_contour_function(gpointer data);

void init_central_cube_shaders() {} // part of init_shaders() now. Delete at some stage

// maybe this can go in the draw-2.hh header
glm::vec4 new_unproject(float z);


float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
      // positions   // texCoords
      -1.0f,  1.0f,  0.0f, 1.0f,
      -1.0f, -1.0f,  0.0f, 0.0f,
       1.0f, -1.0f,  1.0f, 0.0f,

      -1.0f,  1.0f,  0.0f, 1.0f,
       1.0f, -1.0f,  1.0f, 0.0f,
       1.0f,  1.0f,  1.0f, 1.0f
   };

void init_shaders() {
   graphics_info_t::shader_for_maps.init("map.shader", Shader::Entity_t::MAP);
   graphics_info_t::shader_for_models.init("model.shader", Shader::Entity_t::MODEL);
   graphics_info_t::shader_for_central_cube.init("central-cube.shader", Shader::Entity_t::INFRASTRUCTURE);
   graphics_info_t::shader_for_origin_cube.init("central-cube.shader", Shader::Entity_t::INFRASTRUCTURE);
   graphics_info_t::shader_for_hud_text.init("hud-text.shader", Shader::Entity_t::HUD_TEXT);
   // we use the above to make an image/texture in the framebuffer and use then
   // shader_for_screen to convert that framebuffer to the screen buffer.
   graphics_info_t::shader_for_screen.init("screen.shader", Shader::Entity_t::SCREEN);

}

void init_screen_quads() {

   graphics_info_t::shader_for_screen.Use();
   // screen quad VAO
   unsigned int quadVBO;
   glGenVertexArrays(1, &graphics_info_t::screen_quad_vertex_array_id);
   glBindVertexArray(graphics_info_t::screen_quad_vertex_array_id);
   glGenBuffers(1, &quadVBO);
   glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void *>(0));
   glEnableVertexAttribArray(1);
   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(2 * sizeof(float)));
   GLenum err = glGetError();
   if (true) std::cout << "init_screen_quads() err is " << err << std::endl;

}

void init_central_cube();

void init_buffers() {
   init_central_cube();
   init_screen_quads();
}

void init_central_cube() {

   {
      float positions[24] = {
         -0.5,  -0.5, -0.5,
         -0.5,  -0.5,  0.5,
         -0.5,   0.5, -0.5,
         -0.5,   0.5,  0.5,
          0.5,  -0.5, -0.5,
          0.5,  -0.5,  0.5,
          0.5,   0.5, -0.5,
          0.5,   0.5,  0.5
      };

      glUseProgram(graphics_info_t::shader_for_central_cube.get_program_id());
      GLenum err = glGetError();
      if (err) std::cout << "init_central_cube() glUseProgram() err is " << err << std::endl;

      // number of lines * 2:
      unsigned int indices[24] { 0,1, 1,5, 5,4, 4,0, 2,3, 3,7, 7,6, 6,2, 0,2, 1,3, 5,7, 4,6 };

      // GLuint VertexArrayID;
      glGenVertexArrays(1, &graphics_info_t::central_cube_vertexarray_id);
      glBindVertexArray(graphics_info_t::central_cube_vertexarray_id);

      // GLuint vertexbuffer;
      glGenBuffers(1, &graphics_info_t::central_cube_array_buffer_id);
      glBindBuffer(GL_ARRAY_BUFFER, graphics_info_t::central_cube_array_buffer_id);
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, &positions[0], GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

      // unsigned int ibo;
      glGenBuffers(1, &graphics_info_t::central_cube_index_buffer_id);
      err = glGetError();
      if (err) std::cout << "init_central_cube() index glGenBuffers() err is " << err << std::endl;
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphics_info_t::central_cube_index_buffer_id);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 24, &indices[0], GL_STATIC_DRAW);
      err = glGetError();
      if (err) std::cout << "init_central_cube() glBufferData() err is " << err << std::endl;

   }
}

void init_hud_text() {

   std::cout << "------------------ init_hud_text() ---------------------\n";
   graphics_info_t g;
   g.load_freetype_font_textures();
   glUseProgram(g.shader_for_hud_text.get_program_id());
   GLenum err = glGetError();
   if (err) std::cout << "init_hud_text() glUseProgram() err is " << err << std::endl;
   glGenVertexArrays(1, &graphics_info_t::hud_text_vertexarray_id);
   err = glGetError(); if (err) std::cout << "init_hud_text() glGenVertexArrays() err is " << err << std::endl;
   glBindVertexArray(graphics_info_t::hud_text_vertexarray_id);
   err = glGetError(); if (err) std::cout << "init_hud_text() glBindVertexArray() err is " << err << std::endl;
   glGenBuffers(1, &graphics_info_t::hud_text_array_buffer_id);
   err = glGetError(); if (err) std::cout << "init_hud_text() glGenBuffers() err is " << err << std::endl;
   glBindBuffer(GL_ARRAY_BUFFER, graphics_info_t::hud_text_array_buffer_id);
   err = glGetError(); if (err) std::cout << "init_hud_text() glBindBuffer() err is " << err << std::endl;
   glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
   err = glGetError(); if (err) std::cout << "init_hud_text() glBufferData() err is " << err << std::endl;
   glEnableVertexAttribArray(0);
   err = glGetError(); if (err) std::cout << "init_hud_text() glEnableVertexAttribArray() err is " << err << std::endl;

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);
   std::cout << "------------------ done init_hud_text() ---------------------\n";
}

glm::mat4 get_molecule_mvp() {

   // presumes that we are in the correct programID

   float w = static_cast<float>(graphics_info_t::graphics_x_size);
   float h = static_cast<float>(graphics_info_t::graphics_y_size);
   float screen_ratio = w/h;

   // I don't think that the quaternion belongs to the model matrix, it should be
   // part of the view matrix I think.
   // Yes. That's right.
   glm::mat4 model_matrix = glm::mat4(1.0);

   float z = graphics_info_t::zoom * 0.04;
   glm::vec3 sc(z,z,z);
   float ortho_size = 90.0;

   // start: clipping front and back are 0
   // with large depth of field: clipping_front is -10, clipping_back is -9
   // with narrow depth of field: 5 and 6.

   GLfloat near_scale = 0.3;
   GLfloat far  =      -near_scale*graphics_info_t::zoom * (graphics_info_t::clipping_front*-0.3 + 0.1);
   GLfloat near =  0.20*near_scale*graphics_info_t::zoom * (graphics_info_t::clipping_back* -0.3 + 0.1);

   if (false)
      std::cout << "near: " << near << " far " << far
                << " clipping_front " << graphics_info_t::clipping_front
                << " clipping_back "  << graphics_info_t::clipping_back
                << std::endl;

   glm::mat4 projection_matrix = glm::ortho(-ortho_size * screen_ratio, ortho_size * screen_ratio,
                                            -ortho_size, ortho_size,
                                            near, far); // wrong way round?

   glm::vec3 rc = graphics_info_t::get_rotation_centre();
   // std::cout << "rotation centre " << glm::to_string(rc) << std::endl;
   glm::mat4 view_matrix = glm::toMat4(graphics_info_t::glm_quat);
   view_matrix = glm::scale(view_matrix, sc);
   view_matrix = glm::translate(view_matrix, -rc);

   glm::mat4 mvp = projection_matrix * view_matrix * model_matrix;

#if 0
   // for fun/testing
   // turn off view scaling when tinkering with this?
   // there should not be a concept of "zoom" with perspective view, just translation
   // along screen-Z.
   float fov = 40.0;
   glm::vec3 up(0.0, 1.0, 0.0);
   glm::vec3 ep(1,2,30);
   // view_matrix = glm::lookAt(glm::vec3(ep), rc, up);
   view_matrix = glm::translate(view_matrix, -0.2 * rc);
   float z_front = 30.0;
   float z_back = 300.0;
   z_front += 0.2 * graphics_info_t::clipping_front;
   z_back  -= 0.2 * graphics_info_t::clipping_back;
   fov /= 0.01 * graphics_info_t::zoom;
   std::cout << z_front << " " << z_back << " fov " << fov << std::endl;
   glm::mat4 projection_matrix_persp = glm::perspective(glm::radians(fov), screen_ratio, z_front, z_back);
   mvp = projection_matrix_persp * view_matrix * model_matrix;
#endif

   return mvp;
}

glm::vec4 new_unproject(float z) {
   // z is 1 and -1 for front and back (or vice verse).
   GtkAllocation allocation;
   gtk_widget_get_allocation(graphics_info_t::glarea, &allocation);
   int w = allocation.width;
   int h = allocation.height;
   graphics_info_t g;
   float mouseX = g.GetMouseBeginX() / (w * 0.5f) - 1.0f;
   float mouseY = g.GetMouseBeginY() / (h * 0.5f) - 1.0f;
   glm::mat4 mvp = get_molecule_mvp();
   glm::mat4 vp_inv = glm::inverse(mvp);
   float real_y = - mouseY; // in range -1 -> 1
   glm::vec4 screenPos_f = glm::vec4(mouseX, real_y, z, 1.0f);
   glm::vec4 worldPos_f = vp_inv * screenPos_f;
   return worldPos_f;
}


glm::vec4 new_unproject(float x, float y, float z) {
   // z is 1 and -1 for front and back (or vice verse).
   GtkAllocation allocation;
   gtk_widget_get_allocation(graphics_info_t::glarea, &allocation);
   int w = allocation.width;
   int h = allocation.height;
   glm::mat4 mvp = get_molecule_mvp();
   glm::mat4 vp_inv = glm::inverse(mvp);
   glm::vec4 screenPos_f = glm::vec4(x, y, z, 1.0f); // maybe +1
   glm::vec4 worldPos_f = vp_inv * screenPos_f;
   return worldPos_f;
}

glm::mat4 get_view_rotation() {

   // need to be in the correct program

   glm::mat4 view_matrix = glm::toMat4(graphics_info_t::glm_quat);
   return view_matrix;
}

void draw_map_molecules() {
   glLineWidth(1.0f);
   GLenum err = glGetError();
   if (err) std::cout << "gtk3_draw_molecules() glLineWidth " << err << std::endl;

   GLuint pid = graphics_info_t::shader_for_maps.get_program_id();
   glUseProgram(pid);
   err = glGetError();
   if (err) std::cout << "   gtk3_draw_molecules() glUseProgram with GL err "
                      << err << std::endl;


   glm::mat4 mvp = get_molecule_mvp();
   glm::mat4 view_rotation = get_view_rotation(); // hhmm... naming

   glEnable(GL_DEPTH_TEST); // this needs to be in the draw loop!?
   glDepthFunc(GL_LESS);

   for (int ii=graphics_info_t::n_molecules()-1; ii>=0; ii--) {
      if (! graphics_info_t::is_valid_map_molecule(ii)) continue;
      const molecule_class_info_t &m = graphics_info_t::molecules[ii];
      if (! m.draw_it_for_map) continue;
      if (m.n_vertices_for_map_VertexArray > 0) {

         bool draw_with_lines = true;
         if (!m.draw_it_for_map_standard_lines) draw_with_lines = false;

         if (draw_with_lines) {
            glBindVertexArray(graphics_info_t::molecules[ii].m_VertexArrayID_for_map);
            err = glGetError();
            if (err) std::cout << "   draw_map_molecules() glBindVertexArray() "
                               << graphics_info_t::molecules[ii].m_VertexArrayID_for_map
                               << " with GL err " << err << std::endl;

            // I doubt that I need to do these here:
            glBindBuffer(GL_ARRAY_BUFFER,         graphics_info_t::molecules[ii].m_VertexBufferID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphics_info_t::molecules[ii].m_IndexBufferID);

            glUniformMatrix4fv(graphics_info_t::shader_for_maps.mvp_uniform_location,           1, GL_FALSE, &mvp[0][0]);
            err = glGetError();
            if (err) std::cout << "   draw_map_molecules() glUniformMatrix4fv() mvp " << err << std::endl;
            glUniformMatrix4fv(graphics_info_t::shader_for_maps.view_rotation_uniform_location, 1, GL_FALSE, &view_rotation[0][0]);
            err = glGetError();
            if (err) std::cout << "   draw_map_molecules() glUniformMatrix4fv() vr  " << err << std::endl;

            GLuint background_colour_uniform_location = graphics_info_t::shader_for_maps.background_colour_uniform_location;
            glm::vec4 bgc(graphics_info_t::background_colour, 1.0);
            glUniform4fv(background_colour_uniform_location, 1, glm::value_ptr(bgc));
            err = glGetError();
            if (err) std::cout << "   draw_map_molecules() glUniform4fv() for bg  " << err << std::endl;

            GLuint eye_position_uniform_location = graphics_info_t::shader_for_maps.eye_position_uniform_location;
            glm::vec4 ep = new_unproject(0,0,-1);
            glUniform4fv(eye_position_uniform_location, 1, glm::value_ptr(ep));

            glDrawElements(GL_LINES, m.n_vertices_for_map_VertexArray,
                           GL_UNSIGNED_INT, nullptr);
            err = glGetError();
            if (err) std::cout << "   draw_map_molecules() glDrawElements() n_vertices: "
                               << m.n_vertices_for_map_VertexArray
                               << " with GL err " << err << std::endl;
         }

         if (!draw_with_lines) { // draw as a solid object
            if (false)
               std::cout << "   draw_map_molecules(): imol " << ii
                         << " array_id and n_vertices_for_VertexArray: "
                         << graphics_info_t::molecules[ii].m_VertexArrayID_for_map << " "
                         << graphics_info_t::molecules[ii].n_indices_for_triangles
                         << std::endl;

            glBindVertexArray(graphics_info_t::molecules[ii].m_VertexArrayID_for_map);
            err = glGetError();
            if (err) std::cout << "   draw_map_molecules() glBindVertexArray() "
                               << graphics_info_t::molecules[ii].m_VertexArrayID_for_map
                               << " with GL err " << err << std::endl;
            glDisable(GL_BLEND);
            glEnable(GL_BLEND);
            glBindBuffer(GL_ARRAY_BUFFER,         graphics_info_t::molecules[ii].m_VertexBufferID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphics_info_t::molecules[ii].m_IndexBuffer_for_triangles_ID);

            glUniformMatrix4fv(graphics_info_t::shader_for_maps.mvp_uniform_location, 1, GL_FALSE, &mvp[0][0]);
            err = glGetError();
            if (err) std::cout << "   draw_map_molecules() glUniformMatrix4fv() " << err << std::endl;
            glUniformMatrix4fv(graphics_info_t::shader_for_maps.view_rotation_uniform_location, 1, GL_FALSE, &view_rotation[0][0]);
            err = glGetError();
            if (err) std::cout << "   draw_map_molecules() glUniformMatrix4fv() " << err << std::endl;

            GLuint background_colour_uniform_location = graphics_info_t::shader_for_maps.background_colour_uniform_location;
            glm::vec4 bgc(graphics_info_t::background_colour, 1.0);
            glUniform4fv(background_colour_uniform_location, 1, glm::value_ptr(bgc));
            err = glGetError();
            if (err) std::cout << "   draw_map_molecules() glUniform4fv() for bg  " << err << std::endl;

            GLuint eye_position_uniform_location = graphics_info_t::shader_for_maps.eye_position_uniform_location;
            glm::vec4 ep = new_unproject(0,0,-1);
            glUniform4fv(eye_position_uniform_location, 1, glm::value_ptr(ep));

            // glDrawElements() uses a vertex count, not n indices, needs checking
            glDrawElements(GL_TRIANGLES, graphics_info_t::molecules[ii].n_indices_for_triangles,
                           GL_UNSIGNED_INT, nullptr);

            err = glGetError();
            if (err) std::cout << "   draw_map_molecules() glDrawElements() n_indices_for_triangles "
                               << graphics_info_t::molecules[ii].n_indices_for_triangles
                               << " with GL err " << err << std::endl;
         }
      }
   }
}

void
draw_model_molecules() {

   glm::mat4 mvp = get_molecule_mvp();
   glm::mat4 view_rotation = get_view_rotation(); // hhmm... naming

   Shader &shader = graphics_info_t::shader_for_models; // uses uniform map
   for (int ii=graphics_info_t::n_molecules()-1; ii>=0; ii--) {
      const molecule_class_info_t &m = graphics_info_t::molecules[ii];
      if (! graphics_info_t::is_valid_model_molecule(ii)) continue;
      if (! graphics_info_t::molecules[ii].draw_it) continue;

      if (false)
         std::cout << "imol " << ii << " n_vertices_for_model_VertexArray "
                   << graphics_info_t::molecules[ii].n_vertices_for_model_VertexArray << std::endl;
      if (graphics_info_t::molecules[ii].n_vertices_for_model_VertexArray > 0) {

         glDisable(GL_BLEND);
         GLuint pid = shader.get_program_id();
         glUseProgram(pid);
         GLuint err = glGetError(); if (err) std::cout << "   error draw_model_molecules() glUseProgram() "
                                                       << err << std::endl;

         glBindVertexArray(graphics_info_t::molecules[ii].m_VertexArray_for_model_ID);
         err = glGetError();
         if (err) std::cout << "   error draw_model_molecules() glBindVertexArray() "
                               << graphics_info_t::molecules[ii].m_VertexArray_for_model_ID
                               << " with GL err " << err << std::endl;

         glBindBuffer(GL_ARRAY_BUFFER,         graphics_info_t::molecules[ii].m_VertexBuffer_for_model_ID);
         err = glGetError(); if (err) std::cout << "   error draw_model_molecules() glBindBuffer() v " << err << std::endl;
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphics_info_t::molecules[ii].m_IndexBuffer_for_model_ID);
         err = glGetError(); if (err) std::cout << "   error draw_model_molecules() glBindBuffer() i " << err << std::endl;

         GLuint mvp_location           = graphics_info_t::shader_for_models.mvp_uniform_location;
         GLuint view_rotation_location = graphics_info_t::shader_for_models.view_rotation_uniform_location;

         err = glGetError();
         if (err) std::cout << "   error draw_model_molecules() glUniformMatrix4fv() pre mvp " << err << std::endl;
         glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);
         err = glGetError();
         if (err) std::cout << "   error draw_model_molecules() glUniformMatrix4fv() for mvp " << err << std::endl;
         glUniformMatrix4fv(view_rotation_location, 1, GL_FALSE, &view_rotation[0][0]);
         err = glGetError();
         if (err) std::cout << "   error draw_model_molecules() glUniformMatrix4fv() for view_rotation " << err << std::endl;

         GLuint background_colour_uniform_location = graphics_info_t::shader_for_models.background_colour_uniform_location;
         glm::vec4 bgc(graphics_info_t::background_colour, 1.0);
         glUniform4fv(background_colour_uniform_location, 1, glm::value_ptr(bgc));
         err = glGetError();
         if (err) std::cout << "   error draw_model_molecules() glUniform4fv() for background " << err << std::endl;

         GLuint eye_position_uniform_location = shader.eye_position_uniform_location;
         glm::vec4 ep = new_unproject(0,0,-1);
         glUniform4fv(eye_position_uniform_location, 1, glm::value_ptr(ep));
         err = glGetError();
         if (err) std::cout << "   error draw_model_molecules() glUniform4fv() for background " << err << std::endl;

         // draw with the vertex count, not the index count.
         GLuint n_verts = graphics_info_t::molecules[ii].n_indices_for_model_triangles;
         //std::cout << "   Drawing " << n_verts << " model vertices" << std::endl;
         glDrawElements(GL_TRIANGLES, n_verts, GL_UNSIGNED_INT, nullptr);
         err = glGetError();
         if (err) std::cout << "   error draw_model_molecules() glDrawElements() "
                            << n_verts << " with GL err " << err << std::endl;

      }
   }
}

void
draw_molecular_triangles() {
#ifdef USE_MOLECULES_TO_TRIANGLES
   // Martin's triangular molecules
   //
   // centre of the screen
   FCXXCoord pos(graphics_info_t::RotationCentre_x(),
   graphics_info_t::RotationCentre_y(),
   graphics_info_t::RotationCentre_z());
   // where is the eye?  That's what we want.
   // front plane is at z=0;
   GtkAllocation allocation;
   GtkWidget *widget = graphics_info_t::glarea;
   if (! widget) return;
   gtk_widget_get_allocation(widget, &allocation);
   coot::Cartesian tp_1_cart = unproject_xyz(allocation.width/2,
                                             allocation.height/2, 1);
   FCXXCoord tp_1(tp_1_cart.x(), tp_1_cart.y(), tp_1_cart.z());
   FCXXCoord diff = tp_1 - pos;
   FCXXCoord eye_pos = pos + diff * 5.0;
   // std::cout << "eye_pos: " << eye_pos << "\n";
   // coot::Cartesian eye_cart = pos + 20 * diff;
   // FCXXCoord eye_pos(eye_cart.x(), eye_cart.y(), eye_cart.z());
   if (graphics_info_t::mol_tri_scene_setup) {
      if (graphics_info_t::mol_tri_renderer) {
         //Can retrieve reference to the light if so preferred
         // This doesn't move the lights
         // FCXXCoord random_trans(50.0 * coot::util::random()/float(RAND_MAX),
         // 		                 50.0 * coot::util::random()/float(RAND_MAX),
         //                        50.0 * coot::util::random()/float(RAND_MAX));
	      FCXXCoord light_pos = pos + diff * 10; //  + random_trans;
         FCXXCoord neg_light_pos = pos + diff * 10; // - random_trans;

         graphics_info_t::mol_tri_scene_setup->getLight(0)->setTranslation(light_pos);
         graphics_info_t::mol_tri_scene_setup->getLight(1)->setTranslation(neg_light_pos);

         for (int ii=graphics_info_t::n_molecules()-1; ii>=0; ii--) {
            if (graphics_info_t::is_valid_model_molecule(ii)) {
               if (graphics_info_t::molecules[ii].draw_it) {
                  if (graphics_info_t::molecules[ii].molrepinsts.size()) {
                     std::cout << "----------------------- in draw_molecular_triangles() calling Martin code now... \n";
                     // molrepinsts get added to mol_tri_scene_setup when then are made
                     GLenum err = glGetError();
                     if (err) std::cout << "gl error pre-renderer in draw_molecular_triangles() " << err << std::endl;
                     // turns on glLighting.
                     graphics_info_t::mol_tri_scene_setup->renderWithRendererFromViewpoint(graphics_info_t::mol_tri_renderer, eye_pos);
                     err = glGetError();
                     if (err) std::cout << "gl error in draw_molecular_triangles() " << err << std::endl;
                  }
               }
            }
         }
      }
   }
#endif
}

void
draw_molecules() {

   draw_model_molecules();
   draw_map_molecules();
   draw_molecular_triangles(); // Martin's renderings
}

void
draw_cube(GtkGLArea *glarea, unsigned int cube_type) {

   gtk_gl_area_make_current(glarea);
   glLineWidth(2.0);  // GLv4 antialiasing - OpenGL implementations are not required to support this
   GLenum err = glGetError();
   if (err) std::cout << "   error draw_central_cube() A err " << err << std::endl;

   // To see the possible values of the line width in aliased mode:
   // GLfloat line_width_max_min[2] = {0.0f, 0.0f};
   // glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
   // This may not be possible in GL_LINE_SMOOTH mode.

   glm::mat4 mvp = get_molecule_mvp();
   glm::mat4 view_rotation = get_view_rotation(); // hhmm... naming

   glBindVertexArray(graphics_info_t::central_cube_vertexarray_id);
   err = glGetError(); if (err) std::cout << "   error draw_central_cube() B err " << err << std::endl;
   glUseProgram(graphics_info_t::shader_for_central_cube.get_program_id());
   err = glGetError(); if (err) std::cout << "   error draw_central_cube() C err " << err << std::endl;
   glm::mat4 view_orientation = glm::toMat4(graphics_info_t::glm_quat);
   glm::vec3 rc = graphics_info_t::get_rotation_centre();
   if (cube_type == VIEW_CENTRAL_CUBE) {
      mvp = glm::translate(mvp, rc);
      glm::vec3 sc(0.2f, 0.2f, 0.2f);
      mvp = glm::scale(mvp, sc);
   }
   if (cube_type == ORIGIN_CUBE) {
      glm::vec3 sc(0.3f, 0.3f, 0.3f);
      mvp = glm::scale(mvp, sc);
   }

   // we don't diverge here on the cube tye. Maybe change the name of the shader
   // because it does both
   Shader &shader = graphics_info_t::shader_for_central_cube;

   // we do this for all the shaders - Hmm.
   {
      GLuint mvp_location           = shader.mvp_uniform_location;
      GLuint view_rotation_location = shader.view_rotation_uniform_location;

      glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);
      err = glGetError();
      if (err) std::cout << "   error draw_central_cube() glUniformMatrix4fv() for mvp " << err << std::endl;
      glUniformMatrix4fv(view_rotation_location, 1, GL_FALSE, &view_rotation[0][0]);
      err = glGetError();
      if (err) std::cout << "   error draw_central_cube() glUniformMatrix4fv() for view_rotation " << err << std::endl;

      GLuint line_colour_uniform_location = shader.line_colour_uniform_location;
      glm::vec4 lc(0.5, 0.4, 0.4, 1.0);
      if (cube_type == ORIGIN_CUBE)
         lc = glm::vec4(0.6, 0.6, 0.4, 1.0);
      glUniform4fv(line_colour_uniform_location, 1, glm::value_ptr(lc));
      err = glGetError();
      if (err) std::cout << "   error draw_central_cube() glUniform4fv() for line colour " << err << std::endl;

      GLuint background_colour_uniform_location = shader.background_colour_uniform_location;
      glm::vec4 bgc(graphics_info_t::background_colour, 1.0);
      glUniform4fv(background_colour_uniform_location, 1, glm::value_ptr(bgc));
      err = glGetError();
      if (err) std::cout << "   error draw_central_cube() glUniform4fv() for background " << err << std::endl;

   }

   glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
   err = glGetError();
   if (err) std::cout << "draw_central_cube() F glDrawElements() err " << err << std::endl;

   glBindVertexArray(0); // unbind
   glUseProgram(0);
}


void
draw_central_cube(GtkGLArea *glarea) {
   draw_cube(glarea, VIEW_CENTRAL_CUBE);
}

void draw_origin_cube(GtkGLArea *glarea) {
   draw_cube(glarea, ORIGIN_CUBE);
}

GtkWidget *my_gtkglarea(GtkWidget *vbox) {

   GtkWidget *w = gtk_gl_area_new();
   gtk_widget_set_size_request(w, 900, 900);
   gtk_box_pack_start(GTK_BOX(vbox), w, TRUE, TRUE, 2);
   return w;
}

void
on_glarea_realize(GtkGLArea *glarea) {
   std::cout << "realize!" << std::endl;

   GtkAllocation allocation;
   gtk_widget_get_allocation(GTK_WIDGET(glarea), &allocation);
   int w = allocation.width;
   int h = allocation.height;

   gtk_gl_area_make_current(glarea);
   // gtk_gl_area_set_has_alpha(glarea, TRUE);
   gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(glarea), TRUE);
   GLenum err = glGetError();
   err = glGetError(); if (err) std::cout << "on_glarea_realize() A err " << err << std::endl;

   init_shaders();
   init_buffers();
   init_central_cube_shaders();
   err = glGetError();
   std::cout << "on_glarea_realize() post init_shaders() err is " << err << std::endl;

   graphics_info_t::shader_for_screen.Use(); // needed?

   err = glGetError(); std::cout << "start on_glarea_realize() err is " << err << std::endl;

   graphics_info_t::screen_framebuffer.init(w,h);

   err = glGetError(); if (err) std::cout << "start on_glarea_realize() post screen_buffer init() err is " << err << std::endl;

   setup_hud_text(w, h, graphics_info_t::shader_for_hud_text);

   graphics_info_t::shader_for_screen.Use();
   err = glGetError(); if (err) std::cout << "on_glarea_realize() B err " << err << std::endl;
   graphics_info_t::shader_for_screen.set_int_for_uniform("screnTexture", 0);
   err = glGetError(); if (err) std::cout << "on_glarea_realize() C err " << err << std::endl;
   graphics_info_t::shader_for_screen.set_int_for_uniform("screenDepth", 1);
   err = glGetError(); if (err) std::cout << "on_glarea_realize() D err " << err << std::endl;

   gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(glarea), TRUE);

   glEnable(GL_DEPTH_TEST);
   // glEnable(GL_CULL_FACE); // if I enable this, then I get to see th back side
                              // of the bonds (and atoms, possibly) It's a weird look
   glEnable(GL_BLEND);


   // Make antialised lines
   if (false) {
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_LINE_SMOOTH);
   }

   // Martin's Molecular triangles
   setup_for_mol_triangles();

}

gboolean
on_glarea_render(GtkGLArea *glarea) {

   GtkAllocation allocation;
   gtk_widget_get_allocation(GTK_WIDGET(glarea), &allocation);
   int w = allocation.width;
   int h = allocation.height;

   auto tp_0 = std::chrono::high_resolution_clock::now();
   GLenum err = glGetError();
   if (err) std::cout << "on_glarea_render() start " << err << std::endl;

   // is this needed?
   gtk_gl_area_make_current(glarea);
   err = glGetError(); if (err) std::cout << "on_glarea_render() post gtk_gl_area_make_current() " << err << std::endl;

   // glBindFramebuffer(GL_FRAMEBUFFER, graphics_info_t::framebuffer_id);
   graphics_info_t::screen_framebuffer.bind();
   err = glGetError(); if (err) std::cout << "on_glarea_render() post screen_buffer bind() " << err << std::endl;

   glEnable(GL_DEPTH_TEST);

   {
      const glm::vec3 &bg = graphics_info_t::background_colour;
      glClearColor (bg[0], bg[1], bg[2], 1.0);
      err = glGetError(); if (err) std::cout << "on_glarea_render B err " << err << std::endl;
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      err = glGetError(); if (err) std::cout << "on_glarea_render C err " << err << std::endl;

      draw_central_cube(glarea);
      draw_origin_cube(glarea);
      err = glGetError();
      if (err) std::cout << "on_glarea_render  pre-draw-text err " << err << std::endl;
      draw_hud_text(w, h, graphics_info_t::shader_for_hud_text);
      err = glGetError(); if (err) std::cout << "on_glarea_render post-draw-text err " << err << std::endl;

      err = glGetError(); if (err) std::cout << "on_glarea_render gtk3_draw_molecules() " << err << std::endl;

      draw_molecules();
      glBindVertexArray(0);
   }

   // use this, rather than glBindFramebuffer(GL_FRAMEBUFFER, 0); ... just Gtk things.
   gtk_gl_area_attach_buffers(glarea);

   glDisable(GL_DEPTH_TEST);

   graphics_info_t::shader_for_screen.Use();
   glBindVertexArray(graphics_info_t::screen_quad_vertex_array_id);

   glClearColor(0.5, 0.2, 0.2, 1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   GLuint pid = graphics_info_t::shader_for_screen.get_program_id();
   glActiveTexture(GL_TEXTURE0 + 1);
   glBindTexture(GL_TEXTURE_2D, graphics_info_t::screen_framebuffer.get_texture_colour());
   glUniform1i(glGetUniformLocation(pid, "screenTexture"), 1);
   glActiveTexture(GL_TEXTURE0 + 2);
   glBindTexture(GL_TEXTURE_2D, graphics_info_t::screen_framebuffer.get_texture_depth());
   glUniform1i(glGetUniformLocation(pid, "screenDepth"), 2);
   err = glGetError(); if (err) std::cout << "on_glarea_render() D err " << err << std::endl;

   glDrawArrays(GL_TRIANGLES, 0, 6);
   err = glGetError(); if (err) std::cout << "on_glarea_render() E err " << err << std::endl;

   graphics_info_t::frame_counter++;

  return FALSE;
}


void
on_glarea_resize(GtkGLArea *glarea, gint width, gint height) {

   std::cout << "resize!" << std::endl;
   graphics_info_t g;
   g.graphics_x_size = width;
   g.graphics_y_size = height;

}

gboolean
on_glarea_scroll(GtkWidget *widget, GdkEventScroll *event) {

   int direction = 1;
   if (event->direction == GDK_SCROLL_UP)
      direction = -1;

   std::cout << "scroll " << direction << std::endl;

   graphics_info_t g;
   int imol_scroll = graphics_info_t::scroll_wheel_map;

   if (g.is_valid_map_molecule(imol_scroll)) {
      // use direction
      if (direction == 1)
         graphics_info_t::molecules[imol_scroll].pending_contour_level_change_count--;
      if (direction == -1)
         graphics_info_t::molecules[imol_scroll].pending_contour_level_change_count++;
      int contour_idle_token = g_idle_add(idle_contour_function, g.glarea);
      std::cout << "####### Now contour level for map " << imol_scroll << "is "
                << g.molecules[imol_scroll].contour_level << std::endl;
      g.set_density_level_string(imol_scroll, g.molecules[imol_scroll].contour_level);
      g.display_density_level_this_image = 1;
      g.update_maps();
      gtk_widget_queue_draw(widget);
   } else {
      std::cout << "No map" << std::endl;
   }
   return TRUE;
}


gboolean
on_glarea_button_press(GtkWidget *widget, GdkEventButton *event) {

   // std::cout << "button press!" << std::endl;
   graphics_info_t g;
   g.SetMouseBegin(event->x,event->y);
   g.SetMouseClicked(event->x, event->y); // Hmm
   return TRUE;
}

gboolean
on_glarea_button_release(GtkWidget *widget, GdkEventButton *event) {

   if (event->state & GDK_BUTTON2_MASK) {
      graphics_info_t g;
      pick_info nearest_atom_index_info = atom_pick_gtk3(event);
      double delta_x = g.GetMouseClickedX() - event->x;
      double delta_y = g.GetMouseClickedY() - event->y;
      if (std::abs(delta_x) < 10.0) {
         if (std::abs(delta_y) < 10.0) {
            if (nearest_atom_index_info.success == GL_TRUE) {
               g.setRotationCentre(nearest_atom_index_info.atom_index,
				                       nearest_atom_index_info.imol);
            }
         }
      }
   }
   return TRUE;
}

gboolean
on_glarea_motion_notify(GtkWidget *widget, GdkEventMotion *event) {

   int r = 0;
   graphics_info_t g;

   // split this function up before it gets too big.

   g.mouse_current_x = event->x;
   g.mouse_current_y = event->y;

   if (event->state & GDK_BUTTON1_MASK) {

      GtkAllocation allocation;
      gtk_widget_get_allocation(widget, &allocation);
      int w = allocation.width;
      int h = allocation.height;

      glm::quat tb_quat =
         g.trackball_to_quaternion((2.0*g.GetMouseBeginX() - w)/w, (h - 2.0*g.GetMouseBeginY())/h,
           (2.0*g.mouse_current_x - w)/w,  (h - 2.0*g.mouse_current_y)/h,
           g.get_trackball_size());

      glm::mat4 mat_from_quat = glm::toMat4(tb_quat);

      glm::quat product = tb_quat * graphics_info_t::glm_quat;
      graphics_info_t::glm_quat = glm::normalize(product);
   }


   if (event->state & GDK_BUTTON2_MASK) {

      // View Panning

      GtkAllocation allocation;
      gtk_widget_get_allocation(widget, &allocation);
      int w = allocation.width;
      int h = allocation.height;

      glm::mat4 mvp = get_molecule_mvp(); // modeglml matrix includes orientation with the quaternion


      float mouseX_1 = g.GetMouseBeginX() / (w * 0.5f) - 1.0f;
      float mouseY_1 = g.GetMouseBeginY() / (h * 0.5f) - 1.0f;
      float mouseX_2 = g.mouse_current_x  / (w * 0.5f) - 1.0f;
      float mouseY_2 = g.mouse_current_y  / (h * 0.5f) - 1.0f;

      glm::mat4 vp_inv = glm::inverse(mvp);

      glm::vec4 screenPos_1 = glm::vec4(mouseX_1, -mouseY_1, 1.0f, 1.0f);
      glm::vec4 screenPos_2 = glm::vec4(mouseX_2, -mouseY_2, 1.0f, 1.0f);
      glm::vec4 worldPos_1 = vp_inv * screenPos_1;
      glm::vec4 worldPos_2 = vp_inv * screenPos_2;

      glm::vec4 delta(worldPos_1 - worldPos_2);

      g.add_to_rotation_centre(delta);
      g.update_maps();
      int contour_idle_token = g_idle_add(idle_contour_function, g.glarea);

   }

   if (event->state & GDK_BUTTON3_MASK) {

      // Zooming

      double delta_x = event->x - g.GetMouseBeginX();
      double delta_y = event->y - g.GetMouseBeginY();
      double fx = 1.0f +  delta_x/300.0;
      double fy = 1.0f +  delta_y/300.0;
      if (fx > 0.0) g.zoom *= fx;
      if (fy > 0.0) g.zoom *= fy;
      // std::cout << "now zoom: " << g.zoom << std::endl;
   }

   // for next motion
   g.SetMouseBegin(event->x,event->y);
   gtk_widget_queue_draw(widget);
   return TRUE;
}

gint
view_spin_func(gpointer data) {

   float delta = -0.002;
   glm::vec3 EulerAngles(0, delta, 0);
   glm::quat quat_delta(EulerAngles);
   glm::quat normalized_quat_delta(glm::normalize(quat_delta));
   glm::quat product = normalized_quat_delta * graphics_info_t::glm_quat;
   graphics_info_t::glm_quat = glm::normalize(product);
   gtk_widget_queue_draw(graphics_info_t::glarea);

   std::chrono::time_point<std::chrono::system_clock> tp_now = std::chrono::high_resolution_clock::now();
   std::chrono::duration<double> elapsed_seconds = tp_now - graphics_info_t::previous_frame_time;
   if (elapsed_seconds.count() > 1.0) {
      float nf = graphics_info_t::frame_counter - graphics_info_t::frame_counter_at_last_display;
      std::cout << "Frame/second: " << 1000 * elapsed_seconds.count()/nf << " milliseconds\n";
      graphics_info_t::previous_frame_time = tp_now;
      graphics_info_t::frame_counter_at_last_display = graphics_info_t::frame_counter;
   }

   // kludge/race condition?
   if (graphics_info_t::idle_function_spin_rock_token == -1)
      return FALSE;
   else
      return TRUE;
}

#include "c-interface.h" // for update_go_to_atom_from_current_position()

gboolean
on_glarea_key_press_notify(GtkWidget *widget, GdkEventKey *event) {

   std::cout << "on_glarea_key_press_notify() " << std::endl;
   graphics_info_t g;
   gboolean handled = false;

   if (event->keyval == GDK_KEY_n) {
      std::cout << "Zoom in " << std::endl;
      graphics_info_t::zoom *= 0.9;
   }
   if (event->keyval == GDK_KEY_m) {
      std::cout << "Zoom out " << std::endl;
      graphics_info_t::zoom *= 1.1;
   }

   // I want to be able to push out the front clipping plane (say, for making a figure or movie)

   if (event->keyval == GDK_KEY_d) {
      adjust_clipping(0.3);
   }

   if (event->keyval == GDK_KEY_f) {
      adjust_clipping(-0.3);
   }

   if (event->keyval == GDK_KEY_i) {
      std::cout << "Debug idle_function_spin_rock_token " << graphics_info_t::idle_function_spin_rock_token
                << std::endl;
      if (graphics_info_t::idle_function_spin_rock_token != -1) {
         std::cout << "Removing the idle function\n";
         g_idle_remove_by_data(GINT_TO_POINTER(66)); // just a kludge for the moment
         graphics_info_t::idle_function_spin_rock_token = -1;
      } else {
         int toi = g_timeout_add(5, view_spin_func, GINT_TO_POINTER(66));
         graphics_info_t::idle_function_spin_rock_token = toi;
      }
   }

   // GDK_KEY_equals should be the same as GDK_KEY_plus
   if (event->keyval == GDK_KEY_minus || event->keyval == GDK_KEY_plus) {
      int s = graphics_info_t::scroll_wheel_map;
      if (graphics_info_t::is_valid_map_molecule(s)) {
         if (event->keyval == GDK_KEY_minus)
            graphics_info_t::molecules[s].pending_contour_level_change_count--;
         if (event->keyval == GDK_KEY_plus)
            graphics_info_t::molecules[s].pending_contour_level_change_count++;
         int contour_idle_token = g_idle_add(idle_contour_function, g.glarea);
         g.set_density_level_string(s, g.molecules[s].contour_level);
         g.display_density_level_this_image = 1;
         handled = TRUE;
      }
   }

   // fix the type here
   if (int(event->keyval) == graphics_info_t::update_go_to_atom_from_current_residue_key) {
      update_go_to_atom_from_current_position();
      handled = TRUE;
   }

   gtk_widget_queue_draw(widget);

   return handled;

}

gboolean
on_glarea_key_release_notify(GtkWidget *widget, GdkEventKey *event) {

   graphics_info_t g;

   if (event->keyval == GDK_KEY_space) {
      g.reorienting_next_residue_mode = false; // hack
      bool reorienting = graphics_info_t::reorienting_next_residue_mode;
      if (reorienting) {
         if (graphics_info_t::shift_is_pressed) {
            g.reorienting_next_residue(false); // backwards
         } else {
            std::cout << "Do forward reorienting" << std::endl;
            g.reorienting_next_residue(true); // forwards
         }
      } else {
         // old/standard simple translation
         if (graphics_info_t::shift_is_pressed) {
            g.intelligent_previous_atom_centring(g.go_to_atom_window);
         } else {
            g.intelligent_next_atom_centring(g.go_to_atom_window);
         }
      }
   }
   return TRUE;
}

void my_glarea_add_signals_and_events(GtkWidget *glarea) {

   gtk_widget_add_events(glarea, GDK_SCROLL_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON_PRESS_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON_RELEASE_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON1_MOTION_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON2_MOTION_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON3_MOTION_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON1_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON2_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON3_MASK);
   gtk_widget_add_events(glarea, GDK_KEY_PRESS_MASK);

   // key presses for the glarea:
   gtk_widget_set_can_focus(glarea, TRUE);
   gtk_widget_grab_focus(glarea);

   g_signal_connect(glarea, "realize", G_CALLBACK(on_glarea_realize), NULL);
   g_signal_connect(glarea, "render",  G_CALLBACK(on_glarea_render),  NULL);
   g_signal_connect(glarea, "resize",  G_CALLBACK(on_glarea_resize),  NULL);
   g_signal_connect(glarea, "scroll-event",          G_CALLBACK(on_glarea_scroll),             NULL);
   g_signal_connect(glarea, "button-press-event",    G_CALLBACK(on_glarea_button_press),       NULL);
   g_signal_connect(glarea, "button-release-event",  G_CALLBACK(on_glarea_button_release),     NULL);
   g_signal_connect(glarea, "motion-notify-event",   G_CALLBACK(on_glarea_motion_notify),      NULL);
   g_signal_connect(glarea, "key-press-event",       G_CALLBACK(on_glarea_key_press_notify),   NULL);
   g_signal_connect(glarea, "key-release-event",     G_CALLBACK(on_glarea_key_release_notify), NULL);

}