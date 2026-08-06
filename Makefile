FLAGS = -std=c23 \
	-Wpedantic \
	-Wall \
	-Wextra \
	-g \
	-fsanitize=address \
	-fsanitize=undefined


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
	@$(CC) $(FLAGS) $^  -o $@ 

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c Makefile
	@mkdir -p $(dir $@)
	@$(CC) $(FLAGS) -MMD -MP -c $< -o $@

-include $(DEPENDS)

.PHONY : all run clean test
all : $(TARGET)
run : $(TARGET)
	$(TARGET)

TEST_FLAGS = $(FLAGS) -fsanitize=address -fsanitize=undefined

TEST_SRC_DIR = tests/unit
TEST_OBJ_DIR = $(BUILD_DIR)/test/obj
TEST_SOURCES = $(shell find $(TEST_SRC_DIR) -name "*.c")
SOURCES_WITHOUT_MAIN = $(filter-out $(SRC_DIR)/main.c, $(SOURCES))
TEST_OBJECTS = $(TEST_SOURCES:$(TEST_SRC_DIR)/%.c=$(TEST_OBJ_DIR)/%.o) \
               $(SOURCES_WITHOUT_MAIN:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TEST_DEPENDS = $(TEST_SOURCES:$(TEST_SRC_DIR)/%.c=$(TEST_OBJ_DIR)/%.d) \
               $(SOURCES_WITHOUT_MAIN:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.d)
-include $(TEST_DEPENDS)

test : $(TEST_OBJECTS)
	@$(CC) $(TEST_FLAGS) $^  -o jloxtest
	
$(TEST_OBJ_DIR)/%.o : $(TEST_SRC_DIR)/%.c Makefile
	@mkdir -p $(dir $@)
	@$(CC) $(TEST_FLAGS) -MMD -MP -c $< -o $@

clean :
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	rm -f jloxtest
