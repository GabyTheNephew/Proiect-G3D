#pragma once
#include "Model.h"

class FlyingCube : public Model
{
public:
   FlyingCube(string const& path, bool bSmoothNormals, bool gamma = false);
   void SetRootTransf(glm::mat4 rootTransf);

   virtual void Draw(Shader& shader);

private:
   glm::mat4 _rootTransf;
};

