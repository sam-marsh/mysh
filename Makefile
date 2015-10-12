TARGET		=	mysh

CC			=	cc
CFLAGS		=	-std=c99 -Wall -Werror -pedantic

SRC_DIR		=	src
OBJ_DIR		=	obj
BIN_DIR		=	bin

SOURCES		:= $(wildcard $(SRC_DIR)/*.c)
HEADERS		:= $(wildcard $(SRC_DIR)/*.h)
OBJECTS		:= $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

$(BIN_DIR)/$(TARGET) : $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

$(OBJECTS) : $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONEY : clean
	rm -f $(OBJECTS) $(BIN_DIR)/$(TARGET)
