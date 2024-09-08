#include "renderer/base_renderer.hpp"

#include <iostream>
#include <string>

#include "utils/processor.hpp"
#include "thread/thread_pool.hpp"

void BaseRenderer::render(size_t spp, const std::filesystem::path& filename)
{
    size_t current_spp = 0, increase = 1;
    Film& film = camera.getFilm();
    Processor processor(film.getWidth() * film.getHeight() * spp, 1);

    std::string stem = filename.stem().string();
    std::string extension = filename.extension().string();
    std::filesystem::path newPath = filename;

    while(current_spp < spp) {   
        thread_pool.parallelFor(film.getWidth(), film.getHeight(), [&](size_t x, size_t y){
            for(int i=0; i<increase; i++) {
                film.addSample(x, y, renderPixel(glm::ivec2{x, y}));
            }
            processor.update(increase);
        });
        
        thread_pool.wait();
        current_spp += increase;

        newPath.replace_filename(stem + "-" + std::to_string(current_spp) + "spp" + extension);
        film.save(newPath);

        if(current_spp >= spp) {
            increase = spp - current_spp + increase;
        } else {
            increase = std::min<size_t>(current_spp, 32);
        }
        std::cout << current_spp << "spp has been saved to " << newPath << std::endl;
    }

    film.Clear();
}