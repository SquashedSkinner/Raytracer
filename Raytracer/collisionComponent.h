#ifndef COLLISION_COMPONENT_H
#define COLLISION_COMPONENT_H
#include "Ray.h"

struct contact
{
    glm::vec3 point;
    glm::vec3 normal;
    float t;
    bool front;

    // Variables to record the sphere's colour, material, and fuzz
    glm::vec3 col_rec;
    int mat_rec;
    float fuzz_rec;

    void setFaceNormal(const Ray& r, const glm::vec3& out_normal)
    {
        // Sets the face's normal
        if (dot(r.direction, out_normal) > 0.0)
        {
            // Ray is inside the sphere
            normal = -out_normal;
            front = false;
        }
        else
        {
            // Ray is outside the sphere
            normal = out_normal;
            front = true;
        }
    }
};

class collisionComponent
{
    public:
         virtual bool hit(const Ray& r, double timeMinimum, double timeMaximum, contact& rec) const = 0;
};

#endif