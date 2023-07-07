
PLATFORM := xilinx_vck5000_gen4x8_qdma_2_202220_1
TARGET := hw

XPFM = $(shell platforminfo -p $(PLATFORM) --json="file")
XSA = $(strip $(patsubst %.xpfm, % , $(shell basename $(PLATFORM))))
#VISION_LIB = $(shell readlink -f /aie_kernels/imgproc/vision/L1/include/aie/)

# OUTPUT PRODUCTS 
BUILD_DIR = build.$(XSA).$(TARGET)
WORK_DIR = work
SRC_DIR = $(shell readlink -f src/)
DATA_DIR = $(shell readlink -f data/)
CONSTRAINTS_DIR = $(shell readlink -f constraints/)

# DEPENDENCIES for make aie
GRAPH_CPP := $(SRC_DIR)/graph.cpp
DEPS := $(GRAPH_CPP)
DEPS += $(SRC_DIR)/graph.h
DEPS += $(SRC_DIR)/config.hpp
DEPS += $(SRC_DIR)/aie_kernels.h
DEPS += $(SRC_DIR)/aie_kernels/aie_filter2D.cpp
#DEPS += $(SRC_DIR)/aie_kernels/xf_aie_const.hpp


AIE_FLAGS = --platform=$(XPFM)
#AIE_FLAGS += -include=$(shell readlink -f /aie_kernels/include/aie/)
AIE_FLAGS += -include=$(SRC_DIR)/aie_kernels/include/aie/

all: $(BUILD_DIR)/libadf.a

$(BUILD_DIR)/libadf.a: $(DEPS)
	@mkdir -p $(BUILD_DIR);
	cd $(BUILD_DIR); \
	aiecompiler -v --target=$(TARGET) \
		--stacksize=2000 \
		-include="$(XILINX_VITIS)/aietools/include" \
		-include="$(SRC_DIR)"  \
		-include="$(DATA_DIR)" \
		$(AIE_FLAGS) \
		$(GRAPH_CPP) \
		-workdir=$(WORK_DIR) 2>&1 | tee aiecompiler.log

clean:
	rm -rf $(BUILD_DIR)

aieemu:
	cd $(BUILD_DIR); \
	aiesimulator --pkg-dir=$(WORK_DIR) --i=..

x86sim:
	cd $(BUILD_DIR); \
	x86simulator --pkg-dir=$(WORK_DIR) --i=..