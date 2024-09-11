#include <atomic>
#include <iostream>

#include <glm/glm.hpp>

#include "camera/camera.hpp"
#include "camera/film.hpp"

#include "shape/model.hpp"
#include "shape/plane.hpp"
#include "shape/scene.hpp"
#include "shape/sphere.hpp"

#include "utils/rgb.hpp"
#include "utils/rng.hpp"

#include "renderer/normal_renderer.hpp"
#include "renderer/simple_rt_renderer.hpp"
#include "renderer/debug_renderer.hpp"

int main() {
  std::atomic<int> count = 0;

  // Camera
  Film film{192*4, 108*4};
  Camera camera{film, {-3.6, 0, 0}, {0, 0, 0}, 45};

  // Scene
  Model model("models/dragon_871k.obj");
  Sphere sphere{{0, 0, 0}, 1};
  Plane plane{{0, 0, 0}, {0, 1, 0}};

  Scene scene{};
  scene.addShape(model, {RGB(202, 159, 117)},
                 {0, 0, 0}, {1, 3, 2});
  scene.addShape(sphere, {{1, 1, 1}, false, RGB(255, 128, 128)}, {0, 0, 2.5f});
  scene.addShape(sphere, {{1, 1, 1}, false, RGB(128, 128, 255)}, {0, 0, -2.5f});
  scene.addShape(sphere, {{1, 1, 1}, true}, {3, 0.5f, -2});
  scene.addShape(plane, {RGB(120, 204, 157)}, {0, -0.5f, 0});

  NormalRenderer normal_renderer(camera, scene);
  normal_renderer.render(1, "normal.ppm");
  
  BoundsTestCountRenderer btc_renderer(camera, scene);
  btc_renderer.render(1, "debug_BTC.ppm");

  TriangleTestCountRenderer ttc_renderer(camera, scene);
  ttc_renderer.render(1, "debug_TTC.ppm");

  BoundsDepthRenderer bc_renderer(camera, scene);
  bc_renderer.render(1, "debug_DC.ppm");   

  SimpleRTRenderer simple_rt_renderer(camera, scene);
  simple_rt_renderer.render(128, "simple_rt.ppm");

  return 0;
}


// Debug Mode
// Load model: 163ms
// first Parallel for: 773ms
// Parallel for: 485ms
// Save to simple_rt.ppm: 311ms
// Render 1spp normal.ppm: 23993ms
// Render 32spp simple_rt.ppm: 1966102ms
// Render 128spp simple_rt-128spp.ppm: 7726695ms


// Release Mode
// Load model: 29ms
// first Parallel for: 700ms
// Parallel for: 245ms
// Save to simple_rt.ppm: 89ms
// Render 1spp normal.ppm: 1984ms
// Render 32spp simple_rt.ppm: 108775ms


// Change Paralledfor 
// Parallel for: 0ms
// Render 1spp normal.ppm: 1343ms
// Render 128spp simple_rt.ppm: 462954ms

// Change file.save
// Save to simple_rt.ppm: 5ms

// Add bounds
// Load model: 31ms
// Render 1spp normal.ppm: 269ms
// Render 128spp simple_rtspp.ppm: 83884ms

// rapidobj
// Load model: 4ms

// Add bvh
// Render 128spp simple_rt.ppm: 15078ms

// Tree structure To Array structure
// Render 128spp simple_rt.ppm: 15100ms

// node array cache hit rate  1、将三角形数组改成索引数组 2、优化node节点大小为32字节


// dragon_87k.obj
// 向z轴正方向观察: 7631ms
// 向z轴负方向观察: 8887ms

// 左右优化后
// 向z轴负方向观察: 7528ms

// bounds除法优化
// 向z轴负方向观察: 6554ms

// x轴正方向

// Without SHA
// Total_Node_Count: 142643
// Leaf_Node_Count: 71322
// Total_Triangle_Count: 87130
// Mean_leaf_node_triangle_count: 1.22164
// Max_leaf_node_triangle_count: 23      
// Load model models/dragon_87k.obj: 2144ms
// Render 128spp simple_rt.ppm: 10164ms

// With SHA
// Total_Node_Count: 164067
// Leaf_Node_Count: 82034
// Total_Triangle_Count: 87130
// Mean_leaf_node_triangle_count: 1.06212
// Max_leaf_node_triangle_count: 8       
// Load model models/dragon_87k.obj: 14872ms

// With SHA and 3Dim
// Total_Node_Count: 174195
// Leaf_Node_Count: 87098
// Total_Triangle_Count: 87130
// Mean_leaf_node_triangle_count: 1.00037
// Max_leaf_node_triangle_count: 3       
// Load model models/dragon_87k.obj: 52932ms
// Render 128spp simple_rt.ppm: 10516ms

// Render dragon_871k.obj 
// Total_Node_Count: 1742423
// Leaf_Node_Count: 871212
// Total_Triangle_Count: 871306
// Mean_leaf_node_triangle_count: 1.00011
// Max_leaf_node_triangle_count: 3       
// Load model models/dragon_871k.obj:: 544866ms
// Render 128spp simple_rt.ppm: 13007ms

// Bucket quick build
// Total_Node_Count: 1742425
// Leaf_Node_Count: 871213                   
// Total_Triangle_Count: 871306
// Mean_leaf_node_triangle_count: 1.00011
// Max_leaf_node_triangle_count: 3       
// Load model models/dragon_871k.obj: 33247ms
// Render 128spp simple_rt.ppm: 10934ms

// Thread preempting optmization
// Load model models/dragon_871k.obj: 21357ms

// Menory allocator
// Load model models/dragon_871k.obj: 12649ms

// BVH vector reserve and bound assign
// Load model models/dragon_871k.obj: 9650ms



// Final cost

// Release simple_dragon.obj 128spp
// Load model models/dragon_871k.obj: 22ms
// Render 128spp simple_rt.ppm、: 15018ms 

// Release dragon_87k.obj 128spp
// Load model models/dragon_871k.obj: 885ms
// Render 128spp simple_rt.ppm、: 16217ms 

// Release dragon_871k.obj 128spp
// Load model models/dragon_871k.obj: 9858ms
// Render 128spp simple_rt.ppm、: 16346ms 