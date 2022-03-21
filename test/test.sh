#!/usr/bin/bash

rm -f /tmp/foo /tmp/bar /tmp/bar2
PASSWORD_FLAG="-p OpenSesame!!!"

expect () {
	VAL=0
	if [[ $2 == "success" ]]; then
		VAL="[ $? -eq 0 ]"
	else
		VAL="[ $? -ne 0 ]"
	fi
	if $VAL;
	then
		echo $1 "- OK"
	else
		echo $1 "- Failed"
	fi
}

echo "Attention! You shouldn't see error messages below."

./grys -e $PASSWORD_FLAG test/foo.dat /tmp/bar
expect "Encrypt file" success

./grys -d $PASSWORD_FLAG /tmp/bar /tmp/foo 
expect "Decrypt file" success

diff test/foo.dat /tmp/foo
expect "Decrypted file matches" success

./grys -d -p WrongPassword /tmp/bar "" > /dev/null 2>&1
expect "Wrong password fails to decrypt" fail

shuf /tmp/bar -o /tmp/bar.shuf
./grys -d $PASSWORD_FLAG /tmp/bar.shuf /tmp/bar2 > /dev/null 2>&1
expect "Corrupted file fails to decrypt" fail

echo "Tests finished."