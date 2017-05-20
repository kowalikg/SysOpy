CC=gcc
LIST=queue.c semaphores_helper.c

all: clientFactory barber

barber: main.c $(LIST)
	$(CC) -Wall $^ -o $@ -lrt -pthread

clientFactory: clientFactory.c $(LIST)
	$(CC) -Wall $^ -o $@ -lrt -pthread

