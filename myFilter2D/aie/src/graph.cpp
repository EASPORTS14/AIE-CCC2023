
#include "graph.h"

using namespace adf;

// Graph object
Filter2DGraph filter_graph;

int main(void) {
  filter_graph.init();
  filter_graph.run(1);
  filter_graph.end();
  return 0;
}
