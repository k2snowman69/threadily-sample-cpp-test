# Useful methods or functions
include ../shared-makefile/shared-cli.mk
include ../shared-makefile/cpp-constants.mk
# Final outputs
TARGET_NAME = threadily-sample-cpp-test.exe
# Dependent libraries
LIBRARIES_DEBUG = $(call FixPath,../threadily/debug/libthreadily.a) $(call FixPath,../threadily-sample-cpp/debug/libthreadily-sample.a)
LIBRARIES_SHIP = $(call FixPath,../threadily/ship/libthreadily.a) $(call FixPath,../threadily-sample-cpp/ship/libthreadily-sample.a)
LIBRARIES_FLAGS_DEBUG = -I$(call FixPath,../bandit) -I$(call FixPath,../threadily/debug/headers) -L$(call FixPath,../threadily/debug) -lthreadily -I$(call FixPath,../threadily-sample-cpp/debug/headers) -L$(call FixPath,../threadily-sample-cpp/debug) -lthreadily-sample
LIBRARIES_FLAGS_SHIP = -I$(call FixPath,../bandit) -I$(call FixPath,../threadily/ship/headers) -L$(call FixPath,../threadily/ship) -lthreadily -I$(call FixPath,../threadily-sample-cpp/ship/headers) -L$(call FixPath,../threadily-sample-cpp/ship) -lthreadily-sample

include ../shared-makefile/cpp-exe-rules.mk

.PHONY: test
test: $(TARGET_NAME_SHIP)
	./$(TARGET_NAME_SHIP) --reporter=spec