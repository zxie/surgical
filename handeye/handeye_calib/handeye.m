function [Hcam2marker,Hgrid2world] = handeye(Hmarker2world, Hcam2grid, HandEyeMethod)
%Hand-eye calibration
%
%This add on allows to compute the hand to eye calibration based ont he
%calibration toolbox from Jean-Yves Bouguet
%see http://www.vision.caltech.edu/bouguetj/calib_doc/
%more information can be found here:
%http://www.vision.ee.ethz.ch/~cwengert/calibration_toolbox.php
%
%You need to have computed the intrinsic and extrinsic parameters of your
%camera (the extrinsic parameters with respect to the grid's local
%coordinate system). Then you also need to supply the pose / transformation
%of the robot arm / marker with repsect to the robot base / external
%tracking device in the following format:
%Pack into a 4x4xNumber_of_Views Matrix the following data
%Hmarker2world(:,:,i) = [Ri_3x3 ti_3x1;[ 0 0 0 1]] 
%with 
%i = number of the view, 
%Ri_3x3 the rotation matrix 
%ti_3x1 the translation vector.
%
%The following parameters can be set (if not set the default values are
%used):
%doShow [default=0]: 
%Display the results from the calibration
%doSortHandEyeMovement [default=0]:
%Set this to 1 if you want your views sorted in a way that the interstation
%movement is ideal for hand-eye calibration, see [Tsai]
%HandEyeMethod	[default = 'Tsai']
%Here you specifiy which method you want to use for the
%hand-eye calibration, default is using Tsai's method. See
%ftp://ftp.vision.ee.ethz.ch/publications/proceedings/eth_biwi_00363.pdf
%Possible values are 'Tsai', 'Inria', 'Dual_quaternion', 'Navy'
%Tsai, Inria and Navy give the same results and are usually a bit better
%than the Dual quaternion approach
%
%Christian Wengert
%Computer Vision Laboratory
%ETH Zurich
%Sternwartstrasse 7
%CH-8092 Zurich
%www.vision.ee.ethz.ch/cwengert
%wengert@vision.ee.ethz.ch



%With the doShow flag you can show the results
if(~exist('doShow')) 
    doShow = 0;
end

if(~exist('HandEyeMethod')) 
    disp(['Method Inria will be used as default']);
    HandEyeMethod = 'Inria';
end

if(strcmp(HandEyeMethod,'Tsai')~=0)
    disp(['ATTENTION: Tsais method is buggy!']);
end



%Tsai states that the inter-station movement should be as large as possible
%for better accuracy. This flag will sort the movements for higher
%accuracy
if(~exist('doSortHandEyeMovement'))
    doSortHandEyeMovement = 0;
end


if(size(Hmarker2world,3)~=size(Hcam2grid,3))
    disp(['The size of the matrix should be the same']);
    return
end

correctSets = size(Hmarker2world,3);

for i=1:correctSets
    if(size(Hmarker2world(:,:,i),1)~=4 || size(Hmarker2world(:,:,i),2)~=4 || size(Hcam2grid(:,:,i),1)~=4 || size(Hcam2grid(:,:,i),2)~=4)
        disp(['The matrix should be 4x4']);
        return
    end
end

for i=1:correctSets
    Hm2w(:,:,i) = Hmarker2world(:,:,i);
    Hgrid2cam(:,:,i) = inv(Hcam2grid(:,:,i));
end


if(correctSets)        
    if(doSortHandEyeMovement)
        index = sortHandEyeMovement(Hm2w);
    else
        index = 1:size(Hm2w,3);          
    end
    Hm2w2 = Hm2w(:,:,index);
    Hcam2grid2 = Hcam2grid(:,:,index);
    %Now calibrate
    switch(HandEyeMethod)
        case 'Tsai'
            [Hcam2marker_, err] = TSAIleastSquareCalibration(Hm2w2(:,:,1:correctSets), Hcam2grid2(:,:,1:correctSets));
        case 'Inria'
            [Hcam2marker_, err] = inria_calibration(Hm2w(:,:,1:correctSets), Hcam2grid2(:,:,1:correctSets));
        case 'Navy'
            [Hcam2marker_, err] = navy_calibration(Hm2w(:,:,1:correctSets), Hcam2grid2(:,:,1:correctSets));
        case 'Dual_quaternion'
            [Hcam2marker_, err] = hand_eye_dual_quaternion(Hm2w(:,:,1:correctSets), Hcam2grid(:,:,1:correctSets));
        otherwise
            disp(['The method should be either Tsai, Inria, Navy or Dual_quaternion']);
            return;
            
    end
    
    err
    Hcam2marker_;
    
    %Create the average Hworld2grid, givin an idea where the grid is
    %in the coordinate system of the tracker/robot
    for i=1:correctSets
        Hcam2world_(:,:,i) = Hm2w(:,:,i)*Hcam2marker_;  %Hc2m(:,:,k)
        Hworld2cam_(:,:,i) = inv(Hcam2world_(:,:,i));
        %The above is correct as it gives the same as in my
        %simulation
        Hgrid2world_(:,:,i) = Hcam2world_(:,:,i)*Hcam2grid(:,:,i);
        Hworld2grid_(:,:,i) = inv(Hgrid2world_(:,:,i));
    end
    Hgrid2world_;
    %Average it, using the algorithm described in ...                
    Hgrid2worldAvg = averageTransformation(Hgrid2world_)   ; 
    %Errors on the translating part
    disp([sprintf('Backprojection error of the center of the frame : [%g %g %g]\n',std(Hgrid2world_(1,4,1:correctSets)),std(Hgrid2world_(2,4,1:correctSets)),std(Hgrid2world_(3,4,1:correctSets)))]);
    %disp(Hgrid2world_(1,4,1:correctSets))
    %disp(Hgrid2world_(2,4,1:correctSets))
    %disp(Hgrid2world_(3,4,1:correctSets))
    Hcam2marker = Hcam2marker_;
    Hgrid2world = Hgrid2worldAvg;
end
