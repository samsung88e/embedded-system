KDIR := /home/teddy/work/yocto_r3/kernel/kernel-4.4.x
PWD := $(shell pwd)
 
obj-m += gpio_misc_driver.o
 
all:
	make -C $(KDIR) M=$(PWD) modules
 
clean:
	make -C $(KDIR) M=$(PWD) clean


# source /opt/poky/2.5.1/environment-setup-cortexa9hf-neon-poky-linux-gnueabi

# ###############################################################################
# # Build Options
# ###############################################################################
# OPTS    := -Wall -O2 -Wextra -Wcast-align -Wno-unused-parameter \
#       -Wshadow -Wwrite-strings -Wcast-qual -fno-strict-aliasing \
#       -fstrict-overflow -fsigned-char -fno-omit-frame-pointer \
#       -fno-optimize-sibling-calls

# COPTS    := $(OPTS)
# CXXOPTS    := $(OPTS) -Wnon-virtual-dtor

# CFLAGS    += $(OPTS)
# CXXFLAGS  += $(OPTS)
# AFLAGS    +=

# ARFLAGS    += crv
# LDFLAGS    += 

# LIBRARY    :=

# #########################################################################
# # Toolchain.
# #########################################################################
# INCLUDE    +=
# LIBRARY    += -lstdc++

# ################################################################################
# # Target
# #########################################################################
# CXXOBJS  :=
# COBJS  := test.o # 수정

# CFLAGS  += -g

# TARGET  = test # 수정

# ################################################################################
# # Build
# #########################################################################
# OBJS  := $(COBJS) $(CXXOBJS)

# all: $(TARGET)

# $(TARGET):  depend $(OBJS)
# 	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LIBRARY)

# install:

# clean:
# 	rm -rf $(TARGET) *.o .depend

# distclean: clean

# #########################################################################
# # Dependency
# ifeq (.depend,$(wildcard .depend))
# include .depend
# endif

# SRCS := $(COBJS:.o=.c) $(AOBJS:.o=.s) $(CXXOBJS:.o=.cpp)
# INCS := $(INCLUDE)
# depend dep:
# 	$(CC) -M $(CFLAGS) $(AFLGAS) $(CXXFLAGS) $(INCS) $(SRCS) > .depend

# %.o: %.c
# 	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

# %.o: %.s
# 	$(AS) $(AFLAGS) $(INCLUDE) -c -o $@ $<

# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<