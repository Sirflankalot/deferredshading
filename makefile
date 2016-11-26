PROJECT_NAME := deferredlighting

CC        := gcc
CXX       := g++

WARNINGS  := -Wall -Wextra 
FULLWARN  := -Wall -Wextra -Wpedantic -Wconversion
STD       := -std=c++14 
OPTIMIZE  := -Ofast
DEBUG     := 
DEFINES   := 
INCLUDES  := -I/usr/include/SOIL
LINK      := 
LINK      += -lSOIL -lGLEW -lGLU -lGL -lSDL2 -pthread

MODULES   := 

SRC_DIR   := src $(addprefix src/,$(MODULES))
BUILD_DIR := obj $(addprefix obj/,$(MODULES))
TARGET_DIR:= bin

SRC       := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))
OBJ       := $(patsubst src/%.cpp,obj/%.o,$(SRC))

.PHONY: all debug profile warn sanitize asm package

all: checkdirs $(TARGET_DIR)/$(PROJECT_NAME)

debug: DEBUG = -g -DDEBUG
debug: OPTIMIZE = -O0
debug: checkdirs $(TARGET_DIR)/$(PROJECT_NAME)

profile: DEBUG = -pg
profile: checkdirs $(TARGET_DIR)/$(PROJECT_NAME)

warn: WARNINGS = $(FULLWARN)
warn: checkdirs $(TARGET_DIR)/$(PROJECT_NAME)

sanitize: DEBUG = -g -fsanitize=address
sanitize: checkdirs $(TARGET_DIR)/$(PROJECT_NAME)

asm: DEBUG = -S -masm=intel
asm: checkdirs $(OBJ)

vpath %.cpp $(SRC_DIR)

define make-goal
$1/%.o: %.cpp
	@echo $(CXX) $$<
	@$$(CXX) $$(WARNINGS) $$(STD) $$(OPTIMIZE) $$(DEBUG) $$(DEFINES) $$(INCLUDES) -c $$< -o $$@

$1/%-sse.o: %-sse.cpp
	@echo $(CXX) $$< -msse4.1
	@$$(CXX) $$(WARNINGS) $$(STD) $$(OPTIMIZE) -msse4.1 $$(DEBUG) $$(DEFINES) $$(INCLUDES) -c $$< -o $$@

$1/%-sse2.o: %-sse2.cpp
	@echo $(CXX) $$< -msse2
	@$$(CXX) $$(WARNINGS) $$(STD) $$(OPTIMIZE) -msse2 $$(DEBUG) $$(DEFINES) $$(INCLUDES) -c $$< -o $$@

$1/%-sse3.o: %-sse3.cpp
	@echo $(CXX) $$< -mssse3
	@$$(CXX) $$(WARNINGS) $$(STD) $$(OPTIMIZE) -mssse3 $$(DEBUG) $$(DEFINES) $$(INCLUDES) -c $$< -o $$@

$1/%-sse4.o: %-sse4.cpp
	@echo $(CXX) $$< -msse4.1
	@$$(CXX) $$(WARNINGS) $$(STD) $$(OPTIMIZE) -msse4.1 $$(DEBUG) $$(DEFINES) $$(INCLUDES) -c $$< -o $$@

$1/%-sse42.o: %-sse42.cpp
	@echo $(CXX) $$< -msse4.2
	@$$(CXX) $$(WARNINGS) $$(STD) $$(OPTIMIZE) -msse4.2 $$(DEBUG) $$(DEFINES) $$(INCLUDES) -c $$< -o $$@

$1/%-avx.o: %-avx.cpp
	@echo $(CXX) $$< -mavx
	@$$(CXX) $$(WARNINGS) $$(STD) $$(OPTIMIZE) -mavx $$(DEBUG) $$(DEFINES) $$(INCLUDES) -c $$< -o $$@

$1/%-avx2.o: %-avx2.cpp
	@echo $(CXX) $$< -mavx2 -mfma
	@$$(CXX) $$(WARNINGS) $$(STD) $$(OPTIMIZE) -mavx2 -mfma $$(DEBUG) $$(DEFINES) $$(INCLUDES) -c $$< -o $$@
endef



$(TARGET_DIR)/$(PROJECT_NAME): $(OBJ)
	@echo Linking $@
	@$(CXX) $(DEBUG) $(OPTIMIZE) $^ -o $@ $(LINK)

checkdirs: $(BUILD_DIR) $(TARGET_DIR)

$(BUILD_DIR):
	@mkdir -p $@

$(TARGET_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(TARGET_DIR)/*
	@rm -rf obj/*
	@rm -rf $(TARGET_DIR)/$(PROJECT_NAME)
	@rm -f $(TARGET_DIR)/$(PROJECT_NAME)

$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))