function showejes(R,s)
%R = RotationMatrixX(DegreeToRadian(a)) *RotationMatrixY(DegreeToRadian(b)) *RotationMatrixZ(DegreeToRadian(c));
t=1;
C(1,1)=0;
for i=-5:5
    for j=-5:5
        C(1:4,t) = [i;j;0;1;];
        t=t+1;
    end
end
C(1:4,t) = [0;0;1;1;];
%Ejes(:,:) = [0 1 0 0 0 0; 0 0 0 1 0 0; 0 0 0 0 0 1;1 1 1 1 1 1;];
%NE = R*Ejes;
NC = R*C;
plot3(NC(1,:),NC(2,:),NC(3,:),s)
hold on
xlabel('eje x');
ylabel('eje y');
zlabel('eje z');