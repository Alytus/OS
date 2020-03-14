#define HAVE_STRUCT_TIMESPEC
#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <chrono> 
#include <vector>

using namespace std;

typedef vector<vector<int>> vec;

vec A, B, res;
chrono::microseconds runtime1 = (chrono::microseconds)0, 
runtime2 = (chrono::microseconds)0, 
runtime3 = (chrono::microseconds)0, runtime4;

void ordinaryMul() {
	int n = A.size(), m = B[0].size();
	int t = A[0].size();
	auto start = chrono::steady_clock::now();
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; ++j)
		{
			res[i][j] = 0;
			for (int k = 0; k < t; ++k)
			{
				res[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	auto end = chrono::steady_clock::now();
	runtime4 = chrono::duration_cast<chrono::microseconds>(end - start);
}


void *threadMul1(void* args) {
	auto start = chrono::steady_clock::now();
	
	for (int i = ((int*)args)[0]; i < ((int*)args)[1]; ++i)
	{
		for (int j = ((int*)args)[2]; j < ((int*)args)[3]; ++j)
		{
			for (unsigned int k = 0; k < B.size(); ++k)
			{
				res[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	auto end = chrono::steady_clock::now();
	runtime1 += chrono::duration_cast<chrono::microseconds>(end - start);
	return NULL;
}

void *threadMul2(void* args) {
	int n = A.size(), m = B[0].size();
	auto start = chrono::steady_clock::now();

	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < m; ++j)
		{
			for (int k = ((int*)args)[0]; k < ((int*)args)[1]; ++k)
			{
				res[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	auto end = chrono::steady_clock::now();
	runtime2 += chrono::duration_cast<chrono::microseconds>(end - start);
	return NULL;
}

void *threadMul3(void* args) {
	auto start = chrono::steady_clock::now();

	for (int i = ((int*)args)[0]; i < ((int*)args)[1]; ++i)
	{
		for (int j = ((int*)args)[2]; j < ((int*)args)[3]; ++j)
		{
			for (int k = ((int*)args)[4]; k < ((int*)args)[5]; ++k)
			{
				res[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	auto end = chrono::steady_clock::now();
	runtime3 += chrono::duration_cast<chrono::microseconds>(end - start);
	return NULL;
}

void printRes(ofstream& fout) {
	for (unsigned int i = 0; i < res.size(); i++)
	{
		for (unsigned int j = 0; j < res[0].size(); ++j)
		{
			fout << res[i][j] << " ";
		}
		fout << "\n";
	}
	fout << "\n";
}

void clearRes() {
	for (unsigned int i = 0; i < res.size(); i++)
	{
		for (unsigned int j = 0; j < res[0].size(); ++j)
		{
		    res[i][j]  = 0;
		}
	}
}

int main() {
	ifstream fin("input.txt");
	int num, n, t, m;

	fin >> num;
	fin >> n >> t;
	A.resize(n, vector<int>(t));
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < t; ++j)
		{
			fin >> A[i][j];
		}
	}

	fin >> t >> m;
	B.resize(t, vector<int>(m));
	for (int i = 0; i < t; ++i)
	{
		for (int j = 0; j < m; ++j)
		{
			fin >> B[i][j];
		}
	}

	res.resize(A.size(), vector<int>(B[0].size(), 0));

	vector<pthread_t> threads(num);
	int height = ceil(n / (double)num);
	int width = ceil(m / (double)num);
	int beg_i, end_i = height, beg_j, end_j = width, ind = 0;

	for (beg_i = 0; beg_i < n; beg_i += height) {
		for (beg_j = 0; beg_j < m; beg_j += width)
		{
			int args[4] = { beg_i, end_i, beg_j, end_j };
			if (ind == num) {
				ind = 0;
			}
			pthread_create(&threads[ind], NULL, threadMul1, &args);
			pthread_join(threads[ind], NULL);
			end_j += width;
			if (end_j > m || (beg_j + 1) / num == num - 1) {
				end_j = m;
			}
			++ind;
		}
		end_j = width;
		end_i += height;
		if (end_i > n || (beg_i + 1) / num == num - 1) {
			end_i = n;
		}
	}

	cout << "First algorithm runtime: " << runtime1.count() << " mcs\n";

	clearRes();

	int length = ceil(t / (double)num);
	int beg_k, end_k = length;
	ind = 0;

	for (beg_k = 0; beg_k < t; beg_k += length)
	{
		int args[2] = { beg_k, end_k };
		if (ind == num) {
			ind = 0;
		}
		pthread_create(&threads[ind], NULL, threadMul2, &args);
		pthread_join(threads[ind], NULL);
		end_k += length;
		if (end_k > t || (beg_k + 1) / num == num - 1) {
			end_k = t;
		}
		++ind;
	}

	cout << "Second algorithm runtime: " << runtime2.count() << " mcs\n";

	clearRes();

	bool cycle_k = true;

	if (num % 2 != 0) {
		height = ceil(n / (double)(num - 1));
		width = ceil(m / (double)(num - 1));
		length = ceil(t / (double)(num - 1));
	}
	else {
		height = ceil(n / (double)num);
		width = ceil(m / (double)num);
		length = ceil(t / (double)num);
	}
	end_i = height; end_j = width;
	end_k = length;
	ind = 0;
	for (beg_i = 0; beg_i < n; beg_i += height) {
		for (beg_j = 0; beg_j < m; beg_j += width)
		{
			for (beg_k = 0; beg_k < t; beg_k += length)
			{
				if (!cycle_k) {
					length = t;
				}
				int args[6] = { beg_i, end_i, beg_j, end_j, beg_k, end_k };
				if (ind == num) {
					ind = 0;
				}
				pthread_create(&threads[ind], NULL, threadMul3, &args);
				pthread_join(threads[ind], NULL);
				end_k += length;
				if (end_k > t || (beg_k + 1) / num == num - 1) {
					end_k = t;
				}
				++ind;
			}
			end_k = length;
			end_j += width;
			if (end_j > m || (beg_j + 1) / num == num - 1) {
				end_j = m;
			}
		}
		end_j = width;
		end_i += height;
		if (end_i > n || (beg_i + 1) / num == num - 1) {
			end_i = n;
			end_k = t;
			cycle_k = false;
		}
	}

	cout << "Third algorithm runtime: " << runtime3.count() << " mcs\n";

	clearRes();

	ordinaryMul();

	cout << "Ordinary multiplication runtime: " << runtime4.count() << " mcs\n";

	fin.close();
	return 0;
}