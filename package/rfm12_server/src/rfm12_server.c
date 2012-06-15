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
#include <stdint.h>
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

	char filePath[RF12_MAXDATA];
	int partNum, charNum, luastatus;

	struct {
	    char command[5];
	    char parameter[RF12_MAXDATA-5];
	} payload;

	char command[6];
	char parameter[RF12_MAXDATA-4];

	command[5] = '\0';
	parameter[RF12_MAXDATA-5] = '\0';

	while (1) {

		 if (rf12_recvDone() && rf12_crc == 0) {

			// ACK the message
			rf12_sendStart_simple(RF12_ACK_REPLY);

			partNum = 0;
			charNum = 0;

			memcpy(&payload, (char *) rf12_data, sizeof payload);
			memcpy(command, payload.command, sizeof payload.command);
			memcpy(parameter, payload.parameter, sizeof payload.parameter);

			memset(&filePath[0], 0, sizeof(filePath));
			strcat(filePath, "/etc/rfm12.d/");
			strcat(filePath, command);
			strcat(filePath, ".lua");

			luastatus = luaL_loadfile(L, filePath);

			if (luastatus == LUA_ERRFILE) {
				printf("File %s not found!\n", filePath);
			} else if (luastatus == LUA_ERRSYNTAX) {
				printf("Syntax error in file %s!\n", filePath);
			} else {
				luastatus = lua_pcall(L, 0, 0, 0);
				lua_getglobal(L, "process");  /* function to be called */
				lua_pushstring(L, parameter);
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
