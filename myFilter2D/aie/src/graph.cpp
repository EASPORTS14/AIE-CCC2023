
#include "graph.h"

using namespace adf;

// Graph object
Filter2DGraph filter_graph;
simulation::platform<1,1> platform("data/input.txt", "data/output.txt");
connect<> net0(platform.src[0], filter_graph.in);
connect<> net1(filter_graph.out, platform.sink[0]);

int main(void) {
  filter_graph.init();
  filter_graph.run(1);
  filter_graph.end();
  return 0;
}
