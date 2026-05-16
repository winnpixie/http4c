all: server client

server:
	@mkdir -p test-server
	cc -O2 -x c -o test-server/server src/server/*.c

client:
	@mkdir -p test-client
	cc -O2 -x c -o test-client/client src/client/*.c
