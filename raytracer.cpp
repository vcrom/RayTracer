#include "raytracer.h"
#include "ray.h"
#include "hitablelist.h"
#include "sphere.h"
#include "camera.h"
#include <omp.h>
#include <iostream>
#include "utils.h"
#include "lambertian.h"
#include "metal.h"
#include "dielectric.h"
#include "movingsphere.h"
#include "bvh.h"
#include "sahbvh.h"
#include "constanttexture.h"
#include "checkertexture.h"
#include "noisetexture.h"
#include "diffuselight.h"
#include "gradienttexture.h"
#include "xyrect.h"
#include "xzrect.h"
#include "yzrect.h"
#include "flipnormals.h"

using uint = unsigned int;
using uchar = unsigned char;

RayTracer::RayTracer()
{

}

std::vector<uchar> RayTracer::render(int width, int height)
{
    return render(width, height, default_samples, Scene::Cornell);
}

std::vector<uchar> RayTracer::render(int width, int height, int samples,
                                     Scene scene,
                                     std::atomic<int>* rows_done,
                                     BvhKind bvh,
                                     std::optional<unsigned> seed)
{
    auto world = buildWorld(scene, bvh);
    // Re-seed before rendering: building the world consumes random numbers
    // (the midpoint BVH picks a random axis), so without this a fixed seed
    // would still render differently per BVH kind.
    if (seed)
        seedRand(*seed);
    std::cout << "Scene initialized (" << sceneName(scene) << ")" << std::endl;
    auto cam = cameraForScene(scene, width, height);
    std::cout << "Cam initialized" << std::endl;
    return render(width, height, samples, *world, cam, rows_done);
}

std::vector<uchar> RayTracer::render(int width, int height, int samples,
                                     const Hitable& world, const Camera& cam,
                                     std::atomic<int>* rows_done)
{
    auto image = renderWorld(width, height, samples, world, cam, rows_done);
    auto vect = std::vector<unsigned char> (width*height*3, 0);
    for(uint i = 0; i < image.size(); ++i)
        for(uint j = 0; j < image[0].size(); ++j)
        {
            auto idx = (i*image[0].size() + j)*3;
            auto pixel = image[i][j];
            vect[idx] = uchar(pixel.r);
            vect[idx+1] = uchar(pixel.g);
            vect[idx+2] = uchar(pixel.b);
        }
    return vect;
}

glm::vec3 RayTracer::color(const Ray& r, const Hitable& world, int depth)
{
    hit_record rec;
    if(world.hit(r, 0.01f, std::numeric_limits<float>::max(), rec))
    {
        Ray scattered;
        glm::vec3 attenuation;
        glm::vec3 emitted = rec.mat->emitted(rec.uv, rec.p);
        if(depth < max_bounces && rec.mat->scatter(r, rec, attenuation, scattered))
        {
            return emitted + attenuation*color(scattered, world, ++depth);
        }
        return emitted;
    }

    // Miss: black. A scene that needs a sky carries it as an object.
    return glm::vec3(0);

}

// Emissive sky: a huge sphere enclosing the scene, textured with the book's gradient.
std::unique_ptr<Hitable> skySphere()
{
    return std::unique_ptr<Hitable>(new Sphere(glm::vec3(0, 0, 0), 10000,
        std::shared_ptr<Material>(new DiffuseLight(std::shared_ptr<Texture>(new GradientTexture())))));
}

std::unique_ptr<Hitable> makeBvh(std::vector<std::unique_ptr<Hitable>>& list, float t0, float t1, BvhKind kind)
{
    if (kind == BvhKind::Sah)
        return std::unique_ptr<Hitable>(new SAHBVH(list, t0, t1));
    return std::unique_ptr<Hitable>(new BVH(list, t0, t1));
}

std::unique_ptr<Hitable> initScene(float t0, float t1, BvhKind kind)
{
    std::vector<std::unique_ptr<Hitable>> list;
    list.push_back(skySphere());
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(0,0,-1), 0.5, std::shared_ptr<Material>(new Lambertian(glm::vec3(0.8, 0.3, 0.3))))));
    //list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(0,0,-1), 0.5, std::shared_ptr<Material>(new Lambertian(glm::vec3(0.8, 0.3, 0.3))))));

    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(0,-100.5,-1), 100, std::shared_ptr<Material>(new Lambertian(glm::vec3(0.8, 0.8, 0))))));
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(1, 0,-1), 0.5, std::shared_ptr<Material>(new Metal(glm::vec3(0.8, 0.6, 0.2), 0.3f)))));
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(-1, 0,-1), 0.5, std::shared_ptr<Material>(new Dielectric(1.5f)))));
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(-1, 0,-1), -0.45, std::shared_ptr<Material>(new Dielectric(1.5f)))));
    std::cout << "Initializing BVH" << std::endl;
    return makeBvh(list, t0, t1, kind);
}


