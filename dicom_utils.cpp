#include "dicom_utils.h"
#include "Volume4D.h"
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmdata/dctypes.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcuid.h>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

/**
 * Read DICOM file and return pixel values as a Volume4D slice
 * 
 * @param filepath Path to the DICOM file
 * @return Volume4D containing the slice (empty if failed)
 */
Volume4D readDicomToVolume4D(const std::string& filepath) {
    Volume4D volume;
    
    try {
        // Load DICOM file
        DicomImage* image = new DicomImage(filepath.c_str());
        
        if (image == nullptr || image->getStatus() != EIS_Normal) {
            std::cerr << "Error: Could not load DICOM file: " << filepath << std::endl;
            delete image;
            return volume;
        }
        
        // Get image dimensions
        int width = image->getWidth();
        int height = image->getHeight();
        int depth = image->getDepth();
        
        //std::cout << "Reading DICOM file: " << filepath << std::endl;
        //std::cout << "Dimensions: " << width << " x " << height << " (bit depth: " << depth << ")" << std::endl;
        

        volume.resize(width, height, 1, 1);
        
        // Get pixel data
        const void* pixelData = image->getOutputData(depth);
        if (pixelData == nullptr) {
            std::cerr << "Error: Could not get pixel data from DICOM file" << std::endl;
            delete image;
            return volume;
        }
        
        // Convert pixel data based on bit depth and store in Volume4D
        // Handle non-standard bit depths by treating them as 16-bit
        int effectiveDepth = depth;
        if (depth != 8 && depth != 16 && depth != 32) {
            //std::cout << "Note: Converting non-standard bit depth " << depth << " to 16-bit" << std::endl;
            effectiveDepth = 16;
        }
        
        switch (effectiveDepth) {
            case 8: {
                const Uint8* data = static_cast<const Uint8*>(pixelData);
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        int index = y * width + x;
                        volume.at(x, y, 0, 0) = static_cast<float>(data[index]);
                    }
                }
                break;
            }
            case 16: {
                const Uint16* data = static_cast<const Uint16*>(pixelData);
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        int index = y * width + x;
                        volume.at(x, y, 0, 0) = static_cast<float>(data[index]);
                    }
                }
                break;
            }
            case 32: {
                const Uint32* data = static_cast<const Uint32*>(pixelData);
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        int index = y * width + x;
                        volume.at(x, y, 0, 0) = static_cast<float>(data[index]);
                    }
                }
                break;
            }
            default:
                std::cerr << "Error: Unsupported bit depth: " << depth << std::endl;
                delete image;
                return volume;
        }
        
        // Print some statistics
        if (!volume.empty()) {
            float min_val = volume.at(0, 0, 0, 0);
            float max_val = volume.at(0, 0, 0, 0);
            float sum = 0.0f;
            int count = 0;
            
            for (std::size_t x = 0; x < volume.size_x(); x++) {
                for (std::size_t y = 0; y < volume.size_y(); y++) {
                    float val = volume.at(x, y, 0, 0);
                    min_val = std::min(min_val, val);
                    max_val = std::max(max_val, val);
                    sum += val;
                    count++;
                }
            }
            float mean = sum / count;
            
            //std::cout << "✓ Successfully read " << count << " pixel values into Volume4D" << std::endl;
            //std::cout << "Min: " << min_val << ", Max: " << max_val << ", Mean: " << mean << std::endl;
        }
        
        delete image;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception while reading DICOM file: " << e.what() << std::endl;
    }
    
    return volume;
}

Volume4D DicomFolderToVolume4D(const std::string& dicomFolderPath) {
    Volume4D volume;
    // Get filepaths for all files in dicomFolderPath
    std::vector<std::string> dicomFilePaths;

    std::vector<int> dimensions = get4DSize(dicomFolderPath);

    int slices = dimensions[2]*dimensions[3];

    volume.resize(dimensions[0], dimensions[1], dimensions[2], dimensions[3]);
    
    // First, collect all file paths
    for (const auto& entry : std::filesystem::directory_iterator(dicomFolderPath)) {
        if (entry.is_regular_file()) {
            dicomFilePaths.push_back(entry.path().string());
        }
    }
    
    // Sort the file paths after collecting all of them
    std::sort(dicomFilePaths.begin(), dicomFilePaths.end());
    
    
    // Now iterate through the sorted paths
    int t = 0;
    int z = 0;
    for (size_t i = 0; i < dicomFilePaths.size(); i++) {

        //std::cout << "i: " << i << std::endl;
        //std::cout << "Filepath: " << dicomFilePaths[i] << std::endl;
        Volume4D slice = readDicomToVolume4D(dicomFilePaths[i]);
        
        for (int x = 0; x < dimensions[0]; x++) {
            for (int y = 0; y < dimensions[1]; y++) {
                volume.at(x, y, z, t) = slice.at(x, y, 0, 0);
            }
        }

        t = i / dimensions[2];
        z = i % dimensions[2]; 
    }

    //std::cout << "Slices: " << slices << std::endl;
    return volume;
}

