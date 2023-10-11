#ifndef BENCHMARK_H
#define BENCHMARK_H

class Benchmark {
public:
	Benchmark();
	~Benchmark();
	struct BenchmarkInfo {
		int _nucleos;
		int _memoria;
	};

    static void getBenchmarkInfo(BenchmarkInfo *info);
};

#endif