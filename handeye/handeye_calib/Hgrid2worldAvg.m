for i=1:50 
ypr(:,i) = R2ypr(F(1:3,1:3,i));
end
Y = mean(ypr(1,:))
P = mean(ypr(2,:))
R = mean(ypr(3,:))
ROT = ypr2R(Y,P,R)

Hgrid2worldAvg = [ROT [mean(F(1,4,:)); mean(F(2,4,:)); mean(F(3,4,:))]; 0 0 0 1]


% Hgrid2worldAvg =
%    0.9998    0.0049   -0.0178   84.5512
%    0.0054   -0.9995    0.0306 -455.3319
%   -0.0176   -0.0307   -0.9994  755.2799
%         0         0         0    1.0000