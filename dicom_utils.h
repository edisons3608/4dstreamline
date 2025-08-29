#ifndef DICOM_UTILS_H
#define DICOM_UTILS_H

#include <string>
#include <vector>
#include <filesystem>
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


/**
 * Apply VENC scaling to convert phase values to velocity values
 * 
 * @param rescaledPhase Input 4D phase volume
 * @param venc Velocity encoding value in cm/s
 * @return Volume4D containing velocity values
 */
Volume4D applyVENC(Volume4D rescaledPhase, float venc);

/**
 * Generate velocity vector field from a 4D phase volume folder
 * 
 * @param phase_path Path to folder containing DICOM files for the phase volume
 * @return Volume4D containing the velocity field
 */
Volume4D generateVelVecField(const std::string& phase_path);


/**
 * Rescale phase Volume4D using RescaleSlope and RescaleIntercept from DICOM file
 * 
 * @param dicomFolderPath Path to folder containing DICOM files to extract rescaling parameters
 * @return Rescaled Volume4D
 */
Volume4D rescalePhase(const std::string& dicomFolderPath);

#endif // DICOM_UTILS_H 