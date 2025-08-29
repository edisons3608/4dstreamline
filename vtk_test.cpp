#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkMarchingCubes.h>
#include <vtkCamera.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkType.h>

#include <filesystem>
#include <iostream>
#include <string>

#include "Volume4D.h"
#include "dicom_utils.h"

int main() {
    std::string mag_path = "/Users/edisonsun/Documents/4Dsamples/D29/4D/mag";
    if (!std::filesystem::exists(mag_path)) {
        std::cerr << "Mag path not found: " << mag_path << std::endl;
        return 1;
    }

    Volume4D mag = DicomFolderToVolume4D(mag_path);
    if (mag.empty()) {
        std::cerr << "Failed to load magnitude volume" << std::endl;
        return 1;
    }

    std::cout << "Volume loaded successfully!" << std::endl;
    std::cout << "Dimensions: " << mag.size_x() << " x " << mag.size_y() << " x " << mag.size_z() << " x " << mag.size_t() << std::endl;

    // Create renderer
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground(0.1, 0.1, 0.1);

    // Find data range
    float min_val = mag.at(0, 0, 0, 0), max_val = mag.at(0, 0, 0, 0);
    for (std::size_t x = 0; x < mag.size_x(); x++) {
        for (std::size_t y = 0; y < mag.size_y(); y++) {
            for (std::size_t z = 0; z < mag.size_z(); z++) {
                float val = mag.at(x, y, z, 0);
                if (val < min_val) min_val = val;
                if (val > max_val) max_val = val;
            }
        }
    }
    std::cout << "Data range: " << min_val << " to " << max_val << std::endl;

    // Create 3D volume data
    vtkSmartPointer<vtkImageData> volumeData = vtkSmartPointer<vtkImageData>::New();
    volumeData->SetDimensions(mag.size_x(), mag.size_y(), mag.size_z());
    volumeData->AllocateScalars(VTK_FLOAT, 1);
    
    // Copy data to volume (using first time point)
    float* volumePtr = static_cast<float*>(volumeData->GetScalarPointer());
    for (std::size_t z = 0; z < mag.size_z(); z++) {
        for (std::size_t y = 0; y < mag.size_y(); y++) {
            for (std::size_t x = 0; x < mag.size_x(); x++) {
                int index = x + y * mag.size_x() + z * mag.size_x() * mag.size_y();
                volumePtr[index] = mag.at(x, y, z, 0);
            }
        }
    }

    // Create multiple isosurfaces at different thresholds
    // Use lower thresholds to make surfaces more visible
    float thresholds[] = {static_cast<float>(min_val + (max_val - min_val) * 0.1), 
                         static_cast<float>(min_val + (max_val - min_val) * 0.2), 
                         static_cast<float>(min_val + (max_val - min_val) * 0.3)};
    double colors[][3] = {{1.0, 0.8, 0.8}, {0.8, 1.0, 0.8}, {0.8, 0.8, 1.0}};  // More colorful
    
    for (int i = 0; i < 3; i++) {
        // Create marching cubes
        vtkSmartPointer<vtkMarchingCubes> marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
        marchingCubes->SetInputData(volumeData);
        marchingCubes->SetValue(0, thresholds[i]);
        marchingCubes->Update();
        
        // Check if surface was created
        vtkPolyData* surface = marchingCubes->GetOutput();
        if (surface->GetNumberOfPoints() == 0) {
            std::cout << "Warning: No surface created for threshold " << thresholds[i] << std::endl;
            continue;
        }
        
        std::cout << "Created isosurface at threshold " << thresholds[i] 
                  << " with " << surface->GetNumberOfPoints() << " points" << std::endl;
        
        // Create mapper
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(marchingCubes->GetOutputPort());
        
        // Create actor
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(colors[i][0], colors[i][1], colors[i][2]);
        actor->GetProperty()->SetOpacity(0.8);  // More opaque
        actor->GetProperty()->SetAmbient(0.3);  // Add ambient lighting
        actor->GetProperty()->SetDiffuse(0.7);  // Add diffuse lighting
        
        // Add actor to renderer
        renderer->AddActor(actor);
    }

    // Create render window
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(800, 600);
    renderWindow->SetWindowName("4D Volume - 3D Isosurface View");

    // Create render window interactor
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = 
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);

    // Configure interaction style for smoother, less sensitive control
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = 
        vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    renderWindowInteractor->SetInteractorStyle(style);
    
    // Set slower rotation speed
    style->SetMotionFactor(1.8);  // Reduce motion sensitivity

    // Add axes for orientation
    vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
    vtkSmartPointer<vtkOrientationMarkerWidget> widget = 
        vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    widget->SetOutlineColor(0.9300, 0.5700, 0.1300);
    widget->SetOrientationMarker(axes);
    widget->SetInteractor(renderWindowInteractor);
    widget->SetViewport(0.0, 0.0, 0.3, 0.3);
    widget->SetEnabled(1);
    widget->InteractiveOff();

    // Set up camera for better initial view
    renderer->ResetCamera();
    vtkSmartPointer<vtkCamera> camera = renderer->GetActiveCamera();
    camera->SetPosition(1, 1, 1);  // Position camera at an angle
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(0, 0, 1);

    std::cout << "Starting 3D isosurface visualization..." << std::endl;
    std::cout << "Use mouse to rotate, scroll to zoom, and right-click to pan" << std::endl;

    // Start rendering
    renderWindow->Render();
    renderWindowInteractor->Start();

    return 0;
}