function [upper, lower] = clothing_analyze(img_path)

	extract_roi(img_path);

	upper_im_path = [img_path(1:end-4) '_upper.jpg'];
	lower_im_path = [img_path(1:end-4) '_lower.jpg'];
	gabor_cmd = 'gabor -i ';
	if ~ispc()
		gabor_cmd = ['./' gabor_cmd];
	end

	upper.color_moment = ycbcr_hist(upper_im_path);
	system([gabor_cmd upper_im_path]);
	gabor = dlmread([upper_im_path(1:end-4) '.gb'], ' ');
	upper.gabor = gabor(1:48)';

	lower.color_moment = ycbcr_hist(lower_im_path);
	system([gabor_cmd lower_im_path]);
	gabor = dlmread([lower_im_path(1:end-4) '.gb'], ' ');
	lower.gabor = gabor(1:48)';

	load upper_dataset;
	Nimages = length(upper_dataset);
	upper.difference = zeros(Nimages, 2);

	for i = 1:Nimages
		gabor = upper_dataset{i}.gabor;
		color_moment = upper_dataset{i}.color_moment;
		upper.difference(i, 1) = sum(abs(upper.color_moment-color_moment));
		upper.difference(i, 2) = sum(abs(upper.gabor-gabor));
	end

	load lower_dataset;
	Nimages = length(lower_dataset);
	lower.difference = zeros(Nimages, 2);

	for i = 1:Nimages
		gabor = lower_dataset{i}.gabor;
		color_moment = lower_dataset{i}.color_moment;
		lower.difference(i, 1) = sum(abs(lower.color_moment-color_moment));
		lower.difference(i, 2) = sum(abs(lower.gabor-gabor));
	end

	[upper.sortedValues,upper.sortIndex] = sort(upper.difference, 1, 'ascend');
	[lower.sortedValues,lower.sortIndex] = sort(lower.difference, 1, 'ascend');

end
