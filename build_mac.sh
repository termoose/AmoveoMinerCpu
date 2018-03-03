gcc -Wall -O3 -c base64.cpp
gcc -Wall -O3 -c -std=c++11 AmoveoMinerCpu.cpp
g++ -std=c++11 base64.o AmoveoMinerCpu.o -o AmoveoMinerCpu -lstdc++ -lboost_system -lcrypto -lssl -lcpprest -pthread -lboost_thread-mt -lboost_chrono -L/usr/local/Cellar/openssl/1.0.2l/lib/
rm *.o
