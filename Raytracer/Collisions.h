#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "collisionComponent.h"
#include <memory>
#include <vector>
using std::shared_ptr;
using std::make_shared;

class Collisions : public collisionComponent
{
    public:
        Collisions(){}
        virtual bool hit(const Ray& Ray, double tMinimum, double tMaximum, contact& rec) const override;

        //Create vector to store all of the scene objects
        std::vector<shared_ptr<collisionComponent>> sceneObjects;
};

    bool Collisions::hit(const Ray& Ray, double tMinimum, double tMaximum, contact& rec) const 
    {
      // non-permanent record temporarily stores the data of a Ray's path
      contact np_record;
      bool collided = false;
      auto closest = tMaximum;
         for (int i = 0; i < sceneObjects.size(); i++)
         {
             // Abstraction for hittable objects
             if (sceneObjects.at(i)->hit(Ray, tMinimum, closest, np_record))
             {
                collided = true;
                closest = np_record.t;
                rec = np_record;
             }
         }
        return collided;
    }

#endif