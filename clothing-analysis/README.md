Clothing Analysis
=====================

A simple CBIR system based on the pose estimation part of [Kota Yamaguchi's Clothing Parsing](http://www.cs.sunysb.edu/~kyamagu/research/clothing_parsing/).  
And the dataset is from [MR PORTER](http://www.mrporter.com/)

##Setup
You need a compiled gabor excutable file from [mmai-2013fall-project/gabor](https://github.com/sh1r0/mmai-2013fall-project/gabor) and put it into this directory.  
For non-Windows users, run `make.m` to compile the mex files, otherwise, use the Makefile in `lib/+uci/private` instead.  
You can use `demo.m` to test whether the setup is alright.  

##Usage
	[upper,lower] = clothing_analyze(img_path);

##Extracted Features
- global color moments in YCbCr color space
- log Gabor
