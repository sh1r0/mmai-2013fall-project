function output = filter(image)
  % the input is the images dataset with 2 dimensions 
  % (pixel,pixel)
  % to make iamge have the same overall contrast, normalize it to be same
  % variance

  if size(image,3) > 1,
      image = rgb2gray(image);
  end
  
  %image_size
  N= size(image,1);
  M= size(image,2);
  
  % set the parameter of whiten filter
  [fx fy]=meshgrid(-M/2:M/2-1,-N/2:N/2-1);
  rho=sqrt(fx.*fx+fy.*fy);
  f_0=0.4*N;
  filt=rho.*exp(-(rho/f_0).^4);
  
  % convert to frequency domain
  If=fft2(image);
  imagew=real(ifft2(If.*fftshift(filt)));
  IMAGES=reshape(imagew,N*M,1);
  
  % set to normalize with variance of pixels to be 0.1
  IMAGES=sqrt(0.1)*IMAGES/sqrt(mean(var(IMAGES)));
  IMAGES_result = reshape(IMAGES, N, M);
  
  output = IMAGES_result;
end