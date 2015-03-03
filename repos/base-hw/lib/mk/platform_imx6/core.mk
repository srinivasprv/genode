#
# \brief  Build config for core 
# \author Praveen Srinivas (IIT Madras, India)
# \date   2012-10-24
#

# add include paths
INC_DIR += $(REP_DIR)/src/core/include/spec/imx6
INC_DIR += $(REP_DIR)/src/core/include/spec/cortex_a9
INC_DIR += $(REP_DIR)/src/core/include/spec/arm_gic

# add C++ sources
SRC_CC += spec/imx6/platform_support.cc
SRC_CC += platform_services.cc
#SRC_CC += spec/cortex_a9/pic.cc
SRC_CC += spec/arm_gic/pic.cc
#SRC_CC += cpu.cc

# include less specific configuration
include $(REP_DIR)/lib/mk/cortex_a9/core.inc
