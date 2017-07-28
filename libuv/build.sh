cd "$(dirname "$0")"
./autogen.sh
./configure  --disable-shared  --enable-shared=no --prefix=`pwd`/..
make
make install
