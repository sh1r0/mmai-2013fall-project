function [upper, lower] = clothing_analyze(img_path)

	extract_roi(img_path);

	upper_im_path = [img_path(1:end-4) '_upper.jpg'];
	lower_im_path = [img_path(1:end-4) '_lower.jpg'];
	sleeve_im_path = [img_path(1:end-4) '_sleeve.jpg'];

	gabor_cmd = 'gabor -i ';
	if ~ispc()
		gabor_cmd = ['./' gabor_cmd];
	end

	% ====== UPPER ====== %
	% hsv hist
	upper.hsv = hsv_hist(upper_im_path);
	% gabor
	system([gabor_cmd upper_im_path]);
	gabor = dlmread([upper_im_path(1:end-4) '.gb'], ' ');
	upper.gabor = gabor(1:48)';
	% HOG
	addpath '../HOG';
	upper.hog = HOG(upper_im_path);
	% sleeve type
	upper.sleeve_type = sleeve_length_detector(sleeve_im_path);

	load ../data/upper_dataset.mat;
	Nimages = length(upper_dataset);
	upper.difference = zeros(Nimages, 3);

	for i = 1:Nimages
		upper.difference(i, 1) = sum(abs(upper.hsv-upper_dataset{i}.hsv));
		upper.difference(i, 2) = sum(abs(upper.gabor-upper_dataset{i}.gabor));
		upper.difference(i, 3) = sum(abs(upper.hog-upper_dataset{i}.hog));
	end

	[upper.sortedValues,upper.sortIndex] = sort(upper.difference, 1, 'ascend');

	% ====== LOWER ====== %
	% hsv hist
	lower.hsv = hsv_hist(lower_im_path);
	% gabor
	system([gabor_cmd lower_im_path]);
	gabor = dlmread([lower_im_path(1:end-4) '.gb'], ' ');
	lower.gabor = gabor(1:48)';
	% HOG
	addpath '../HOG';
	lower.hog = HOG(lower_im_path);

	load ../data/lower_dataset.mat;
	Nimages = length(lower_dataset);
	lower.difference = zeros(Nimages, 3);

	for i = 1:Nimages
		lower.difference(i, 1) = sum(abs(lower.hsv-lower_dataset{i}.hsv));
		lower.difference(i, 2) = sum(abs(lower.gabor-lower_dataset{i}.gabor));
		lower.difference(i, 3) = sum(abs(lower.hog-lower_dataset{i}.hog));
	end

	[lower.sortedValues,lower.sortIndex] = sort(lower.difference, 1, 'ascend');

	% remove existed htmls
	if exist('result.html') == 2
		delete('*.html');
	end

	% ====== RETRIVAL RESULTS ====== %
	fid = fopen('result.html', 'a');
	fprintf(fid, '<html>\n<title>Results</title>\n<body>\n<table border="1" align=center width="70%%">\n<tr align=center>\n<td colspan="4"><a href="%s"><img src="%s" id="query"></a></td>\n</tr>\n', [img_path(1:end-4) '_pose.jpg'], [img_path(1:end-4) '_pose.jpg']);
	fprintf(fid, '<tr align=center>\n<td colspan="2">Sleeve Type</td>\n<td colspan="2">%s</td>\n</tr>\n', upper.sleeve_type);

	categories = {'HSV', 'Gabor', 'HOG'};

	upper_products = {};
	for i = 1:3
		for j = 1:3
			upper_products = [upper_products upper_dataset{upper.sortIndex(j,i)}.product_code];
		end

		fprintf(fid, '<tr align=center>\n<td>%s</td>\n', categories{i});
		for j = 1:3
			fprintf(fid, '<td><a href="%s"><img class="autoResizeImage" src="%s"></a></td>\n', ['rcmd_' upper_products{(i-1)*3+j} '.html'] ,['../../clothing_dataset_v3/images/' upper_products{(i-1)*3+j} '_p.jpg']);
		end
		fprintf(fid, '</tr>\n');
	end

	lower_products = {};
	for i = 1:3
		for j = 1:3
			lower_products = [lower_products lower_dataset{lower.sortIndex(j,i)}.product_code];
		end

		fprintf(fid, '<tr align=center>\n<td>%s</td>\n', categories{i});
		for j = 1:3
			fprintf(fid, '<td><a href="%s"><img class="autoResizeImage" src="%s"></a></td>\n', ['rcmd_' lower_products{(i-1)*3+j} '.html'], ['../../clothing_dataset_v3/images/' lower_products{(i-1)*3+j} '_p.jpg']);
		end
		fprintf(fid, '</tr>\n');
	end


	fprintf(fid, '</table>\n</body>\n<style type="text/css">\n.autoResizeImage {\nmax-width: 100%%;\nheight: auto;\nwidth: 100%%;\n}\n#query {max-height: 500px;}\n</style>\n</html>\n');
	fclose(fid);

	% ====== RECOMMENDATION PAGES ====== %
	for i = 1:3
		for j = 1:3
			rcmd_items = upper_dataset{upper.sortIndex(j,i)}.pair_item;

			if exist(['rcmd_' upper_products{(i-1)*3+j} '.html']) == 2
				continue;
			end

			fid = fopen(['rcmd_' upper_products{(i-1)*3+j} '.html'], 'a');
			fprintf(fid, '<html>\n<title>Results</title>\n<body>\n<table border="1" align=center width="70%%">\n');

			for k = 1:length(rcmd_items)
				fprintf(fid, '<tr align=center>\n<td><img src="%s"></td>\n</tr>\n', ['../../clothing_dataset_v3/images/' rcmd_items{k} '_ou.jpg']);
			end

			fprintf(fid, '</table>\n</body>\n</html>\n');
			fclose(fid);
		end
	end

	for i = 1:3
		for j = 1:3
			rcmd_items = lower_dataset{lower.sortIndex(j,i)}.pair_item;

			if exist(['rcmd_' lower_products{(i-1)*3+j} '.html']) == 2
				continue;
			end

			fid = fopen(['rcmd_' lower_products{(i-1)*3+j} '.html'], 'a');
			fprintf(fid, '<html>\n<title>Results</title>\n<body>\n<table border="1" align=center width="70%%">\n');

			for k = 1:length(rcmd_items)
				fprintf(fid, '<tr align=center>\n<td><img src="%s"></td>\n</tr>\n', ['../../clothing_dataset_v3/images/' rcmd_items{k} '_ou.jpg']);
			end

			fprintf(fid, '</table>\n</body>\n</html>\n');
			fclose(fid);
		end
	end
end
