#include <vtkColorTransferFunction.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkImageData.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

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

  vtkSmartPointer<vtkImageData> imageData =
      vtkSmartPointer<vtkImageData>::New();
  imageData->SetDimensions(mag.size_x(), mag.size_y(), mag.size_z());
  imageData->AllocateScalars(VTK_FLOAT, 1);

  for (std::size_t z = 0; z < mag.size_z(); ++z) {
    for (std::size_t y = 0; y < mag.size_y(); ++y) {
      for (std::size_t x = 0; x < mag.size_x(); ++x) {
        float *pixel =
            static_cast<float *>(imageData->GetScalarPointer(x, y, z));
        *pixel = mag.at(x, y, z, 0);
      }
    }
  }

  vtkSmartPointer<vtkGPUVolumeRayCastMapper> mapper =
      vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
  mapper->SetInputData(imageData);

  vtkSmartPointer<vtkColorTransferFunction> colorFunc =
      vtkSmartPointer<vtkColorTransferFunction>::New();
  colorFunc->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
  colorFunc->AddRGBPoint(255.0, 1.0, 1.0, 1.0);

  vtkSmartPointer<vtkPiecewiseFunction> opacityFunc =
      vtkSmartPointer<vtkPiecewiseFunction>::New();
  opacityFunc->AddPoint(0.0, 0.0);
  opacityFunc->AddPoint(255.0, 1.0);

  vtkSmartPointer<vtkVolumeProperty> volumeProperty =
      vtkSmartPointer<vtkVolumeProperty>::New();
  volumeProperty->SetColor(colorFunc);
  volumeProperty->SetScalarOpacity(opacityFunc);
  volumeProperty->ShadeOff();
  volumeProperty->SetInterpolationTypeToLinear();

  vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
  volume->SetMapper(mapper);
  volume->SetProperty(volumeProperty);

  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renderer->AddVolume(volume);
  renderer->SetBackground(0.1, 0.1, 0.1);

  vtkSmartPointer<vtkRenderWindow> renderWindow =
      vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(600, 600);

  vtkSmartPointer<vtkRenderWindowInteractor> interactor =
      vtkSmartPointer<vtkRenderWindowInteractor>::New();
  interactor->SetRenderWindow(renderWindow);

  renderWindow->Render();
  interactor->Start();

  return 0;
}
