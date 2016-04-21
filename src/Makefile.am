# 
# variables
#
AM_CXXFLAGS = @../build/WARNINGS
AM_LDFLAGS = -lm

if GCC
  AM_CXXFLAGS += -Wunsafe-loop-optimizations -Wzero-as-null-pointer-constant -Wuseless-cast 
endif

if DEBUG
  AM_CXXFLAGS += -g -ggdb3 -O0 -DDEBUG -fno-inline-functions 
  AM_LDFLAGS += -g
if GCC
  AM_CXXFLAGS += -fno-inline-small-functions
endif
else
  AM_CXXFLAGS += -DNDEBUG -Ofast -fomit-frame-pointer -ffast-math -mfpmath=sse -fPIC -msse -msse2 -msse3 -mssse3 -msse4 -flto
  AM_LDFLAGS += -flto
endif

if GOLD
  AM_LDFLAGS += -fuse-ld=gold
endif

#
# programs to build
#
bin_PROGRAMS = zoe
zoe_SOURCES = main.cc

.PHONY: cloc
