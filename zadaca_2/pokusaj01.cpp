#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"
using namespace std; 


struct Object;
struct Light;
typedef std::vector<Object*> Objects;
typedef std::vector<Light> Lights;
typedef std::vector<Vec3f> Image;

struct Light {
    Light(const Vec3f &p, const float &l) : position(p), intensity(l) {}
    Vec3f position;
    float intensity;
};
struct Material {
    Vec2f albedo; // difuzni i spekularni koeficijenti refleksije
    Vec3f diffuse_color;
    float specular_exponent;

    Material(const Vec2f &a, const Vec3f &color, const float &coef) : albedo(a), diffuse_color(color), specular_exponent(coef) {}
    Material() : albedo(Vec2f(1, 0)), diffuse_color(), specular_exponent(1.f) {}
};
struct Object
{
    Material material;
    virtual bool ray_intersect(const Vec3f &p, const Vec3f &d, float &t) const = 0;
    virtual Vec3f normal(const Vec3f &p) const = 0;
};


//sfera 
struct Sphere : Object
{
    Vec3f c; // centar
    float r; // radius
    
    Sphere(const Vec3f &c, const float &r, const Material &m) : c(c), r(r)
    {
        Object::material = m;
    }
    
    bool ray_intersect(const Vec3f &p, const Vec3f &d, float &t) const
    {
        Vec3f v = c - p; // vektor izmedju izvora zrake i centra
        
        if (d * v < 0) // skalarni produkt
        {
            // sfera se nalazi iza zrake
            return false;
        }
        else
        {
            // izracunaj projekciju
            Vec3f pc = p + d * ((d * v)/d.norm());
            if ((c - pc)*(c - pc) > r*r)
            {
                // nema sjeciste
                return false;                
            }
            else
            {
                float dist = sqrt(r*r - (c - pc) * (c - pc));
                
                if (v*v > r*r) // izvor pogleda izvan sfere
                {
                    t = (pc - p).norm() - dist;
                }
                else // izvor pogleda unutar sfere
                {
                    t = (pc - p).norm() + dist;                    
                }
                
                return true;
            }
        }
    }
    
    Vec3f normal(const Vec3f &p) const
    {
        return (p - c).normalize();        
    }
};







//cuboid 
struct Cuboid : Object
{

}; 




//cylinder 
struct Cylinder : Object
{
    Vec3f centar; 
    float r; 
    float h; 


    Cylinder (const Vec3f &centar, const float &r, const float &h, const Material &m) : centar(centar) , r(r), h(h)
    {
        Object::material = m;
    }

    bool ray_intersect(const Vec3f &p, const Vec3f &d, float &t) const
    {
        Vec3f v = centar - p; // vektor izmedju izvora zrake i centra
        
        if (d * v < 0) // skalarni produkt
        {
            // cilindar se nalazi iza zrake
            return false;
        }
        else
        {   // izracunaj proekciju 
            float a = (d.x * d.x) + (d.z * d.z);
            float b = 2*(d.x*(p.x-centar.x) + d.z*(p.z-centar.z));
            float c = (p.x - centar.x) * (p.x - centar.x) + (p.z - centar.z) * (p.z - centar.z) - (r*r);

            float diskriminanta = b*b - 4*(a * c);

            if (diskriminanta < 0)
            {
                // nema sjeciste
                return false;                
            }
            else
            {   float t1 = ((-b) - sqrt(diskriminanta))/(2*a);
                float t2 = ((-b) + sqrt(diskriminanta))/(2*a);
    
                if (t1>t2) 
                    t = t2;
                else 
                    t = t1;
    
                float r = p.y + t*d.y;
    
                if ((r >= centar.y) and (r <= centar.y + h))
                    return t;  
                else 
                    return false;
                
            }
        }
    }
    
    Vec3f normal(const Vec3f &p) const
    {   
        Vec3f n = (p - centar).normalize();   
        n.y=0;
        return n;   
    }
       
};

















// funkcija koja koristimo za crtanje scene
bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const Objects &objs, Vec3f &hit,  Material &material , Vec3f &N)
{
    float dist = std::numeric_limits<float>::max();
    float obj_dist = std::numeric_limits<float>::max();
    
    for (auto obj : objs)
    {
        if (obj->ray_intersect(orig, dir, obj_dist) && obj_dist < dist)
        {
            dist = obj_dist;
            hit = orig + dir * obj_dist;
            N = obj -> normal(hit); 
            material = obj -> material;             
        }
    }
    
    // provjeri je li sjeci??te predaleko 
    return dist < 1000;
}








