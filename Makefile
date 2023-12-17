CC = gcc
CFLAGS = -Wall -Wextra
DEBUG_FLAGS = -DDEBUG
OBJDIR = ./obj

# Source files
SERVER_SRC = server.c lib/inih-r57/ini.c lib/mongoose-7.12/mongoose.c
ENGINE_SRC = engine.c lib/inih-r57/ini.c

# Object files
SERVER_OBJ = $(SERVER_SRC:%.c=$(OBJDIR)/%.o)
ENGINE_OBJ = $(ENGINE_SRC:%.c=$(OBJDIR)/%.o)

# Executable names
SERVER_EXEC = server
ENGINE_EXEC = engine
DEBUG_SERVER_EXEC = server_debug

# Default target
all: $(SERVER_EXEC) $(ENGINE_EXEC)

# Compile and link the server
$(SERVER_EXEC): $(SERVER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

# Debug build for the server
server-debug: CFLAGS += $(DEBUG_FLAGS)
server-debug: $(SERVER_OBJ)
	$(CC) $(CFLAGS) $^ -o $(DEBUG_SERVER_EXEC)

# Compile and link the engine
$(ENGINE_EXEC): $(ENGINE_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to make object files
$(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(SERVER_OBJ) $(ENGINE_OBJ) $(SERVER_EXEC) $(ENGINE_EXEC) $(DEBUG_SERVER_EXEC)
	rm -rf $(OBJDIR)

.PHONY: all clean server-debug
