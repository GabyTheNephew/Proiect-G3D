#include "FlyingCube.h"
#include <glfw3.h>

FlyingCube::FlyingCube(string const& path, bool bSmoothNormals, bool gamma) : 
   Model(path, bSmoothNormals, gamma)
{
}

void FlyingCube::SetRootTransf(glm::mat4 rootTransf)
{
   _rootTransf = rootTransf;
}

void FlyingCube::Draw(Shader& shader)
{
   shader.setMat4("model", _rootTransf);
   for (unsigned int i = 0; i < meshes.size(); i++) {
      if (meshes[i].name == "Cube.002") {
         double currentFrame = glfwGetTime();
         glm::mat4 rotation = glm::rotate(_rootTransf, (float)(20.0f*currentFrame), glm::vec3(0.0f, 1.0f, 0.0f));
         shader.setMat4("model", rotation);
      }
      meshes[i].Draw(shader);
   }
}