std::unique_ptr<Hitable> bookCover(float t0, float t1, BvhKind kind)
{
    std::vector<std::unique_ptr<Hitable>> list;
    list.push_back(skySphere());
    std::shared_ptr<Texture> tex = std::shared_ptr<Texture>(new CheckerTexture(std::shared_ptr<Texture>(new ConstantTexture(glm::vec3(1))), std::shared_ptr<Texture>(new ConstantTexture(glm::vec3(0)))));
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(0,-500,0), 500, std::shared_ptr<Material>(new Lambertian(tex)))));

    for(int a = -10; a < 10; ++a)
        for(int b = -10; b < 10; ++b)
        {
            auto choose_mat = randNum();
            glm::vec3 cent(static_cast<float>(a)*1.0f+randNum(), 0.2/*+randNum()*0.5f*/, static_cast<float>(b)*1.0f + randNum());
            if(glm::length(cent - glm::vec3(4, 0.2, 0)) > 0.9)
            {
                if(choose_mat < 0.8)
                {
                    list.push_back(std::unique_ptr<Hitable>(new MovingSphere(cent, cent + glm::vec3(0,0.2f*randNum(),0), t0, t1, 0.2, std::shared_ptr<Material>(new  Lambertian(glm::vec3(randNum()*randNum(), randNum()*randNum(), randNum()*randNum()))))));
                }
                else if(choose_mat < 0.95)
                {
                    list.push_back(std::unique_ptr<Hitable>(new Sphere(cent, 0.2, std::shared_ptr<Material>(new  Metal(glm::vec3(0.5*(1+randNum()), 0.5*(1+randNum()), 0.5*(1+randNum())), randNum())))));
                }
                else
                {
                    list.push_back(std::unique_ptr<Hitable>(new Sphere(cent, 0.2, std::shared_ptr<Material>(new  Dielectric(0.5+randNum())))));
                }
            }
        }

    //list.push_back(std::shared_ptr<Hitable>(new Sphere(glm::vec3(4,1,-2), 1.0, std::shared_ptr<Material>(new Lambertian(glm::vec3(0.4, 0.2, 0.1))))));

    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(0,1,0), 1.0, std::shared_ptr<Material>(new Dielectric(1.5)))));
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(-4,1,0), 1.0, std::shared_ptr<Material>(new Lambertian(glm::vec3(0.4, 0.2, 0.1))))));
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(4,1,0), 1.0, std::shared_ptr<Material>(new Metal(glm::vec3(0.7, 0.6, 0.5), 1.0f)))));
    std::cout << list.size() << std::endl;
    return makeBvh(list, t0, t1, kind);
}

HitableList bookCoverList(float t0, float t1)
{
    std::vector<std::shared_ptr<Hitable>> list;
    list.push_back(std::shared_ptr<Hitable>(new Sphere(glm::vec3(0,-500,0), 500, std::shared_ptr<Material>(new Lambertian(glm::vec3(0.5, 0.5, 0.5))))));

    for(int a = -10; a < 10; ++a)
        for(int b = -10; b < 10; ++b)
        {
            auto choose_mat = randNum();
            glm::vec3 cent(static_cast<float>(a)*1.0f+randNum(), 0.2/*+randNum()*0.5f*/, static_cast<float>(b)*1.0f + randNum());
            if(glm::length(cent - glm::vec3(4, 0.2, 0)) > 0.9)
            {
                if(choose_mat < 0.8)
                {
                    list.push_back(std::shared_ptr<Hitable>(new MovingSphere(cent, cent + glm::vec3(0,0.2f*randNum(),0), t0, t1, 0.2, std::shared_ptr<Material>(new  Lambertian(glm::vec3(randNum()*randNum(), randNum()*randNum(), randNum()*randNum()))))));
                }
                else if(choose_mat < 0.95)
                {
                    list.push_back(std::shared_ptr<Hitable>(new Sphere(cent, 0.2, std::shared_ptr<Material>(new  Metal(glm::vec3(0.5*(1+randNum()), 0.5*(1+randNum()), 0.5*(1+randNum())), randNum())))));
                }
                else
                {
                    list.push_back(std::shared_ptr<Hitable>(new Sphere(cent, 0.2, std::shared_ptr<Material>(new  Dielectric(0.5+randNum())))));
                }
            }
        }

    //list.push_back(std::shared_ptr<Hitable>(new Sphere(glm::vec3(4,1,-2), 1.0, std::shared_ptr<Material>(new Lambertian(glm::vec3(0.4, 0.2, 0.1))))));

    list.push_back(std::shared_ptr<Hitable>(new Sphere(glm::vec3(0,1,0), 1.0, std::shared_ptr<Material>(new Dielectric(1.5)))));
    list.push_back(std::shared_ptr<Hitable>(new Sphere(glm::vec3(-4,1,0), 1.0, std::shared_ptr<Material>(new Lambertian(glm::vec3(0.4, 0.2, 0.1))))));
    list.push_back(std::shared_ptr<Hitable>(new Sphere(glm::vec3(4,1,0), 1.0, std::shared_ptr<Material>(new Metal(glm::vec3(0.7, 0.6, 0.5), 1.0f)))));
    std::cout << list.size() << std::endl;
    return HitableList(list);
}

