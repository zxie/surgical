function [q] = R2q(H)

dx = 1 + H(1,1) - H(2,2) - H(3,3);
dy = 1 - H(1,1) + H(2,2) - H(3,3);
dz = 1 - H(1,1) - H(2,2) + H(3,3);
ds = 1 + H(1,1) + H(2,2) + H(3,3);

q=max([dx,dy,dz,ds]);

if q==dx
       qvx = sqrt(dx)/2;
       qvy = (H(2,1) + H(1,2)) / (4 * qvx);
       qvz = (H(3,1) + H(1,3)) / (4 * qvx);
       qs = (H(3,2) - H(2,3)) / (4 * qvx);
elseif q==dy
   qvy = sqrt(dy)/2;
       qvx = (H(2,1) + H(1,2)) / (4 * qvy);
       qvz = (H(3,2) + H(2,3)) / (4 * qvy);
       qs = (H(1,3) - H(3,1)) / (4 * qvy);
elseif q==dz
   qvz = sqrt(dz)/2;
       qvx = (H(3,1) + H(1,3)) / (4 * qvz);
       qvy = (H(3,2) + H(2,3)) / (4 * qvz);
       qs = (H(2,1) - H(1,2)) / (4 * qvz);
elseif q==ds
   qs= sqrt(ds)/2;
   qvx = (H(3,2) - H(2,3)) / (4 * qs);
   qvy = (H(1,3) - H(3,1)) / (4 * qs);
       qvz = (H(2,1) - H(1,2)) / (4 * qs);
end

qv=[qvx,qvy,qvz];

q = [qs;qv'];