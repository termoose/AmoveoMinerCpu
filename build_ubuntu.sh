gcc -Wall -c base64.cpp
gcc -Wall -c -std=c++11 AmoveoMinerCpu.cpp
g++ -std=c++11 base64.o AmoveoMinerCpu.o -o AmoveoMinerCpu -lstdc++ -lboost_system -lcrypto -lssl -lcpprest -pthread
rm *.o