std::unique_ptr<Hitable> basicScene(float t0, float t1, BvhKind kind)
{
    std::vector<std::unique_ptr<Hitable>> list;
    std::shared_ptr<Texture> check_tex = std::shared_ptr<Texture>(new CheckerTexture(std::shared_ptr<Texture>(new ConstantTexture(glm::vec3(1))), std::shared_ptr<Texture>(new ConstantTexture(glm::vec3(0)))));
    std::shared_ptr<Texture> noise_text = std::shared_ptr<Texture>(new NoiseTexture(5.0f));
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(0,-500,0), 500, std::shared_ptr<Material>(new Lambertian(noise_text)))));


    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(0,1,0), 1.0, std::shared_ptr<Material>(new Dielectric(1.5)))));
    //list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(0,1,0), -0.75, std::shared_ptr<Material>(new Dielectric(1.5)))));

    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(0,1,4), 1.0, std::shared_ptr<Material>(new Lambertian(noise_text)))));
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(0,1,-4), 1.0, std::shared_ptr<Material>(new Lambertian(check_tex)))));
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(4,1,0), 1.0, std::shared_ptr<Material>(new Metal(glm::vec3(0.7, 0.6, 0.5), 1.0f)))));
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(-4,1,0), 1.0, std::shared_ptr<Material>(new Metal(glm::vec3(0, 0.2, 0.1))))));
    list.push_back(std::unique_ptr<Hitable>(new XYRect(glm::vec2(-4,4), glm::vec2(0,3), -4.0, std::shared_ptr<Material>(new Metal(glm::vec3(0.7, 0.7, 0.7), 0.95f)))));

    //lights
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(0,6,0), 3, std::shared_ptr<Material>(new DiffuseLight(glm::vec3(1.0))))));
    std::cout << list.size() << std::endl;
    return makeBvh(list, t0, t1, kind);
}

std::unique_ptr<Hitable> cornellBox(float t0, float t1, BvhKind kind)
{
    std::vector<std::unique_ptr<Hitable>> list;
    auto red = std::shared_ptr<Material>(new Lambertian(glm::vec3(0.65, 0.05, 0.05)));
    auto white = std::shared_ptr<Material>(new Lambertian(glm::vec3(0.73, 0.73, 0.73)));
    auto green = std::shared_ptr<Material>(new Lambertian(glm::vec3(0.12, 0.45, 0.15)));
    auto light = std::shared_ptr<Material>(new DiffuseLight(glm::vec3(15.0f)));

    //walls
    list.push_back(std::unique_ptr<Hitable>(new FlipNormals(std::shared_ptr<Hitable>(new YZRect(glm::vec2(0, 555), glm::vec2(0, 555), 555, green)))));//left
    list.push_back(std::unique_ptr<Hitable>(new YZRect(glm::vec2(0, 555), glm::vec2(0, 555), 0, red))); //right
    list.push_back(std::unique_ptr<Hitable>(new XZRect(glm::vec2(0, 555), glm::vec2(0, 555), 0, white))); //ground
    list.push_back(std::unique_ptr<Hitable>(new FlipNormals(std::shared_ptr<Hitable>(new XZRect(glm::vec2(0, 555), glm::vec2(0, 555), 555, white)))));//ceiling
    list.push_back(std::unique_ptr<Hitable>(new FlipNormals(std::shared_ptr<Hitable>(new XYRect(glm::vec2(0, 555), glm::vec2(0, 555), 555, white)))));//back

    //light
    list.push_back(std::unique_ptr<Hitable>(new XZRect(glm::vec2(213, 343), glm::vec2(227, 332), 554, light)));

    std::cout << list.size() << std::endl;
    return makeBvh(list, t0, t1, kind);
}

