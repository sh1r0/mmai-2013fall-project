function hist = hsv_global_hist(img_path)

im_rgb = imread(img_path);
im_hsv = rgb2hsv(im_rgb);

[height, width, depth] = size(im_hsv);
h = im_hsv(:,:,1);
s = im_hsv(:,:,2);
v = im_hsv(:,:,3);

qt_h = floor(h*255/16); % 4 bits
qt_s = floor(s*255/64); % 2 bits
qt_v = floor(v*255/64); % 2 bits

I = uint8(qt_h * 16 + qt_s * 4 + qt_v);
counts = imhist(I);
hist = counts/(height*width);
