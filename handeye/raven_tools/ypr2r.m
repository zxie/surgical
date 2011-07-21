% matlab script to convert yaw, pitch, roll to rotation matrix (xyz convention)
% used to check raven log data

function R = ypr2r(yaw, pitch, roll)
    R_z = [cos(yaw) -sin(yaw) 0; sin(yaw) cos(yaw) 0; 0 0 1];
    R_y = [cos(pitch) 0 sin(pitch); 0 1 0; -sin(pitch) 0 cos(pitch)];
    R_x = [1 0 0; 0 cos(roll) -sin(roll); 0 sin(roll) cos(roll)];
    R = R_z * R_y * R_x
