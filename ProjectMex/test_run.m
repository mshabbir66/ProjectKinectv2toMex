clear all

opengl hardware;
set(gcf,'Renderer','OpenGL');

kinectInterface = class_interface();
kinectInterface.InitializeKinectDevice();
for i =1:100000
    disp(kinectInterface.GrabDepth());
        
end

clear kinectInterface;