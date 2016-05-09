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
#include "constanttexture.h"
#include "checkertexture.h"
#include "noisetexture.h"

using uint = unsigned int;
using uchar = unsigned char;

RayTracer::RayTracer()
{

}

std::vector<uchar> RayTracer::render(int width, int height)
{
    auto image = renderImage(width, height);
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
        if(depth < max_bounces && rec.mat->scatter(r, rec, attenuation, scattered))
        {
            return attenuation*color(scattered, world, ++depth);
        }
        return glm::vec3(0);
    }
    auto dir = r.dir();
    float t = 0.5f*(dir.y + 1.0);
    return glm::mix(glm::vec3(1), glm::vec3(0.5, 0.7, 1.0), t);

}

BVH initScene(float t0, float t1)
{
    std::vector<std::unique_ptr<Hitable>> list;
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(0,0,-1), 0.5, std::shared_ptr<Material>(new Lambertian(glm::vec3(0.8, 0.3, 0.3))))));
    //list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(0,0,-1), 0.5, std::shared_ptr<Material>(new Lambertian(glm::vec3(0.8, 0.3, 0.3))))));

    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(0,-100.5,-1), 100, std::shared_ptr<Material>(new Lambertian(glm::vec3(0.8, 0.8, 0))))));
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(1, 0,-1), 0.5, std::shared_ptr<Material>(new Metal(glm::vec3(0.8, 0.6, 0.2), 0.3f)))));
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(-1, 0,-1), 0.5, std::shared_ptr<Material>(new Dielectric(1.5f)))));
    list.push_back(std::unique_ptr<Hitable>(new Sphere(glm::vec3(-1, 0,-1), -0.45, std::shared_ptr<Material>(new Dielectric(1.5f)))));
    std::cout << "Initializing BVH" << std::endl;
    return BVH(list, t0, t1);
}


BVH bookCover(float t0, float t1)
{
    std::vector<std::unique_ptr<Hitable>> list;
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
    return BVH(list, t0, t1);
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

BVH basicScene(float t0, float t1)
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
    std::cout << list.size() << std::endl;
    return BVH(list, t0, t1);
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
    auto pos = glm::vec3(6, 2, 6);
    auto look_at = glm::vec3(0, 1, 0);
    auto up = glm::vec3(0, 1, 0);
    auto fov = glm::radians(50.0f);
    auto aspect = static_cast<float>(w)/static_cast<float>(h);
    auto aperture = 0.05f;
    auto dist_to_focus = glm::length(pos - look_at);
    return Camera(pos, look_at, up, fov, aspect, aperture, dist_to_focus, time0, time1);
}

std::vector<std::vector<glm::vec3>> RayTracer::renderImage(int width, int height)
{
    auto image = std::vector<std::vector<glm::vec3>>(height, std::vector<glm::vec3>(width));

    //auto scene = bookCover(time0, time1);
    //auto scene = bookCoverList(time0, time1);
    //auto scene = initScene(time0, time1);
    auto scene = basicScene(time0, time1);

    std::cout << "Scene initialized " << std::endl;
    //auto cam = initCam(width, height, time0, time1);
    auto cam = initCamBasic(width, height, time0, time1);
    std::cout << "Cam initialized" << std::endl;

    double start_time = omp_get_wtime();
    # pragma omp parallel for collapse(2) schedule(dynamic)
    for(uint i = 0; i < image.size(); ++i)
        for(uint j = 0; j < image[0].size(); ++j)
        {
            auto p_color = glm::vec3(0);
            for(int k = 0; k < num_rays; ++k)
            {
                auto v = (static_cast<float>(i)+randNum())/static_cast<float>(height);
                auto u = (static_cast<float>(j)+randNum())/static_cast<float>(width);
                auto r = cam.getRay(u, v);
                p_color += color(r, scene, 1);
            }
            p_color/=static_cast<float>(num_rays);
            p_color = glm::pow(p_color, glm::vec3(1/2.2f));
            p_color *= 255;
            image[image.size()-1 - i][j] = p_color;
        }
    double end_time = omp_get_wtime();
    std::cout << "Rendered in " << end_time-start_time << " seconds." << std::endl;
    return image;
}
