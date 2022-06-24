BUILDDIR := build
EXECUTABLE := $(BUILDDIR)/main

.PHONY: all run clean

# far from optimal, but it works
all:
	@mkdir -p $(BUILDDIR)
	@cmake -GNinja -S . -B $(BUILDDIR)
	@cmake --build $(BUILDDIR)

run:
	@$(EXECUTABLE)

clean:
	rm -rf build/