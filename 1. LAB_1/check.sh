if [ ! -f ./otp ]; then
	make otp
fi

if [ ! -f ./input.txt ]; then
	rm input.txt
fi

if [ ! -f ./input2.txt ]; then
	rm input2.txt
fi

if [ ! -f ./output.txt ]; then
	rm output.txt
fi

rm input.txt input2.txt output.txt

# max: 10 ^ 11;

base64 /dev/urandom | head -c 10000000 > input.txt

./otp -i input.txt -o output.txt -x 4212 -a 84589 -c 45989 -m 217728

./otp -i output.txt -o input2.txt -x 4212 -a 84589 -c 45989 -m 217728

if diff -s input.txt input2.txt; then
	printf 'SUCCESS\n'
else
	printf 'FAIL\n'
fi

