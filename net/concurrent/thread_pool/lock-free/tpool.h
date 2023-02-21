/**
 *    author:  mikezheng
 *    created: 01-08-2022 14:08:37
 *    source: https://github.com/xhjcehust/LFTPool
**/

#pragma once
#include <iostream>

using namespace std;

void *tpool_init(string path);

int tpool_inc_threads(void* pool, int num_inc);

void tpool_dec_threads(void* pool, int num_dec);

int tpool_add_work(void* pool, void* (*routine)(void*), void* arg);

void tpool_destroy(void* pool, int finish);
