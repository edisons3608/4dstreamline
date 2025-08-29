#ifndef DICOM_UTILS_H
#define DICOM_UTILS_H

#include <string>
#include <vector>
#include "Volume4D.h"

/**
 * Read DICOM file and return pixel values as a Volume4D slice
 * 
 * @param filepath Path to the DICOM file
 * @return Volume4D containing the slice (empty if failed)
 */
Volume4D readDicomToVolume4D(const std::string& filepath);

/**
 * Read DICOM files from a folder and return a Volume4D object
 * 
 * @param dicomFolderPath Path to the folder containing DICOM files
 * @return Volume4D containing the 4D volume (empty if failed)
 */
Volume4D DicomFolderToVolume4D(const std::string& dicomFolderPath);

/**
 * Get 4D volume dimensions from a folder containing DICOM files
 * 
 * @param dicomFolderPath Path to the folder containing DICOM files
 * @return Vector with 4 elements: [xLength, yLength, zLength, tLength]
 */
std::vector<int> get4DSize(const std::string& dicomFolderPath);

#endif // DICOM_UTILS_H 