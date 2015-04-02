%CLASS_INTERFACE Example MATLAB class wrapper to an underlying C++ class
classdef class_interface < handle
    properties (SetAccess = private, Hidden = true)
        objectHandle; % Handle to the underlying C++ class instance
    end
    methods
        %% Constructor - Create a new C++ class instance 
        function this = class_interface(varargin)
            this.objectHandle = ProjectMex('new', varargin{:});
        end
        
        %% Destructor - Destroy the C++ class instance
        function delete(this)
            ProjectMex('delete', this.objectHandle);
        end

        %% InitializeKinectDevice - an example class method call
        function varargout = InitializeKinectDevice(this, varargin)
            [varargout{1:nargout}] = ProjectMex('InitializeKinectDevice', this.objectHandle, varargin{:});
        end

        %% UpdateData - another example class method call
        function varargout = UpdateData(this, varargin)
            [varargout{1:nargout}] = ProjectMex('UpdateData', this.objectHandle, varargin{:});
        end
		%% GrabDepth - another example class method call
        function varargout = GrabDepth(this, varargin)
            [varargout{1:nargout}] = ProjectMex('GrabDepth', this.objectHandle, varargin{:});
        end
    end
end