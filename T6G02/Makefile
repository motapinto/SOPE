all:
	gcc server.c log.c  utilities.c queue.c -fpack-struct -D_REENTRANT -lpthread -lrt -lm -Wall -g -o server
	gcc user.c log.c  utilities.c queue.c -fpack-struct -D_REENTRANT -lpthread -lrt -lm -Wall -g -o user

clean:
	rm server
	rm user
