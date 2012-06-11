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
#include <sys/socket.h>
#include <sys/un.h>


#define SOCKETFILE	"/tmp/rfm12socket"
#define MAXMSG	512


volatile int hasdata = 0;
volatile int nodeid;
volatile int8_t payload[RF12_MAXDATA];

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

int make_named_socket (const char *filename)
{
  struct sockaddr_un name;
  int sock;
  size_t size;

  /* Create the socket.   */

  sock = socket (PF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    {
      perror ("socket");
      exit (EXIT_FAILURE);
    }

  /* Bind a name to the socket.   */

  name.sun_family = AF_FILE;
  strcpy (name.sun_path, filename);

  /* The size of the address is
     the offset of the start of the filename,
     plus its length,
     plus one for the terminating null byte.  */
  size = (offsetof (struct sockaddr_un, sun_path)
	  + strlen (name.sun_path) + 1);

  unlink(filename);

  if (bind (sock, (struct sockaddr *) &name, size) < 0)
    {
      perror ("bind");
      exit (EXIT_FAILURE);
    }


  // fcntl(sock,F_SETFL,x | O_NONBLOCK);

  return sock;
}

// Server thread
void serve(int *socket) {

    size_t size, payload_size;
    int8_t payload[RF12_MAXDATA];
    // int8_t sendBuf[RF12_MAXDATA + 2];
    int has_message = 0;


	while (1) {

		// receive input - nonblocking
		size = recv(*socket, (void *) payload, RF12_MAXDATA, MSG_NOSIGNAL | MSG_DONTWAIT);

		if (size == 0) break;
		if (size != -1) {
			payload_size = size;
			has_message = 1;
		}

		// Send message - sends it with an ack header
		if (has_message && rf12_canSend()) {
		  rf12_sendStart(RF12_ACK_REPLY, payload, payload_size);
		  has_message = 0;
		}
		// If we received a message put it on the socket
		else if (rf12_recvDone() && rf12_crc == 0) {

			// Fill sendbuffer with the header and the payload
			// memcpy(sendBuf,(void *) &rf12_hdr, 1);
			// memcpy((void *) &sendBuf[1], (void *) rf12_data, rf12_len);
			// if(send(*socket, (void *) sendBuf, rf12_len + 1, MSG_NOSIGNAL) == -1) break;

			// Send it to the socket
			if(send(*socket, (void *) rf12_data, rf12_len, MSG_NOSIGNAL) == -1) break;
		}

		// Wait 1/10th of a second to allow receiving/sending
		usleep(100000);

	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int sock;
	int s = 0;
	pthread_t pollthread, servethread;

	// Create the socket
	sock = make_named_socket(SOCKETFILE);
	listen(sock, 5);

	// Init rf12 module
	rf12_initialize(1, RF12_868MHZ, 1);

	// create pollthread
	pthread_create(&pollthread, NULL, (void *) pollRFM, NULL);

	// The main loop
	while (1) {
			// wait for socket connection
			s = accept(sock, NULL, NULL);

			// create threads and pass the socket
			pthread_create(&servethread, NULL, (void *) serve, &s);
	}

	return ret;
}
