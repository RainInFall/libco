cd "$(dirname "$0")"
./autogen.sh
./configure --prefix=`pwd`/..
make
make install
