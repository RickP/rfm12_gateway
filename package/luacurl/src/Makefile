CFLAGS += -Wall --std=gnu99 
LDFLAGS += -shared


OBJ := luacurl.o 
LIB := -lssl -lz -lcrypto -lcurl -llua -lm -lcrypt

%.o: %.c
	$(CC) -fPIC $(CFLAGS) -c -o $@ $<

all: $(OBJ)
	$(CC) -o luacurl.so $(LDFLAGS) $(LIB) $(OBJ)

clean:
	rm -f *.o *.so

