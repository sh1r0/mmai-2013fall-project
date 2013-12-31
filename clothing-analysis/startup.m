function startup
%STARTUP set up path information on matlab startup
ROOT = fileparts(mfilename('fullpath'));
addpath(fullfile(ROOT,'lib'));
end
