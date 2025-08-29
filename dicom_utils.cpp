#include "dicom_utils.h"
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmdata/dctypes.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcuid.h>
#include <iostream>
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
        
        std::cout << "Reading DICOM file: " << filepath << std::endl;
        std::cout << "Dimensions: " << width << " x " << height << " (bit depth: " << depth << ")" << std::endl;
        
        // Create Volume4D with single time point (1 slice)
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
            std::cout << "Note: Converting non-standard bit depth " << depth << " to 16-bit" << std::endl;
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
            
            std::cout << "✓ Successfully read " << count << " pixel values into Volume4D" << std::endl;
            std::cout << "Min: " << min_val << ", Max: " << max_val << ", Mean: " << mean << std::endl;
        }
        
        delete image;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception while reading DICOM file: " << e.what() << std::endl;
    }
    
    return volume;
}

std::vector<int> get4DSize(const std::string& dicomFolderPath) {
    std::vector<int> dimensions = {0, 0, 0, 0}; // [xLength, yLength, zLength, tLength]
    
    try {
        // We'll get the volume size (Rows, Columns, Number of Slices) from the DICOM header.
        // To do this, we need to read one DICOM file in the folder and extract Rows (0028,0010), 
        // Columns (0028,0011), and Number of Slices (0020,1002) or count files as fallback.

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

                    // Alternative: try CardiacNumberOfImages
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