load 'fileList.mat';
load 'filenames.mat';
Nimages = length(fileList);

bin = 8;
angle = 360;
L=3;

for i = 1:Nimages
	fprintf('%d/%d: %s\n', i, Nimages, fileList{i});
	feat = anna_phog(fileList{i}, bin, angle, L);
	save(['features/' filenames{i} '.phog'], 'feat');
end
