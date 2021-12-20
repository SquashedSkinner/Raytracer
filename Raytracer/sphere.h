#ifndef SPHERE_H
#define SPHERE_H
#include "collisionComponent.h"

class Sphere : public collisionComponent 
{
    public:
        // Establish the constituents of the spheres
      Sphere(glm::vec3 mid, float r, glm::vec3 col, int mat, float fuzz)
      {
        centre = mid;
        colour = col;
        // Whether it is metallic or diffuse
        material = mat;
        // Size of the sphere
        radius = r;
        // "fuzz" of a material (how it diffuses light)
        sphereFuzz = fuzz;
      };

      virtual bool hit(const Ray& r, double timeMin, double timeMax, contact& rec) const override;

   public:
     glm::vec3 centre;
     glm::vec3 colour;
     int material;
     float radius;
     float sphereFuzz;    
};

bool Sphere::hit(const Ray& r, double timeMin, double timeMax, contact& rec) const 
{
    // Calculating the Ray-sphere Intersections
    // Any pixel that hits a small sphere placed at negative 1 on the z-axis
    float x = dot(r.direction, r.direction);
    float y = dot(r.origin - centre, r.direction);
    float z = dot(r.origin - centre, r.origin - centre) - (radius * radius);
    float discrimVal = (y * y) - (x * z);
    if (discrimVal < 0)
    {
        return false;
    }   
    float rsIntersects = (-y - sqrt(discrimVal)) / x;
    if (rsIntersects < timeMin || timeMax < rsIntersects) 
    {
        rsIntersects = (-y + sqrt(discrimVal)) / x;
        if (rsIntersects < timeMin || timeMax < rsIntersects)
        {
            return false;
        }
    }
    rec.t = rsIntersects;
    rec.point = r.point(rec.t);
    glm::vec3 out_normal = (rec.point - centre) / radius;
    rec.setFaceNormal(r, out_normal);
    rec.col_rec = colour;
    rec.fuzz_rec = sphereFuzz;
    rec.mat_rec = material;
    
    return true;
}

#endif