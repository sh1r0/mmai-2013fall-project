
% In directory phog, for lower part
load lower_dataset.mat
path = '../lower_part/*_patch.jpg';
imgs = dir(path);
Nimages = size(imgs,1);

%parameters of phog
param.imageSize = [256 256]; % it works also with non-square images
param.orientationsPerScale = [8 8 8 8];
param.numberBlocks = 4;
param.fc_prefilt = 4;
for i = 1 : Nimages,
    sprintf('%d/%d: %s\n', i, Nimages, imgs(i).name)
    img = imread(sprintf('../lower_part/%s', imgs(i).name));
    [gist1, param] = LMgist(img, '', param);
	lower_dataset{i}.gist = gist1;
end
save('lower_dataset.mat','lower_dataset');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
clear path; clear imgs; clear Nimages;
% In directory phog, for lower part
load upper_dataset.mat
path = '../upper_part/*_patch.jpg';
imgs = dir(path);
Nimages = size(imgs,1);

%parameters of phog
clear param;
param.imageSize = [256 256]; % it works also with non-square images
param.orientationsPerScale = [8 8 8 8];
param.numberBlocks = 4;
param.fc_prefilt = 4;
for i = 1 : Nimages,
    sprintf('%d/%d: %s\n', i, Nimages, imgs(i).name)
    img = imread(sprintf('../upper_part/%s', imgs(i).name));
    [gist2, param] = LMgist(img, '', param);
	upper_dataset{i}.gist = gist2;
end
save('upper_dataset.mat','upper_dataset');



