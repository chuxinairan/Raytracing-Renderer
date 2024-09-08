#include "shape/model.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

Model::Model(const std::filesystem::path& filename)
{
    // v 1.0 2.0 3.0
    std::vector<glm::vec3> positions;
    // vn 1.0 2.0 3.0
    std::vector<glm::vec3> normals;
    // f 1//2 2//3 3//4

    std::ifstream file(filename);
    if(!file.good()) {
        std::cout << "文件读取失败" << std::endl;
    }

    std::string line;
    char trash;
    while(!file.eof()) {
        std::getline(file, line);
        std::istringstream iss(line);

        if(line.compare(0, 2, "v ") == 0){
            // v 1.0 2.0 3.0
            glm::vec3 position;
            iss >> trash >> position.x >> position.y >> position.z;
            positions.push_back(position);
        } else if(line.compare(0, 3, "vn ") == 0) {
            // vn 1.0 2.0 3.0
            glm::vec3 normal;
            iss >> trash >> trash >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        } else if(line.compare(0, 2, "f ") == 0) {
            // f 1//2 2//3 3//4
            glm::ivec3 ind_v, ind_vn;
            iss >> trash;
            iss >> ind_v.x >> trash >> trash >> ind_vn.x;
            iss >> ind_v.y >> trash >> trash >> ind_vn.y;
            iss >> ind_v.z >> trash >> trash >> ind_vn.z;

            Triangle triangle(
                positions[ind_v.x - 1], positions[ind_v.y - 1], positions[ind_v.z - 1],   // obj下标是以1开头的
                normals[ind_vn.x - 1], normals[ind_vn.y - 1], normals[ind_vn.z - 1]
            );
            triangles.push_back(triangle);
        }
    }
}

std::optional<HitInfo> Model::intersect(Ray& ray, float t_min, float t_max) const
{
    std::optional<HitInfo> closest_hit_info{};
    for(const auto& triangle : triangles)
    {
        auto result = triangle.intersect(ray, t_min, t_max);
        if(result.has_value()) {
            t_max = result.value().t;
            closest_hit_info = result;
        }
    }
    return closest_hit_info;
}