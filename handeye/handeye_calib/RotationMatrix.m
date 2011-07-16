function r = RotationMatrix(alpha, beta, gamma)
  r = euler2Rot(alpha, beta, gamma, 'rad', 'xyz');
end