GCRYPT_FLAGS=`pkg-config --cflags --libs libgcrypt` `pkg-config --cflags --libs gpg-error`
ARGON_FLAGS=`pkg-config --cflags --libs libargon2`

.PHONY: all test clean

all: grys

grys:
	$(CC) src/* -Iinclude/ -o grys $(GCRYPT_FLAGS) $(ARGON_FLAGS) -lpthread

test: grys
	bash test/test.sh

clean:
	rm -f grys
