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

int main() {
    
    std::string x_phase_path = "/Users/edisonsun/Documents/4Dsamples/D29/4D/1";
    std::string y_phase_path = "/Users/edisonsun/Documents/4Dsamples/D29/4D/2";
    std::string z_phase_path = "/Users/edisonsun/Documents/4Dsamples/D29/4D/3";
    std::string mag_path = "/Users/edisonsun/Documents/4Dsamples/D29/4D/mag";

    Volume4D x_vel = generateVelVecField(x_phase_path);
    Volume4D y_vel = generateVelVecField(y_phase_path);
    Volume4D z_vel = generateVelVecField(z_phase_path);

    // Get 4D volume dimensions using the utility function
    /*std::vector<int> dimensions = get4DSize(x_phase_path);
    int xLength = dimensions[0];
    int yLength = dimensions[1];
    int zLength = dimensions[2];
    int tLength = dimensions[3];
    */
    
    // Check if velocity volumes were loaded successfully
    if (x_vel.empty() || y_vel.empty() || z_vel.empty()) {
        std::cerr << "Error: Failed to load velocity volumes" << std::endl;
        return 1;
    }
    
    std::cout << "Velocity volumes loaded successfully!" << std::endl;
    std::cout << "X velocity dimensions: " << x_vel.size_x() << " x " << x_vel.size_y() << " x " << x_vel.size_z() << " x " << x_vel.size_t() << std::endl;
    std::cout << "Y velocity dimensions: " << y_vel.size_x() << " x " << y_vel.size_y() << " x " << y_vel.size_z() << " x " << y_vel.size_t() << std::endl;
    std::cout << "Z velocity dimensions: " << z_vel.size_x() << " x " << z_vel.size_y() << " x " << z_vel.size_z() << " x " << z_vel.size_t() << std::endl;

    // Create VTK visualization for velocity field
    std::cout << "\nCreating VTK velocity field visualization..." << std::endl;
    
    // Create renderer
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground(0.1, 0.1, 0.1);

    // Create points and vectors for the velocity field
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkFloatArray> vectors = vtkSmartPointer<vtkFloatArray>::New();
    vectors->SetNumberOfComponents(3);
    vectors->SetName("Velocity");

    // Sample the velocity field (use every nth point to avoid overcrowding)
    int sampleRate = 4; // Adjust this to control density of arrows
    int timePoint = 0; // Use first time point for now
    
    // Threshold parameters for aorta flow
    float minVelocityThreshold = 1000.0; // cm/s - minimum velocity to show (reduced from 5.0)
    float maxVelocityThreshold = 4000.0; // cm/s - maximum velocity to show (increased from 150.0)
    
    // Simple ROI parameters for aorta (adjust these based on your data)
    // Assuming aorta is roughly in the center of the volume
    float roiCenterX = 0.0; // Center of normalized coordinates
    float roiCenterY = 0.0;
    float roiCenterZ = 0.0;
    float roiRadius = 0.8; // Radius of ROI (increased from 0.6 to cover more of the volume)
    
    std::cout << "Sampling velocity field with sample rate: " << sampleRate << std::endl;
    std::cout << "Velocity thresholds: " << minVelocityThreshold << " to " << maxVelocityThreshold << " cm/s" << std::endl;
    std::cout << "ROI radius: " << roiRadius << " (normalized coordinates)" << std::endl;
    
    for (std::size_t z = 0; z < x_vel.size_z(); z += sampleRate) {
        for (std::size_t y = 0; y < x_vel.size_y(); y += sampleRate) {
            for (std::size_t x = 0; x < x_vel.size_x(); x += sampleRate) {
                // Get velocity components
                float vx = x_vel.at(x, y, z, timePoint);
                float vy = y_vel.at(x, y, z, timePoint);
                float vz = z_vel.at(x, y, z, timePoint);
                
                // Calculate velocity magnitude
                float magnitude = sqrt(vx*vx + vy*vy + vz*vz);
                std::cout << "magnitude: " << magnitude << std::endl;
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
                        points->InsertNextPoint(nx, ny, nz);
                        
                        // Store normalized velocity vector for consistent arrow sizes
                        if (magnitude > 0) {
                            vectors->InsertNextTuple3(vx/magnitude, vy/magnitude, vz/magnitude);
                        } else {
                            vectors->InsertNextTuple3(0, 0, 0);
                        }
                    }
                }
            }
        }
    }
    
    std::cout << "Created " << points->GetNumberOfPoints() << " velocity vectors" << std::endl;

    // Create polydata
    vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints(points);
    polydata->GetPointData()->SetVectors(vectors);

    // Create arrow source for vector representation
    vtkSmartPointer<vtkArrowSource> arrowSource = vtkSmartPointer<vtkArrowSource>::New();
    arrowSource->SetTipLength(0.2);
    arrowSource->SetTipRadius(0.1);
    arrowSource->SetShaftRadius(0.03);

    // Create glyph3D to place arrows at each point
    vtkSmartPointer<vtkGlyph3D> glyph3D = vtkSmartPointer<vtkGlyph3D>::New();
    glyph3D->SetInputData(polydata);
    glyph3D->SetSourceConnection(arrowSource->GetOutputPort());
    glyph3D->SetVectorModeToUseVector();
    glyph3D->SetScaleModeToScaleByVector();
    glyph3D->SetScaleFactor(0.3); // Adjust arrow size
    glyph3D->Update();

    // Create mapper
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(glyph3D->GetOutputPort());

    // Create actor
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0.0, 1.0, 0.0); // Green arrows
    actor->GetProperty()->SetOpacity(0.8);

    // Add actor to renderer
    renderer->AddActor(actor);

    // Create render window
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(1000, 800);
    renderWindow->SetWindowName("4D Aorta Velocity Field Visualization");

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

    std::cout << "Starting aorta velocity field visualization..." << std::endl;
    std::cout << "Use mouse to rotate, scroll to zoom, and right-click to pan" << std::endl;
    std::cout << "Green arrows represent aorta velocity vectors" << std::endl;
    std::cout << "Velocity range: " << minVelocityThreshold << " to " << maxVelocityThreshold << " m/s" << std::endl;

    // Start rendering
    renderWindow->Render();
    renderWindowInteractor->Start();

    return 0;
}

