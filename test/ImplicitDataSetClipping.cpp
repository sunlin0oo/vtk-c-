#include <vtkCamera.h>
#include <vtkNamedColors.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkClipPolyData.h>
#include <vtkDataSetWriter.h>
#include <vtkIdFilter.h>
#include <vtkIdTypeArray.h>
#include <vtkImplicitDataSet.h>
#include <vtkNew.h>
#include <vtkVersion.h>
#include <vtkProperty.h>
#include <vtkSphereSource.h>
#include <vtkCubeSource.h>
#include <vtkBox.h>
#include <vtkPlaneCollection.h>
#include <vtkPlane.h>
#include <vtkPlaneSource.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);

#if VTK_VERSION_NUMBER >= 89000000000ULL
#define VTK890 1
#endif


int main(int argc, char* argv[])
{
    vtkNew<vtkNamedColors> colors;

    // --------------- Read VTPFile ---------------
    std::string filename = argv[1];
    //std::cout << argv[1] << std::endl;
    // Read all the data from the file
    vtkNew<vtkXMLPolyDataReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();
    // --------------- END ---------------

    //--------------- 单元过滤器 ---------------
    // Add ids to the points and cells of the vtpFile
    vtkNew<vtkIdFilter> cellIdFilter;
    cellIdFilter->SetInputConnection(reader->GetOutputPort());
    cellIdFilter->SetCellIds(true);
    cellIdFilter->SetPointIds(false);
#if VTK890
    cellIdFilter->SetCellIdsArrayName("CellIds");
#else
    cellIdFilter->SetIdsArrayName("CellIds");
#endif
    cellIdFilter->Update();

    //--------------- END ---------------  得到cellIdFilter

    //--------------- 点过滤器 ---------------
    vtkNew<vtkIdFilter> pointIdFilter;
    pointIdFilter->SetInputConnection(cellIdFilter->GetOutputPort());
    pointIdFilter->SetCellIds(false);
    pointIdFilter->SetPointIds(true);
#if VTK890
    pointIdFilter->SetPointIdsArrayName("PointIds");
#else
    pointIdFilter->SetIdsArrayName("PointIds");
#endif
    pointIdFilter->Update();
    vtkDataSet* clipWithIds = pointIdFilter->GetOutput();
    //--------------- END ---------------    得到clipWithIds

    // --------------- implicit DataSet ---------------
    // PolyData to process
    vtkSmartPointer<vtkPolyData> polyData = reader->GetOutput();
    auto center = polyData->GetCenter();
    vtkNew<vtkPlane> plane1;
    plane1->SetOrigin(center[0], center[1], center[2]);
    plane1->SetNormal(-0.5, -0.5, 0.0);
    //vtkNew<vtkPlane> plane2;
    //plane2->SetOrigin(center[0], center[1], center[2]);
    //plane2->SetNormal(0.0, 0.0, 1.0);
    //vtkNew<vtkPlaneCollection> planes;
    //planes->AddItem(plane1);
    //planes->AddItem(plane2);
    // --------------- END ---------------

    //--------------- implicit DataSet display ---------------
    vtkNew<vtkPlaneSource> planeSource;
    planeSource->SetCenter(center[0], center[1], center[2]);
    planeSource->SetNormal(-0.5, -0.5, 0.0);
    planeSource->Update();

    vtkPolyData* plane = planeSource->GetOutput();
    //--------------- END ---------------

    // --------------- implicit DataSet display Mapper ---------------
    // Create a mapper and actor for cube
    vtkNew<vtkPolyDataMapper> implicitMapper;
    implicitMapper->SetInputData(plane);

    vtkNew<vtkActor> implicitActor;
    implicitActor->SetMapper(implicitMapper);
    implicitActor->GetProperty()->SetRepresentationToSurface();
    //implicitActor->GetProperty()->SetOpacity(0);
    implicitActor->GetProperty()->SetColor(colors->GetColor3d("red").GetData());
    // --------------- END ---------------

    // --------------- Clip ---------------
    vtkNew<vtkClipPolyData> clipper;
    clipper->SetClipFunction(plane1);// 切割的隐式数据集
    //clipper->SetClipFunction(plane2);// 切割的隐式数据集
    clipper->SetInputData(clipWithIds);
    clipper->InsideOutOn();
    clipper->Update();
    // --------------- END ---------------

    // --------------- Create a mapper and actor for clipped --------------
    vtkNew<vtkPolyDataMapper> clippedMapper;
    clippedMapper->SetInputConnection(clipper->GetOutputPort());
    clippedMapper->ScalarVisibilityOff();

    vtkNew<vtkActor> clippedActor;
    clippedActor->SetMapper(clippedMapper);
    clippedActor->GetProperty()->SetRepresentationToSurface();
    clippedActor->GetProperty()->SetColor(colors->GetColor3d("Yellow").GetData());
    //--------------- END ---------------

    // --------------- Render ---------------
    // Create a renderer, render window, and interactor
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkRenderWindow> renderWindow;
    renderWindow->AddRenderer(renderer);
    renderWindow->SetWindowName("ImplicitDataSetClipping");

    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
    renderWindowInteractor->SetRenderWindow(renderWindow);

    // Add the actor to the scene
    renderer->AddActor(clippedActor);
    renderer->AddActor(implicitActor);
    renderer->SetBackground(colors->GetColor3d("SlateGray").GetData());

    // Generate an interesting view
    //
    renderer->ResetCamera();
    renderer->GetActiveCamera()->Azimuth(-30);
    renderer->GetActiveCamera()->Elevation(30);
    renderer->GetActiveCamera()->Dolly(1.0);
    renderer->ResetCameraClippingRange();

    // Render and interact
    renderWindow->Render();
    renderWindowInteractor->Start();
    // --------------- END ---------------

    return EXIT_SUCCESS;
}

