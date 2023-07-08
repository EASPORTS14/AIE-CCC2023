
#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <adf.h>
#include "aie_kernels.h"
#include <common/xf_aie_const.hpp>
//#include "config.hpp"

using namespace adf;

using DATA_TYPE = int16_t;
static constexpr int TILE_WIDTH = 64;
static constexpr int TILE_HEIGHT = 64;
static constexpr int TILE_ELEMENTS = (TILE_WIDTH * TILE_HEIGHT);
static constexpr int TILE_WINDOW_SIZE = ((TILE_ELEMENTS * sizeof(DATA_TYPE)) + xf::cv::aie::METADATA_SIZE);

/* Graph specific configuration */
static constexpr int VECTORIZATION_FACTOR = 16;

class Filter2DGraph : public adf::graph {
    private:
        kernel f2d;
    public:
        input_plio in;
        output_plio out;
        // port<input> in;
        // port<output> out;

        Filter2DGraph() {
            // create kernel
            f2d = kernel::create(filter2D);

            // in = input_plio::create("DataIn1", plio_128_bits, "data/input.txt");
            // out = output_plio::create("DataOut1", plio_128_bits, "data/output.txt");

            //Make AIE connections
            adf::connect<window<TILE_WINDOW_SIZE> >(in.out[0], f2d.in[0]);
            adf::connect<window<TILE_WINDOW_SIZE> >(f2d.out[0], out.in[0]);
            // connect< window<TILE_WINDOW_SIZE> > net0 (in, f2d.in[0]);
            // connect< window<TILE_WINDOW_SIZE> > net1 (f2d.out[0], out);

            source(f2d) = "aie_kernels/aie_filter2D.cpp";
            runtime<ratio>(f2d) = 0.99;
    };
};

#endif //__GRAPH_H__

