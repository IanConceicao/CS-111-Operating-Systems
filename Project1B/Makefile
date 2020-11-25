#NAME: Ian Conceicao
#EMAIL: IanCon234@gmail.com
#ID: 505153981


default:
	gcc lab1b-server.c -Wall -Wextra -o lab1b-server -lz
	gcc lab1b-client.c -Wall -Wextra -o lab1b-client -lz

client:
	gcc lab1b-client.c -Wall -Wextra -o lab1b-client -lz

server:
	gcc lab1b-server.c -Wall -Wextra -o lab1b-server -lz

clean:
	rm -f lab1b-server lab1b-client lab1b-505153981.tar.gz

dist:
	tar -czvf lab1b-505153981.tar.gz lab1b-server.c lab1b-client.c README Makefile
	