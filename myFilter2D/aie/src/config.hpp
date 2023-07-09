
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <common/xf_aie_const.hpp>

// tile dimensions are normally computed by tiler but we need to
// hardcode these values to set the graph window sizes
using DATA_TYPE = int16_t;
static constexpr int TILE_WIDTH = 64;
static constexpr int TILE_HEIGHT = 64;
static constexpr int TILE_ELEMENTS = (TILE_WIDTH * TILE_HEIGHT);
static constexpr int TILE_WINDOW_SIZE = ((TILE_ELEMENTS * sizeof(DATA_TYPE)) + xf::cv::aie::METADATA_SIZE);
//static constexpr int TILE_WINDOW_SIZE = (TILE_ELEMENTS * sizeof(DATA_TYPE));

/* Graph specific configuration */
//static constexpr int VECTORIZATION_FACTOR = 16;

#endif //__CONFIG_H_