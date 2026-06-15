# ==============================================================
# mylibc — freestanding C++ standard library (static archive)
# ==============================================================
# Usage:
#   make              → Debug build  → bin/x64/Debug/libc.a
#   make CONFIG=Release
#   make clean
# ==============================================================

CONFIG   ?= Debug
PLATFORM := x64

CXX  := g++
AR   := ar rcs

CXXFLAGS := \
  -m64 -std=c++20 -masm=intel \
  -ffreestanding -fno-rtti -fno-exceptions \
  -nostdlib -nostartfiles \
  -mno-sse -mno-sse2 -mno-mmx -mno-3dnow -mno-80387 \
  -msoft-float -mno-red-zone

ifeq ($(CONFIG),Debug)
  CXXFLAGS += -g -O0
else
  CXXFLAGS += -O2
endif

INCLUDES := -Iinclude

SRCS := $(wildcard src/*.cpp)

OUTDIR  := bin/$(PLATFORM)/$(CONFIG)
TARGET  := $(OUTDIR)/libc.a
SOLNBIN := ../bin/$(PLATFORM)/$(CONFIG)

OBJS := $(patsubst src/%.cpp,$(OUTDIR)/%.o,$(SRCS))

# ---------------------------------------------------------------

.PHONY: all clean

all: $(TARGET)
	@mkdir -p $(SOLNBIN)
	cp $(TARGET) $(SOLNBIN)/libc.a
	@echo "[mylibc] done → $(TARGET)"

$(TARGET): $(OBJS)
	@mkdir -p $(OUTDIR)
	$(AR) $@ $^

$(OUTDIR)/%.o: src/%.cpp
	@mkdir -p $(OUTDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf bin
