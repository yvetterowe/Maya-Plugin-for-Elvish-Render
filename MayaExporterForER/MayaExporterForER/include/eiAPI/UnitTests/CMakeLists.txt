include_directories("${ER_INCLUDE_DIR}")
include_directories("${CXXTEST_INCLUDE_DIR}")

add_definitions(-DUNIT_TESTS)

CXXTEST_ADD_TEST(ei_raytracer_unittest ei_raytracer_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_raytracer_unittest.h")
target_link_libraries(ei_raytracer_unittest eiCORE)
target_link_libraries(ei_raytracer_unittest eiAPI)

CXXTEST_ADD_TEST(ei_raytracer_hair_unittest ei_raytracer_hair_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_raytracer_hair_unittest.h")
target_link_libraries(ei_raytracer_hair_unittest eiCORE)
target_link_libraries(ei_raytracer_hair_unittest eiAPI)

CXXTEST_ADD_TEST(ei_scene_unittest ei_scene_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_scene_unittest.h")
target_link_libraries(ei_scene_unittest eiCORE)
target_link_libraries(ei_scene_unittest eiAPI)

CXXTEST_ADD_TEST(ei_hair_unittest ei_hair_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_hair_unittest.h")
target_link_libraries(ei_hair_unittest eiCORE)
target_link_libraries(ei_hair_unittest eiAPI)

CXXTEST_ADD_TEST(ei_texture_unittest ei_texture_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_texture_unittest.h")
target_link_libraries(ei_texture_unittest eiCORE)
target_link_libraries(ei_texture_unittest eiAPI)

CXXTEST_ADD_TEST(ei_displace_unittest ei_displace_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_displace_unittest.h")
target_link_libraries(ei_displace_unittest eiCORE)
target_link_libraries(ei_displace_unittest eiAPI)

CXXTEST_ADD_TEST(ei_approx_viewdep_unittest ei_approx_viewdep_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_approx_viewdep_unittest.h")
target_link_libraries(ei_approx_viewdep_unittest eiCORE)
target_link_libraries(ei_approx_viewdep_unittest eiAPI)

CXXTEST_ADD_TEST(ei_approx_stitch_unittest ei_approx_stitch_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_approx_stitch_unittest.h")
target_link_libraries(ei_approx_stitch_unittest eiCORE)
target_link_libraries(ei_approx_stitch_unittest eiAPI)

CXXTEST_ADD_TEST(ei_approx_grid_unittest ei_approx_grid_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_approx_grid_unittest.h")
target_link_libraries(ei_approx_grid_unittest eiCORE)
target_link_libraries(ei_approx_grid_unittest eiAPI)

CXXTEST_ADD_TEST(ei_vertex_color_unittest ei_vertex_color_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_vertex_color_unittest.h")
target_link_libraries(ei_vertex_color_unittest eiCORE)
target_link_libraries(ei_vertex_color_unittest eiAPI)

CXXTEST_ADD_TEST(ei_face_color_unittest ei_face_color_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_face_color_unittest.h")
target_link_libraries(ei_face_color_unittest eiCORE)
target_link_libraries(ei_face_color_unittest eiAPI)

CXXTEST_ADD_TEST(ei_shader_graph_unittest ei_shader_graph_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_shader_graph_unittest.h")
target_link_libraries(ei_shader_graph_unittest eiCORE)
target_link_libraries(ei_shader_graph_unittest eiAPI)

CXXTEST_ADD_TEST(ei_checker_uv_unittest ei_checker_uv_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_checker_uv_unittest.h")
target_link_libraries(ei_checker_uv_unittest eiCORE)
target_link_libraries(ei_checker_uv_unittest eiAPI)

CXXTEST_ADD_TEST(ei_texture_filter_unittest ei_texture_filter_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_texture_filter_unittest.h")
target_link_libraries(ei_texture_filter_unittest eiCORE)
target_link_libraries(ei_texture_filter_unittest eiAPI)

CXXTEST_ADD_TEST(ei_instancing_unittest ei_instancing_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_instancing_unittest.h")
target_link_libraries(ei_instancing_unittest eiCORE)
target_link_libraries(ei_instancing_unittest eiAPI)

CXXTEST_ADD_TEST(ei_motion_blur_unittest ei_motion_blur_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_motion_blur_unittest.h")
target_link_libraries(ei_motion_blur_unittest eiCORE)
target_link_libraries(ei_motion_blur_unittest eiAPI)

CXXTEST_ADD_TEST(ei_dof_unittest ei_dof_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_dof_unittest.h")
target_link_libraries(ei_dof_unittest eiCORE)
target_link_libraries(ei_dof_unittest eiAPI)

CXXTEST_ADD_TEST(ei_env_unittest ei_env_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_env_unittest.h")
target_link_libraries(ei_env_unittest eiCORE)
target_link_libraries(ei_env_unittest eiAPI)

CXXTEST_ADD_TEST(ei_imager_unittest ei_imager_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_imager_unittest.h")
target_link_libraries(ei_imager_unittest eiCORE)
target_link_libraries(ei_imager_unittest eiAPI)

CXXTEST_ADD_TEST(ei_volume_unittest ei_volume_unittest.cpp "${CMAKE_CURRENT_SOURCE_DIR}/ei_volume_unittest.h")
target_link_libraries(ei_volume_unittest eiCORE)
target_link_libraries(ei_volume_unittest eiAPI)
