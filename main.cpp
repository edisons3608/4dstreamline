#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcuid.h>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include "dicom_utils.h"
#include "Volume4D.h"

// VTK includes for visualization
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkGlyph3D.h>
#include <vtkArrowSource.h>
#include <vtkPoints.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkCamera.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkType.h>

// VTK includes for streamline visualization
#include <vtkStreamTracer.h>
#include <vtkLineSource.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkLookupTable.h>
#include <vtkColorTransferFunction.h>

int main() {
    
    std::string x_phase_path = "/Users/edisonsun/Documents/4Dsamples/2150/4D/1";
    std::string y_phase_path = "/Users/edisonsun/Documents/4Dsamples/2150/4D/2";
    std::string z_phase_path = "/Users/edisonsun/Documents/4Dsamples/2150/4D/3";
    std::string mag_path = "/Users/edisonsun/Documents/4Dsamples/2150/4D/mag";

    Volume4D x_vel = generateVelVecField(x_phase_path);
    Volume4D y_vel = generateVelVecField(y_phase_path);
    Volume4D z_vel = generateVelVecField(z_phase_path);
    Volume4D mag = DicomFolderToVolume4D(mag_path);

    // Check if velocity volumes were loaded successfully
    if (x_vel.empty() || y_vel.empty() || z_vel.empty()) {
        std::cerr << "Error: Failed to load velocity volumes" << std::endl;
        return 1;
    }
    
    std::cout << "Velocity volumes loaded successfully!" << std::endl;
    std::cout << "X velocity dimensions: " << x_vel.size_x() << " x " << x_vel.size_y() << " x " << x_vel.size_z() << " x " << x_vel.size_t() << std::endl;
    std::cout << "Y velocity dimensions: " << y_vel.size_x() << " x " << y_vel.size_y() << " x " << y_vel.size_z() << " x " << y_vel.size_t() << std::endl;
    std::cout << "Z velocity dimensions: " << z_vel.size_x() << " x " << z_vel.size_y() << " x " << z_vel.size_z() << " x " << x_vel.size_t() << std::endl;

    // Create VTK visualization for velocity field
    std::cout << "\nCreating VTK streamline visualization..." << std::endl;
    
    // Create renderer
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground(0.1, 0.1, 0.1);

    // Create 3D volume data for velocity field
    vtkSmartPointer<vtkImageData> velocityField = vtkSmartPointer<vtkImageData>::New();
    velocityField->SetDimensions(x_vel.size_x(), x_vel.size_y(), x_vel.size_z());
    velocityField->AllocateScalars(VTK_FLOAT, 3); // 3 components for velocity vectors
    
    // Copy velocity data to VTK volume (using first time point)
    int timePoint = 0;
    for (std::size_t z = 0; z < x_vel.size_z(); z++) {
        for (std::size_t y = 0; y < x_vel.size_y(); y++) {
            for (std::size_t x = 0; x < x_vel.size_x(); x++) {
                float vx = x_vel.at(x, y, z, timePoint);
                float vy = y_vel.at(x, y, z, timePoint);
                float vz = z_vel.at(x, y, z, timePoint);
                
                // Store velocity components
                float* ptr = static_cast<float*>(velocityField->GetScalarPointer(x, y, z));
                ptr[0] = vx;
                ptr[1] = vy;
                ptr[2] = vz;
            }
        }
    }
    
    // Set velocity field as vector field
    vtkSmartPointer<vtkFloatArray> vectors = vtkSmartPointer<vtkFloatArray>::New();
    vectors->SetNumberOfComponents(3);
    vectors->SetName("Velocity");
    
    // Copy velocity data to vector array
    for (std::size_t z = 0; z < x_vel.size_z(); z++) {
        for (std::size_t y = 0; y < x_vel.size_y(); y++) {
            for (std::size_t x = 0; x < x_vel.size_x(); x++) {
                float vx = x_vel.at(x, y, z, timePoint);
                float vy = y_vel.at(x, y, z, timePoint);
                float vz = z_vel.at(x, y, z, timePoint);
                vectors->InsertNextTuple3(vx, vy, vz);
            }
        }
    }
    
    velocityField->GetPointData()->SetVectors(vectors);
    
    // Create seed points for streamlines
    vtkSmartPointer<vtkPoints> seedPoints = vtkSmartPointer<vtkPoints>::New();
    
    // Threshold parameters for aorta flow
    float minVelocityThreshold = 1.0; // cm/s - minimum velocity to show
    float maxVelocityThreshold = 300.0; // cm/s - maximum velocity to show
    
    // ROI parameters for aorta
    float roiCenterX = 0.0;
    float roiCenterY = 0.0;
    float roiCenterZ = 0.0;
    float roiRadius = 0.8;
    
    std::cout << "Creating seed points for streamlines..." << std::endl;
    std::cout << "Velocity thresholds: " << minVelocityThreshold << " to " << maxVelocityThreshold << " cm/s" << std::endl;
    std::cout << "ROI radius: " << roiRadius << " (normalized coordinates)" << std::endl;
    
    // Sample points for seed generation (use every nth point to avoid overcrowding)
    int sampleRate = 8; // Adjust this to control density of streamlines
    
    for (std::size_t z = 0; z < x_vel.size_z(); z += sampleRate) {
        for (std::size_t y = 0; y < x_vel.size_y(); y += sampleRate) {
            for (std::size_t x = 0; x < x_vel.size_x(); x += sampleRate) {
                // Get velocity components
                float vx = x_vel.at(x, y, z, timePoint);
                float vy = y_vel.at(x, y, z, timePoint);
                float vz = z_vel.at(x, y, z, timePoint);
                
                // Calculate velocity magnitude
                float magnitude = sqrt(vx*vx + vy*vy + vz*vz);
                
                // Check velocity magnitude threshold (aorta flow range)
                if (magnitude >= minVelocityThreshold && magnitude <= maxVelocityThreshold) {
                    // Normalize coordinates to [-1, 1] range
                    float nx = (2.0f * x / x_vel.size_x()) - 1.0f;
                    float ny = (2.0f * y / x_vel.size_y()) - 1.0f;
                    float nz = (2.0f * z / x_vel.size_z()) - 1.0f;
                    
                    // Check if point is within ROI (simple spherical region)
                    float distFromCenter = sqrt((nx - roiCenterX)*(nx - roiCenterX) + 
                                              (ny - roiCenterY)*(ny - roiCenterY) + 
                                              (nz - roiCenterZ)*(nz - roiCenterZ));
                    
                    if (distFromCenter <= roiRadius) {
                        seedPoints->InsertNextPoint(nx, ny, nz);
                    }
                }
            }
        }
    }
    
    std::cout << "Created " << seedPoints->GetNumberOfPoints() << " seed points for streamlines" << std::endl;
    
    // Create polydata for seed points
    vtkSmartPointer<vtkPolyData> seedData = vtkSmartPointer<vtkPolyData>::New();
    seedData->SetPoints(seedPoints);
    
    // Create streamline tracer
    vtkSmartPointer<vtkStreamTracer> streamTracer = vtkSmartPointer<vtkStreamTracer>::New();
    streamTracer->SetInputData(velocityField);
    streamTracer->SetSourceData(seedData);
    streamTracer->SetMaximumPropagation(100); // Maximum steps for streamline
    streamTracer->SetIntegrationStepUnit(2); // Cell length units
    streamTracer->SetInitialIntegrationStep(0.1); // Initial step size
    streamTracer->SetMinimumIntegrationStep(0.01); // Minimum step size
    streamTracer->SetMaximumIntegrationStep(0.5); // Maximum step size
    streamTracer->SetIntegrationDirection(0); // Forward integration
    streamTracer->SetComputeVorticity(1); // Compute vorticity for coloring
    streamTracer->Update();
    
    std::cout << "Generated " << streamTracer->GetOutput()->GetNumberOfLines() << " streamlines" << std::endl;
    
    // Create color lookup table for velocity magnitude
    vtkSmartPointer<vtkLookupTable> colorTable = vtkSmartPointer<vtkLookupTable>::New();
    colorTable->SetNumberOfColors(256);
    colorTable->SetHueRange(0.667, 0.0); // Blue to Red (low to high velocity)
    colorTable->SetSaturationRange(1.0, 1.0);
    colorTable->SetValueRange(1.0, 1.0);
    colorTable->SetAlphaRange(0.8, 1.0);
    colorTable->Build();
    
    // Create mapper for streamlines
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(streamTracer->GetOutputPort());
    mapper->SetScalarModeToUsePointFieldData();
    mapper->SelectColorArray("Vorticity");
    mapper->SetScalarRange(minVelocityThreshold, maxVelocityThreshold);
    mapper->SetLookupTable(colorTable);
    
    // Create actor for streamlines
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetLineWidth(2.0); // Make streamlines thicker
    actor->GetProperty()->SetOpacity(0.9);
    
    // Add actor to renderer
    renderer->AddActor(actor);

    // Create render window
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(1000, 800);
    renderWindow->SetWindowName("4D Aorta Streamline Visualization");

    // Create render window interactor
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = 
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);

    // Configure interaction style
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = 
        vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    renderWindowInteractor->SetInteractorStyle(style);
    style->SetMotionFactor(1.5);

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
    camera->SetPosition(2, 2, 2);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(0, 0, 1);

    std::cout << "Starting aorta streamline visualization..." << std::endl;
    std::cout << "Use mouse to rotate, scroll to zoom, and right-click to pan" << std::endl;
    std::cout << "Colored streamlines represent aorta flow patterns" << std::endl;
    std::cout << "Color indicates velocity magnitude (Blue=low, Red=high)" << std::endl;
    std::cout << "Velocity range: " << minVelocityThreshold << " to " << maxVelocityThreshold << " cm/s" << std::endl;

    // Start rendering
    renderWindow->Render();
    renderWindowInteractor->Start();

    return 0;
}

