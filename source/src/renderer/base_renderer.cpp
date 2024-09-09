#include "renderer/base_renderer.hpp"

#include <iostream>
#include <string>

#include "utils/processor.hpp"
#include "utils/profile.hpp"
#include "thread/thread_pool.hpp"

void BaseRenderer::render(size_t spp, const std::filesystem::path& filename)
{
    PROFILE("Render " + std::to_string(spp) + "spp " + filename.string())

    size_t current_spp = 0, increase = 1;
    Film& film = camera.getFilm();
    Processor processor(film.getWidth() * film.getHeight() * spp, 20);

    while(current_spp < spp) {   
        thread_pool.parallelFor(film.getWidth(), film.getHeight(), [&](size_t x, size_t y){
            for(int i=0; i<increase; i++) {
                film.addSample(x, y, renderPixel({x, y}));
            }
            processor.update(increase);
        });
        thread_pool.wait();

        current_spp += increase;

        increase = std::min<size_t>(current_spp, 32);
        if(current_spp + increase > spp){
            increase = spp - current_spp;
        }
        
        film.save(filename);
        std::cout << current_spp << "spp " << "has been saved to " << filename << std::endl;
    }
    film.Clear();
}