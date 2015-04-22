clear all

opengl hardware;
set(gcf,'Renderer','OpenGL');

kinectInterface = class_interface();
kinectInterface.InitializeKinectDevice();
for i =1:100
    imshow(kinectInterface.GrabDepth()');
    drawnow();
end

clear kinectInterface;
