# AmoveoMinerCpu
Amoveo Cryptocurrency Miner for Cpu work to be used with http://AmoveoPool.com

Usage Template:

AmoveoMinerCpu.exe <WalletAddress> <Threads> <PoolUrl>
  
Threads is optional and defaults to 4.
PoolUrl is optional and defaults to http://amoveopool.com/work
  
Example Usage:  

AmoveoMinerCpu.exe BPA3r0XDT1V8W4sB14YKyuu/PgC6ujjYooVVzq1q1s5b6CAKeu9oLfmxlplcPd+34kfZ1qx+Dwe3EeoPu0SpzcI=

Example Usage with Threads and PoolUrl set:

AmoveoMinerCpu.exe BPA3r0XDT1V8W4sB14YKyuu/PgC6ujjYooVVzq1q1s5b6CAKeu9oLfmxlplcPd+34kfZ1qx+Dwe3EeoPu0SpzcI= 7 http://amoveopool.com/work

This builds with Visual Studio 2015. I think you need RestCPP and boost installed too. You can get an already built tool after you enter a valid Amoveo address here (near the bottom of the page): http://amoveopool.com/miner

A Linux build is not yet available.