Camera initCam(int w, int h, float time0, float time1)
{
    auto pos = glm::vec3(6, 1.5, 2);
    auto look_at = glm::vec3(0, 1, 0);
    auto up = glm::vec3(0, 1, 0);
    auto fov = glm::radians(50.0f);
    auto aspect = static_cast<float>(w)/static_cast<float>(h);
    auto aperture = 0.05f;
    auto dist_to_focus = glm::length(pos - look_at);
    return Camera(pos, look_at, up, fov, aspect, aperture, dist_to_focus, time0, time1);
}

Camera initCamBasic(int w, int h, float time0, float time1)
{
    auto pos = glm::vec3(6, 3, 6);
    auto look_at = glm::vec3(0, 1, 0);
    auto up = glm::vec3(0, 1, 0);
    auto fov = glm::radians(50.0f);
    auto aspect = static_cast<float>(w)/static_cast<float>(h);
    auto aperture = 0.05f;
    auto dist_to_focus = glm::length(pos - look_at);
    return Camera(pos, look_at, up, fov, aspect, aperture, dist_to_focus, time0, time1);
}

Camera initCamCornell(int w, int h, float time0, float time1)
{
    auto pos = glm::vec3(278, 278, -800);
    auto look_at = glm::vec3(278, 278, 0);
    auto up = glm::vec3(0, 1, 0);
    auto fov = glm::radians(40.0f);
    auto aspect = static_cast<float>(w)/static_cast<float>(h);
    auto aperture = 0.00f;
    auto dist_to_focus = 10.0f;//glm::length(pos - look_at);
    return Camera(pos, look_at, up, fov, aspect, aperture, dist_to_focus, time0, time1);
}

namespace {
// Motion-blur time range shared by scene construction and cameras.
constexpr float worldTime0 = 0.0f;
constexpr float worldTime1 = 1.0f;
} // namespace

std::unique_ptr<Hitable> buildWorld(Scene scene, BvhKind kind)
{
    switch (scene) {
        case Scene::BookCover: return bookCover(worldTime0, worldTime1, kind);
        case Scene::Basic:     return basicScene(worldTime0, worldTime1, kind);
        case Scene::Simple:    return initScene(worldTime0, worldTime1, kind);
        case Scene::Cornell:
        default:               return cornellBox(worldTime0, worldTime1, kind);
    }
}

Camera cameraForScene(Scene scene, int w, int h)
{
    switch (scene) {
        case Scene::BookCover:
        case Scene::Simple:    return initCam(w, h, worldTime0, worldTime1);
        case Scene::Basic:     return initCamBasic(w, h, worldTime0, worldTime1);
        case Scene::Cornell:
        default:               return initCamCornell(w, h, worldTime0, worldTime1);
    }
}

std::vector<std::vector<glm::vec3>> RayTracer::renderWorld(int width, int height,
                                                          int samples,
                                                          const Hitable& world,
                                                          const Camera& cam,
                                                          std::atomic<int>* rows_done)
{
    auto image = std::vector<std::vector<glm::vec3>>(height, std::vector<glm::vec3>(width));

    double start_time = omp_get_wtime();
    // Parallel over rows (was collapse(2) over pixels) so progress can be
    // reported per finished row to the UI / CLI.
    // Signed loop variable: MSVC's OpenMP requires a signed integral type (C3016).
    # pragma omp parallel for schedule(dynamic)
    for(int i = 0; i < static_cast<int>(image.size()); ++i)
        for(uint j = 0; j < image[0].size(); ++j)
        {
            auto p_color = glm::vec3(0);
            for(int k = 0; k < samples; ++k)
            {
                auto v = (static_cast<float>(i)+randNum())/static_cast<float>(height);
                auto u = (static_cast<float>(j)+randNum())/static_cast<float>(width);
                auto r = cam.getRay(u, v);
                p_color += color(r, world, 1);
            }
            p_color/=static_cast<float>(samples);
            p_color = glm::pow(p_color, glm::vec3(1/2.2f));
            p_color *= 255;
            p_color = glm::clamp(p_color, 0.0f, 255.0f);
            image[image.size()-1 - i][j] = p_color;
            if (j + 1 == image[0].size() && rows_done != nullptr)
                rows_done->fetch_add(1, std::memory_order_relaxed);
        }
    double end_time = omp_get_wtime();
    std::cout << "Rendered in " << end_time-start_time << " seconds." << std::endl;
    return image;
}
