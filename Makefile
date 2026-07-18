CC      := gcc
TARGET  := republica
SRC     := $(wildcard src/*.c)
OBJ     := $(SRC:.c=.o)

CFLAGS  := -std=c11 -D_GNU_SOURCE -Wall -Wextra -Werror -pthread
LDFLAGS := -pthread

.PHONY: all debug tsan run clean

# Build padrão, otimizado.
all: CFLAGS += -O2
all: $(TARGET)

# Build de depuração com AddressSanitizer + UndefinedBehaviorSanitizer.
debug: CFLAGS += -g -O0 -fsanitize=address,undefined
debug: LDFLAGS += -fsanitize=address,undefined
debug: clean $(TARGET)

# Build com ThreadSanitizer, para validar ausência de data races.
tsan: CFLAGS += -g -O1 -fsanitize=thread
tsan: LDFLAGS += -fsanitize=thread
tsan: clean $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f src/*.o $(TARGET)
