# AmoveoMinerCpu

Amoveo Cryptocurrency Miner for Cpu work to be used with [AmoveoPool.com](http://AmoveoPool.com)


## Windows

### Releases

   [Latest pre-built releases are here](https://github.com/Mandelhoff/AmoveoMinerCpu/releases)


### Run
Usage Template:
```
AmoveoMinerCpu.exe {WalletAddress} {Threads} {PoolUrl}
```
* Threads is optional and defaults to 4.
* PoolUrl is optional and defaults to http://amoveopool.com/work
    
Example Usage:  
```
AmoveoMinerCpu.exe BPA3r0XDT1V8W4sB14YKyuu/PgC6ujjYooVVzq1q1s5b6CAKeu9oLfmxlplcPd+34kfZ1qx+Dwe3EeoPu0SpzcI=
```

Example Usage with Threads and PoolUrl set:
```
AmoveoMinerCpu.exe BPA3r0XDT1V8W4sB14YKyuu/PgC6ujjYooVVzq1q1s5b6CAKeu9oLfmxlplcPd+34kfZ1qx+Dwe3EeoPu0SpzcI= 7 http://amoveopool.com/work
```

### Build
The Windows releases are built with Visual Studio 2015 with RestCPP, boost, and openSSL.




## Ubuntu/Linux

### Dependencies
```
   sudo apt-get install libcpprest-dev libncurses5-dev libssl-dev unixodbc-dev g++ git
```

### Build
```
git clone https://github.com/Mandelhoff/AmoveoMinerCpu.git
cd AmoveoMinerCpu
sh build_ubuntu.sh
```

### Run
```
./AmoveoMinerCpu {WalletAddress} {Threads} {PoolUrl}
```
* Threads is optional and defaults to 4.
* PoolUrl is optional and defaults to http://amoveopool.com/work





