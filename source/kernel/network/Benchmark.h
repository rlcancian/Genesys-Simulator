/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Benchmark.h
 * Author: José Luiz de Souza
 * Email: joseloolo@hotmail.com
 *
 * Created on 28 de Maio de 2022, 23:30
 */

#ifndef BENCHMARK_H
#define BENCHMARK_H

class Benchmark {
public:
    struct Machine_Info {
		int _nucleos;
		int _memoria;
        int _benchmark_1;
	};
public:
    static void getMachineInfo(Machine_Info *info);
    static void getBenchmark1(Machine_Info *info);
};

#endif