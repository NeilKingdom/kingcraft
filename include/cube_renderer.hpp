#pragma once

#include "../include/cube_factory.hpp"
#include "../include/cube.hpp"

typedef struct 
{
   float x;
   float y;
   float z;
} Point;

class CubeFactory 
{
public:
   CubeFactory();
   ~CubeFactory();

   makeCube()

private:  
   static CubeFactory cubeFactory;


};
