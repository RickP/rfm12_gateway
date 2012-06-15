/*
 * rfm12_server.c
 *
 *  Created on: 22.05.2012
 *      Author: rick
 */

#include "rfm12.h"
#include "gpio_int.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <sys/socket.h>
#include <sys/un.h>


volatile int8_t payload[RF12_MAXDATA];

lua_State *L;

/*
 * This is the thread for fetching/pushing data to the rf module
 * the rfm12 lib is thread safe
 *
 * @TODO: Use poll() or interrupts instead of polling the irq gpio
 */
void pollRFM() {
	gpio_init(0);

	while (1) {
		if (gpio_poll()) {
			rf12_communicate();
		} else {
			usleep(400);
		}
	}
}

// Server thread
void serve() {

	char data_parts[2][RF12_MAXDATA+1];
	char filePath[RF12_MAXDATA];
	int partNum, charNum, i, luastatus;

	while (1) {

		 if (rf12_recvDone() && rf12_crc == 0 && rf12_len > 0) {

			// ACK the message
			rf12_sendStart_simple(RF12_ACK_REPLY);

			partNum = 0;
			charNum = 0;

			for (i=0; i<2; i++) {
				data_parts[i][0] = '\0';
			}

			for (i=0; i<rf12_len-1; i++) {
			    if( (char) rf12_data[i] == '|' && (char) rf12_data[i+1] == '|')
			    {
			    	data_parts[partNum++][charNum] = '\0';
			    	charNum = 0;
			    	i++;
			    } else {
			    	data_parts[partNum][charNum++] = rf12_data[i];
			    }
			}
			data_parts[partNum][charNum] = '\0';

			memset(&filePath[0], 0, sizeof(filePath));
			strcat(filePath, "/etc/rfm12.d/");
			strcat(filePath, data_parts[0]);
			strcat(filePath, ".lua");

			luastatus = luaL_loadfile(L, filePath) || lua_pcall(L, 0, 0, 0);

			if (luastatus) {
			  printf("File %s not found!", filePath);
			  puts("");
			} else {
				lua_getglobal(L, "process");  /* function to be called */
				lua_pushstring(L, data_parts[1]);
				lua_pcall(L, 1, 1, 0);
				rf12_sendStart(RF12_ACK_REPLY, lua_tostring(L,-1), lua_strlen(L,-1));
				lua_pop(L, -1);
			}
		}

		// Wait 1/10th of a second to allow receiving/sending
		usleep(100000);
	}
}


int main(int argc, char *argv[])
{
	int ret = 0;
	pthread_t pollthread, servethread;

	// Init lua
	L = luaL_newstate();
	luaL_openlibs(L);

	// Init rf12 module
	rf12_initialize(1, RF12_868MHZ, 1);

	// create pollthread
	pthread_create(&pollthread, NULL, (void *) pollRFM, NULL);

	// create threads and pass the socket
	pthread_create(&servethread, NULL, (void *) serve, NULL);

	pthread_join(servethread, 0);
	pthread_join(pollthread, 0);

	lua_close(L);

	return ret;
}
