function feat=ycbcr_hist(img_path)

im_rgb = imread(img_path);
NPixels = size(im_rgb, 1) * size(im_rgb, 2);
im_ycbcr = rgb2ycbcr(im_rgb);
N=256;
feat = zeros(9,1);

% calculate statistics for each channel
for i = 1:3
	counts = imhist(im_ycbcr(:,:,i)); % histogram
	PrH = counts/NPixels;
	t.m = sum([1:N]'.*PrH); % mean
	t.s = sqrt(sum(([1:N]'-t.m).^2.*PrH)); % standard deviation
	mode = find(PrH == max(PrH));
	t.sk2 = (t.m-mode(ceil(length(mode)/2)))/t.s; % skewness
	feat(i*3-2:i*3) = [t.m t.s t.sk2];
end
