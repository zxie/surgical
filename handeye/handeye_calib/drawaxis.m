function done=drawaxis(H,scale,i)

% draw an axis on position [0,0,0]
% transformed at coordinate frame H, with axis scale long

o=[0;0;0;1];
x=[scale;0;0;1];
y=[0;scale;0;1];
z=[0;0;scale;1];
p=H*o;
x=H*x;
y=H*y;
z=H*z;

line([p(1) x(1)],[p(2) x(2)],[p(3) x(3)],'color',[1 0 0]);
text(x(1), x(2), x(3), 'X');
line([p(1) y(1)],[p(2) y(2)],[p(3) y(3)],'color',[0 1 0]);
text(y(1), y(2), y(3), 'Y');
line([p(1) z(1)],[p(2) z(2)],[p(3) z(3)],'color',[0 0 1]);
text(z(1), z(2), z(3), 'Z');

%text(p(1)+scale/4, p(2), p(3)+scale/4, num2str(i));
text(p(1)+scale/12, p(2)+scale/12, p(3)+scale/12, num2str(i));
