BUILD := build
BUILT := $(BUILD)/built
CMAKE := cmake
MAKE := make
CMAKE_FLAGS := -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DCMAKE_INSTALL_PREFIX:PATH=$(abspath $(BUILT)) \
  -DLLVM_EXTERNAL_CLANG_SOURCE_DIR:PATH=$(abspath clang)

.PHONY: llvm

llvm:
	mkdir -p $(BUILD)/$@
	cd $(BUILD)/$@ ; $(CMAKE) $(CMAKE_FLAGS) ../../llvm
	cd $(BUILD)/$@ ; $(MAKE); $(MAKE) install

clean:
	rm -rf $(BUILD) *~
