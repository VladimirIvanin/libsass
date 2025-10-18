CXX ?= c++

CXXFLAGS := -I ../include/ -g -O1 -fno-omit-frame-pointer

LIBSASS_CPPSTD ?= c++11
CXXFLAGS += -std=$(LIBSASS_CPPSTD)
LDFLAGS  += -std=$(LIBSASS_CPPSTD)

test: test_shared_ptr test_util_string test_color_level4

test_shared_ptr: build/test_shared_ptr
	@ASAN_OPTIONS="symbolize=1" build/test_shared_ptr

test_util_string: build/test_util_string
	@ASAN_OPTIONS="symbolize=1" build/test_util_string

test_color_level4: build/test_color_level4
	@ASAN_OPTIONS="symbolize=1" build/test_color_level4

test_color_functions: build/test_color_functions
	@ASAN_OPTIONS="symbolize=1" build/test_color_functions

build:
	@mkdir build

build/test_shared_ptr: test_shared_ptr.cpp ../src/memory/shared_ptr.cpp | build
	$(CXX) $(CXXFLAGS) ../src/memory/allocator.cpp ../src/memory/shared_ptr.cpp -o build/test_shared_ptr test_shared_ptr.cpp

build/test_util_string: test_util_string.cpp ../src/util_string.cpp | build
	$(CXX) $(CXXFLAGS) ../src/memory/allocator.cpp ../src/util_string.cpp -o build/test_util_string test_util_string.cpp

build/test_color_level4: test_color_level4.cpp ../src/color_spaces.cpp ../lib/libsass.a | build
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o build/test_color_level4 test_color_level4.cpp ../lib/libsass.a -ldl -lpthread

build/test_color_functions: test_color_functions.cpp ../lib/libsass.a | build
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o build/test_color_functions test_color_functions.cpp ../lib/libsass.a -ldl -lpthread

clean: | build
	rm -rf build

.PHONY: test test_shared_ptr test_util_string test_color_level4 test_color_functions clean
