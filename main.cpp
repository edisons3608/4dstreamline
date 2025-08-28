#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <iostream>
#include <string>
#include <filesystem>

int main() {
    std::string dicomFolderPath = "/Users/edisonsun/Documents/4Dsamples/D29/4D/1";
    
    // Iterate through all files in the folder
    for (const auto& entry : std::filesystem::directory_iterator(dicomFolderPath)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().string();
            
            // Try to load as DICOM file
            DcmFileFormat fileformat;
            OFCondition status = fileformat.loadFile(filename.c_str());
            
            if (status.good()) {
                std::cout << "Successfully loaded: " << filename << std::endl;
                
                // Access dataset
                DcmDataset *dataset = fileformat.getDataset();
                
                // Example: Read patient name
                OFString patientName;
                if (dataset->findAndGetOFString(DCM_PatientName, patientName).good()) {
                    std::cout << "Patient Name: " << patientName << std::endl;
                }
            }
        }
    }
    
    return 0;
}

