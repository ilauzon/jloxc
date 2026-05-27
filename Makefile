COMPILE_FLAGS = \
	-std=c17 \
	-Wpedantic \
	-Wall \
	-Wextra \
	-g \
	-fsanitize=address
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = .
TARGET = $(BIN_DIR)/jlox
CC = gcc

SOURCES = $(shell find src -name "*.c")
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPENDS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.d)

$(TARGET) : $(OBJECTS)
	@mkdir -p $(dir $@)
	@$(CC) $(COMPILE_FLAGS) $^  -o $@ 

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c Makefile
	@mkdir -p $(dir $@)
	@$(CC) $(COMPILE_FLAGS) -MMD -MP -c $< -o $@

-include $(DEPENDS)

.PHONY : all run clean
all : $(TARGET)
run : $(TARGET)
	$(TARGET)
clean :
	rm -f $(OBJECTS) $(DEPENDS) $(TARGET)

