# Cache-Simulation

C parameterized cache simulator that reads a memory address trace and provides statistics about cache accesses and hits.

Summary: in this assignment, I was required to write a cache simulator which reads a trace of memory references and reports 3 statistics (accesses, hits and hit rate). The cache configuration is determined by parameters that are passed as command line arguments. The command-line parameters are cache size, cache mapping (DM/FA), and cache organization (Unified/Split).

The parameters can take following values:
cache size: 128 – 4096 (has to be a power of 2)
cache mapping: dm (Direct Mapped) or fa (Fully associative) 
cache organization: uc (Unified cache) or sc (Split Cache)

To run the simulator you need to pass command-line parameters as follows:
./cache_sim 1024 dm uc