std::vector<int> get4DSize(const std::string& dicomFolderPath) {
    std::vector<int> dimensions = {0, 0, 0, 0}; // [xLength, yLength, zLength, tLength]
    
    try {


        int xLength = 0, yLength = 0, zLength = 0, tLength = 0, numSlices = 0;
        bool gotVolumeInfo = false;

        // Find the first DICOM file in the folder to read header info
        for (const auto& entry : std::filesystem::directory_iterator(dicomFolderPath)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().string();
                DcmFileFormat fileformat;
                OFCondition status = fileformat.loadFile(filename.c_str());
                if (status.good()) {
                    DcmDataset *dataset = fileformat.getDataset();
                    
                    // Get Rows (0028,0010)
                    Uint16 tempRows;
                    if (dataset->findAndGetUint16(DCM_Rows, tempRows).good()) {
                        yLength = tempRows;
                    }
                    
                    // Get Columns (0028,0011)
                    Uint16 tempCols;
                    if (dataset->findAndGetUint16(DCM_Columns, tempCols).good()) {
                        xLength = tempCols;
                    }

                    // chekced w/ MATLAB dicominfo - should be in this elem
                    if (tLength == 0) {
                        // Try to read CardiacNumberOfImages as string first (since it's stored as string)
                        OFString tempCardiacImagesStr;
                        if (dataset->findAndGetOFString(DcmTag(0x0018, 0x1090), tempCardiacImagesStr).good()) {
                            // Convert string to integer
                            tLength = std::stoi(tempCardiacImagesStr.c_str());
                            std::cout << "Found CardiacNumberOfImages: " << tLength << std::endl;
                        } else {
                            std::cout << "CardiacNumberOfImages not found. Status: " << status.text() << std::endl;
                        }
                    }

                    gotVolumeInfo = true;
                    break;
                }
            }
        }

        // If NumberOfSlices tag is not present, count the number of DICOM files in the folder
        if (gotVolumeInfo && zLength == 0) {
            numSlices = 0;
            for (const auto& entry : std::filesystem::directory_iterator(dicomFolderPath)) {
                if (entry.is_regular_file()) {
                    numSlices++;
                }
            }
        }

        // Analyze file structure to understand 4D organization
        std::cout << "\nAnalyzing file structure..." << std::endl;
        std::cout << "Total files found: " << numSlices << std::endl;

        if (gotVolumeInfo) {
            zLength = numSlices/tLength;
            std::cout << "Volume size: " << xLength << " x " << yLength << " x " << zLength;
            if (tLength > 0) {
                std::cout << " x " << tLength << " (4D with temporal dimension)";
            }
            std::cout << std::endl;
            
            dimensions[0] = xLength;
            dimensions[1] = yLength;
            dimensions[2] = zLength;
            dimensions[3] = tLength;
        } else {
            std::cout << "Could not determine volume size from DICOM headers." << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Exception while getting 4D size: " << e.what() << std::endl;
    }
    
    return dimensions;
}

Volume4D generateVelVecField(const std::string& phase_path){
    std::cout << "phase_path: " << phase_path << std::endl;
    float venc = 1.70;


    Volume4D rescale = rescalePhase(phase_path);



    Volume4D vel = applyVENC(rescale, venc);


    return vel;

}
Volume4D applyVENC(Volume4D rescaledPhase, float venc){
    Volume4D velocityField = rescaledPhase;
    
    // Convert phase to velocity: velocity = (phase / π) × VENC
    const float PI = 3.14159265359f;
    
    for (std::size_t x = 0; x < velocityField.size_x(); x++) {
        for (std::size_t y = 0; y < velocityField.size_y(); y++) {
            for (std::size_t z = 0; z < velocityField.size_z(); z++) {
                for (std::size_t t = 0; t < velocityField.size_t(); t++) {
                    float phaseValue = velocityField.at(x, y, z, t);
                    float velocityValue = (phaseValue / PI) * venc;
                    velocityField.at(x, y, z, t) = velocityValue;
                }
            }
        }
    }
    
    return velocityField;
}
Volume4D rescalePhase(const std::string& dicomFolderPath) {
    Volume4D volume = DicomFolderToVolume4D(dicomFolderPath);
    
    // Find the first DICOM file in the folder to extract rescaling parameters
    std::string firstDicomFile;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(dicomFolderPath)) {
            if (entry.is_regular_file()) {
                firstDicomFile = entry.path().string();
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error accessing folder: " << e.what() << std::endl;
        return volume;
    }
    
    if (firstDicomFile.empty()) {
        std::cerr << "No DICOM files found in folder: " << dicomFolderPath << std::endl;
        return volume;
    }
    
    DcmFileFormat fileformat;
    if (fileformat.loadFile(firstDicomFile.c_str()).bad()) {
        std::cerr << "Error loading DICOM file: " << firstDicomFile << std::endl;
        return volume;
    }
    
    DcmDataset *dataset = fileformat.getDataset();
    
    Float64 rescaleSlope = 1.0;
    Float64 rescaleIntercept = 0.0;
    
    // Try to get rescaling parameters, use defaults if not found
    if (dataset->findAndGetFloat64(DcmTag(0x0028, 0x1053), rescaleSlope).bad()) {
        rescaleSlope = 1.0;
    }
    if (dataset->findAndGetFloat64(DcmTag(0x0028, 0x1052), rescaleIntercept).bad()) {
        rescaleIntercept = 0.0;
    }
    // for now
    rescaleSlope = 1.0;
    rescaleIntercept = 0.0;
    std::cout << "rescaleSlope: " << rescaleSlope << std::endl;
    std::cout << "rescaleIntercept: " << rescaleIntercept << std::endl;
    for (std::size_t x = 0; x < volume.size_x(); x++) {
        for (std::size_t y = 0; y < volume.size_y(); y++) {
            for (std::size_t z = 0; z < volume.size_z(); z++) {
                for (std::size_t t = 0; t < volume.size_t(); t++) {
                    float originalValue = volume.at(x, y, z, t);
                    float rescaledValue = originalValue * rescaleSlope + rescaleIntercept;
                    volume.at(x, y, z, t) = rescaledValue;
                }
            }
        }
    }
    
    return volume;
}