// funkcija koja vraca udaljenost sjecista pravca i sfere
Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const Objects &objs, const Lights &lights)
{
    Vec3f hit_point;
    Vec3f hit_normal; // normala na povrsinu
    Material hit_material;
    
    if (!scene_intersect(orig, dir, objs, hit_point, hit_material, hit_normal))
    {
        return Vec3f(0.7, 0.9, 0.7); // vrati boju pozadine
    }
    else
    {
        float diffuse_light_intensity = 0;
        float specular_light_intensity = 0;

        for(auto light : lights)
        {
            Vec3f light_dir = (light.position - hit_point).normalize(); // smjer svjetla
            float light_dist = (light.position - hit_point).norm(); // udaljenost do svjetla
						
            // sjene
            Vec3f shadow_orig;
            Vec3f shadow_hit_point;
            Vec3f shadow_hit_normal;
            Material shadow_material;
            
            // epsilon pomak od tocke sjecista
            if (light_dir * hit_normal < 0)
            {
                    shadow_orig = hit_point - hit_normal * 0.001;
            }
            else
            {
                    shadow_orig = hit_point + hit_normal * 0.001;
            }
            
            if (scene_intersect(shadow_orig, light_dir, objs, shadow_hit_point, shadow_material, shadow_hit_normal) && (shadow_hit_point - shadow_orig).norm() < light_dist)
            {
                    continue;
            }

            // sjencanje
            // lambertov model
            diffuse_light_intensity += light.intensity * std::max(0.f,light_dir * hit_normal); 
            
            // blinn-phongov model
            // smjer pogleda
            Vec3f view_dir = (orig - hit_point).normalize();
            // poluvektor
            Vec3f half_vec = (view_dir+light_dir).normalize();    
            specular_light_intensity += light.intensity * powf(std::max(0.f,half_vec * hit_normal), hit_material.specular_exponent); 
        }
        return hit_material.diffuse_color * hit_material.albedo[0] * diffuse_light_intensity  // diffuse dio
               + Vec3f(1,1,1) * hit_material.albedo[1] * specular_light_intensity;            // specular dio
    }
}











// funkcija za iscrtavanje
void render(const Objects objs, Lights &lights)
{
     // velicina slike
    const int width = 1024;
    const int height = 768;
    const int fov = 3.1415926 / 2.0;
    
    // spremnik za sliku
    Image buffer(width * height);
    
    // nacrtaj u sliku
    for (size_t j = 0; j < height; ++j)
    {
        for (size_t i = 0; i < width; ++i)
        {
            // po??alji zraku u svaki piksel
            float x =  (2 * (i + 0.5)/(float)width  - 1) * tan(fov/2.) * width/(float)height;
            float y = -(2 * (j + 0.5)/(float)height - 1) * tan(fov/2.);
            
            // definiraj smjer
            Vec3f dir = Vec3f(x, y, -1).normalize();
            
            buffer[i + j*width] = cast_ray (Vec3f(0, 0, 0), dir, objs, lights );
        }            
    }
    
    
    // spremanje slike u vanjsku datoteku
    std::ofstream ofs;
    ofs.open("./cylinder.ppm", std::ofstream::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (size_t i = 0; i < height * width; ++i)
    {
        for (size_t j = 0; j < 3; ++j)
        {
            ofs << (unsigned char)(255.f * std::max(0.f, std::min(1.f, buffer[i][j])));
        }            
    }
    // zatvori datoteku
    ofs.close();

}





int main()
{
     // definiranje materijala
    Material red = Material(Vec2f(0.6,0.3), Vec3f(1, 0, 0), 60);
    Material green = Material(Vec2f(0.6,0.3), Vec3f(0, 0.5, 0), 60);
    Material blue = Material(Vec2f(0.9,0.1), Vec3f(0, 0, 1), 10);
    Material gray = Material(Vec2f(0.9,0.1), Vec3f(0.5, 0.5, 0.5), 10);
    
    // definiranje sfera
    Sphere s1(Vec3f(2,    -1,   -20), 2, blue);


    //definiranje cuboid 
    //Cuboid podloga();    
    //Cuboid c1(); 


    //definiranje cylinder 
    Cylinder cylinder(Vec3f(-6, -3, -20), 2, 4, green);


    // definiraj objekte u sceni, popis objekata u sceni
    Objects objs = { &s1 , &cylinder  };

    
    // definiraj svjetla
    Light l1 = Light(Vec3f(-20, 20, 20), 1.5);
    Light l2 = Light(Vec3f(20, 30, 20), 1.8);
    Lights lights = { l1, l2 };
    
    render(objs, lights);
    return 0;
}






























