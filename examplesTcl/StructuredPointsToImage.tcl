# Convert a structured points data set into an image.
# Display the image in an XViewer.


source ../imaging/examplesTcl/vtkImageInclude.tcl


vtkVolume16Reader v16
    v16 SetDataDimensions 256 256
    v16 SetFilePrefix "../../data/fullHead/headsq";
    v16 SetDataOrigin -127.5 -127.5 5.33
    v16 SetImageRange 1 4
    v16 SetDataSpacing 1 1 5.33
    v16 SwapBytesOn
    v16 Update

vtkStructuredPointsToImage sti
sti SetInput [v16 GetOutput]
sti Update

puts [[sti GetOutput] Print]
exit

#vtkImageAnisotropicDiffusion2d diffusion;
#diffusion SetInput [sti GetOutput];
#diffusion SetDiffusionFactor 0.15;
#diffusion SetDiffusionThreshold 20.0;
#diffusion SetNumberOfIterations 10;
#diffusion ReleaseDataFlagOff;


vtkImageXViewer viewer;
#viewer DebugOn;
viewer SetInput [sti GetOutput];
viewer SetZSlice 22;
viewer SetColorWindow 3000
viewer SetColorLevel 1500


# make interface
source ../imaging/examplesTcl/WindowLevelInterface.tcl














