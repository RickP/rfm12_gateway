CFLAGS += -Wall --std=gnu99


OBJ := gpio_int.o rfm12.o rfm12_server.o spi.o 
LIB := -lpthread


world: compile

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

compile: $(OBJ) 
	$(CC) -o rfm12_server $(LDFLAGS) $(LIB) $(OBJ)

clean:
	rm -f *.o *.so rfm12_server
