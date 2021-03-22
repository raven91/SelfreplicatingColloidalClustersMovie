#include "Model.hpp"
#include "Renderer.hpp"
#include "RendererForStaticTetrahedrons.hpp"

int main()
{
  Model model;
  Renderer renderer(&model);
  renderer.Start();

  return 0;
}