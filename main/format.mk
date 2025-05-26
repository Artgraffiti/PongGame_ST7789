MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
CODE_STYLE_PATH = .clang-format

format:
	find $(MAKEFILE_DIR) -type f \( -name "*.h" -o -name "*.c" -o -name "*.hpp" -o -name "*.cpp" \) -exec clang-format -i --style=file:$(CODE_STYLE_PATH) {} \;