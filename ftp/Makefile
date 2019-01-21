port := 1338
tmpdir := tmpdir

test-server: build
	./server $(port)

test-client: client
	mkdir -p $(tmpdir)
	cd $(tmpdir); ../client localhost $(port); cd ..

build: client server

client: client.c
	gcc -g client.c -o client

server: server.c
	gcc -g server.c -o server

clean:
	rm -rf $(tmpdir) pid client server testfile*

# generates 1.1GB file, creates temporary directory and gets the file inside
test: clean client server
	./server $(port) & echo $$! > pid
	[ -f testfile ] || openssl rand -out testfile -base64 792917038
	mkdir -p $(tmpdir)
	cd $(tmpdir); printf "get testfile\nquit\n" | ../client localhost $(port)
	md5sum $(tmpdir)/testfile testfile
	kill -9 `cat pid`
