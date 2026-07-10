/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   SorttFile.h
 * Author: rlcancian
 *
 * Created on 20 de novembro de 2019, 22:27
 */

#ifndef SORTTFILE_H
#define SORTTFILE_H

#include <string>

/*!
 * \brief Utility class that sorts numeric values persisted in a data file.
 *
 * Used by file-based statistics implementations to support quantile/median
 * computations that require ordered samples.
 */
class SortFile {
public:
	/*! \brief Creates an unbound sorter (filename must be configured before use). */
	SortFile();
	~SortFile() = default;
public:
	/*!
	 * \brief Executes in-file sorting for the configured dataset.
	 * \return True if sorting operation completes successfully.
	 */
	bool sort();
	/*!
	 * \brief Sets the target data filename to be sorted.
	 * \param filename Path to the backing data file.
	 */
	void setDataFilename(std::string filename);
private:
	/*! \brief Recursive quicksort step over range [start, end]. */
	void run(int start, int end);
	/*! \brief Swaps values at positions \p position1 and \p position2 in the file. */
	void swap(int position1, int position2);
	/*! \brief Partitions range [start, end] and returns pivot final position. */
	int partition(int start, int end);
	/*! \brief Writes value at a given file position. */
	void addValue(double value, int position);
	/*! \brief Reads and returns value at a given file position. */
	double getValue(unsigned long position);
private:
	std::string _filename;
	double _numElements = 0;
};

#endif /* SORTTFILE_H */
