function hist = hsv_global_hist(input_img)
%H = fspecial('average',[5,5]);
%blurimg = imfilter(rgbimg,H,'replicate');

[height, width,temp] = size(input_img);
rgbimg = zeros(height, width, 3);
if temp == 1
	
	rgbimg(:,:,1) = input_img;
	rgbimg(:,:,2) = rgbimg(:,:,1);
	rgbimg(:,:,3) = rgbimg(:,:,1);
	rgbimg = rgbimg/255;
	
else
	rgbimg = input_img;
end 
img = rgb2hsv(rgbimg);
[height, width,temp] = size(img);
h = img(:,:,1);
s = img(:,:,2);
v = img(:,:,3);
% quantize h,s,v color space to 0,1,2,3
qt_h = floor(h*255/16);
qt_s = floor(s*255/64);
qt_v = floor(v*255/64);
hist = zeros(1,16*4*4);
for c = 1:width
	for r = 1:height
		%calucate 256 bins histogram
		color_index = qt_h(r,c) * 16  + qt_s(r,c) * 4 + qt_v(r,c);
		hist(color_index+1) = hist(color_index+1)+1;
	end
end
hist = hist/double(height*width);