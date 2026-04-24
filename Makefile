COMPILE_FLAGS = -std=c17 -Wpedantic -Wall -Wextra
SRC_DIR = src
INCLUDE_DIR = $(SRC_DIR)/include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = .
TARGET = $(BIN_DIR)/jlox

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPENDS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.d)

$(TARGET) : $(OBJECTS)
	@mkdir -p $(dir $@)
	@cc $(COMPILE_FLAGS) $^  -o $@ 

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c Makefile
	@mkdir -p $(dir $@)
	@cc $(COMPILE_FLAGS) -MMD -MP -c $< -I$(INCLUDE_DIR) -o $@

-include $(DEPENDS)

.PHONY : all run clean
all : $(TARGET)
run : $(TARGET)
	$(TARGET)
clean :
	rm -f $(OBJECTS) $(DEPENDS) $(TARGET)

