#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcuid.h>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "dicom_utils.h"

int main() {
    // Load a single DICOM file using the new function
    std::string dicomFilePath = "/Users/edisonsun/Documents/4Dsamples/D29/4D/1/000001.dcm";
    std::cout << "Loading DICOM file: " << dicomFilePath << std::endl;
    
    Volume4D dicomSlice = readDicomToVolume4D(dicomFilePath);
    
    if (!dicomSlice.empty()) {
        std::cout << "Successfully loaded DICOM slice!" << std::endl;
        std::cout << "Volume4D dimensions: " << dicomSlice.size_x() << " x " 
                  << dicomSlice.size_y() << " x " << dicomSlice.size_z() 
                  << " x " << dicomSlice.size_t() << std::endl;
        
        // Access a sample pixel value
        float sampleValue = dicomSlice.at(0, 0, 0, 0);
        std::cout << "Sample pixel value at (0,0,0,0): " << sampleValue << std::endl;
    } else {
        std::cout << "Failed to load DICOM file!" << std::endl;
    }
    
    std::cout << "\n--- Original code continues below ---\n" << std::endl;
    
    std::string x_phase_path = "/Users/edisonsun/Documents/4Dsamples/D29/4D/1";
    std::string y_phase_path = "/Users/edisonsun/Documents/4Dsamples/D29/4D/2";
    std::string z_phase_path = "/Users/edisonsun/Documents/4Dsamples/D29/4D/3";
    std::string mag_path = "/Users/edisonsun/Documents/4Dsamples/D29/4D/mag";
    
    // Get 4D volume dimensions using the utility function
    std::vector<int> dimensions = get4DSize(x_phase_path);
    int xLength = dimensions[0];
    int yLength = dimensions[1];
    int zLength = dimensions[2];
    int tLength = dimensions[3];
    
    // Display the extracted dimensions
    std::cout << "\nExtracted dimensions from get4DSize function:" << std::endl;
    std::cout << "xLength (Columns): " << xLength << std::endl;
    std::cout << "yLength (Rows): " << yLength << std::endl;
    std::cout << "zLength (Slices): " << zLength << std::endl;
    std::cout << "tLength (Time points): " << tLength << std::endl;

    Volume4D x_phase = DicomFolderToVolume4D(x_phase_path);
    Volume4D y_phase = DicomFolderToVolume4D(y_phase_path);
    Volume4D z_phase = DicomFolderToVolume4D(z_phase_path);
    Volume4D mag = DicomFolderToVolume4D(mag_path);

    
    
    

    return 0;
}

